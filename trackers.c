#include "trackers.h"

// #include <stddef.h>
#include <stdio.h>
// #include <time.h>
#include <math.h>
// #include <pthread.h>
#include <allegro.h>

#include "baseUtils.h"
#include "common.h"

//-----------------------------------------------------
// PUBLIC VARIABLES
//-----------------------------------------------------
struct point	current_points_tracked[MAX_TRACKERS];		// array of all currently tracked points
int				tracker_is_active[MAX_TRACKERS];			// flag activity for trackers
//-----------------------------------------------------
// PRIVATE VARIABLES
//-----------------------------------------------------
int				tracker_view[MAX_TRACKERS][TRACKER_RES][TRACKER_RES];// image buffer for trackers
struct tracker	tracked_points[MAX_TRACKERS];				// history of last 3 points tracked for each tracker

//-----------------------------------------------------
// PRIVATE FUNCTIONS
//-----------------------------------------------------

// Store pixels contained in a square centered in (x0, y0) and width TRACKER_RES.
void get_image(int tracker_i, int x0, int y0) {
	int i, j; // image matrix indexes
	int x, y; // graphical coordinates
	for (i = 0; i < TRACKER_RES; i++)
		for (j = 0; j < TRACKER_RES; j++) {
			x = x0 - TRACKER_RES / 2 + i;
			y = y0 - TRACKER_RES / 2 + j;

			// take only points in world
			if (x > WORLD_BOX_X1 && x < WORLD_BOX_X2
			        && y > WORLD_BOX_Y1 && y < WORLD_BOX_Y2)
				tracker_view[tracker_i][i][j] = getpixel(screen, x, y);
			else tracker_view[tracker_i][i][j] = BKG;
		}
}

// Evaluate centroid of element seen by tracker_i, relative to the center of image.
struct point compute_centroid(int tracker_i) {
	struct point c_rel;	// coord. of centroid
	int n = 0;			// number of points found for x and y
	int i, j;

	c_rel.x = 0;
	c_rel.y = 0;
	for (i = 0; i < TRACKER_RES; i++)
		for (j = 0; j < TRACKER_RES; j++) {
			// radar sprite doesn't count
			if (tracker_view[tracker_i][i][j] != BKG &&
			        tracker_view[tracker_i][i][j] != BLU &&
			        tracker_view[tracker_i][i][j] != GREY) {
				c_rel.x += i - TRACKER_RES / 2;
				c_rel.y += j - TRACKER_RES / 2;
				n++;
			}
		}

	if (n != 0) {
		c_rel.x /= n;
		c_rel.y /= n;
	}
	else tracker_is_active[tracker_i] = 0;

	return c_rel;
}

// Store last 3 points tracked to compute vel and acc.
void store_point(int i) {
	int k;

	if (i >= MAX_TRACKERS) return;

	k = tracked_points[i].top;
	k = (k + 1) % 3;

	clock_gettime(CLOCK_MONOTONIC, &tracked_points[i].t[k]);
	tracked_points[i].x[k] = current_points_tracked[i].x;
	tracked_points[i].y[k] = current_points_tracked[i].y;
	tracked_points[i].top = k;

	tracked_points[i].n_samples++;
}

