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
	double theta, sec_theta, s_theta, c_theta, t_theta, sqrt_part, x1, x2, t1, t2;

	// go to (45+180)° and stop
	angle_des = 45 + 180;
	theta = angle_des / 180 * PI;
	sec_theta = 1 / cos(theta);
	s_theta = sin(theta);
	c_theta = cos(theta);
	t_theta = tan(theta);

	// going to operative position
	if (abs(angle - angle_des) > 0.001) {
		angle = pole * angle_prev + (1 - pole) * angle_des_prev;
		angle_prev = angle;
		angle_des_prev = angle_des;
	}
	else {
		for (tracker_i = 0; tracker_i < MAX_TRACKERS; tracker_i++) {
			if (tracker_is_active[tracker_i]) {

				// evaluate x positions where the trajectors collide
				sqrt_part = sqrt(pow(tracked_points[tracker_i].vx, 2) *
				                 pow(LAUNCHER_V0, 2) *
				                 (pow(G0, 2) * pow(sec_theta, 2) *
				                  pow(tracked_points[tracker_i].x[tracked_points[tracker_i].top] - abs2world_x(LAUNCHER_PIVOT_X), 2) -
				                  pow(sec_theta, 2) * tracked_points[tracker_i].vx * tracked_points[tracker_i].vy *
				                  (sin(2 * theta) * pow(LAUNCHER_V0, 2) +
				                   2 * G0 * (-tracked_points[tracker_i].x[tracked_points[tracker_i].top] + abs2world_x(LAUNCHER_PIVOT_X))) +
				                  pow(tracked_points[tracker_i].vx, 2) *
				                  (2 * G0 * pow(sec_theta, 2) *
				                   (-tracked_points[tracker_i].y[tracked_points[tracker_i].top] + abs2world_y(LAUNCHER_PIVOT_Y)) +
				                   pow(LAUNCHER_V0, 2) * pow(t_theta, 2)) +
				                  pow(LAUNCHER_V0, 2) *
				                  (pow(tracked_points[tracker_i].vy, 2) +
				                   2 * G0 * (tracked_points[tracker_i].y[tracked_points[tracker_i].top] - abs2world_y(LAUNCHER_PIVOT_Y) +
				                             (-tracked_points[tracker_i].x[tracked_points[tracker_i].top] + abs2world_x(LAUNCHER_PIVOT_X)) * t_theta))));
				x1 = (G0 * pow(sec_theta, 2) * pow(tracked_points[tracker_i].vx, 2) *
				      abs2world_x(LAUNCHER_PIVOT_X) + pow(LAUNCHER_V0, 2) *
				      (-(tracked_points[tracker_i].vx * tracked_points[tracker_i].vy) -
				       G0 * tracked_points[tracker_i].x[tracked_points[tracker_i].top] + pow(tracked_points[tracker_i].vx, 2) * t_theta)
				      - sqrt_part) / (G0 * (pow(sec_theta, 2) * pow(tracked_points[tracker_i].vx, 2) -
				                            pow(LAUNCHER_V0, 2)));
				x2 = (G0 * pow(sec_theta, 2) * pow(tracked_points[tracker_i].vx, 2) *
				      abs2world_x(LAUNCHER_PIVOT_X) + pow(LAUNCHER_V0, 2) *
				      (-(tracked_points[tracker_i].vx * tracked_points[tracker_i].vy) -
				       G0 * tracked_points[tracker_i].x[tracked_points[tracker_i].top] + pow(tracked_points[tracker_i].vx, 2) * t_theta)
				      + sqrt_part) / (G0 * (pow(sec_theta, 2) * pow(tracked_points[tracker_i].vx, 2) -
				                            pow(LAUNCHER_V0, 2)));
				// printf("x1 %f\tx2 %f\n", x1, x2);

				// evaluate values of t where enemy missile will be intercepted
				t1 = (x1 - tracked_points[tracker_i].x[tracked_points[tracker_i].top]) /
				     tracked_points[tracker_i].vx;
				t2 = (x2 - tracked_points[tracker_i].x[tracked_points[tracker_i].top]) /
				     tracked_points[tracker_i].vx;
				// printf("t1 %f\tt2 %f\n", t1, t2);

				// and now search for the solution with t positive
				float t_impact, t_tot, t_wait;
				if (t1 > 0) {
					t_impact = t1;
					t_tot = sec_theta * (x1 - abs2world_x(LAUNCHER_PIVOT_X)) / LAUNCHER_V0;
				}
				else if (t2 > 0) {
					t_impact = t2;
					t_tot = sec_theta * (x2 - abs2world_x(LAUNCHER_PIVOT_X)) / LAUNCHER_V0;
				}
				else {
					printf("No point of interception. We're gonna die.\n");
					return;
				}
				t_wait = t_impact - t_tot;
				printf("Shoot in %f\n", t_wait);
			}
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

		default:
			demo_inertia();
			break;
		}

		wait_for_period(i);
	}
}