/**
 * -----------------------------------------------------------------------
 * Rocket launcher: Patriot missile generator.
 * Missiles are produced given current launcher angle and initial velocity.
 * -----------------------------------------------------------------------
 */
#include "rocket_launcher.h"

#include <stdio.h>
#include <math.h>
#include <allegro.h>

#include "baseUtils.h"
#include "task_constants.h"
#include "common.h"
#include "colors.h"
#include "trackers.h"
#include "missiles.h"

// Public variables
int launch_velocity;		// start velocity of missiles produced by rocket launcher
int launcher_angle_des;		// desired launcher angle in degree
float launcher_angle_current;// current launcher angle in degree
int show_predictions = PRED_INIT_VAL;
int patriot_guidance = GUIDANCE_INIT_VAL; // allows Patriot to edit velocities of its missile when they're flying

// Private variables
float angle_prev;			// previous launcher angle in degree
float pole;					// pole of the first order schematization of launcher movements
float shoot_timer;			// time to next shoot
struct timespec last_time_shoot;
double stored_x[MAX_TRACKERS]; // x coordinate of interception for each Patriot when launched

// Draw rochet launcher.
void draw_launcher() {
	BITMAP *body; // sprite of rocket launcher's body

	// rotations occour on center of bitmap, so I make it double of my rectangle to pivot at a corner
	body = create_bitmap(LAUNCHER_WIDTH * 2, LAUNCHER_HEIGHT * 2);
	clear_to_color(body, TRANSP);
	rect(body, LAUNCHER_WIDTH, LAUNCHER_HEIGHT, LAUNCHER_WIDTH * 2 - 1, LAUNCHER_HEIGHT * 2 - 1, LAUNCHER_COL);
	rotate_sprite(screen_buff, body,
	              LAUNCHER_PIVOT_X - LAUNCHER_WIDTH, LAUNCHER_PIVOT_Y - LAUNCHER_HEIGHT,
	              deg2fix(launcher_angle_current));

	// the base and the bolt
	rect(screen_buff, LAUNCHER_PIVOT_X - LAUNCHER_BASE_W, LAUNCHER_PIVOT_Y,
	     LAUNCHER_PIVOT_X, LAUNCHER_PIVOT_Y + LAUNCHER_BASE_H, LAUNCHER_COL);
	circlefill(screen_buff, LAUNCHER_PIVOT_X, LAUNCHER_PIVOT_Y, LAUNCHER_BOLT_R, BKG);
	circle(screen_buff, LAUNCHER_PIVOT_X, LAUNCHER_PIVOT_Y, LAUNCHER_BOLT_R, LAUNCHER_COL);

	// the hydraulic pump
	line(screen_buff, LAUNCHER_PIVOT_X - LAUNCHER_BASE_W / 3, LAUNCHER_PIVOT_Y,
	     LAUNCHER_PIVOT_X + LAUNCHER_WIDTH / 3 * 2 * cos(launcher_angle_current * PI / 180),
	     LAUNCHER_PIVOT_Y + LAUNCHER_WIDTH / 3 * 2 * sin(launcher_angle_current * PI / 180), LAUNCHER_COL);

	destroy_bitmap(body);
}

// Show actual trajectory of rocket launcher.
void draw_current_trajectory() {
	float x, y, v, vx, vy, ax, ay, alpha, delta_t;
	int c = 500; // max number of iterations

	alpha = launcher_angle_current * PI / 180;
	x = abs2world_x(LAUNCHER_PIVOT_X);
	y = abs2world_y(LAUNCHER_PIVOT_Y);
	v = launch_velocity;
	vx = v * cos(alpha);
	vy = -v * sin(alpha);
	ay = -G0;
	delta_t = TSCALE * (float)20 / 1000;

	while (x > 0 && y > 0 && c > 0) {
		x += vx * delta_t;
		y += (0.5 * ay * delta_t + vy) * delta_t;
		vy += ay * delta_t;

		if (x > 0 && x < WORLD_BOX_WIDTH && y > 0 && y < WORLD_BOX_HEIGHT)
			putpixel(screen_buff, world2abs_x(x), world2abs_y(y), TCOL);
		c--;
	}
}