void evaluate_v_and_a(int tracker_i) {
	int k, prev_k, prev_prev_k;	// previous index point and its previous
	float vx_prev, vy_prev;	// previous velocity, needed to evaluate acc
	float dt1, dt2;			// delta t from k to prev_k and from prev_k to prev_prev_k

	if (tracked_points[tracker_i].n_samples < 3) return; // not enough points

	k = tracked_points[tracker_i].top;
	prev_k = (k - 1 + 3) % 3;
	prev_prev_k = (prev_k - 1 + 3) % 3;

	dt1 = TSCALE * time_diff_ms(tracked_points[tracker_i].t[k], tracked_points[tracker_i].t[prev_k]) / 1000.0;
	dt2 = TSCALE * time_diff_ms(tracked_points[tracker_i].t[prev_k], tracked_points[tracker_i].t[prev_prev_k]) / 1000.0;

	// Eulerian method
	/*tracked_points[tracker_i].vx = (tracked_points[tracker_i].x[k] - tracked_points[tracker_i].x[prev_k]) / dt1;
	tracked_points[tracker_i].vy = (tracked_points[tracker_i].y[k] - tracked_points[tracker_i].y[prev_k]) / dt1;
	vx_prev = (tracked_points[tracker_i].x[prev_k] - tracked_points[tracker_i].x[prev_prev_k]) / dt2;
	vy_prev = (tracked_points[tracker_i].y[prev_k] - tracked_points[tracker_i].y[prev_prev_k]) / dt2;
	tracked_points[tracker_i].ax = (tracked_points[tracker_i].vx - vx_prev) / dt1;
	tracked_points[tracker_i].ay = (tracked_points[tracker_i].vy - vy_prev) / dt1;*/

	// Verlet method: compute v at time prev_k
	tracked_points[tracker_i].vx =
	    (tracked_points[tracker_i].x[k] - tracked_points[tracker_i].x[prev_prev_k]) / (dt1 + dt2);
	tracked_points[tracker_i].vy =
	    (tracked_points[tracker_i].y[k] - tracked_points[tracker_i].y[prev_prev_k]) / (dt1 + dt2);

	tracked_points[tracker_i].ax =
	    ((tracked_points[tracker_i].x[k] - tracked_points[tracker_i].x[prev_k]) / dt1 +
	     (tracked_points[tracker_i].x[prev_prev_k] - tracked_points[tracker_i].x[prev_k]) / dt2)
	    * 2 / (dt1 + dt2);
	tracked_points[tracker_i].ay =
	    ((tracked_points[tracker_i].y[k] - tracked_points[tracker_i].y[prev_k]) / dt1 +
	     (tracked_points[tracker_i].y[prev_prev_k] - tracked_points[tracker_i].y[prev_k]) / dt2)
	    * 2 / (dt1 + dt2);

	printf("vx: %f\tvy: %f\tax: %f\tay: %f\n",
	       tracked_points[tracker_i].x[k], tracked_points[tracker_i].x[prev_k],
	       tracked_points[tracker_i].vx, tracked_points[tracker_i].vy,
	       tracked_points[tracker_i].ax, tracked_points[tracker_i].ay);
}

//-----------------------------------------------------
// PUBLIC FUNCTIONS
//-----------------------------------------------------

void *tracker_task(void* arg) {
	int task_i, tracker_i;

	task_i = get_task_index(arg);
	tracker_i = task_i - TRACKER_BASE_INDEX;

	tracked_points[tracker_i].n_samples = 0;

	// printf("Start tracking (%d, %d)\n", current_points_tracked[tracker_i].x, current_points_tracked[tracker_i].y);
	tracker_is_active[tracker_i] = 1;

	set_period(task_i);
	while (!sigterm_tasks && tracker_is_active[tracker_i]) {
		get_image(tracker_i, current_points_tracked[tracker_i].x, current_points_tracked[tracker_i].y);

		// now compute centroid, then update current_points_tracked (the center to follow)
		struct point c = compute_centroid(tracker_i);
		current_points_tracked[tracker_i].x += c.x;
		current_points_tracked[tracker_i].y += c.y;
		store_point(tracker_i);
		evaluate_v_and_a(tracker_i);

		wait_for_period(task_i);
	}
	printf("Tracker detached. Missed %d deadlines on %d runs.\n", tp[task_i].dmiss, tp[task_i].counts);
	tp[task_i].index = -1;
}

// Shows what a tracker sees on right side of the screen.
void tracker_display(int tracker_i) {
	int i, j;	// image matrix indexes
	int x0, y0;	// center to draw display
	int x, y;	// graphical coordinates

	// find out where to draw this display
	switch (tracker_i) {
	case 0:
		x0 = TRACK_D0_X;
		y0 = TRACK_D0_Y;
		break;
	case 1:
		x0 = TRACK_D1_X;
		y0 = TRACK_D1_Y;
		break;
	case 2:
		x0 = TRACK_D2_X;
		y0 = TRACK_D2_Y;
		break;
	case 3:
		x0 = TRACK_D3_X;
		y0 = TRACK_D3_Y;
		break;
	default: return;
	}

	// background
	rectfill(screen_buff, x0 - TRACKER_RES * TRACK_DSCALE / 2, y0 - TRACKER_RES * TRACK_DSCALE / 2,
	         x0 + TRACKER_RES * TRACK_DSCALE / 2, y0 + TRACKER_RES * TRACK_DSCALE / 2, BKG);

	// draws what's seen by tracker, magnified by factor TRACK_DSCALE
	for (i = 0; i < TRACKER_RES; i++)
		for (j = 0; j < TRACKER_RES; j++) {
			x = (int)x0 - TRACKER_RES * TRACK_DSCALE / 2 + i * TRACK_DSCALE;
			y = (int)y0 - TRACKER_RES * TRACK_DSCALE / 2 + j * TRACK_DSCALE;
			putpixel(screen_buff, x, y, tracker_view[tracker_i][i][j]);
		}

	// border
	rect(screen_buff, x0 - TRACKER_RES * TRACK_DSCALE / 2, y0 - TRACKER_RES * TRACK_DSCALE / 2,
	     x0 + TRACKER_RES * TRACK_DSCALE / 2, y0 + TRACKER_RES * TRACK_DSCALE / 2, LBLU);
}