#include "radar.h"

// #include <stddef.h>
#include <stdio.h>
// #include <time.h>
#include <math.h>
// #include <pthread.h>
#include <allegro.h>

#include "baseUtils.h"
#include "common.h"
#include "trackers.h"

//-----------------------------------------------------
// RADAR VARIABLES
//-----------------------------------------------------
struct scan	radar[ARES];	// struct  of points detected
int			current_i;		// actual index of radar array; for graphic porpuses

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
	} while ((d <= RMAX) && (c == BKG || c == BLU || c == GREY));

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
		// if inside a circular round of any active current_points_tracked I'll consider it already monitored
		if (abs(current_points_tracked[i].x - radar[current_i].x) <= TRACKER_RES &&
		        abs(current_points_tracked[i].y - radar[current_i].y) <= TRACKER_RES &&
		        tracker_is_active[i])
			return;
	}

	// proceed locking tracker on current point, if there's not yet too many trackers
	new_task_i = find_free_slot(TRACKER_BASE_INDEX, TRACKER_TOP_INDEX);
	if (new_task_i != -1) {
		new_tracker_i = new_task_i - TRACKER_BASE_INDEX;
		current_points_tracked[new_tracker_i].x = radar[current_i].x;
		current_points_tracked[new_tracker_i].y = radar[current_i].y;

		start_task(tracker_task, TRACKER_PER, TRACKER_DREL, TRACKER_PRI, new_task_i);
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