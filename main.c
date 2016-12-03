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

	start_task(display, 20, 20, 10, MAX_TASKS);
	start_task(radar_task, 5/TSCALE, 5/TSCALE, 30, MAX_TASKS + 1);
	interp_id = start_task(interp, 40, 40, 10, MAX_TASKS + 2);

	pthread_join(interp_id, NULL);

	allegro_exit();
	return 0;
}