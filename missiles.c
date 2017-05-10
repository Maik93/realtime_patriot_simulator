/**
 * -----------------------------------------------------------------------
 * Enemy missiles. Casually spowned from top or left corner, they fall
 * cause of gravity until reached the ground or destroied by Patriot.
 * -----------------------------------------------------------------------
 */
#include "missiles.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <pthread.h>
#include <allegro.h>

#include "baseUtils.h"
#include "common.h"
#include "rocket_laucher.h" // TODO: correct in rocket_launcher!!
// #include "radar_and_trackers.h"

// DBG
// float vx[MAX_ENEMY_MISSILES], vy[MAX_ENEMY_MISSILES];

struct missile	missile[MAX_ENEMY_MISSILES];	// missile buffer
struct cbuf		trail[MAX_ENEMY_MISSILES];		// trail buffer

// TODO: reset to 0 befor delivery
int tflag = 1;	// switcher for trail's visibility [0-1]
int tl = 30;	// actual trail length

// Store position of missile i.
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
	if (i >= MAX_ENEMY_MISSILES) return;
	for (j = 0; j < TLEN; j++) {
		trail[i].x[j] = 0;
		trail[i].y[j] = 0;
	}
	trail[i].top = 0;
}

// Draw trail of missile i, for the length of w.
void draw_trail(int i, int w) {
	int j, k; // indexes
	int x, y; // positions
	for (j = 0; j < w; j++) {
		k = (trail[i].top + TLEN - j) % TLEN;
		x = world2abs_x(trail[i].x[k]);
		y = world2abs_y(trail[i].y[k]);
		putpixel(screen_buff, x, y, TCOL);
	}
}

// What to do when a missile explode.
void missile_explode(int index) {
	// this starts explosion animation in graphic task (draw_missile)
	missile[index].in_destruction = 1;
}

// Makes disappear a missile from graphics, without exploding.
void missile_vanish(int index) {
	missile[index].in_destruction = DESTR_BMP_NUM + 1;
	tp[index].index = -1;
}

// A missile should vanish if outside top, left or right borders, explode if bottom.
void handle_corners(int i) {
	int left, right, top, bottom;

	left = (missile[i].x < missile[i].r);
	right = (missile[i].x > WORLD_BOX_WIDTH - missile[i].r);
	top = (missile[i].y > WORLD_BOX_HEIGHT - missile[i].r);
	bottom = (missile[i].y <= missile[i].r);

	if (left || right) missile_vanish(i);
	if (bottom) missile_explode(i);
	if (top) missile[i].y = WORLD_BOX_HEIGHT - missile[i].r;
}

// Draw missile i in graphic coordinates.
void draw_missile(int i) {
	float p1x, p1y, p2x, p2y, p3x, p3y; // world coord.
	float ca, sa;

	// for explosions
	BITMAP* explosion_bmp;
	char explosion_file[30];

	if (missile[i].in_destruction == 0) {
		ca = cos(missile[i].alpha);
		sa = sin(missile[i].alpha);

		p1x = missile[i].x + ML * ca; // nose point
		p1y = missile[i].y + ML * sa;
		p2x = missile[i].x - MW * sa; // left wing
		p2y = missile[i].y + MW * ca;
		p3x = missile[i].x + MW * sa; // right wing
		p3y = missile[i].y - MW * ca;

		triangle(screen_buff,
		         world2abs_x(p1x), world2abs_y(p1y), // nose point
		         world2abs_x(p2x), world2abs_y(p2y), // left wing
		         world2abs_x(p3x), world2abs_y(p3y), // right wing
		         missile[i].c);
	}
	else {
		sprintf(explosion_file, "explosion/%d.bmp", missile[i].in_destruction);
		explosion_bmp = load_bitmap(explosion_file, NULL);
		draw_sprite(screen_buff, explosion_bmp,
		            world2abs_x(missile[i].x) - explosion_bmp->w / 2,
		            world2abs_y(missile[i].y) - explosion_bmp->h / 2);
		destroy_bitmap(explosion_bmp);

		missile[i].in_destruction++;

		// if we're at last animation step, flag missile task as free
		if (missile[i].in_destruction > DESTR_BMP_NUM)
			tp[i].index = -1;
	}
}

/**
 * Inspecting missile's index is possible to know if it is owned by enemies or by the Patriot.
 * Return 0 for enemies, 1 for Patriot, -1 otherwise.
 */
int check_missile_type(int index) {
	if (index >= ENEMY_MISSILES_BASE_INDEX && index < ENEMY_MISSILES_TOP_INDEX)
		return 0;
	if (index >= PATRIOT_MISSILES_BASE_INDEX && index < PATRIOT_MISSILES_TOP_INDEX)
		return 1;
	return -1;
}

/**
 * Initialize the just created missile, caracterizing it as enemy or patriot's.
 * Return 1 if everything went well.
 */
int init_missile(int i) {
	float r, v, alpha;

	switch (check_missile_type(i)) {
	case 0: // Enemy missile
		// An enemy missile can spawn from top or left side, each one with 1/2 of probability.
		r = frand(0, 2);
		// if (1) { // DBG
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
		missile[i].in_destruction = 0;
		missile[i].c = RED;
		missile[i].r = ML;
		break;

	case 1: // Patriot missile
		alpha = (30 + 180) / 180 * PI; // TODO: let user change it
		missile[i].vx = LAUNCHER_V0 * cos(alpha);
		missile[i].vy = LAUNCHER_V0 * sin(alpha);
		missile[i].in_destruction = 0;
		missile[i].c = BLU;
		missile[i].r = ML;
		break;

	default:
		printf("Error creating missile task. %d seems to be an invalid index\n", i);
		return 0;
	}

	// DBG
	// printf("Missile vx: %f\tvy: %f\n", missile[i].vx, missile[i].vy);
	// vx[i] = missile[i].vx;
	// vy[i] = missile[i].vy;

	return 1;
}

void *missile_task(void* arg) {
	int i;	// task index
	int ok;	// flag for correct missile initialization
	float dt;
	i = get_task_index(arg);

	ok = init_missile(i);

	dt = TSCALE * (float)get_task_period(i) / 1000;

	set_period(i);
	while (ok && !sigterm_tasks && missile[i].in_destruction == 0) {
		missile[i].vy -= G0 * dt;
		missile[i].y += missile[i].vy * dt - G0 * dt * dt / 2;
		missile[i].x += missile[i].vx * dt;
		missile[i].alpha = atan2(missile[i].vy, missile[i].vx);

		handle_corners(i);
		store_trail(i);

		// DBG
		// printf("Actual v = (%f; %f)\n", missile[i].vx, missile[i].vy);

		wait_for_period(i);
	}

	// the missile stops to move, but I don't flag its task slot as free,
	// because I've to make explosion animation

	clear_trail(i);
}