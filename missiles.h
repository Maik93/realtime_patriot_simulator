#ifndef MISSILES_H
#define MISSILES_H

#include "baseUtils.h"
#include "common.h"

//-----------------------------------------------------
// GRAPHICAL CONSTANTS
//-----------------------------------------------------
#define ML		15					// length and width of the missile
#define MW		6
#define TCOL	GREY		// trail color
//-----------------------------------------------------
// PHYSICS CONSTANTS
//-----------------------------------------------------
#define VMINL	15					// min and max initial hor. speed for missiles coming from left (m/s)
#define VMAXL	60
#define VMINT	5					// min and max initial hor. speed for missiles coming from top (m/s)
#define VMAXT	10
#define AMINL	-PI/4				// min and max initial alpha for missiles coming from left (rad)
#define AMAXL	0
#define AMINT	-PI/2 - PI/4		// min and max initial alpha for missiles coming from top (rad)
#define AMAXT	-PI/2 + PI/4
#define XMINT	WORLD_BOX_X1 + 100 + ML	// min and max horizontal position for top spawn (world coord.)
#define XMAXT	WORLD_BOX_WIDTH - 200
#define YMINL	WORLD_BOX_HEIGHT - 100	// min and max vertical position for left spawn (world coord.)
#define YMAXL	WORLD_BOX_HEIGHT - ML
// #define DA_DOT	1*PI/180		// max alpha_dot variation per step (rad/s)
// #define XCEN	0			// x center for the missile arrow
// #define YCEN	0			// y center for the missile arrow
//-----------------------------------------------------
// TASK CONSTANTS
//-----------------------------------------------------
#define MISSILE_PER		20	// task period in ms
#define MISSILE_DREL	MISSILE_PER	// relative deadline in ms
#define MISSILE_PRI		60	// task priority
//-----------------------------------------------------
#define TLEN	30			// circular buffer length
//-----------------------------------------------------
// STRUCT
//-----------------------------------------------------
struct missile {	// missile structure
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
// PUBLIC VARIABLES
//-----------------------------------------------------
extern struct missile	missile[MAX_ENEMY_MISSILES];	// missile buffer
extern struct cbuf		trail[MAX_ENEMY_MISSILES];		// trail buffer
extern int		tflag;		// trail flag
extern int		tl;			// actual trail length
//-----------------------------------------------------
// PUBLIC FUNCTIONS
//-----------------------------------------------------
void draw_trail(int i, int w);
void draw_missile(int i);
void *missile_task(void* arg);

#endif