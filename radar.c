/**
 * -----------------------------------------------------------------------
 * Radar. A laser that scan the sky searching for objects.
 * When detected, it will attach a tracker on them.
 * -----------------------------------------------------------------------
 */
#include "radar.h"

// #include <stddef.h>
#include <stdio.h>
// #include <time.h>
#include <math.h>
// #include <pthread.h>
#include <allegro.h>

#include "baseUtils.h"
#include "task_constants.h"
#include "common.h"
#include "colors.h"
#include "missiles.h"
#include "trackers.h"

struct scan	radar[ARES];	// struct  of points detected
int			current_i;		// actual index of radar array; for graphic porpuses

// Makes a scan in direction imposed by the passed angle (in degree).
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
	} while ((d <= RMAX) && (c == BKG || c == PATMISS_COL ||
	                         c == RSENSOR_COL || c == TCOL)); // radar, launcher and trails are ignored

	// store absolute values (i.e. of the screen)
	radar[degree - RAMIN].x = x;
	radar[degree - RAMIN].y = y;
	radar[degree - RAMIN].d = d;
}

// Check if a point is yet tracked, if not proceed spawning a new tracker_task.
void lock_new_target() {
	int i, new_task_i, new_tracker_i;

	// check if not yet tracked
	for (i = 0; i < MAX_TRACKERS; i++) {
		// if inside a rectangle of any active tracker I'll consider it already monitored
		if (abs(current_points_tracked[i].x - radar[current_i].x) <= TRACKER_RES / 2 &&
		        abs(current_points_tracked[i].y - radar[current_i].y) <= TRACKER_RES / 2 &&
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

// Task that makes radial line scans with origin (RPOSX, RPOSY).
// If something is detected, a tracker is attached in that position.
void *radar_task(void* arg) {
	int angle, j, i = get_task_index(arg);

	angle = RAMIN;

	// init radar detected distances
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
			// DBG
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

// Draws a nice radar symbol in the center of world ground.
void draw_radar_symbol() {
	arc(screen_buff, RPOSX, RPOSY, deg2fix(45), deg2fix(135), 2, RSENSOR_COL);
	arc(screen_buff, RPOSX, RPOSY, deg2fix(45), deg2fix(135), 10, RSENSOR_COL);
	arc(screen_buff, RPOSX, RPOSY, deg2fix(45), deg2fix(135), 20, RSENSOR_COL);
}

// Draws output of the radar in top right of window's corner.
void draw_radar_display() {
	int i;
	int rx, ry, ax, ay; // relative and absolute positions of points
	char str[14];

	// label
	sprintf(str, "Radar display");
	textout_centre_ex(screen_buff, font, str, RDISPLAY_TITLE_X, RDISPLAY_TITLE_Y, TEXT_TITL_COL, -1);

	// borders
	arc(screen_buff, RDISPLAY_ORIGIN_X, RDISPLAY_ORIGIN_Y,
	    deg2fix(RAMIN), deg2fix(RAMAX), RDISPLAY_RADIOUS, BORDER_COL);
	line(screen_buff, RDISPLAY_SL_X, RDISPLAY_SL_Y, RDISPLAY_LEFT_X, RDISPLAY_LEFT_Y, BORDER_COL);
	line(screen_buff, RDISPLAY_SR_X, RDISPLAY_SR_Y, RDISPLAY_RIGHT_X, RDISPLAY_RIGHT_Y, BORDER_COL);
	arc(screen_buff, RDISPLAY_ORIGIN_X, RDISPLAY_ORIGIN_Y,
	    deg2fix(RAMIN), deg2fix(RAMAX), RDISPLAY_START_RAD, BORDER_COL);

	// draws all detected distances
	for (i = 0; i < ARES; i++) {
		// convert from absolute position to radar's origin
		rx = abs2world_x(radar[i].x - RPOSX_W);
		ry = abs2world_y(radar[i].y + RPOSY_W);

		// locate it in radar display, absolute referring system
		ax = RDISPLAY_ORIGIN_X + round(rx * RSCALE);
		ay = RDISPLAY_ORIGIN_Y - round(ry * RSCALE);

		// show a dot on the display
		if (radar[i].d < RMAX)
			putpixel(screen_buff, ax, ay, RDISPLAY_DOT_COL);
	}
}