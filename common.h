#ifndef COMMON_H
#define COMMON_H

#define TSCALE		2
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
#define MENU_CONTENT1_Y	MENU_TITLE_Y + 1.5 * CHAR_HEIGHT
#define MENU_CONTENT2_X	MENU_CONTENT1_X
#define MENU_CONTENT2_Y	MENU_CONTENT1_Y + 1.5 * CHAR_HEIGHT
#define MENU_CONTENT3_X	MENU_CONTENT2_X
#define MENU_CONTENT3_Y	MENU_CONTENT2_Y + 1.5 * CHAR_HEIGHT
#define MENU_CONTENT4_X	MENU_CONTENT3_X
#define MENU_CONTENT4_Y	MENU_CONTENT3_Y + 1.5 * CHAR_HEIGHT
#define MENU_CONTENT5_X	MENU_CONTENT4_X
#define MENU_CONTENT5_Y	MENU_CONTENT4_Y + 1.5 * CHAR_HEIGHT
#define MENU_CONTENT6_X	MENU_CONTENT5_X
#define MENU_CONTENT6_Y	MENU_CONTENT5_Y + 1.5 * CHAR_HEIGHT
#define MENU_CONTENT7_X	MENU_CONTENT6_X
#define MENU_CONTENT7_Y	MENU_CONTENT6_Y + 1.5 * CHAR_HEIGHT
#define MENU_CONTENT8_X	MENU_CONTENT7_X
#define MENU_CONTENT8_Y	MENU_CONTENT7_Y + 1.5 * CHAR_HEIGHT
//-----------------------------------------------------
#define MENU_ERROR2_X	MENU_BOX_X1 + 20
#define MENU_ERROR2_Y	MENU_BOX_Y2 - CHAR_HEIGHT - 10
#define MENU_ERROR1_X	MENU_ERROR2_X
#define MENU_ERROR1_Y	MENU_ERROR2_Y - 1.5 * CHAR_HEIGHT
//-----------------------------------------------------
#define SCORE_X1		LAUNCHER_TITLE_POSX
#define SCORE_Y1		LAUNCHER_STAT5_Y + 4 * CHAR_HEIGHT
#define SCORE_X2		SCORE_X1
#define SCORE_Y2		SCORE_Y1 + 1.5 * CHAR_HEIGHT
#define SCORE_BOX_X1	LAUNCHER_TITLE_POSX - 80
#define SCORE_BOX_Y1	SCORE_Y1 - CHAR_HEIGHT
#define SCORE_BOX_X2	LAUNCHER_TITLE_POSX + 80
#define SCORE_BOX_Y2	SCORE_Y2 + CHAR_HEIGHT
#define SCORE_X0		SCORE_X1
#define SCORE_Y0		SCORE_BOX_Y1 - CHAR_HEIGHT / 4

//-----------------------------------------------------
// PUBLIC FUNCTIONS
//-----------------------------------------------------
int world2abs_x(int x);
int world2abs_y(int y);
int abs2world_x(int x);
int abs2world_y(int y);
void *graphic_task(void* arg);
void *interp_task(void* arg);

#endif