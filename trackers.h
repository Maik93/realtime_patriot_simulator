#ifndef TRACKERS_H
#define TRACKERS_H

#include <time.h>
#include "baseUtils.h"
#include "common.h"
//-----------------------------------------------------
// TRACKER PARAMETERS AND DISPLAY POSITIONS
//-----------------------------------------------------
#define TRACKER_RES			60		// dimension of square box acquisition
#define TRACK_DSCALE		3/2		// scale for displays
#define TRACK_D0_X			WORLD_BOX_X2 + 5 + TRACKER_RES*TRACK_DSCALE/2
#define TRACK_D0_Y			WORLD_BOX_Y1 + TRACKER_RES*TRACK_DSCALE/2
#define TRACK_D1_X			TRACK_D0_X + TRACKER_RES*TRACK_DSCALE + 4
#define TRACK_D1_Y			TRACK_D0_Y
#define TRACK_D2_X			TRACK_D0_X
#define TRACK_D2_Y			TRACK_D0_Y + TRACKER_RES*TRACK_DSCALE + 4
#define TRACK_D3_X			TRACK_D1_X
#define TRACK_D3_Y			TRACK_D2_Y
//-----------------------------------------------------
// TASK CONSTANTS
//-----------------------------------------------------
#define TRACKER_PER		50	// task period in ms
#define TRACKER_DREL	TRACKER_PER	// relative deadline in ms
#define TRACKER_PRI		50	// task priority
//-----------------------------------------------------
// STRUCT
//-----------------------------------------------------
struct point {
	int x;
	int y;
};
struct tracker {		// circular buffer structure + vel and acc measured
	int top;		// index of the current point element
	int x[3];	// array of x coordinates
	int y[3];	// array of y coordinates
	struct timespec t[3]; // time of acquisition of coord.
	int n_samples;	// number of points detected
	float vx;		// stimed velecity for x
	float ax;		// stimed acceleration for x
	float vy;		// same for y
	float ay;
};
//-----------------------------------------------------
// PUBLIC VARIABLES
//-----------------------------------------------------
extern struct point current_points_tracked[MAX_TRACKERS];
extern int tracker_is_active[MAX_TRACKERS];
//-----------------------------------------------------
// PUBLIC FUNCTIONS
//-----------------------------------------------------
void *tracker_task(void* arg);
void tracker_display(int tracker_i);

#endif