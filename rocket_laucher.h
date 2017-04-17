#ifndef ROCKET_LAUCHER_H
#define ROCKET_LAUCHER_H

#include "common.h"

//-----------------------------------------------------
// GRAPHIC CONSTANTS
//-----------------------------------------------------
#define LAUNCHER_WIDTH		75
#define LAUNCHER_HEIGHT		25
#define LAUNCHER_PIVOT_X	WORLD_BOX_X2 - 30	// absolute coords
#define LAUNCHER_PIVOT_Y	WORLD_BOX_Y2 - 20	// absolute coords
#define LAUNCHER_BASE_W		50
#define LAUNCHER_BASE_H		10
#define LAUNCHER_BOLT_R		4
//-----------------------------------------------------
// PHYSICS CONSTANTS
//-----------------------------------------------------
#define LAUNCHER_V0			75	// velocity (in module) to shoot every missile
//-----------------------------------------------------

void draw_launcher();
void draw_current_trajectory();
void *rocket_laucher_task(void* arg);

#endif