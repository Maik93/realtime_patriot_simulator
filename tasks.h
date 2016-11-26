#ifndef THREAD_UTILS_H
#define THREAD_UTILS_H

//-----------------------------------------------------
// BOX DIMENSIONS
//-----------------------------------------------------
#define MARGIN			5
#define MENU_BOX_WIDTH	600
#define MENU_BOX_EIGHT	185
#define WORLD_BOX_WIDTH	600
#define WORLD_BOX_EIGHT	400
//-----------------------------------------------------
#define MENU_BOX_X1		MARGIN
#define MENU_BOX_Y1		MARGIN
#define MENU_BOX_X2		MARGIN + MENU_BOX_WIDTH
#define MENU_BOX_Y2		MARGIN + MENU_BOX_EIGHT
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
#define ML		15				// length of the missile
#define MW		6				// width of the missile
// #define XCEN	0			// x center for the missile arrow
// #define YCEN	0			// y center for the missile arrow
// #define RMIN	5				// min radius
// #define RMAX	10				// man radius
#define G0		9.8				// acceleration of gravity
// #define HMAX	WIN_HEIGHT - YBOX -RMAX -10 // max initial height
// #define HMIN	HMAX - 150		// min initial height
#define VMIN	10				// min initial hor. speed
#define VMAX	20				// max initial hor. speed
#define PI		3.14
#define AMIN	0				// min initial hor. speed
#define AMAX	2*PI			// max initial hor. speed
#define DELTA_A	0*PI/180		// max alpha variation per step (rad)
#define DELTA_V	20				// max vel variation per step (m/s)
// #define DUMP	.9				// dumping coefficient
#define TSCALE	2				// time scale factor
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
	float v;		// velocity (m/s)
	float alpha;	// orientation angle (rads)
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
// void handle_bounce(int i);
// void init_missile(int i);
// void *missiletask(void* arg);
//-----------------------------------------------------
void *display(void* arg);
void *interp(void* arg);

#endif