#ifndef MISSILES_H
#define MISSILES_H

#include "baseUtils.h"
#include "common.h"

// extern float vx[MAX_ENEMY_MISSILES], vy[MAX_ENEMY_MISSILES];

//-----------------------------------------------------
// GRAPHICAL CONSTANTS
//-----------------------------------------------------
#define ML		15		// length of the missile
#define MW		6		// width of the missile
#define TCOL	GREY	// trail color
#define DESTR_BMP_NUM 10 // number of bitmaps used in missile destruction animation
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
//-----------------------------------------------------
// TASK CONSTANTS
//-----------------------------------------------------
#define MISSILE_PER		20			// task period in ms
#define MISSILE_DREL	MISSILE_PER	// relative deadline in ms
#define MISSILE_PRI		60			// task priority
//-----------------------------------------------------
#define TLEN			30			// circular buffer length (used to store trails)

//-----------------------------------------------------
// STRUCTS
//-----------------------------------------------------
struct missile {	// missile structure
	int c;			// color [1-15]
	float r;		// radius of its hitbox (m)
	float x;		// x coordinate (m)
	float y;		// y coordinate (m)
	float vx;		// x velocity (m/s)
	float vy;		// y velocity (m/s)
	float alpha;	// orientation angle (rads)
	int in_destruction; // if 0 the missile is intact, otherwise it represents explosion animation step (max 10)
};
struct cbuf {		// circular buffer structure
	int top;		// index of the last element inserted
	int x[TLEN];	// array for x coordinates
	int y[TLEN];	// array for y coordinates
};

//-----------------------------------------------------
// PUBLIC VARIABLES
//-----------------------------------------------------
extern struct missile
	missile[MAX_ENEMY_MISSILES];	// missile buffer
extern struct cbuf
	trail[MAX_ENEMY_MISSILES];		// trail buffer

extern int		tflag;		// switcher for trail's visibility [0-1]
extern int		tl;			// actual trail length

//-----------------------------------------------------
// PUBLIC FUNCTIONS
//-----------------------------------------------------
void draw_trail(int i, int w);
void draw_missile(int i);
void *missile_task(void* arg);

#endif