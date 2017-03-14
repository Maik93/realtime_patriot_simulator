#ifndef COMMON_H
#define COMMON_H

//-----------------------------------------------------
// BOX DIMENSIONS
//-----------------------------------------------------
#define MARGIN				5
#define MENU_BOX_WIDTH		600
#define MENU_BOX_HEIGHT		185
#define WORLD_BOX_WIDTH		600
#define WORLD_BOX_HEIGHT	400
//-----------------------------------------------------
#define MENU_BOX_X1		MARGIN
#define MENU_BOX_Y1		MARGIN
#define MENU_BOX_X2		MARGIN + MENU_BOX_WIDTH
#define MENU_BOX_Y2		MARGIN + MENU_BOX_HEIGHT
#define WORLD_BOX_X1	MENU_BOX_X1
#define WORLD_BOX_Y1	MENU_BOX_Y2 + MARGIN
#define WORLD_BOX_X2	MENU_BOX_X2
#define WORLD_BOX_Y2	WIN_HEIGHT - MARGIN
//-----------------------------------------------------
// TASK CONSTANTS
//-----------------------------------------------------
#define TSCALE		5	// time scale factor
//-----------------------------------------------------
// PUBLIC FUNCTIONS
//-----------------------------------------------------
int world2abs_x(int x);
int world2abs_y(int y);
int abs2world_x(int x);
int abs2world_y(int y);
void *graphic_task(void* arg);
void *interp(void* arg);

#endif