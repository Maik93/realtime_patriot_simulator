/**
 * Trackers: they're attached by the radar on every object seen in the sky.
 * Following this object, they try to evaluate velocities and accelerations.
 */
#include "trackers.h"

// #include <stddef.h>
#include <stdio.h>
// #include <time.h>
#include <math.h>
// #include <pthread.h>
#include <allegro.h>

#include "baseUtils.h"
// #include "common.h"

// float pred_x, pred_y;

// public variables
struct point	current_points_tracked[MAX_TRACKERS];	// array of all currently tracked points (abs. coord)
int				tracker_is_active[MAX_TRACKERS];		// flag activity for trackers

// private variables
int tracker_view[MAX_TRACKERS][TRACKER_RES][TRACKER_RES]; // image buffer for trackers
struct tracker
	tracked_points[MAX_TRACKERS]; // memory for trackers, containing points acquired and vel and acc evaluated

// Store pixels contained in the square centered in (x0, y0) and width TRACKER_RES.
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
			// radar and rocket_launcher sprites are excluded (BLU), as enemy missile trails (GREY)
			if (tracker_view[tracker_i][i][j] != BKG &&
			        tracker_view[tracker_i][i][j] != BLU &&
			        tracker_view[tracker_i][i][j] != GREY) {
				c_rel.x += i - TRACKER_RES / 2;
				c_rel.y += j - TRACKER_RES / 2;
				n++;
			}
		}

	// check if some point is seen, otherwise deactivate the tracker
	if (n != 0) {
		c_rel.x /= n;
		c_rel.y /= n;

		// put a green dot on the evaluated centroid
		tracker_view[tracker_i][c_rel.x + TRACKER_RES / 2][c_rel.y + TRACKER_RES / 2] = GREEN;
	}
	else tracker_is_active[tracker_i] = 0;

	return c_rel;
}

// Store last TSTORE points tracked.
void store_point(int i) {
	int k;

	if (i >= MAX_TRACKERS) return;

	k = tracked_points[i].top;

	// don't store same coord.
	if (current_points_tracked[i].x == tracked_points[i].x[k] ||
	        current_points_tracked[i].y == tracked_points[i].y[k])
		return;

	k = (k + 1) % TSTORE;
	clock_gettime(CLOCK_MONOTONIC, &tracked_points[i].t[k]);
	tracked_points[i].x[k] = abs2world_x(current_points_tracked[i].x);
	tracked_points[i].y[k] = abs2world_y(current_points_tracked[i].y);
	tracked_points[i].top = k;

	tracked_points[i].n_samples++;
}

void draw_predictions(int tracker_i, float delta_t) {
	float x, y, vy;
	int k, prev_k;

	// if (tracked_points[tracker_i].vx == 0) return;

	k = tracked_points[tracker_i].top;
	prev_k = (k - 1 + TSTORE) % TSTORE;

	x = tracked_points[tracker_i].x[k];
	y = tracked_points[tracker_i].y[k];
	vy = tracked_points[tracker_i].vy;

	int c = 0;
	while (y > 0 && c < 500) {
		c++;
		x += tracked_points[tracker_i].vx * delta_t;
		y += (0.5 * tracked_points[tracker_i].ay * delta_t + vy) * delta_t;
		vy += tracked_points[tracker_i].ay * delta_t;

		putpixel(screen_buff, world2abs_x(x), world2abs_y(y), BLU);
	}
}

// Cause of ax = 0, vx is constant. Evaluating the best aproximating line for x positions,
// I can exctract vx as the coefficent of the first-order term. Mean squar error minimization is used.
float evaluate_vx(int tracker_i) {
	int i, j, k, first_k; // indexes
	float a0, a1; // coefficients of the fitting line
	float x[TSTORE], sumx = 0, sumxy = 0, sumx2 = 0;
	int y[TSTORE], sumy = 0;

	first_k = (tracked_points[tracker_i].top + 1) % TSTORE;
	k = first_k; // start from older one

	// load points to fit: y is hte horizontal position of missile at time x
	for (j = 0; j < TSTORE; j++) {
		x[j] = TSCALE *
		       time_diff_ms(tracked_points[tracker_i].t[k], tracked_points[tracker_i].t[first_k]) / 1000.0;
		y[j] = tracked_points[tracker_i].x[k];

		k = (k + 1) % TSTORE;
	}

	for (i = 0; i < TSTORE; i++) {
		sumx += x[i];
		sumx2 += x[i] * x[i];
		sumy += y[i];
		sumxy += x[i] * y[i];
	}

	// a0 = ((sumx2 * sumy - sumx * sumxy) * 1.0 / (TSTORE * sumx2 - sumx * sumx) * 1.0);
	a1 = ((TSTORE * sumxy - sumx * sumy) * 1.0 / (TSTORE * sumx2 - sumx * sumx) * 1.0);

	// printf("From fitting: Vx = %f\n", a1);
	return a1;
}

