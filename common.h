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
// TEXT MENU POSITIONS
//-----------------------------------------------------
#define MENU_TITLE_X	MENU_BOX_X1 + 10
#define MENU_TITLE_Y	MENU_BOX_Y1 + 10
#define MENU_CONTENT1_X	MENU_TITLE_X + 10
#define MENU_CONTENT1_Y	MENU_TITLE_Y + 2 * CHAR_HEIGHT
#define MENU_CONTENT2_X	MENU_CONTENT1_X
#define MENU_CONTENT2_Y	MENU_CONTENT1_Y + 2 * CHAR_HEIGHT
#define MENU_CONTENT3_X	MENU_CONTENT2_X
#define MENU_CONTENT3_Y	MENU_CONTENT2_Y + 2 * CHAR_HEIGHT
#define MENU_CONTENT4_X	MENU_CONTENT3_X
#define MENU_CONTENT4_Y	MENU_CONTENT3_Y + 2 * CHAR_HEIGHT
#define MENU_CONTENT5_X	MENU_CONTENT4_X
#define MENU_CONTENT5_Y	MENU_CONTENT4_Y + 2 * CHAR_HEIGHT
#define MENU_CONTENT6_X	MENU_CONTENT5_X
#define MENU_CONTENT6_Y	MENU_CONTENT5_Y + 2 * CHAR_HEIGHT
//-----------------------------------------------------
// TASK CONSTANTS
//-----------------------------------------------------
#define TSCALE		2	// time scale factor - DBG: best is 5
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