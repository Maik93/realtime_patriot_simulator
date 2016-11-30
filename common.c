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

void draw_world() {
	rectfill(screen_buff, WORLD_BOX_X1, WORLD_BOX_Y1, WORLD_BOX_X2, WORLD_BOX_Y2, BKG);
	line(screen_buff, XMINT, WORLD_BOX_Y1, XMAXT, WORLD_BOX_Y1, RED); // top missile spawn
	line(screen_buff, WORLD_BOX_X1, SCREEN_W - (WORLD_BOX_Y1 + YMINL),
	     WORLD_BOX_X1, SCREEN_W - (WORLD_BOX_Y1 + YMAXL), RED); // left missile spawn
}

void *display(void* arg) {
	int i, a;
	char str[20];

	a = get_task_index(arg);
	set_period(a);
	while (!sigterm_tasks) {
		clear_to_color(screen_buff, GND);

		// menu
		rectfill(screen_buff, MENU_BOX_X1, MENU_BOX_Y1, MENU_BOX_X2, MENU_BOX_Y2, BKG);

		draw_world();

		putpixel(screen_buff, RPOSX, RPOSY, BLU);
		arc(screen_buff, RPOSX, RPOSY, deg2fix(45), deg2fix(135), 10, BLU);
		arc(screen_buff, RPOSX, RPOSY, deg2fix(45), deg2fix(135), 20, BLU);

		// radar
		arc(screen_buff,
		    WORLD_BOX_X2 + 108, WORLD_BOX_Y2, deg2fix(45), deg2fix(160), 110, WHITE);

		for (i = 0; i < MAX_TASKS; i++) {
			if (!missile[i].destroied && tp[i].index != -1) {
				draw_missile(i);
				if (tflag) draw_trail(i, tl);
			}
		}

		// this will hide a green dot in (0, 0) caused by trails
		putpixel(screen_buff, WORLD_BOX_X1, WORLD_BOX_Y2, BKG);

		// errore di segmentazione:
		/*if (find_free_slot() == -1) {
			sprintf(str, "Max number of task reached");
			textout_ex(screen_buff, font, str,
			           MENU_BOX_X1 + 20, MENU_BOX_Y2 - CHAR_HEIGHT - 20, RED, -1);
		}*/
		/*if (deadline_miss(a))
			show_dmiss(a);*/

		blit(screen_buff, screen, 0, 0, 0, 0, screen_buff->w, screen_buff->h);

		wait_for_period(a);
	}
}

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
			new_i = find_free_slot();
			if (new_i != -1)
				start_task(missiletask, PER, DREL, PRI, new_i);
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