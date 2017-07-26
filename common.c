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
#include "colors.h"
#include "missiles.h"
#include "radar.h"
#include "trackers.h"
#include "rocket_laucher.h"

// Static part of graphics, in which we draw each time.
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

// Draws word box.
void draw_world() {
	rectfill(screen_buff, WORLD_BOX_X1, WORLD_BOX_Y1, WORLD_BOX_X2, WORLD_BOX_Y2, BKG);
	rect(screen_buff, WORLD_BOX_X1, WORLD_BOX_Y1, WORLD_BOX_X2, WORLD_BOX_Y2, BORDER_COL);
	line(screen_buff, XMINT, WORLD_BOX_Y1, XMAXT, WORLD_BOX_Y1, ENEMY_COL); // top missile spawn
	line(screen_buff, WORLD_BOX_X1, SCREEN_W - (WORLD_BOX_Y1 + YMINL),
	     WORLD_BOX_X1, SCREEN_W - (WORLD_BOX_Y1 + YMAXL), ENEMY_COL); // left missile spawn
}

void top_menu() {
	char str[50];

	// border and filling
	rectfill(screen_buff, MENU_BOX_X1, MENU_BOX_Y1, MENU_BOX_X2, MENU_BOX_Y2, BKG);
	rect(screen_buff, MENU_BOX_X1, MENU_BOX_Y1, MENU_BOX_X2, MENU_BOX_Y2, BORDER_COL);

	// TODO: add something, like command explainations

	// warning for max enemy missiles reached
	if (find_free_slot(ENEMY_MISSILES_BASE_INDEX, ENEMY_MISSILES_TOP_INDEX)
	        == -1) {
		sprintf(str, "Max number of enemy missiles reached");
		textout_ex(screen_buff, font, str,
		           MENU_BOX_X1 + 20, MENU_BOX_Y2 - CHAR_HEIGHT - 20, RED, -1);
	}

	// warning for max trackers reached
	if (find_free_slot(TRACKER_BASE_INDEX, TRACKER_TOP_INDEX) == -1) {
		sprintf(str, "Max number of trackers reached");
		textout_ex(screen_buff, font, str,
		           MENU_BOX_X1 + 20, MENU_BOX_Y2 - 2 * CHAR_HEIGHT - 20, RED, -1);
	}
}

void right_menu() {
	int i;
	char str[8];

	// trackers label
	sprintf(str, "Trackers");
	textout_centre_ex(screen_buff, font, str,
	                  TRACK_D0_X + TRACKER_RES * TRACK_DSCALE / 2,
	                  TRACK_D0_Y - TRACKER_RES * TRACK_DSCALE / 2 - 2 * CHAR_HEIGHT, TEXT_COL, -1);

	// tracker empty boxes
	rect(screen_buff, TRACK_D0_X - TRACKER_RES * TRACK_DSCALE / 2, TRACK_D0_Y - TRACKER_RES * TRACK_DSCALE / 2,
	     TRACK_D0_X + TRACKER_RES * TRACK_DSCALE / 2, TRACK_D0_Y + TRACKER_RES * TRACK_DSCALE / 2, BORDER_COL);
	rect(screen_buff, TRACK_D1_X - TRACKER_RES * TRACK_DSCALE / 2, TRACK_D1_Y - TRACKER_RES * TRACK_DSCALE / 2,
	     TRACK_D1_X + TRACKER_RES * TRACK_DSCALE / 2, TRACK_D1_Y + TRACKER_RES * TRACK_DSCALE / 2, BORDER_COL);
	rect(screen_buff, TRACK_D2_X - TRACKER_RES * TRACK_DSCALE / 2, TRACK_D2_Y - TRACKER_RES * TRACK_DSCALE / 2,
	     TRACK_D2_X + TRACKER_RES * TRACK_DSCALE / 2, TRACK_D2_Y + TRACKER_RES * TRACK_DSCALE / 2, BORDER_COL);
	rect(screen_buff, TRACK_D3_X - TRACKER_RES * TRACK_DSCALE / 2, TRACK_D3_Y - TRACKER_RES * TRACK_DSCALE / 2,
	     TRACK_D3_X + TRACKER_RES * TRACK_DSCALE / 2, TRACK_D3_Y + TRACKER_RES * TRACK_DSCALE / 2, BORDER_COL);
}

