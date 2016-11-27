#ifndef THREAD_UTILS_H
#define THREAD_UTILS_H

//-----------------------------------------------------
// BOX DIMENSIONS
//-----------------------------------------------------
#define MARGIN				5
#define MENU_BOX_WIDTH		600
#define MENU_BOX_HEIGHT		185
#define WORLD_BOX_WIDTH		600
#define WORLD_BOX_HEIGHT	400
//-----------------------------------------------------
#define MENU_BOX_X1			MARGIN
#define MENU_BOX_Y1			MARGIN
#define MENU_BOX_X2			MARGIN + MENU_BOX_WIDTH
#define MENU_BOX_Y2			MARGIN + MENU_BOX_HEIGHT
#define WORLD_BOX_X1		MENU_BOX_X1
#define WORLD_BOX_Y1		MENU_BOX_Y2 + MARGIN
#define WORLD_BOX_X2		MENU_BOX_X2
#define WORLD_BOX_Y2		WIN_HEIGHT - MARGIN
//-----------------------------------------------------
// TASK CONSTANTS
//-----------------------------------------------------
#define PER		20	// task period in ms
#define DREL	PER	// relative deadline in ms
#define PRI		60	// task priority
//-----------------------------------------------------
// MISSILE CONSTANTS
//-----------------------------------------------------
#define ML		15				// length and width of the missile
#define MW		6
#define VMINL	15				// min and max initial hor. speed for missiles coming from left (m/s)
#define VMAXL	40
#define VMINT	5				// min and max initial hor. speed for missiles coming from top (m/s)
#define VMAXT	10
#define AMINL	-PI/4			// min and max initial alpha for missiles coming from left (rad)
#define AMAXL	0
#define AMINT	-PI/2 - PI/4	// min and max initial alpha for missiles coming from top (rad)
#define AMAXT	-PI/2 + PI/4
#define XMINT	ML				// min and max horizontal position for top spawn (world coord.)
#define XMAXT	WORLD_BOX_WIDTH - 200
#define YMINL	WORLD_BOX_HEIGHT - 100	// min and max vertical position for left spawn (world coord.)
#define YMAXL	WORLD_BOX_HEIGHT - ML
// #define DA_DOT	1*PI/180	// max alpha_dot variation per step (rad/s)
// #define XCEN	0				// x center for the missile arrow
// #define YCEN	0				// y center for the missile arrow
// #define RMIN	5				// min radius
// #define RMAX	10				// man radius
// #define HMAX	WIN_HEIGHT - YBOX -RMAX -10 // max initial height
// #define HMIN	HMAX - 150		// min initial height
#define TSCALE	5				// time scale factor
#define TLEN	30				// circular buffer length
//-----------------------------------------------------
#define TCOL	GREEN			// trail color
//-----------------------------------------------------

//-----------------------------------------------------
// STRUCT
//-----------------------------------------------------
struct missile {		// missile structure
	int c;			// color [1-15]
	float r;		// radius of its hitbox (m)
	float x;		// x coordinate (m)
	float y;		// y coordinate (m)
	float vx;		// x velocity (m/s)
	float vy;		// y velocity (m/s)
	float alpha;	// orientation angle (rads)
	// float alpha_dot;
	int destroied;	// flag for collisions ecc
};
struct cbuf {		// circular buffer structure
	int top;		// index of the current element
	int x[TLEN];	// array of x coordinates
	int y[TLEN];	// array of y coordinates
};

//-----------------------------------------------------
// PRIVATES
// void store_trail(int i);
// void draw_trail(int i, int w);
// void draw_missile(int i);
// void handle_corners(int i);
// void init_missile(int i);
// void *missiletask(void* arg);
//-----------------------------------------------------
void *display(void* arg);
void *interp(void* arg);

#endif