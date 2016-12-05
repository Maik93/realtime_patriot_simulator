#include "common.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <pthread.h>
#include <allegro.h>

#include "baseUtils.h"
#include "missiles.h"
#include "radar_and_trackers.h"

// Draws word box.
void draw_world() {
	rectfill(screen_buff, WORLD_BOX_X1, WORLD_BOX_Y1, WORLD_BOX_X2, WORLD_BOX_Y2, BKG);
	rect(screen_buff, WORLD_BOX_X1, WORLD_BOX_Y1, WORLD_BOX_X2, WORLD_BOX_Y2, LBLU);
	line(screen_buff, XMINT, WORLD_BOX_Y1, XMAXT, WORLD_BOX_Y1, RED); // top missile spawn
	line(screen_buff, WORLD_BOX_X1, SCREEN_W - (WORLD_BOX_Y1 + YMINL),
	     WORLD_BOX_X1, SCREEN_W - (WORLD_BOX_Y1 + YMAXL), RED); // left missile spawn
}

// Draws a nice radar symbol.
void draw_radar_symbol() {
	arc(screen_buff, RPOSX, RPOSY, deg2fix(45), deg2fix(135), 2, BLU);
	arc(screen_buff, RPOSX, RPOSY, deg2fix(45), deg2fix(135), 10, BLU);
	arc(screen_buff, RPOSX, RPOSY, deg2fix(45), deg2fix(135), 20, BLU);
}

// Graphical task.
void *graphic_task(void* arg) {
	int i, a;
	char str[50];

	a = get_task_index(arg);
	set_period(a);
	while (!sigterm_tasks) {
		clear_to_color(screen_buff, GND);

		// top menu
		rectfill(screen_buff, MENU_BOX_X1, MENU_BOX_Y1, MENU_BOX_X2, MENU_BOX_Y2, BKG);
		rect(screen_buff, MENU_BOX_X1, MENU_BOX_Y1, MENU_BOX_X2, MENU_BOX_Y2, LBLU);

		draw_world();
		draw_radar_symbol();

		// enemy missiles
		for (i = 0; i < MAX_ENEMY_MISSILES; i++) {
			if (!missile[i].destroied && tp[i].index != -1) {
				draw_missile(i);
				if (tflag) draw_trail(i, tl);
			}
		}

		draw_radar_display();

		// this will hide a green dot in (0, 0) caused by trails
		putpixel(screen_buff, WORLD_BOX_X1, WORLD_BOX_Y2, BKG);

		// check how many enemy missiles
		if (find_free_slot(ENEMY_MISSILES_BASE_INDEX, ENEMY_MISSILES_TOP_INDEX)
		        == -1) {
			sprintf(str, "Max number of enemy missiles reached");
			textout_ex(screen_buff, font, str,
			           MENU_BOX_X1 + 20, MENU_BOX_Y2 - CHAR_HEIGHT - 20, RED, -1);
		}

		// check how many trackers
		if (find_free_slot(TRACKER_BASE_INDEX, TRACKER_TOP_INDEX) == -1) {
			sprintf(str, "Max number of trackers reached");
			textout_ex(screen_buff, font, str,
			           MENU_BOX_X1 + 20, MENU_BOX_Y2 - 2 * CHAR_HEIGHT - 20, RED, -1);
		}

		// tracker views on right side of the screen
		for (i = TRACKER_BASE_INDEX; i < TRACKER_TOP_INDEX; i++)
			if (tp[i].index != -1)
				tracker_display(i - TRACKER_BASE_INDEX);

		/*if (deadline_miss(a))
			show_dmiss(a);*/

		blit(screen_buff, screen, 0, 0, 0, 0, screen_buff->w, screen_buff->h);

		// if not already running, starts radar_task
		if (tp[MAX_THREADS - 2].index == -1)
			start_task(radar_task, 2, 2, 30, MAX_THREADS - 2);

		wait_for_period(a);
	}
}

// Keyboard interpeter task.
void *interp(void* arg) {
	int a, scan, new_i;
	a = get_task_index(arg);
	set_period(a);
	while (!sigterm_tasks) {
		scan = listen_scancode();
		// if (scan != 0) printf("Readed keyscan: %d\n", (int)scan);
		switch (scan) {
		/*case 3: // C key
			for (int j = 0; j < active_missiles; ++j)
				printf("Logging missile %d: x=%f y=%f v=%d va=%f.\n",
				       j, missile[j].x, missile[j].y, (int)missile[j].v, missile[j].alpha);
			break;*/
		case 24: // X key
			tflag = !tflag;
			printf("tflag setted to %d\n", tflag);
			break;

		case KEY_SPACE:
			new_i = find_free_slot(ENEMY_MISSILES_BASE_INDEX, ENEMY_MISSILES_TOP_INDEX);
			if (new_i != -1)
				start_task(missile_task, PER, DREL, PRI, new_i);
			break;

		case KEY_UP:
			// TODO
			break;
		case KEY_DOWN:
			// TODO
			break;

		case KEY_LEFT:
			if (tl > 10) tl--;
			break;
		case KEY_RIGHT:
			if (tl < TLEN) tl++;
			break;

		case KEY_ESC:
			kill_all_task();
			break;

		default: break;
		}
		wait_for_period(a);
	}
}