// Textual infos in the bottom right of the window.
void print_launcher_status() {
	char str[20]; // for text outputs

	sprintf(str, "Patriot status");
	textout_centre_ex(screen_buff, font, str, LAUNCHER_TITLE_POSX, LAUNCHER_TITLE_POSY, TEXT_TITL_COL, -1);

	if (patriot_guidance)
		sprintf(str, "assisted (D key): on");
	else
		sprintf(str, "assisted (D key): off");
	textout_ex(screen_buff, font, str, LAUNCHER_STAT1_X, LAUNCHER_STAT1_Y, TEXT_COL, -1);

	if (trail_flag)
		sprintf(str, "-> trails:      on");
	else
		sprintf(str, "-> trails:      off");
	textout_ex(screen_buff, font, str, LAUNCHER_STAT2_X, LAUNCHER_STAT2_Y, TEXT_COL, -1);

	if (show_predictions)
		sprintf(str, "-> predictions: on");
	else
		sprintf(str, "-> predictions: off");
	textout_ex(screen_buff, font, str, LAUNCHER_STAT3_X, LAUNCHER_STAT3_Y, TEXT_COL, -1);

	sprintf(str, "-> velocity: %d m/s", launch_velocity);
	textout_ex(screen_buff, font, str, LAUNCHER_STAT4_X, LAUNCHER_STAT4_Y, TEXT_COL, -1);

	sprintf(str, "-> angle:    %d??", launcher_angle_des - 180); // desired angle converted with horizon as baseline
	textout_ex(screen_buff, font, str, LAUNCHER_STAT5_X, LAUNCHER_STAT5_Y, TEXT_COL, -1);

	if (shoot_timer != -1)
		sprintf(str, "-> shoot in: %.2f s", shoot_timer);
	else
		sprintf(str, "-> shoot idle", shoot_timer);
	textout_ex(screen_buff, font, str, LAUNCHER_STAT6_X, LAUNCHER_STAT6_Y, TEXT_COL, -1);
}

// Retrieve smallest time_to_shoot from every tracker.
void update_shoot_timer() {
	int tracker_i;

	shoot_timer = -1;
	for (tracker_i = 0; tracker_i < MAX_TRACKERS; tracker_i++) {
		if (tracker_is_active[tracker_i]) {
			if (shoot_timer == -1 && tracked_points[tracker_i].time_to_shoot > 0)
				shoot_timer = tracked_points[tracker_i].time_to_shoot;
			else if (tracked_points[tracker_i].time_to_shoot < shoot_timer &&
			         tracked_points[tracker_i].time_to_shoot > 0)
				shoot_timer = tracked_points[tracker_i].time_to_shoot;
		}
	}
}

// Move laucher to desired position.
void move_launcher() {
	if (abs(launcher_angle_current - launcher_angle_des) > 0.001) {
		launcher_angle_current = pole * angle_prev + (1 - pole) * launcher_angle_des;
		angle_prev = launcher_angle_current;
	} else
		launcher_angle_current = launcher_angle_des;
}

// Shoot a new Patriot and return its index.
int shoot_now() {
	struct timespec now;
	int new_missile_index;

	// if a missile is tried to launch earlier then LAUNCHER_T_INTERVAL don't do nothing
	clock_gettime(CLOCK_MONOTONIC, &now);
	if (time_diff_ms(now, last_time_shoot) < LAUNCHER_T_INTERVAL)
		return -1;
	// otherwise store actual time as last_time_shoot
	time_copy(&last_time_shoot, now);

	// produce a new Patriot missile, if there's not too much already present
	new_missile_index = find_free_slot(PATRIOT_MISSILES_BASE_INDEX, PATRIOT_MISSILES_TOP_INDEX);
	if (new_missile_index != -1)
		start_task(missile_task, MISSILE_PER, MISSILE_DREL, MISSILE_PRI, new_missile_index);

	return new_missile_index;
}

