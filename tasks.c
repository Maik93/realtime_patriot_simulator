#include "tasks.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <pthread.h>
#include <allegro.h>

#include "baseUtils.h"

struct missile	missile[MAX_TASKS];	// missile buffer
struct cbuf		trail[MAX_TASKS];	// trail buffer

int naf = 0;	// number of active flies
int tflag = 1;	// trail flag
int tl = 15;	// actual trail length
// float g = G0;	// acceleration of gravity

/**
 * Store position of element i.
 */
void store_trail(int i) {
	int k;
	if (i >= MAX_TASKS) return;
	k = trail[i].top;
	k = (k + 1) % TLEN;
	trail[i].x[k] = missile[i].x;
	trail[i].y[k] = missile[i].y;
	trail[i].top = k;
}

/**
 * Draws trail of element i, for the length of w.
 */
void draw_trail(int i, int w) {
	int j, k;
	int x, y;
	for (j = 0; j < w; j++) {
		k = (trail[i].top + TLEN - j) % TLEN;
		x = WORLD_BOX_X1 + trail[i].x[k];
		y = WORLD_BOX_Y2 - trail[i].y[k];
		putpixel(screen_buff, x, y, TCOL);
	}
}

void missile_vanish(int index) {
	missile[index].destroied = 1;
}

void missile_explode(int index) {
	missile_vanish(index);
}

void handle_corners(int i) {
	int left, right, top, bottom;

	left = (missile[i].x < missile[i].r);
	right = (missile[i].x > WORLD_BOX_WIDTH - missile[i].r);
	top = (missile[i].y > WORLD_BOX_HEIGHT - missile[i].r);
	bottom = (missile[i].y <= missile[i].r);
	// printf("%d %d %d %d.\n", left, right, top, bottom);

	if (left || right) missile_vanish(i);
	if (bottom) missile_explode(i);
	if (top) missile[i].y = WORLD_BOX_HEIGHT - missile[i].r;
}

/**
 * A missile can spawn from top or left side, each one with 1/2 of probability.
 */
void init_missile(int i) {
	float r, v;

	r = frand(0, 2);
	if (r < 1) { // left side
		missile[i].x = ML;
		missile[i].y = frand(YMINL, YMAXL);
		missile[i].alpha = frand(AMINL, AMAXL);
		v = frand(VMINL, VMAXL);
	}
	else { // top side
		missile[i].x = frand(XMINT, XMAXT);
		missile[i].y = WORLD_BOX_HEIGHT - ML;
		missile[i].alpha = frand(AMINT, AMAXT);
		v = frand(VMINT, VMAXT);
	}

	missile[i].vx = v * cos(missile[i].alpha);
	missile[i].vy = v * sin(missile[i].alpha);
	missile[i].destroied = 0;
	missile[i].c = RED;
	missile[i].r = ML;
}

void *missiletask(void* arg) {
	int i; // task index
	float dt, dv; //, da_dot;
	i = get_task_index(arg);

	init_missile(i);
	dt = TSCALE * (float)get_task_period(i) / 1000;

	set_period(i);
	while (!sigterm_tasks && !missile[i].destroied) {
		// while (0) {
		// da_dot = frand(-DA_DOT, DA_DOT);

		missile[i].vy -= G0 * dt;
		missile[i].y += missile[i].vy * dt - G0 * dt * dt / 2;
		missile[i].x += missile[i].vx * dt;
		missile[i].alpha = atan2(missile[i].vy, missile[i].vx);

		// old, without gravity
		/*int vx, vy;
		missile[i].alpha_dot += da_dot;
		missile[i].alpha += missile[i].alpha_dot * dt;
		vx = missile[i].v * cos(missile[i].alpha);
		vy = missile[i].v * sin(missile[i].alpha);
		missile[i].x += vx * dt;
		missile[i].y += vy * dt;*/

		handle_corners(i);
		store_trail(i);

		wait_for_period(i);
	}
}

