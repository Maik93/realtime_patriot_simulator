#include "radar.h"

// #include <stddef.h>
#include <stdio.h>
// #include <time.h>
#include <math.h>
// #include <pthread.h>
#include <allegro.h>

#include "baseUtils.h"
#include "common.h"

// int radar[ARES];

/**
 * Return the distance of the first pixel != BKG found from (x,y) along direction alpha
 */
int read_sensor(int x0, int y0, int degree) {
	int x, y, c, d;
	float alpha;

	alpha = degree * PI / 180; // angle in rads
	d = RMIN;
	do {
		x = x0 + d * cos(alpha);
		y = y0 - d * sin(alpha);
		c = getpixel(screen, x, y);
		d = d + RSTEP;
	} while ((d <= RMAX) && (c == BKG));

	line(screen_buff, x0, y0, x, y, BLU);

	// if (d <= RMAX) {
	// 	printf("%d %d -> %d\n", x, y, c);
	// }
	return d;
}

void *radar_task(void* arg) {
	int angle, d, i = get_task_index(arg);
	float dt;

	angle = RAMIN;
	// dt = TSCALE * (float)get_task_period(i) / 1000;

	set_period(i);
	while (!sigterm_tasks) {
		d = read_sensor(RPOSX, RPOSY, angle);

		angle++;
		if (angle > RAMAX) angle = RAMIN;

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