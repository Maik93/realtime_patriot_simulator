#include <stdlib.h>
#include <stdio.h>
#include <allegro.h>
#include <time.h>

#include "baseUtils.h"
#include "common.h"
// #include "radar_and_trackers.h"

int main(int argc, char const *argv[]) {
	pthread_t interp_id;
	int i;

	init(0);			// without mouse integration
	srand(time(NULL));	// initialize random generator

	start_task(graphic_task, 20, 20, 10, GRAPHIC_INDEX);
	// and it will spawn radar and rocket_laucher tasks

	// now radar_task spawns in graphic task
	/*struct timespec now;
	clock_gettime(CLOCK_MONOTONIC, &now);
	time_add_ms(&now, 10);
	clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &now, NULL);

	start_task(radar_task, 2, 2, 30, RADAR_INDEX);*/

	interp_id = start_task(interp, 40, 40, 10, INTERPRETER_INDEX);

	pthread_join(interp_id, NULL);

	allegro_exit();
	return 0;
}