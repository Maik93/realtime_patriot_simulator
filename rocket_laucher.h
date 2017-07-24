#ifndef ROCKET_LAUCHER_H
#define ROCKET_LAUCHER_H

#include "common.h"

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
// PHYSICS CONSTANTS
//-----------------------------------------------------
#define LAUNCHER_V0			120			// velocity (in module) to shoot every missile
#define LAUNCHER_ANGLE_DEG	(180 + 30)	// angle from horizon. Graphic is counterclockwise
#define LAUNCHER_ANGLE_RAD	(LAUNCHER_ANGLE_DEG / 180.0 * PI)
//-----------------------------------------------------

void draw_launcher();
void draw_current_trajectory();
void *rocket_laucher_task(void* arg);

#endif