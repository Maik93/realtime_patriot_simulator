#ifndef RADAR_H
#define RADAR_H

//-----------------------------------------------------
// WORLD POSITIONS
//-----------------------------------------------------
#define RPOSX_W	300		// x and y wolrd position of radar
#define RPOSY_W	0
#define RMIN		150		// min and max sensor distance (px)
#define RMAX		350
#define RSTEP		1		// sensor resolution (px)
#define RAMIN	38		// min and max alpha for radar scanning (deg)
#define RAMAX	180 - RAMIN
//-----------------------------------------------------
// ABLSOLUTE POSITIONS
//-----------------------------------------------------
#define RPOSX	RPOSX_W + WORLD_BOX_X1
#define RPOSY	WORLD_BOX_Y2 - RPOSY_W
//-----------------------------------------------------
#define ARES		RAMAX - RAMIN

void *radar_task(void* arg);

#endif