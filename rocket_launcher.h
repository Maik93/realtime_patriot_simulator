#ifndef ROCKET_LAUNCHER_H
#define ROCKET_LAUNCHER_H

#include "common.h"
#include "trackers.h"

//-----------------------------------------------------
// PUBLIC VARIABLES
//-----------------------------------------------------
int launch_velocity;
int launcher_angle_des;
float launcher_angle_current;
extern int show_predictions;

//-----------------------------------------------------
// PHYSICS CONSTANTS
//-----------------------------------------------------
#define LAUNCHER_V0			140			// velocity (in module) to shoot every missile
#define LAUNCHER_VMIN		20			// min and max velocity to shoot every missile
#define LAUNCHER_VMAX		200
#define LAUNCHER_ANGLE_DEG	(180 + 20)	// angle from horizon. Graphic is counterclockwise
#define LAUNCHER_ANGLE_MIN	180			// min and max angle
#define LAUNCHER_ANGLE_MAX	(180 + 90)
#define LAUNCHER_DEG0		180			// rocket launcher starts from horizontal
#define LAUNCHER_POLE		0.5			// rocket launcher motion is a first order system
#define LAUNCHER_ANGLE_RAD	(LAUNCHER_ANGLE_DEG / 180.0 * PI)
#define LAUNCHER_T_INTERVAL	300			// interval between launches (ms)
#define LAUNCHER_MIN_SAMPLE	15			// minimum number of samples to start working on trajectory evaluation
#define LAUNCHER_SHOOT_THRESHOLD 0.01	// time window to shoot
//-----------------------------------------------------
// GRAPHIC CONSTANTS
//-----------------------------------------------------
#define LAUNCHER_WIDTH		75
#define LAUNCHER_HEIGHT		25
#define LAUNCHER_PIVOT_X	(WORLD_BOX_X2 - 30)	// absolute coords
#define LAUNCHER_PIVOT_Y	(WORLD_BOX_Y2 - 20)	// absolute coords
#define LAUNCHER_BASE_W		50
#define LAUNCHER_BASE_H		10
#define LAUNCHER_BOLT_R		4
//-----------------------------------------------------
// TEXT MENU CONSTANTS
//-----------------------------------------------------
#define LAUNCHER_TITLE_POSX	TRACK_D0_X + TRACKER_RES * TRACK_DSCALE / 2
#define LAUNCHER_TITLE_POSY	TRACK_D2_Y + TRACKER_RES * TRACK_DSCALE / 2 + 5 * CHAR_HEIGHT
#define LAUNCHER_STAT1_X	TRACK_D0_X - TRACKER_RES * TRACK_DSCALE / 2 + CHAR_WIDTH
#define LAUNCHER_STAT1_Y	LAUNCHER_TITLE_POSY + 2 * CHAR_HEIGHT
#define LAUNCHER_STAT2_X	LAUNCHER_STAT1_X
#define LAUNCHER_STAT2_Y	LAUNCHER_STAT1_Y + 1.5 * CHAR_HEIGHT
#define LAUNCHER_STAT3_X	LAUNCHER_STAT2_X
#define LAUNCHER_STAT3_Y	LAUNCHER_STAT2_Y + 1.5 * CHAR_HEIGHT
#define LAUNCHER_STAT4_X	LAUNCHER_STAT3_X
#define LAUNCHER_STAT4_Y	LAUNCHER_STAT3_Y + 1.5 * CHAR_HEIGHT
#define LAUNCHER_STAT5_X	LAUNCHER_STAT4_X
#define LAUNCHER_STAT5_Y	LAUNCHER_STAT4_Y + 1.5 * CHAR_HEIGHT

//-----------------------------------------------------
// PUBLIC FUNCTIONS
//-----------------------------------------------------
void draw_launcher();
void draw_current_trajectory();
void print_launcher_status();
void *rocket_launcher_task(void* arg);

#endif