// Graphical task. Before the main loop, we can compose basic window background,
// then we can draw on it each time, insead of drawing again everything.
void *graphic_task(void* arg) {
	int i, a;
	float delta_t = TSCALE * (float)20 / 1000;

	// initializing screen bitmaps
	screen_base = create_bitmap(SCREEN_W, SCREEN_H);
	clear_bitmap(screen_base);
	clear_to_color(screen_buff, GND);

	draw_world();
	draw_radar_symbol();

	// DBG: dots to test scanner
	/*circlefill(screen_buff, 343, 319, 10, RED);
	circlefill(screen_buff, 338, 404, 10, GREEN);
	circlefill(screen_buff, 253, 319, 10, BORDER_COL);
	circlefill(screen_buff, 190, 404, 10, WHITE);*/

	top_menu();
	right_menu();

	// store this basic screen_buff
	blit(screen_buff, screen_base, 0, 0, 0, 0, screen_buff->w, screen_buff->h);

	// update screen with the content of screen_buff
	blit(screen_buff, screen, 0, 0, 0, 0, screen_buff->w, screen_buff->h);

	// starts radar_task and rocket_laucher_task
	start_task(radar_task, 2, 2, 30, RADAR_INDEX);
	start_task(rocket_laucher_task, 50, 50, 50, ROCKET_LAUCHER_INDEX);

	a = get_task_index(arg);
	set_period(a);
	while (!sigterm_tasks) {

		// recover basic screen_buff
		blit(screen_base, screen_buff, 0, 0, 0, 0, screen_buff->w, screen_buff->h);

		draw_launcher();
		draw_current_trajectory();

		// enemy and Patriot missiles
		for (i = ENEMY_MISSILES_BASE_INDEX; i < PATRIOT_MISSILES_TOP_INDEX; i++) {
			if (tp[i].index != -1) {
				// DBG
				// printf("Drawing missile %d\n", i);
				draw_missile(i);
				if (tflag) draw_trail(i, tl);
			}
		}

		// predictions for enemy missiles
		for (i = 0; i < MAX_TRACKERS; i++)
			if (tp[i].index != -1)
				draw_predictions(i, delta_t / 3);

		draw_radar_display();

		// tracker views on right side of the screen
		for (i = TRACKER_BASE_INDEX; i < TRACKER_TOP_INDEX; i++)
			if (tp[i].index != -1)
				tracker_display(i - TRACKER_BASE_INDEX);

		// TODO: handle deadlines
		/*if (deadline_miss(a))
			show_dmiss(a);*/

		// this will hide a green dot in (0, 0) caused by trails
		putpixel(screen_buff, WORLD_BOX_X1, WORLD_BOX_Y2, BKG);

		// update screen with the content of screen_buff
		blit(screen_buff, screen, 0, 0, 0, 0, screen_buff->w, screen_buff->h);

		wait_for_period(a);
	}
}

// Keyboard interpeter task.
void *interp(void* arg) {
	int a, scan, new_missile_index;
	a = get_task_index(arg);
	set_period(a);
	while (!sigterm_tasks) {
		scan = listen_scancode();
		// if (scan != 0) printf("Readed keyscan: %d\n", (int)scan);
		switch (scan) {
		case 24: // X key - turn on/off trails for enemy missiles
			tflag = !tflag;
			printf("tflag setted to %d\n", tflag);
			break;

		case KEY_SPACE: // spawn a new enemy missile
			new_missile_index = find_free_slot(ENEMY_MISSILES_BASE_INDEX, ENEMY_MISSILES_TOP_INDEX);
			if (new_missile_index != -1)
				start_task(missile_task, MISSILE_PER, MISSILE_DREL, MISSILE_PRI, new_missile_index);
			break;

		case KEY_UP:
			// TODO: something
			break;
		case KEY_DOWN:
			// TODO: something
			break;

		case KEY_LEFT: // reduce trail length
			if (tl > 10) tl--;
			break;
		case KEY_RIGHT: // increment trail length
			if (tl < TLEN) tl++;
			break;

		case KEY_ESC: // close everything
			kill_all_task();
			break;

		default: break;
		}
		wait_for_period(a);
	}
}