/**
 * Draw missile i in graphic coordinates.
 * @param i [description]
 */
void draw_missile(int i) {
	float p1x, p1y, p2x, p2y, p3x, p3y; // world coord.
	float ca, sa;

	ca = cos(missile[i].alpha);
	sa = sin(missile[i].alpha);

	p1x = missile[i].x + ML * ca; // nose point
	p1y = missile[i].y + ML * sa;
	p2x = missile[i].x - MW * sa; // left wing
	p2y = missile[i].y + MW * ca;
	p3x = missile[i].x + MW * sa; // right wing
	p3y = missile[i].y - MW * ca;

	triangle(screen_buff,
	         WORLD_BOX_X1 + p1x, WORLD_BOX_Y2 - p1y, // nose point
	         WORLD_BOX_X1 + p2x, WORLD_BOX_Y2 - p2y, // left wing
	         WORLD_BOX_X1 + p3x, WORLD_BOX_Y2 - p3y, // right wing
	         missile[i].c);
}

void *display(void* arg) {
	int i, a;
	char str[20];

	a = get_task_index(arg);
	set_period(a);
	while (!sigterm_tasks) {
		clear_to_color(screen_buff, GND);
		rectfill(screen_buff, MENU_BOX_X1, MENU_BOX_Y1, MENU_BOX_X2, MENU_BOX_Y2, BKG);

		// world margins
		rectfill(screen_buff, WORLD_BOX_X1, WORLD_BOX_Y1, WORLD_BOX_X2, WORLD_BOX_Y2, BKG);
		rect(screen_buff, WORLD_BOX_X1, WORLD_BOX_Y1, WORLD_BOX_X2, WORLD_BOX_Y2, GREEN);
		line(screen_buff, XMINT, WORLD_BOX_Y1, XMAXT, WORLD_BOX_Y1, RED); // top missile spawn
		// questo disegna a caso
		line(screen_buff, WORLD_BOX_X1, WORLD_BOX_Y2 - YMINL,
		     WORLD_BOX_X1, WORLD_BOX_Y2 - YMAXL, RED); // left missile spawn

		/*arc(screen_buff,
		    WORLD_BOX_X2 + 95, WORLD_BOX_Y2, itofix(45 * 256 / 360), itofix(160 * 256 / 360), 50, BKG);*/

		for (i = 0; i < naf; i++) {
			if (!missile[i].destroied) {
				draw_missile(i);
				if (tflag) draw_trail(i, tl);
			}
		}

		if (naf == MAX_TASKS) {
			sprintf(str, "MAX_TASKS reached");
			textout_ex(screen_buff, font, str, WIN_WIDTH - 17 * CHAR_WIDTH - 20, WIN_HEIGHT - 20, RED, -1);
		}
		/*if (deadline_miss(a))
			show_dmiss(a);*/

		blit(screen_buff, screen, 0, 0, 0, 0, screen_buff->w, screen_buff->h);

		wait_for_period(a);
	}
}

void *interp(void* arg) {
	int a, scan;
	a = get_task_index(arg);
	set_period(a);
	while (!sigterm_tasks) {
		scan = listen_scancode();
		if (scan != 0) printf("Readed keyscan: %d\n", (int)scan);
		switch (scan) {
		/*case 3: // C key
			for (int j = 0; j < naf; ++j)
				printf("Logging missile %d: x=%f y=%f v=%d va=%f.\n",
				       j, missile[j].x, missile[j].y, (int)missile[j].v, missile[j].alpha);
			break;*/
		case 24: // X key
			tflag = !tflag;
			printf("tflag setted to %d\n", tflag);
			break;

		case KEY_SPACE:
			if (naf < MAX_TASKS) {
				start_task(missiletask, PER, DREL, PRI, naf++);
				// printf("Adding missile. Now they are %d.\n", naf);
			}
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