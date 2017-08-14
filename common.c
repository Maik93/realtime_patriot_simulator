/**
 * -----------------------------------------------------------------------
 * Functions related to graphics and keyboard controls.
 * -----------------------------------------------------------------------
 */
#include "common.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <pthread.h>
#include <allegro.h>

#include "baseUtils.h"
#include "task_constants.h"
#include "colors.h"
#include "missiles.h"
#include "radar.h"
#include "trackers.h"
#include "rocket_launcher.h"

// DBG
int ball_x, ball_y, ball_r = 10, ball_active = 0;

// Static part of graphics, a basic scenario in which we draw each time.
BITMAP	*screen_base;

int world2abs_x(int x) {
	return WORLD_BOX_X1 + x;
}

int world2abs_y(int y) {
	return WORLD_BOX_Y2 - y;
}

int abs2world_x(int x) {
	return x - WORLD_BOX_X1;
}

int abs2world_y(int y) {
	return WORLD_BOX_Y2 - y;
}

// Draw word box. This function is called only one time.
void draw_world() {
	rectfill(screen_buff, WORLD_BOX_X1, WORLD_BOX_Y1, WORLD_BOX_X2, WORLD_BOX_Y2, BKG);
	rect(screen_buff, WORLD_BOX_X1, WORLD_BOX_Y1, WORLD_BOX_X2, WORLD_BOX_Y2, BORDER_COL);

	line(screen_buff, world2abs_x(XMINT), WORLD_BOX_Y1,
	     world2abs_x(XMAXT), WORLD_BOX_Y1, ENEMY_COL); // top missile spawn
	line(screen_buff, WORLD_BOX_X1, world2abs_y(YMINL),
	     WORLD_BOX_X1, world2abs_y(YMAXL), ENEMY_COL); // left missile spawn
}

// Draw static part of top menu. This function is called only one time.
void top_menu_static() {
	char str[50];

	// border
	rect(screen_buff, MENU_BOX_X1, MENU_BOX_Y1, MENU_BOX_X2, MENU_BOX_Y2, BORDER_COL);

	// list of commands
	sprintf(str, "List of avaiable commands");
	textout_ex(screen_buff, font, str, MENU_TITLE_X, MENU_TITLE_Y, TEXT_TITL_COL, -1);
	sprintf(str, "space:      spawn new enemy missile");
	textout_ex(screen_buff, font, str, MENU_CONTENT1_X, MENU_CONTENT1_Y, TEXT_COL, -1);
	sprintf(str, "enter:      spawn new distubing object");
	textout_ex(screen_buff, font, str, MENU_CONTENT2_X, MENU_CONTENT2_Y, TEXT_COL, -1);
	sprintf(str, "X:          toggle enemy missile trails");
	textout_ex(screen_buff, font, str, MENU_CONTENT3_X, MENU_CONTENT3_Y, TEXT_COL, -1);
	sprintf(str, "Z-C:        dec./inc. enemy missile trails");
	textout_ex(screen_buff, font, str, MENU_CONTENT4_X, MENU_CONTENT4_Y, TEXT_COL, -1);
	sprintf(str, "left-right: dec./inc. Patriot shoot velocity");
	textout_ex(screen_buff, font, str, MENU_CONTENT5_X, MENU_CONTENT5_Y, TEXT_COL, -1);
	sprintf(str, "up-down:    inc./dec. Patriot shoot angle");
	textout_ex(screen_buff, font, str, MENU_CONTENT6_X, MENU_CONTENT6_Y, TEXT_COL, -1);
	sprintf(str, "Esc:        close program");
	textout_ex(screen_buff, font, str, MENU_CONTENT7_X, MENU_CONTENT7_Y, TEXT_COL, -1);
}

