#include "rocket_laucher.h"

#include <stdio.h>
#include <math.h>
#include <allegro.h>

#include "baseUtils.h"
#include "common.h"

// for demo without inertia
// int inc = 1;

// for demo with inertia
float angle = 180;
float angle_prev = 180;
float pole = 0.5;
int angle_des, angle_des_prev;

void draw_launcher() {
	BITMAP *body;

	// rotations occour on center of bitmap, so I make it double of my rectangle to pivot at a corner
	body = create_bitmap(LAUNCHER_WIDTH * 2, LAUNCHER_HEIGHT * 2);
	clear_to_color(body, TRANSP);
	rect(body, LAUNCHER_WIDTH, LAUNCHER_HEIGHT, LAUNCHER_WIDTH * 2 - 1, LAUNCHER_HEIGHT * 2 - 1, BLU);
	rotate_sprite(screen_buff, body,
	              LAUNCHER_PIVOT_X - LAUNCHER_WIDTH, LAUNCHER_PIVOT_Y - LAUNCHER_HEIGHT, deg2fix(angle));

	// the base and the bolt
	rect(screen_buff, LAUNCHER_PIVOT_X - LAUNCHER_BASE_W, LAUNCHER_PIVOT_Y,
	     LAUNCHER_PIVOT_X, LAUNCHER_PIVOT_Y + LAUNCHER_BASE_H, BLU);
	circlefill(screen_buff, LAUNCHER_PIVOT_X, LAUNCHER_PIVOT_Y, LAUNCHER_BOLT_R, BKG);
	circle(screen_buff, LAUNCHER_PIVOT_X, LAUNCHER_PIVOT_Y, LAUNCHER_BOLT_R, BLU);

	// the hydraulic pump
	line(screen_buff, LAUNCHER_PIVOT_X - LAUNCHER_BASE_W / 3, LAUNCHER_PIVOT_Y,
	     LAUNCHER_PIVOT_X + LAUNCHER_WIDTH / 3 * 2 * cos(angle * PI / 180),
	     LAUNCHER_PIVOT_Y + LAUNCHER_WIDTH / 3 * 2 * sin(angle * PI / 180), BLU);

	destroy_bitmap(body);
}

void *rocket_laucher_task(void* arg) {
	int i = get_task_index(arg);
	// float dt = TSCALE * (float)get_task_period(i) / 1000;

	set_period(i);
	while (!sigterm_tasks) {
		// without inertia
		// angle += inc;
		// if (angle <= 180 || angle >= 270) inc *= (-1);

		// with inertia
		angle = pole * angle_prev + (1 - pole) * angle_des_prev;
		if (angle <= 181) angle_des = 270;
		if (angle >= 269) angle_des = 180;
		// printf("angle: %f\tangle_des: %d\tangle_prev:%f\n", angle, angle_des, angle_prev);
		angle_prev = angle;
		angle_des_prev = angle_des;

		wait_for_period(i);
	}
}