#include "missiles.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <pthread.h>
#include <allegro.h>

#include "baseUtils.h"
#include "common.h"
// #include "radar_and_trackers.h"

struct missile	missile[MAX_ENEMY_MISSILES];	// missile buffer
struct cbuf		trail[MAX_ENEMY_MISSILES];	// trail buffer

int tflag = 1;	// trail flag
int tl = 30;	// actual trail length

// Store position of element i.
void store_trail(int i) {
	int k;
	if (i >= MAX_ENEMY_MISSILES) return;
	k = trail[i].top;
	k = (k + 1) % TLEN;
	trail[i].x[k] = missile[i].x;
	trail[i].y[k] = missile[i].y;
	trail[i].top = k;
}

// Clear trail stack.
void clear_trail(int i) {
	int j;
	for (j = 0; j < TLEN; j++) {
		trail[i].x[j] = 0;
		trail[i].y[j] = 0;
	}
	trail[i].top=0;
}

// Draws trail of element i, for the length of w.
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

// Makes disappear a missile from graphics.
void missile_vanish(int index) {
	missile[index].destroied = 1;
}

// What to do when a missile explode.
void missile_explode(int index) {
	missile_vanish(index);
	// TODO: add some explosions!
}

// A missile should vanish if outside top, left or right borders, explode if bottom.
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

// Draw missile i in graphic coordinates.
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

// A missile can spawn from top or left side, each one with 1/2 of probability.
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

	// printf("Missile vx: %f\tvy: %f\n", missile[i].vx, missile[i].vy);
}

// Brain of an enemy missile.
void *missile_task(void* arg) {
	int i; // task index
	float dt, dv; //, da_dot;
	i = get_task_index(arg);

	init_missile(i);
	dt = TSCALE * (float)get_task_period(i) / 1000;

	set_period(i);
	while (!sigterm_tasks && !missile[i].destroied) {
		// while (0) {

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
	tp[i].index = -1;
	clear_trail(i);
}