// Draw dynamic part of top menu. This function is called in every graphic_task cicle.
void top_menu_dynamic() {
	char str[50];

	// warning for max enemy missiles reached
	if (find_free_slot(ENEMY_MISSILES_BASE_INDEX, ENEMY_MISSILES_TOP_INDEX)
	        == -1) {
		sprintf(str, "Max number of enemy missiles reached");
		textout_ex(screen_buff, font, str, MENU_ERROR1_X, MENU_ERROR1_Y, RED, -1);
	}

	// warning for max trackers reached
	if (find_free_slot(TRACKER_BASE_INDEX, TRACKER_TOP_INDEX) == -1) {
		sprintf(str, "Max number of trackers reached");
		textout_ex(screen_buff, font, str, MENU_ERROR2_X, MENU_ERROR2_Y, RED, -1);
	}
}

// Draw tracker boxes and its title on right side. This function is called only one time.
void right_menu() {
	int i;
	char str[8];

	// trackers label
	sprintf(str, "Trackers");
	textout_centre_ex(screen_buff, font, str,
	                  TRACK_D0_X + TRACKER_RES * TRACK_DSCALE / 2,
	                  TRACK_D0_Y - TRACKER_RES * TRACK_DSCALE / 2 - 2 * CHAR_HEIGHT, TEXT_TITL_COL, -1);

	// tracker empty boxes
	rect(screen_buff, TRACK_BOX1_X0, TRACK_BOX1_Y0, TRACK_BOX1_X1, TRACK_BOX1_Y1, BORDER_COL);
	rect(screen_buff, TRACK_BOX2_X0, TRACK_BOX2_Y0, TRACK_BOX2_X1, TRACK_BOX2_Y1, BORDER_COL);
	rect(screen_buff, TRACK_BOX3_X0, TRACK_BOX3_Y0, TRACK_BOX3_X1, TRACK_BOX3_Y1, BORDER_COL);
	rect(screen_buff, TRACK_BOX4_X0, TRACK_BOX4_Y0, TRACK_BOX4_X1, TRACK_BOX4_Y1, BORDER_COL);
}

// Graphical task. Before the main loop, we can compose a basic scenario,
// then we can draw on it each time, insead of drawing again everything.
void *graphic_task(void* arg) {
	int i, a, tracker_i;
	float delta_t = TSCALE * (float)GRAPH_PER / 1000;

	// initializing screen bitmaps
	screen_base = create_bitmap(SCREEN_W, SCREEN_H);
	clear_bitmap(screen_base);
	clear_to_color(screen_buff, GND);

	draw_world();
	draw_radar_symbol();

	// DBG: dots for test
	/*circlefill(screen_buff, 343, 319, 10, RED);
	circlefill(screen_buff, 338, 404, 10, GREEN);
	circlefill(screen_buff, 253, 319, 10, BORDER_COL);
	circlefill(screen_buff, 190, 404, 10, WHITE);*/

	top_menu_static();
	right_menu();

	// store this basic scenario
	blit(screen_buff, screen_base, 0, 0, 0, 0, screen_buff->w, screen_buff->h);

	// starts radar_task and rocket_launcher_task.
	// It's done only now because they need a basic scenario to analize.
	start_task(radar_task, RADAR_PER, RADAR_DREL, RADAR_PRI, RADAR_INDEX);
	start_task(rocket_launcher_task, LAUNCHER_PER, LAUNCHER_DREL, LAUNCHER_PRI, ROCKET_LAUNCHER_INDEX);

	a = get_task_index(arg);
	set_period(a);
	while (!sigterm_tasks) {

		// recover basic scenario in screen_buff
		blit(screen_base, screen_buff, 0, 0, 0, 0, screen_buff->w, screen_buff->h);

		top_menu_dynamic();

		draw_radar_display();

		// draw rocket launcher and its trajectory
		draw_launcher();
		draw_current_trajectory();
		print_launcher_status();

		// enemy and Patriot missiles
		for (i = ENEMY_MISSILES_BASE_INDEX; i < PATRIOT_MISSILES_TOP_INDEX; i++) {
			if (tp[i].index != -1) {
				draw_missile(i);
				if (tflag) draw_trail(i, tl);
			}
		}

		// DBG: ball
		if (ball_active)
			circlefill(screen_buff, ball_x, ball_y, ball_r, ENEMY_COL);

		for (i = TRACKER_BASE_INDEX; i < TRACKER_TOP_INDEX; i++)
			if (tp[i].index != -1) {
				tracker_i = i - TRACKER_BASE_INDEX;

				// predicted trajectories for enemy missiles
				draw_predictions(tracker_i, delta_t / 3);

				// tracker views on right side of the screen
				tracker_display(tracker_i);
			}

		// this will hide a green dot in (0, 0) caused by trails
		putpixel(screen_buff, WORLD_BOX_X1, WORLD_BOX_Y2, BKG);

		// update screen with the content of screen_buff
		blit(screen_buff, screen, 0, 0, 0, 0, screen_buff->w, screen_buff->h);

		wait_for_period(a);
	}
}

