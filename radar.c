#include "radar.h"

// #include <stddef.h>
#include <stdio.h>
// #include <time.h>
#include <math.h>
// #include <pthread.h>
#include <allegro.h>

#include "baseUtils.h"
#include "common.h"

struct scan radar[ARES];

void read_sensor(int degree) {
	int x, y, c, d;
	float alpha;

	d = RMIN;
	alpha = degree * PI / 180; // angle in rads
	do {
		x = RPOSX + d * cos(alpha);
		y = RPOSY - d * sin(alpha);
		c = getpixel(screen, x, y);
		d = d + RSTEP;
	} while ((d <= RMAX) && (c == BKG || c == BLU));

	// line(screen_buff, RPOSX, RPOSY, x, y, BLU);
	radar[degree-RAMIN].x = x;
	radar[degree-RAMIN].y = y;

	// if (d <= RMAX) {
	// 	printf("%d %d -> %d\n", x, y, c);
	// }
}

void *radar_task(void* arg) {
	int angle, i = get_task_index(arg);
	float dt;

	angle = RAMIN;
	// dt = TSCALE * (float)get_task_period(i) / 1000;

	set_period(i);
	while (!sigterm_tasks) {
		read_sensor(angle);

		angle++;
		if (angle > RAMAX) {
			// printf("*bip*\n");
			angle = RAMIN;
		}

		wait_for_period(i);
	}
	tp[i].index = -1;
}

/**
 *
void line_scan(int x0, int y0, int degree) {
	int d;
	float alpha;

	alpha = degree * PI / 180; // angle in rads
	for (d = RMIN; d <= RMAX; d += RSTEP) {
		x = x0 + d * cos(alpha);
		y = y0 - d * sin(alpha);
		radar[degree][d] = getpixel(screen, x, y);
	}
}
 */
