#ifndef RADAR_H
#define RADAR_H

#include <time.h>
#include "common.h"

//-----------------------------------------------------
// RADAR SENSOR PARAMETERS
//-----------------------------------------------------
#define RPOSX_W	300						// x and y world position of radar
#define RPOSY_W	0
#define RPOSX	WORLD_BOX_X1 + RPOSX_W	// ablsolute positions
#define RPOSY	WORLD_BOX_Y2 - RPOSY_W
#define RMIN	150						// min and max sensor distance (px)
#define RMAX	350
#define RSTEP	1						// sensor resolution (px)
#define RAMIN	38						// min and max alpha for radar scanning (deg)
#define RAMAX	180 - RAMIN
#define ARES	RAMAX - RAMIN			// 104° - total res angle
//-----------------------------------------------------
// RADAR DISPLAY (1/3 of sensor radar, at top right of window)
// 		R*sin(RAMIN)=72 	 R*cos(RAMIN)=92
//-----------------------------------------------------
#define RSCALE				1/3
#define RDISPLAY_RADIOUS	117						// max distance that can be scanned (RMAX / RSCALE)
#define RDISPLAY_START_RAD	49						// minimum radious that can be scanned (RMIN / RSCALE)
#define RDISPLAY_Y_MARGIN	39						// margin from top of the window
#define RDISPLAY_ORIGIN_X	MENU_BOX_X2 + 6 + 92	// vertex of the radar
#define RDISPLAY_ORIGIN_Y	RDISPLAY_Y_MARGIN + RDISPLAY_RADIOUS
#define RDISPLAY_RIGHT_X	RDISPLAY_ORIGIN_X + 92	// positions of right and left extremes of the radar
#define RDISPLAY_RIGHT_Y	RDISPLAY_ORIGIN_Y - 72
#define RDISPLAY_SR_X		RDISPLAY_ORIGIN_X + 40	// where radar starts (right value)
#define RDISPLAY_SR_Y		RDISPLAY_ORIGIN_Y - 30
#define RDISPLAY_LEFT_X		RDISPLAY_ORIGIN_X - 92
#define RDISPLAY_LEFT_Y		RDISPLAY_RIGHT_Y
#define RDISPLAY_SL_X		RDISPLAY_ORIGIN_X - 40	// where radar ends (left value)
#define RDISPLAY_SL_Y		RDISPLAY_SR_Y
//-----------------------------------------------------
#define RDISPLAY_TITLE_X	RDISPLAY_ORIGIN_X
#define RDISPLAY_TITLE_Y	(RDISPLAY_ORIGIN_Y - RDISPLAY_RADIOUS) - 2 * CHAR_HEIGHT

//-----------------------------------------------------
// STRUCT
//-----------------------------------------------------
struct scan {	// point acquired by scanner, with relative distances from vertex
	int x;		// position detected
	int y;
	int d;		// distance measured
};

//-----------------------------------------------------
// PUBLIC FUNCTIONS
//-----------------------------------------------------
void draw_radar_symbol();
void draw_radar_display();
void *radar_task(void* arg);

#endif