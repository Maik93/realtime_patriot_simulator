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
#include "missiles.h"
#include "radar.h"
#include "trackers.h"
#include "rocket_laucher.h"

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
	rect(screen_buff, WORLD_BOX_X1, WORLD_BOX_Y1, WORLD_BOX_X2, WORLD_BOX_Y2, LBLU);
	line(screen_buff, XMINT, WORLD_BOX_Y1, XMAXT, WORLD_BOX_Y1, RED); // top missile spawn
	line(screen_buff, WORLD_BOX_X1, SCREEN_W - (WORLD_BOX_Y1 + YMINL),
	     WORLD_BOX_X1, SCREEN_W - (WORLD_BOX_Y1 + YMAXL), RED); // left missile spawn
}

// Graphical task.
void *graphic_task(void* arg) {
	int i, a;
	char str[50];

	// convert to transparent, then scale down a bitmap file
	/*int n = 10;
	char file_name_in[30], file_name_out[30];
	for (i = 1; i <= n; i++) {
		sprintf(file_name_in, "explosion/%d.bmp", i);
		sprintf(file_name_out, "explosion/%d.bmp", i);
		make_bmp_transp(file_name_in, file_name_out);
		make_bmp_half(file_name_in, file_name_out);
	}*/

	a = get_task_index(arg);
	set_period(a);
	while (!sigterm_tasks) {
		clear_to_color(screen_buff, GND);

		// top menu
		rectfill(screen_buff, MENU_BOX_X1, MENU_BOX_Y1, MENU_BOX_X2, MENU_BOX_Y2, BKG);
		rect(screen_buff, MENU_BOX_X1, MENU_BOX_Y1, MENU_BOX_X2, MENU_BOX_Y2, LBLU);

		draw_world();
		draw_radar_symbol();
		draw_launcher();

		draw_current_trajectory();

		// enemy missiles
		for (i = ENEMY_MISSILES_BASE_INDEX; i < ENEMY_MISSILES_TOP_INDEX; i++) {
			if (tp[i].index != -1) {
				draw_missile(i);
				if (tflag) draw_trail(i, tl);
			}
		}

		// Patriot missiles
		for (i = PATRIOT_MISSILES_BASE_INDEX; i < PATRIOT_MISSILES_TOP_INDEX; i++) {
			if (tp[i].index != -1) {
				// printf("Drawing missile %d\n", i);
				draw_missile(i);
				if (tflag) draw_trail(i, tl);
			}
		}

		draw_radar_display();

		// this will hide a green dot in (0, 0) caused by trails
		putpixel(screen_buff, WORLD_BOX_X1, WORLD_BOX_Y2, BKG);

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

		// tracker views on right side of the screen
		for (i = TRACKER_BASE_INDEX; i < TRACKER_TOP_INDEX; i++)
			if (tp[i].index != -1)
				tracker_display(i - TRACKER_BASE_INDEX);

		// TODO: handle deadlines
		/*if (deadline_miss(a))
			show_dmiss(a);*/

		// dots to test scanner
		// circlefill(screen_buff, 343, 319, 10, RED);
		// circlefill(screen_buff, 338, 404, 10, GREEN);
		// circlefill(screen_buff, 253, 319, 10, LBLU);
		// circlefill(screen_buff, 190, 404, 10, WHITE);

		// next point predicted
		// circle(screen_buff, world2abs_x(pred_x), world2abs_y(pred_y), 5, BLU);

		float delta_t = TSCALE * (float)20 / 1000;
		for (i = 0; i < MAX_TRACKERS; i++)
			if (tp[i].index != -1)
				draw_predictions(i, delta_t / 3);

		// update screen with the content of screen_buff
		blit(screen_buff, screen, 0, 0, 0, 0, screen_buff->w, screen_buff->h);

		// if not already running, starts radar_task and rocket_laucher_task
		if (tp[RADAR_INDEX].index == -1)
			start_task(radar_task, 2, 2, 30, RADAR_INDEX);
		if (tp[ROCKET_LAUCHER_INDEX].index == -1)
			start_task(rocket_laucher_task, 50, 50, 50, ROCKET_LAUCHER_INDEX);

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
