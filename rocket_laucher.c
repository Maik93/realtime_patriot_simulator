#include "rocket_laucher.h"

#include <stdio.h>
#include <math.h>
#include <allegro.h>

#include "baseUtils.h"
#include "task_constants.h"
#include "common.h"
#include "colors.h"
#include "trackers.h"
#include "missiles.h"

// TODO: remove unused launcher modes
/*#define LAUNCHER_MODE	2
// 0: demo without inertia, 1: demo with inertia, 2: fixed at 45°

// for demo without inertia
int inc = 1;*/

// TODO: find a good place for angle and angle_prev
// for demo with inertia
float angle = 180;
float angle_prev = 180;
float pole = 0.5;
int angle_des, angle_des_prev;

float shoot_timer; // time to next shoot

void draw_launcher() {
	BITMAP *body;

	// rotations occour on center of bitmap, so I make it double of my rectangle to pivot at a corner
	body = create_bitmap(LAUNCHER_WIDTH * 2, LAUNCHER_HEIGHT * 2);
	clear_to_color(body, TRANSP);
	rect(body, LAUNCHER_WIDTH, LAUNCHER_HEIGHT, LAUNCHER_WIDTH * 2 - 1, LAUNCHER_HEIGHT * 2 - 1, LAUNCHER_COL);
	rotate_sprite(screen_buff, body,
	              LAUNCHER_PIVOT_X - LAUNCHER_WIDTH, LAUNCHER_PIVOT_Y - LAUNCHER_HEIGHT, deg2fix(angle));

	// the base and the bolt
	rect(screen_buff, LAUNCHER_PIVOT_X - LAUNCHER_BASE_W, LAUNCHER_PIVOT_Y,
	     LAUNCHER_PIVOT_X, LAUNCHER_PIVOT_Y + LAUNCHER_BASE_H, LAUNCHER_COL);
	circlefill(screen_buff, LAUNCHER_PIVOT_X, LAUNCHER_PIVOT_Y, LAUNCHER_BOLT_R, BKG);
	circle(screen_buff, LAUNCHER_PIVOT_X, LAUNCHER_PIVOT_Y, LAUNCHER_BOLT_R, LAUNCHER_COL);

	// the hydraulic pump
	line(screen_buff, LAUNCHER_PIVOT_X - LAUNCHER_BASE_W / 3, LAUNCHER_PIVOT_Y,
	     LAUNCHER_PIVOT_X + LAUNCHER_WIDTH / 3 * 2 * cos(angle * PI / 180),
	     LAUNCHER_PIVOT_Y + LAUNCHER_WIDTH / 3 * 2 * sin(angle * PI / 180), LAUNCHER_COL);

	destroy_bitmap(body);
}

// Show actual trajectory of rocket launcher
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
			putpixel(screen_buff, world2abs_x(x), world2abs_y(y), TCOL);
		c--;
	}
}

// Textual infos in the bottom right of the window.
void print_launcher_status() {
	char str[20];
	sprintf(str, "Patriot status");
	textout_centre_ex(screen_buff, font, str, LAUNCHER_TITLE_POSX, LAUNCHER_TITLE_POSY, TEXT_TITL_COL, -1);
	sprintf(str, "-> velocity: %d m/s", LAUNCHER_V0); // TODO: update it with current value
	textout_ex(screen_buff, font, str, LAUNCHER_STAT1_X, LAUNCHER_STAT1_Y, TEXT_COL, -1);
	sprintf(str, "-> angle:    %d°", LAUNCHER_ANGLE_DEG - 180); // TODO: update it with current value
	textout_ex(screen_buff, font, str, LAUNCHER_STAT2_X, LAUNCHER_STAT2_Y, TEXT_COL, -1);
	sprintf(str, "-> shoot in: %.2f s", shoot_timer); // TODO: update it with current value
	textout_ex(screen_buff, font, str, LAUNCHER_STAT3_X, LAUNCHER_STAT3_Y, TEXT_COL, -1);
}

// TODO: remove this
// without inertia
/*void demo_base() {
	angle += inc;
	if (angle <= 180 || angle >= 270) inc *= (-1);
}*/

// TODO: remove this
/*void demo_inertia() {
	angle = pole * angle_prev + (1 - pole) * angle_des_prev;
	if (angle <= 181) angle_des = 270;
	if (angle >= 269) angle_des = 180;
	// printf("angle: %f\tangle_des: %d\tangle_prev:%f\n", angle, angle_des, angle_prev);
	angle_prev = angle;
	angle_des_prev = angle_des;
}*/

// TODO: fix update_shoot_timer
void update_shoot_timer(float suggested_t) {
	if (shoot_timer == 0 && suggested_t > 0)
		shoot_timer = suggested_t;
	else if (shoot_timer > 0 && suggested_t < shoot_timer)
		shoot_timer = suggested_t;
	else// if (shoot_timer < 0)
		shoot_timer = 0;
}

void shoot_now() {
	int new_missile_index;

	new_missile_index = find_free_slot(PATRIOT_MISSILES_BASE_INDEX, PATRIOT_MISSILES_TOP_INDEX);
	// if (new_missile_index != -1)
	if (new_missile_index == PATRIOT_MISSILES_BASE_INDEX) {
		printf("Shoot now!!\n");
		start_task(missile_task, MISSILE_PER, MISSILE_DREL, MISSILE_PRI, new_missile_index);
	}
}

// TODO: change fixed_angle name
void fixed_angle() {
	int tracker_i;
	double theta, sec_theta, s_theta, c_theta, t_theta, sqrt_part, x1, x2, t1, t2;

	// go to (45+180)° and stop
	// angle_des = 45 + 180;
	theta = LAUNCHER_ANGLE_RAD;
	sec_theta = 1 / cos(theta);
	s_theta = sin(theta);
	c_theta = cos(theta);
	t_theta = tan(theta);

	// going to operative position
	if (abs(angle - LAUNCHER_ANGLE_DEG) > 0.001) {
		angle = pole * angle_prev + (1 - pole) * angle_des_prev;
		angle_prev = angle;
		angle_des_prev = LAUNCHER_ANGLE_DEG;
	}

	// evaluate when Patriot have to shoot
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
					printf("No point of interception. We're gonna die. Have a nice day!\n");
					return;
				}
				t_wait = t_impact - t_tot;
				// printf("Shoot in %f\n", t_wait);
				update_shoot_timer(t_wait);

				if (abs(t_wait) < 0.01) {
					shoot_now();
					return;
				}
			}
		}
	}
}

void *rocket_laucher_task(void* arg) {
	int i = get_task_index(arg);
	// float dt = TSCALE * (float)get_task_period(i) / 1000;

	set_period(i);
	while (!sigterm_tasks) {

		fixed_angle();

		// TODO: remove this
		/*switch (LAUNCHER_MODE) {
		case 0: // without inertia
			demo_base();
			break;

		case 1: // with inertia
			demo_inertia();
			break;

		case 2: // fixed at a certain angle
			fixed_angle();
			break;

		default:
			demo_inertia();
			break;
		}*/

		wait_for_period(i);
	}
}