// DBG
void *ball_task(void* arg) {
	int ball_vx, ball_vy, i;
	float dt;

	ball_active = 1;

	ball_x = WORLD_BOX_X1 + ball_r + 1;
	ball_y = WORLD_BOX_Y1 + 250;
	ball_vx = 50;
	ball_vy = -15;

	i = get_task_index(arg);
	dt = TSCALE * (float)get_task_period(i) / 1000;

	set_period(i);
	while (!sigterm_tasks &&
	        ball_x > WORLD_BOX_X1 + ball_r && ball_x < WORLD_BOX_X2 - ball_r &&
	        ball_y > WORLD_BOX_Y1 + ball_r && ball_y < WORLD_BOX_Y2 - ball_r) {
		ball_x += ball_vx * dt;
		ball_y += ball_vy * dt;
		wait_for_period(i);
	}

	ball_active = 0;
}

// Keyboard interpeter task.
void *interp_task(void* arg) {
	int a, scan, new_missile_index;
	a = get_task_index(arg);

	set_period(a);
	while (!sigterm_tasks) {
		scan = listen_scancode();
		// if (scan != 0) printf("Readed keyscan: %d\n", (int)scan);

		switch (scan) {
		case 67: // Enter key
			if (!ball_active)
				start_task(ball_task, MISSILE_PER, MISSILE_DREL, MISSILE_PRI, DISTURB_OBJECT_INDEX);
			break;
		case 24: // X key - turn on/off trails for enemy missiles
			tflag = !tflag;
			printf("tflag setted to %d\n", tflag);
			break;

		case 26: // Z key - reduce trail length
			if (tl > 10) tl--;
			break;
		case 3: // C key - increment trail length
			if (tl < TLEN) tl++;
			break;

		case KEY_SPACE: // spawn a new enemy missile
			new_missile_index = find_free_slot(ENEMY_MISSILES_BASE_INDEX, ENEMY_MISSILES_TOP_INDEX);
			if (new_missile_index != -1)
				start_task(missile_task, MISSILE_PER, MISSILE_DREL, MISSILE_PRI, new_missile_index);
			break;

		case KEY_UP: // reduce patriot missile initial velocities
			if (launch_velocity < LAUNCHER_VMAX)
				launch_velocity++;
			break;
		case KEY_DOWN: // increment patriot missile initial velocities
			if (launch_velocity > LAUNCHER_VMIN)
				launch_velocity--;
			break;

		case KEY_LEFT: // reduce patriot angle
			if (launcher_angle_des > LAUNCHER_ANGLE_MIN)
				launcher_angle_des--;
			break;
		case KEY_RIGHT: // increment patriot angle
			if (launcher_angle_des < LAUNCHER_ANGLE_MAX)
				launcher_angle_des++;
			break;

		case KEY_ESC: // close everything
			kill_all_task();
			break;

		default: break;
		}
		wait_for_period(a);
	}
}
