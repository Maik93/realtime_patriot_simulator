#include "radar.h"

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
int image[TRACKER_RES][TRACKER_RES]; // global image buffer
int points_tracked[MAX_TRACKERS][2]; // list of all currently tracked points

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
	int i, new_tracker_i;

	// check if not yet tracked
	for (i = 0; i < MAX_TRACKERS; i++) {
		// if inside a circular round of any points_tracked I'll consider it already monitored
		if (abs(points_tracked[i][0] - radar[current_i].x) <= TRACKER_RES &&
		        abs(points_tracked[i][1] - radar[current_i].y) <= TRACKER_RES)
			return;
	}

	// proceed locking tracker on current point, if there's not yet too many trackers
	new_tracker_i = find_free_slot(TRACKER_BASE_INDEX, TRACKER_TOP_INDEX);
	if (new_tracker_i != -1) {
		points_tracked[new_tracker_i - TRACKER_BASE_INDEX][0] = radar[current_i].x;
		points_tracked[new_tracker_i - TRACKER_BASE_INDEX][1] = radar[current_i].y;
		
		start_task(tracker_task, PER, DREL, PRI, new_tracker_i);
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

	for (j = 0; j < ARES; j++)
		radar[j].d = RMAX;

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
		if (angle > RAMAX) {
			// printf("*bip*\n");
			angle = RAMIN;
		}

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
void get_image(int x0, int y0) {
	int i, j; // image matrix indexes
	int x, y; // graphical coordinates
	for (i = 0; i < TRACKER_RES; i++)
		for (j = 0; j < TRACKER_RES; j++) {
			x = x0 - TRACKER_RES / 2 + i;
			y = y0 - TRACKER_RES / 2 + j;
			image[i][j] = getpixel(screen, x, y);
		}
}

void *tracker_task(void* arg) {
	int task_i, tracker_i;

	task_i = get_task_index(arg);
	tracker_i = task_i - TRACKER_BASE_INDEX;

	printf("Tracking (%d, %d)\n", points_tracked[tracker_i][0], points_tracked[tracker_i][1]);

	set_period(task_i);
	while (!sigterm_tasks) {
		get_image(points_tracked[tracker_i][0], points_tracked[tracker_i][1]);

		// now compute centroid, then refresh points_tracked

		wait_for_period(task_i);
	}
}

// Shows what a tracker sees on right side of the screen.
void tracker_display(int x0, int y0) {
	int i, j; // image matrix indexes
	int x, y; // graphical coordinates
	for (i = 0; i < TRACKER_RES; i++)
		for (j = 0; j < TRACKER_RES; j++) {
			x = x0 - TRACKER_RES / 2 + i;
			y = y0 - TRACKER_RES / 2 + j;
			putpixel(screen_buff, x, y, image[i][j]);
		}
}