void evaluate_v_and_a(int task_i, int tracker_i) {
	int k, prev_k, prev_prev_k;	// previous index point and its previous
	float vx_prev, vy_prev;		// previous velocity, needed to evaluate acc
	float dt1, dt2;				// delta t from k to prev_k and from prev_k to prev_prev_k

	// if (tracked_points[tracker_i].n_samples < TSTORE) return; // not enough points
	if (tracked_points[tracker_i].n_samples < 2) return; // not enough points

	// if (tracked_points[tracker_i].x[k] == tracked_points[tracker_i].x[prev_k]) return;

	k = tracked_points[tracker_i].top;
	prev_k = (k - 1 + TSTORE) % TSTORE;
	prev_prev_k = (prev_k - 1 + TSTORE) % TSTORE;

	// float dt = TSCALE * (float)get_task_period(task_i) / 1000;
	float dt = TSCALE * time_diff_ms(tracked_points[tracker_i].t[k], tracked_points[tracker_i].t[prev_k]) / 1000.0;

	tracked_points[tracker_i].ax = 0;
	// tracked_points[tracker_i].vx = (tracked_points[tracker_i].x[k] - tracked_points[tracker_i].x[prev_k]) / dt;
	tracked_points[tracker_i].vx = evaluate_vx(tracker_i);
	// pred_x = tracked_points[tracker_i].vx * dt + tracked_points[tracker_i].x[k];

	tracked_points[tracker_i].ay = -G0;
	tracked_points[tracker_i].vy = (tracked_points[tracker_i].y[k] - tracked_points[tracker_i].y[prev_k]) / dt;
	/*float prev_vy;
	if (tracked_points[tracker_i].vy == 0)
		prev_vy = (tracked_points[tracker_i].y[k] - tracked_points[tracker_i].y[prev_k]) / dt;
	else
		prev_vy = tracked_points[tracker_i].vy;
	tracked_points[tracker_i].vy = tracked_points[tracker_i].ay * dt + prev_vy;*/

	/*float pred_vy = tracked_points[tracker_i].ay * dt + tracked_points[tracker_i].vy;
	pred_y = pred_vy * dt + tracked_points[tracker_i].y[k];*/
	// pred_y = (1 / 2 * tracked_points[tracker_i].ay * dt + tracked_points[tracker_i].vy) * dt + tracked_points[tracker_i].y[k];

	// DBG
	// if (tracker_i == 0) {
	printf("Predicted v = (%f; %f)\n", tracked_points[tracker_i].vx, tracked_points[tracker_i].vy);
	/*if (tracked_points[tracker_i].vx == 0)
		printf("it seems that vx is zero!\n");*/
	// }

	// Eulerian method
	/*tracked_points[tracker_i].vx = (tracked_points[tracker_i].x[k] - tracked_points[tracker_i].x[prev_k]) / dt1;
	tracked_points[tracker_i].vy = (tracked_points[tracker_i].y[k] - tracked_points[tracker_i].y[prev_k]) / dt1;
	vx_prev = (tracked_points[tracker_i].x[prev_k] - tracked_points[tracker_i].x[prev_prev_k]) / dt2;
	vy_prev = (tracked_points[tracker_i].y[prev_k] - tracked_points[tracker_i].y[prev_prev_k]) / dt2;
	tracked_points[tracker_i].ax = (tracked_points[tracker_i].vx - vx_prev) / dt1;
	tracked_points[tracker_i].ay = (tracked_points[tracker_i].vy - vy_prev) / dt1;*/

	// Verlet method: compute v at time prev_k
	/*tracked_points[tracker_i].vx =
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
	    * 2 / (dt1 + dt2);*/

	/*tracked_points[tracker_i].vx = linear_curve_fitting(tracker_i);
	tracked_points[tracker_i].vy = -1;
	tracked_points[tracker_i].ax = 0;
	tracked_points[tracker_i].ay = -G0;*/

	/*printf("vx: %f\tvy: %f\tax: %f\tay: %f\n",
	//       tracked_points[tracker_i].x[k], tracked_points[tracker_i].x[prev_k],
	       tracked_points[tracker_i].vx, tracked_points[tracker_i].vy,
	       tracked_points[tracker_i].ax, tracked_points[tracker_i].ay);*/
}

void *tracker_task(void* arg) {
	int task_i, tracker_i; // indexes

	task_i = get_task_index(arg);
	tracker_i = task_i - TRACKER_BASE_INDEX;

	tracked_points[tracker_i].n_samples = 0;
	tracker_is_active[tracker_i] = 1;

	// printf("Start tracking (%d, %d)\n", current_points_tracked[tracker_i].x, current_points_tracked[tracker_i].y);

	set_period(task_i);
	while (!sigterm_tasks && tracker_is_active[tracker_i]) {
		get_image(tracker_i, current_points_tracked[tracker_i].x, current_points_tracked[tracker_i].y);

		// DBG: maybe there are different pixels
		struct point c = compute_centroid(tracker_i); // relative from center of tracking box
		current_points_tracked[tracker_i].x += c.x; // altering the center to follow
		current_points_tracked[tracker_i].y += c.y;
		store_point(tracker_i);
		evaluate_v_and_a(task_i, tracker_i);

		wait_for_period(task_i);
	}
	printf("Tracker detached. Missed %d deadlines on %d runs.\n", tp[task_i].dmiss, tp[task_i].counts);
	tp[task_i].index = -1;
}

// Shows what each tracker sees in the right side of the screen.
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