#ifndef RADAR_H
#define RADAR_H

//-----------------------------------------------------
// WORLD POSITIONS
//-----------------------------------------------------
#define RPOSX_W	300		// x and y wolrd position of radar
#define RPOSY_W	0
#define RMIN	150		// min and max sensor distance (px)
#define RMAX	350
#define RSTEP	1		// sensor resolution (px)
#define RAMIN	38		// min and max alpha for radar scanning (deg)
#define RAMAX	180 - RAMIN
// #define RTSCAN	1040	// time to go from RAMIN to RAMAX (milliseconds)
//-----------------------------------------------------
// ABLSOLUTE POSITIONS
//-----------------------------------------------------
#define RPOSX	WORLD_BOX_X1 + RPOSX_W
#define RPOSY	WORLD_BOX_Y2 - RPOSY_W
//-----------------------------------------------------
#define ARES	RAMAX - RAMIN	// 104°
//-----------------------------------------------------
// STRUCT
//-----------------------------------------------------
struct scan {
	int x;
	int y;
};

extern struct scan radar[ARES];

void *radar_task(void* arg);

#endif