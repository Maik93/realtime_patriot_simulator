#include <stdlib.h>
#include <stdio.h>
#include <allegro.h>
#include <time.h>

#include "baseUtils.h"
#include "common.h"
#include "radar.h"

int main(int argc, char const *argv[]) {
	pthread_t interp_id;
	int i;

	init(0);			// without mouse integration
	srand(time(NULL));	// initialize random generator

	start_task(graphic_task, 20, 20, 10, MAX_THREADS - 3);

	// now radar_task spawns in graphic task
	/*struct timespec now;
	clock_gettime(CLOCK_MONOTONIC, &now);
	time_add_ms(&now, 10);
	clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &now, NULL);

	start_task(radar_task, 2, 2, 30, MAX_THREADS - 2);*/

	interp_id = start_task(interp, 40, 40, 10, MAX_THREADS - 1);

	pthread_join(interp_id, NULL);

	allegro_exit();
	return 0;
}