// Evaluate when Patriot has to shoot.
void shoot_evaluation() {
	int tracker_i;
	double theta, sec_theta, s_theta, c_theta, t_theta, sqrt_part, x1, x2, choosen_x, delta_x, t1, t2;
	float t_impact, t_tot, t_wait;

	theta = -launcher_angle_des / 180.0 * PI;
	sec_theta = 1 / cos(theta);
	t_theta = tan(theta);

	for (tracker_i = 0; tracker_i < MAX_TRACKERS; tracker_i++) {
		if (tracker_is_active[tracker_i] && tracked_points[tracker_i].n_samples > LAUNCHER_MIN_SAMPLE &&
		        tracked_points[tracker_i].traj_error < TRAJ_MAX_ERROR) {

			// evaluate x positions where the trajectories collide
			sqrt_part = pow(tracked_points[tracker_i].vx, 2) * sqrt((G0 *
			            (2 * tracked_points[tracker_i].vx * tracked_points[tracker_i].vy * (tracked_points[tracker_i].x[tracked_points[tracker_i].top] - abs2world_x(LAUNCHER_PIVOT_X)) + G0 * pow(tracked_points[tracker_i].x[tracked_points[tracker_i].top] - abs2world_x(LAUNCHER_PIVOT_X), 2) -
			             2 * pow(tracked_points[tracker_i].vx, 2) * (tracked_points[tracker_i].y[tracked_points[tracker_i].top] - abs2world_y(LAUNCHER_PIVOT_Y))) * pow(sec_theta, 2) +
			            pow(launch_velocity, 2) * (pow(tracked_points[tracker_i].vy, 2) + 2 * G0 * tracked_points[tracker_i].y[tracked_points[tracker_i].top] - 2 * G0 * abs2world_y(LAUNCHER_PIVOT_Y) -
			                                       2 * (tracked_points[tracker_i].vx * tracked_points[tracker_i].vy + G0 * tracked_points[tracker_i].x[tracked_points[tracker_i].top] - G0 * abs2world_x(LAUNCHER_PIVOT_X)) * t_theta +
			                                       pow(tracked_points[tracker_i].vx, 2) * pow(t_theta, 2))) /
			            (pow(tracked_points[tracker_i].vx, 2) * pow(launch_velocity, 2)));

			x1 = (-(G0 * pow(tracked_points[tracker_i].vx, 2) * abs2world_x(LAUNCHER_PIVOT_X) *
			        pow(sec_theta, 2)) + pow(launch_velocity, 2) * (tracked_points[tracker_i].vx * tracked_points[tracker_i].vy + G0 * tracked_points[tracker_i].x[tracked_points[tracker_i].top] - pow(tracked_points[tracker_i].vx, 2) * t_theta
			                + sqrt_part)) / (G0 * (pow(launch_velocity, 2) - pow(tracked_points[tracker_i].vx, 2) * pow(sec_theta, 2)));

			x2 = (-(G0 * pow(tracked_points[tracker_i].vx, 2) * abs2world_x(LAUNCHER_PIVOT_X) *
			        pow(sec_theta, 2)) + pow(launch_velocity, 2) * (tracked_points[tracker_i].vx * tracked_points[tracker_i].vy + G0 * tracked_points[tracker_i].x[tracked_points[tracker_i].top] - pow(tracked_points[tracker_i].vx, 2) * t_theta
			                - sqrt_part)) / (G0 * (pow(launch_velocity, 2) - pow(tracked_points[tracker_i].vx, 2) * pow(sec_theta, 2)));

			// DBG
			// printf("Intercepting coordinates:\tx1 %f\tx2 %f\n", x1, x2);

			// evaluate values of t where enemy missile will be intercepted
			t1 = (x1 - tracked_points[tracker_i].x[tracked_points[tracker_i].top]) /
			     tracked_points[tracker_i].vx;
			t2 = (x2 - tracked_points[tracker_i].x[tracked_points[tracker_i].top]) /
			     tracked_points[tracker_i].vx;
			// DBG
			// printf("Intercepting times:\tt1 %f\tt2 %f\n", t1, t2);

			// and now search for the solution with smaller positive t
			if (t1 <= 0 && t2 <= 0) { // if both are negative, there's no future interception
				// printf("No point of interception. We're gonna die. Have a nice day!\n");
				return;
			}
			if (t1 > 0 && t2 > 0) { // if both positive, we've to take the smaller one
				if (t2 > t1) {
					t_impact = t1;
					t_tot = sec_theta * (x1 - abs2world_x(LAUNCHER_PIVOT_X)) / launch_velocity;
					choosen_x = x1;
				} else {
					t_impact = t2;
					t_tot = sec_theta * (x2 - abs2world_x(LAUNCHER_PIVOT_X)) / launch_velocity;
					choosen_x = x2;
				}
			} else { // otherwise we take the only one positive
				if (t1 > 0) {
					t_impact = t1;
					t_tot = sec_theta * (x1 - abs2world_x(LAUNCHER_PIVOT_X)) / launch_velocity;
					choosen_x = x1;
				} else {
					t_impact = t2;
					t_tot = sec_theta * (x2 - abs2world_x(LAUNCHER_PIVOT_X)) / launch_velocity;
					choosen_x = x2;
				}
			}

			// if there's already a Patriot going to destroy this missile
			if (patriot_guidance && already_shooted[tracker_i] && shooted_missile_id[tracker_i] != -1) {
				// check consistency
				if (shooted_missile_id[tracker_i] < PATRIOT_MISSILES_BASE_INDEX ||
				        shooted_missile_id[tracker_i] >= PATRIOT_MISSILES_TOP_INDEX)
					return;

				delta_x = choosen_x - stored_x[tracker_i];
				if (delta_x > 1) { // working with an offset makes missile variations smoother
					missile[shooted_missile_id[tracker_i]].vy -= GUIDANCE_VX_COEFF * delta_x;
					missile[shooted_missile_id[tracker_i]].vx += GUIDANCE_VY_COEFF * delta_x;
				}
				else if (delta_x < 1) {
					missile[shooted_missile_id[tracker_i]].vy += GUIDANCE_VX_COEFF * delta_x;
					missile[shooted_missile_id[tracker_i]].vx -= GUIDANCE_VY_COEFF * delta_x;
				}
				stored_x[tracker_i] = choosen_x;

			} else { // if there's no Patriot already launched or patriot_guidance disabled
				t_wait = t_impact - t_tot;
				tracked_points[tracker_i].time_to_shoot = t_wait;

				if (abs(t_wait) < LAUNCHER_SHOOT_THRESHOLD) {
					stored_x[tracker_i] = choosen_x;
					shooted_missile_id[tracker_i] = shoot_now();
					already_shooted[tracker_i] = 1;
					return;
				}
			}
		}
	}
}

void *rocket_launcher_task(void* arg) {
	int i = get_task_index(arg);

	launch_velocity = LAUNCHER_V0;

	// initializing launcher angle
	launcher_angle_des = LAUNCHER_ANGLE_DEG;
	launcher_angle_current = LAUNCHER_DEG0;
	pole = LAUNCHER_POLE;
	angle_prev = launcher_angle_current;

	set_period(i);
	while (!sigterm_tasks) {

		move_launcher();
		shoot_evaluation();
		update_shoot_timer();

		wait_for_period(i);
	}
}