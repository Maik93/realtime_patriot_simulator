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
float g = G0;	// acceleration of gravity

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
		x = XBOX + 1 + trail[i].x[k];
		y = WIN_HEIGHT - FLEV - trail[i].y[k];
		putpixel(screen_buff, x, y, TCOL);
	}
}

void handle_bounce(int i) {
	int left, right, top, bottom;

	left = (missile[i].x <= XBOX + missile[i].r);
	right = (missile[i].x >= RBOX - missile[i].r);
	top = (missile[i].y <= YBOX + missile[i].r);
	bottom = (missile[i].y >= BBOX - missile[i].r);
	// printf("%d %d %d %d.\n", left, right, top, bottom);

	if (left) missile[i].x = XBOX + missile[i].r;
	if (right) missile[i].x = RBOX - missile[i].r;
	if (top) missile[i].y = YBOX + missile[i].r;
	if (bottom) missile[i].y = BBOX - missile[i].r;

	if (left || right) missile[i].alpha = PI - missile[i].alpha;
	if (top || bottom) missile[i].alpha = - missile[i].alpha;
}

void init_missile(int i) {
	missile[i].c = 2 + i % 14; // color in [2,15]
	missile[i].r = FL;
	missile[i].x = frand(15, RBOX - 15);
	missile[i].y = frand(15, BBOX - 15);
	missile[i].v = frand(VMIN, VMAX);
	missile[i].alpha = frand(0, 2 * PI);
	/*printf("Init missile %d: x=%f y=%f v=%d a=%f.\n",
	       i, missile[i].x, missile[i].y, (int)missile[i].v, missile[i].a);*/
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

	p1x = missile[i].x + FL * ca; // nose point
	p1y = missile[i].y + FL * sa;
	p2x = missile[i].x - FW * sa; // left wing
	p2y = missile[i].y + FW * ca;
	p3x = missile[i].x + FW * sa; // right wing
	p3y = missile[i].y - FW * ca;

	triangle(screen_buff,
	         p1x + XCEN, WIN_HEIGHT - YCEN - p1y, // nose point
	         p2x + XCEN, WIN_HEIGHT - YCEN - p2y, // left wing
	         p3x + XCEN, WIN_HEIGHT - YCEN - p3y, // right wing
	         missile[i].c);
}

void *missiletask(void* arg) {
	int i; // task index
	float dt, da, dv;
	i = get_task_index(arg);

	init_missile(i);
	dt = TSCALE * (float)get_task_period(i) / 1000;

	set_period(i);
	while (!sigterm_tasks) {
		da = frand(-DELTA_A, DELTA_A);
		// dv = frand(-DELTA_V, DELTA_V);

		missile[i].alpha += da;
		missile[i].v += dv;
		missile[i].x += missile[i].v * cos(missile[i].alpha) * dt;
		missile[i].y += missile[i].v * sin(missile[i].alpha) * dt;

		handle_bounce(i);
		store_trail(i);

		wait_for_period(i);
	}
}

void *display(void* arg) {
	int i, a;
	char str[20];

	a = get_task_index(arg);
	set_period(a);
	while (!sigterm_tasks) {
		clear_to_color(screen_buff, GND);
		rectfill(screen_buff, XBOX + 1, YBOX + 1, RBOX - 1, BBOX - 1, BKG);

		for (i = 0; i < naf; i++) {
			draw_missile(i);
			if (tflag) draw_trail(i, tl);
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
		case 3: // C key
			for (int j = 0; j < naf; ++j)
				printf("Logging missile %d: x=%f y=%f v=%d va=%f.\n",
				       j, missile[j].x, missile[j].y, (int)missile[j].v, missile[j].alpha);
			break;
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