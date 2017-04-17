#include "rocket_laucher.h"

#include <stdio.h>
#include <math.h>
#include <allegro.h>

#include "baseUtils.h"
#include "common.h"
#include "trackers.h"

#define LAUNCHER_MODE	2
// 0: demo without inertia, 1: demo with inertia, 2: fixed at 45°

// for demo without inertia
int inc = 1;

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

void draw_current_trajectory() {
	float x, y, v, vx, vy, ax, ay, alpha, delta_t;
	int c = 500; // max number of iterations

	alpha = angle * PI / 180;
	x = abs2world_x(LAUNCHER_PIVOT_X);
	y = abs2world_y(LAUNCHER_PIVOT_Y);
	v = LAUNCHER_V0;
	vx = v * cos(alpha);
	vy = -v * sin(alpha);
	ay = -G0;
	delta_t = TSCALE * (float)20 / 1000;

	while (y > 0 && c > 0) { // add conditions to fit worldbox
		x += vx * delta_t;
		y += (0.5 * ay * delta_t + vy) * delta_t;
		vy += ay * delta_t;

		if (x < WORLD_BOX_WIDTH && y < WORLD_BOX_HEIGHT)
			putpixel(screen_buff, world2abs_x(x), world2abs_y(y), BLU);
		c--;
	}
}

// without inertia
void demo_base() {
	angle += inc;
	if (angle <= 180 || angle >= 270) inc *= (-1);
}

void demo_inertia() {
	angle = pole * angle_prev + (1 - pole) * angle_des_prev;
	if (angle <= 181) angle_des = 270;
	if (angle >= 269) angle_des = 180;
	// printf("angle: %f\tangle_des: %d\tangle_prev:%f\n", angle, angle_des, angle_prev);
	angle_prev = angle;
	angle_des_prev = angle_des;
}

void fixed_angle() {
	int tracker_i;
	float pwr, delta_sqrt, t1, t2;

	// go to (45+180)° and stop
	angle_des = 45 + 180;

	// going to operative position
	if (abs(angle - angle_des) > 0.001) {
		angle = pole * angle_prev + (1 - pole) * angle_des_prev;
		angle_prev = angle;
		angle_des_prev = angle_des;
	}

	for (tracker_i = 0; tracker_i < MAX_TRACKERS; tracker_i++) {
		if (tracker_is_active[tracker_i]) {

			// evaluate time to wait for open fire
			pwr = -2 * sqrt(2) * tracked_points[tracker_i].vx * LAUNCHER_V0 -
			      2 * sqrt(2) * tracked_points[tracker_i].vy * LAUNCHER_V0 -
			      4 * G0 * tracked_points[tracker_i].x[tracked_points[tracker_i].top] +
			      4 * G0 * abs2world_x(LAUNCHER_PIVOT_X);
			delta_sqrt = sqrt(pwr * pwr - 4 * (-2 * G0 * tracked_points[tracker_i].vx +
			                                   sqrt(2) * G0 * LAUNCHER_V0) *
			                  (4 * tracked_points[tracker_i].vy * tracked_points[tracker_i].x[tracked_points[tracker_i].top] -
			                   2 * sqrt(2) * LAUNCHER_V0 * tracked_points[tracker_i].x[tracked_points[tracker_i].top] -
			                   4 * tracked_points[tracker_i].vy * abs2world_x(LAUNCHER_PIVOT_X) +
			                   2 * sqrt(2) * LAUNCHER_V0 * abs2world_x(LAUNCHER_PIVOT_X) -
			                   4 * tracked_points[tracker_i].vx * tracked_points[tracker_i].y[tracked_points[tracker_i].top] -
			                   2 * sqrt(2) * LAUNCHER_V0 * tracked_points[tracker_i].y[tracked_points[tracker_i].top] +
			                   4 * tracked_points[tracker_i].vx * abs2world_y(LAUNCHER_PIVOT_Y) +
			                   2 * sqrt(2) * LAUNCHER_V0 * abs2world_y(LAUNCHER_PIVOT_Y)));
			t1 = (2 * sqrt(2) * tracked_points[tracker_i].vx * LAUNCHER_V0 +
			      2 * sqrt(2) * tracked_points[tracker_i].vy * LAUNCHER_V0 +
			      4 * G0 * tracked_points[tracker_i].x[tracked_points[tracker_i].top] - 4 * G0 * abs2world_x(LAUNCHER_PIVOT_X)
			      - delta_sqrt) /
			     (2.*(-2 * G0 * tracked_points[tracker_i].vx +
			          sqrt(2) * G0 * LAUNCHER_V0));
			t2 = (2 * sqrt(2) * tracked_points[tracker_i].vx * LAUNCHER_V0 +
			      2 * sqrt(2) * tracked_points[tracker_i].vy * LAUNCHER_V0 +
			      4 * G0 * tracked_points[tracker_i].x[tracked_points[tracker_i].top] - 4 * G0 * abs2world_x(LAUNCHER_PIVOT_X)
			      + delta_sqrt) /
			     (2.*(-2 * G0 * tracked_points[tracker_i].vx +
			          sqrt(2) * G0 * LAUNCHER_V0));
			printf("%f\t%f\n", t1, t2);
		}
	}
}

void *rocket_laucher_task(void* arg) {
	int i = get_task_index(arg);
	// float dt = TSCALE * (float)get_task_period(i) / 1000;

	set_period(i);
	while (!sigterm_tasks) {

		switch (LAUNCHER_MODE) {
		case 0: // without inertia
			demo_base();
			break;

		case 1: // with inertia
			demo_inertia();
			break;

		case 2: // fixed at 45°
			fixed_angle();
			break;
		}

		wait_for_period(i);
	}
}