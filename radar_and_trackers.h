#ifndef RADAR_AND_TRACKERS_H
#define RADAR_AND_TRACKERS_H

#include <time.h>

#include "common.h"

//-----------------------------------------------------
// RADAR SENSOR PARAMETERS
//-----------------------------------------------------
#define RPOSX_W	300					// x and y wolrd position of radar
#define RPOSY_W	0
#define RPOSX	WORLD_BOX_X1 + RPOSX_W	// ablsolute positions
#define RPOSY	WORLD_BOX_Y2 - RPOSY_W
#define RMIN	150					// min and max sensor distance (px)
#define RMAX	350
#define RSTEP	1					// sensor resolution (px)
#define RAMIN	38					// min and max alpha for radar scanning (deg)
#define RAMAX	180 - RAMIN
#define ARES	RAMAX - RAMIN		// 104°
//-----------------------------------------------------
// RADAR DISPLAY (1/3 of sensor radar, at top right of window)
// 		R*sin(RAMIN)=72 | R*cos(RAMIN)=92
//-----------------------------------------------------
#define RSCALE				1/3
#define RDISPLAY_RADIOUS	117		// max distance that can be scanned
#define RDISPLAY_START_RAD	49		// minimum radious that can be scanned
#define RDISPLAY_Y_MARGIN	39		// margin from top of the window
#define RDISPLAY_ORIGIN_X	MENU_BOX_X2 + 6 + 92	// vertex of the radar
#define RDISPLAY_ORIGIN_Y	RDISPLAY_Y_MARGIN + RDISPLAY_RADIOUS
//-----------------------------------------------------
#define RDISPLAY_RIGHT_X	RDISPLAY_ORIGIN_X + 92	// positions of right and left extremes of the radar
#define RDISPLAY_RIGHT_Y	RDISPLAY_ORIGIN_Y - 72
#define RDISPLAY_SR_X		RDISPLAY_ORIGIN_X + 40	// where radar starts (right value)
#define RDISPLAY_SR_Y		RDISPLAY_ORIGIN_Y - 30
//-----------------------------------------------------
#define RDISPLAY_LEFT_X		RDISPLAY_ORIGIN_X - 92
#define RDISPLAY_LEFT_Y		RDISPLAY_RIGHT_Y
#define RDISPLAY_SL_X		RDISPLAY_ORIGIN_X - 40	// where radar starts (left value)
#define RDISPLAY_SL_Y		RDISPLAY_SR_Y
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
//-----------------------------------------
//-----------------------------------------------------
// STRUCT
//-----------------------------------------------------
struct scan {	// point acquired by scanner, with relative distance
	int x;		// position detected
	int y;
	int d;		// distance measured
};
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
// PUBLIC FUNCTIONS
//-----------------------------------------------------
void *radar_task(void* arg);
void draw_radar_display();
void *tracker_task(void* arg);
void tracker_display(int tracker_i);

#endif