#ifndef TRACKERS_H
#define TRACKERS_H

#include <time.h>
#include "baseUtils.h"
#include "common.h"

//-----------------------------------------------------
// TRACKER PARAMETERS
//-----------------------------------------------------
#define TRACKER_RES			60		// dimension of square box acquisition
#define TSTORE				20		// number of points stored by the tracker
#define TRAJ_MAX_ERROR		100		// max ammount of trajectory error in order to be assumed ballistic
//-----------------------------------------------------
// DISPLAY CENTERS AND B0XES POSITIONS
//-----------------------------------------------------
#define TRACK_DSCALE		3/2		// scale for displayers
#define TRACK_D0_X			WORLD_BOX_X2 + 5 + TRACKER_RES * TRACK_DSCALE / 2
#define TRACK_D0_Y			WORLD_BOX_Y1 + TRACKER_RES * TRACK_DSCALE / 2
#define TRACK_D1_X			TRACK_D0_X + TRACKER_RES * TRACK_DSCALE + 4
#define TRACK_D1_Y			TRACK_D0_Y
#define TRACK_D2_X			TRACK_D0_X
#define TRACK_D2_Y			TRACK_D0_Y + TRACKER_RES * TRACK_DSCALE + 4
#define TRACK_D3_X			TRACK_D1_X
#define TRACK_D3_Y			TRACK_D2_Y
//-----------------------------------------------------
#define TRACK_BOX1_X0		TRACK_D0_X - TRACKER_RES * TRACK_DSCALE / 2
#define TRACK_BOX1_Y0		TRACK_D0_Y - TRACKER_RES * TRACK_DSCALE / 2
#define TRACK_BOX1_X1		TRACK_D0_X + TRACKER_RES * TRACK_DSCALE / 2
#define TRACK_BOX1_Y1		TRACK_D0_Y + TRACKER_RES * TRACK_DSCALE / 2
#define TRACK_BOX2_X0		TRACK_D1_X - TRACKER_RES * TRACK_DSCALE / 2
#define TRACK_BOX2_Y0		TRACK_D1_Y - TRACKER_RES * TRACK_DSCALE / 2
#define TRACK_BOX2_X1		TRACK_D1_X + TRACKER_RES * TRACK_DSCALE / 2
#define TRACK_BOX2_Y1		TRACK_D1_Y + TRACKER_RES * TRACK_DSCALE / 2
#define TRACK_BOX3_X0		TRACK_D2_X - TRACKER_RES * TRACK_DSCALE / 2
#define TRACK_BOX3_Y0		TRACK_D2_Y - TRACKER_RES * TRACK_DSCALE / 2
#define TRACK_BOX3_X1		TRACK_D2_X + TRACKER_RES * TRACK_DSCALE / 2
#define TRACK_BOX3_Y1		TRACK_D2_Y + TRACKER_RES * TRACK_DSCALE / 2
#define TRACK_BOX4_X0		TRACK_D3_X - TRACKER_RES * TRACK_DSCALE / 2
#define TRACK_BOX4_Y0		TRACK_D3_Y - TRACKER_RES * TRACK_DSCALE / 2
#define TRACK_BOX4_X1		TRACK_D3_X + TRACKER_RES * TRACK_DSCALE / 2
#define TRACK_BOX4_Y1		TRACK_D3_Y + TRACKER_RES * TRACK_DSCALE / 2

//-----------------------------------------------------
// STRUCTS
//-----------------------------------------------------
struct point {
	int x;
	int y;
};

struct tracker {
	// circular buffer for last TSTORE points acquired
	int top;		// index of the current point element
	int x[TSTORE];	// array for x coordinates (world coord)
	int y[TSTORE];	// array for y coordinates (world coord)
	struct timespec t[TSTORE]; // time of acquisition of coord. Allows more accuracy than just TRACKER_PER
	int n_samples;	// number of points in this circular buffer (matters only for first steps)
	// -----------------------------------------------------
	// estimated velocities and accelerations
	float vx;		// estimated velecity for x and y
	float vy;
	float ax;		// estimated acceleration for x and y
	float ay;
	// -----------------------------------------------------
	// evaluated trajectory inaccuracy
	float traj_error;
	// -----------------------------------------------------
	// interception time evaluation
	float time_to_shoot; // in milliseconds
};

//-----------------------------------------------------
// PUBLIC VARIABLES
//-----------------------------------------------------
int tracker_disp_mode;
extern struct point
	current_points_tracked[MAX_TRACKERS];
extern int tracker_is_active[MAX_TRACKERS];
struct tracker
	tracked_points[MAX_TRACKERS];

//-----------------------------------------------------
// PUBLIC FUNCTIONS
//-----------------------------------------------------
void tracker_display(int tracker_i);
void draw_predictions(int tracker_i, float delta_t);
void *tracker_task(void* arg);

#endif