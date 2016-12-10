#ifndef ROCKET_LAUCHER_H
#define ROCKET_LAUCHER_H

#include "common.h"

#define LAUNCHER_WIDTH		75
#define LAUNCHER_HEIGHT		25
#define LAUNCHER_PIVOT_X	WORLD_BOX_X2 - 30
#define LAUNCHER_PIVOT_Y	WORLD_BOX_Y2 - 20
#define LAUNCHER_BASE_W		50
#define LAUNCHER_BASE_H		10
#define LAUNCHER_BOLT_R		4

void draw_launcher();

#endif