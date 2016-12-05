#include "radar_and_trackers.h"

// #include <stddef.h>
#include <stdio.h>
// #include <time.h>
#include <math.h>
// #include <pthread.h>
#include <allegro.h>

#include "baseUtils.h"
#include "common.h"

//-----------------------------------------------------
// RADAR VARIABLES
//-----------------------------------------------------
struct scan	radar[ARES];	// struct  of points detected
int			current_i;		// actual index of radar array; for graphic porpuses
//-----------------------------------------------------
// TRACKER VARIABLES
//-----------------------------------------------------
int tracker_view[MAX_TRACKERS][TRACKER_RES][TRACKER_RES]; // image buffer for trackers
struct point points_tracked[MAX_TRACKERS];	// array of all currently tracked points
int tracker_is_active[MAX_TRACKERS];		// flag activity for trackers

//-----------------------------------------------------
// RADAR FUNCTIONS
//-----------------------------------------------------

// Makes a scan in direction imposed by the passed angle.
void read_sensor(int degree) {
	int x, y, c, d;
	float alpha;

	d = RMIN;
	alpha = degree * PI / 180; // angle in rads
	do {
		x = RPOSX + d * cos(alpha);
		y = RPOSY - d * sin(alpha);
		c = getpixel(screen, x, y);
		d = d + RSTEP;
	} while ((d <= RMAX) && (c == BKG || c == BLU));

	// store absolute values (i.e. of the screen)
	radar[degree - RAMIN].x = x;
	radar[degree - RAMIN].y = y;
	radar[degree - RAMIN].d = d;
}

// Check if yet tracked, if not proceed spawning a new tracker_task.
void lock_new_target() {
	int i, new_task_i, new_tracker_i;

	// check if not yet tracked
	for (i = 0; i < MAX_TRACKERS; i++) {
		// if inside a circular round of any active points_tracked I'll consider it already monitored
		if (abs(points_tracked[i].x - radar[current_i].x) <= TRACKER_RES &&
		        abs(points_tracked[i].y - radar[current_i].y) <= TRACKER_RES &&
		        tracker_is_active[i])
			return;
	}

	// proceed locking tracker on current point, if there's not yet too many trackers
	new_task_i = find_free_slot(TRACKER_BASE_INDEX, TRACKER_TOP_INDEX);
	if (new_task_i != -1) {
		new_tracker_i = new_task_i - TRACKER_BASE_INDEX;
		points_tracked[new_tracker_i].x = radar[current_i].x;
		points_tracked[new_tracker_i].y = radar[current_i].y;

		start_task(tracker_task, PER, DREL, PRI, new_task_i);
	}
}

/**
 * Task that makes radial line scans with origin (RPOSX, RPOSY),
 * 		if something is deteched, a tracker is attached in that position.
 */
void *radar_task(void* arg) {
	int angle, j, i = get_task_index(arg);
	float dt;

	angle = RAMIN;
	// dt = TSCALE * (float)get_task_period(i) / 1000;

	// init radar distances
	for (j = 0; j < ARES; j++)
		radar[j].d = RMAX;

	// set all trackers inactive
	for (j = 0; j < MAX_TRACKERS; j++)
		tracker_is_active[j] = 0;

	set_period(i);
	while (!sigterm_tasks) {
		read_sensor(angle);
		current_i = angle - RAMIN;

		if (radar[current_i].d < RMAX) {
			/*printf("Distance: %d / %d - angle: %d p: %d %d\n", radar[current_i].d, RMAX,
			       angle, radar[current_i].x, radar[current_i].y);*/
			lock_new_target();
		}

		angle++;
		if (angle > RAMAX)
			angle = RAMIN;

		wait_for_period(i);
	}
}

// Draws output of the radar in top right of window's corner.
void draw_radar_display() {
	int i;
	int rx, ry, ax, ay;	// relative and absolute positions of points
	// int cx, cy;			// current point detected

	// borders
	arc(screen_buff, RDISPLAY_ORIGIN_X, RDISPLAY_ORIGIN_Y,
	    deg2fix(RAMIN), deg2fix(RAMAX), RDISPLAY_RADIOUS, LBLU);
	line(screen_buff, RDISPLAY_SL_X, RDISPLAY_SL_Y, RDISPLAY_LEFT_X, RDISPLAY_LEFT_Y, LBLU);
	line(screen_buff, RDISPLAY_SR_X, RDISPLAY_SR_Y, RDISPLAY_RIGHT_X, RDISPLAY_RIGHT_Y, LBLU);
	arc(screen_buff, RDISPLAY_ORIGIN_X, RDISPLAY_ORIGIN_Y,
	    deg2fix(RAMIN), deg2fix(RAMAX), RDISPLAY_START_RAD, LBLU);

	// draws all detected distances
	for (i = 0; i < ARES; i++) {
		// convert from absolute position to radar's origin
		rx = radar[i].x - RPOSX_W - WORLD_BOX_X1;
		ry = WORLD_BOX_Y2 - RPOSY_W - radar[i].y;

		// locate it in radar display, absolute referring system
		ax = RDISPLAY_ORIGIN_X + round(rx * RSCALE);
		ay = RDISPLAY_ORIGIN_Y - round(ry * RSCALE);

		if (radar[i].d < RMAX)
			putpixel(screen_buff, ax, ay, GREEN);

		// store actual point scanned
		/*if (i == current_i) {
			cx = ax;
			cy = ay;
		}*/
	}

	// draws actual distance scanned
	// line(screen_buff, RDISPLAY_ORIGIN_X, RDISPLAY_ORIGIN_Y, cx, cy, BLU);
}

//-----------------------------------------------------
// TRACKER FUNCTIONS
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
			        tracker_view[tracker_i][i][j] != BLU) {
				c_rel.x += i - TRACKER_RES / 2;
				c_rel.y += j - TRACKER_RES / 2;
				n++;
			}
		}

	if (n != 0) {
		c_rel.x /= n;
		c_rel.y /= n;
	}
	else {
		// printf("Tracker deactivated.\n");
		tracker_is_active[tracker_i] = 0;
	}

	return c_rel;
}

void *tracker_task(void* arg) {
	int task_i, tracker_i;

	task_i = get_task_index(arg);
	tracker_i = task_i - TRACKER_BASE_INDEX;

	// printf("Start tracking (%d, %d)\n", points_tracked[tracker_i].x, points_tracked[tracker_i].y);
	tracker_is_active[tracker_i] = 1;

	set_period(task_i);
	while (!sigterm_tasks && tracker_is_active[tracker_i]) {
		get_image(tracker_i, points_tracked[tracker_i].x, points_tracked[tracker_i].y);

		// now compute centroid, then update points_tracked (the center to follow)
		struct point c = compute_centroid(tracker_i);
		points_tracked[tracker_i].x += c.x;
		points_tracked[tracker_i].y += c.y;

		wait_for_period(task_i);
	}
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