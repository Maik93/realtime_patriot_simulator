#ifndef TRACKERS_H
#define TRACKERS_H

#include <time.h>
#include "baseUtils.h"
#include "common.h"

// extern float pred_x, pred_y;

//-----------------------------------------------------
// TRACKER PARAMETERS AND DISPLAY POSITIONS
//-----------------------------------------------------
#define TRACKER_RES			60		// dimension of square box acquisition
#define TSTORE				10		// number of points stored by the tracker
//-----------------------------------------------------
#define TRACK_DSCALE		3/2		// scale for displays
#define TRACK_D0_X			WORLD_BOX_X2 + 5 + TRACKER_RES * TRACK_DSCALE / 2
#define TRACK_D0_Y			WORLD_BOX_Y1 + TRACKER_RES * TRACK_DSCALE / 2
#define TRACK_D1_X			TRACK_D0_X + TRACKER_RES * TRACK_DSCALE + 4
#define TRACK_D1_Y			TRACK_D0_Y
#define TRACK_D2_X			TRACK_D0_X
#define TRACK_D2_Y			TRACK_D0_Y + TRACKER_RES * TRACK_DSCALE + 4
#define TRACK_D3_X			TRACK_D1_X
#define TRACK_D3_Y			TRACK_D2_Y
//-----------------------------------------------------
// TASK CONSTANTS
//-----------------------------------------------------
#define TRACKER_PER		20			// task period in ms
#define TRACKER_DREL	TRACKER_PER	// relative deadline in ms
#define TRACKER_PRI		50			// task priority

//-----------------------------------------------------
// STRUCT
//-----------------------------------------------------
struct point {
	int x;
	int y;
};
struct tracker {	// circular buffer for last TSTORE points acquired + vel and acc measured
	int top;		// index of the current point element
	int x[TSTORE];	// array for x coordinates (world coord)
	int y[TSTORE];	// array for y coordinates (world coord)
	struct timespec t[TSTORE]; // time of acquisition of coord. Allows more accuracy than just TRACKER_PER
	int n_samples;	// number of points in this circular buffer (matters only for first steps)
	float vx;		// estimated velecity for x and y
	float vy;
	float ax;		// estimated acceleration for x and y
	float ay;
};

//-----------------------------------------------------
// PUBLIC VARIABLES
//-----------------------------------------------------
extern struct point
	current_points_tracked[MAX_TRACKERS];
extern int tracker_is_active[MAX_TRACKERS];

//-----------------------------------------------------
// PUBLIC FUNCTIONS
//-----------------------------------------------------
void tracker_display(int tracker_i);
void draw_predictions(int tracker_i, float delta_t);
void *tracker_task(void* arg);

#endif