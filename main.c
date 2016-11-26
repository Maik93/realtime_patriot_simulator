#include <stdlib.h>
#include <stdio.h>
#include <allegro.h>
#include <time.h>

#include "baseUtils.h"
#include "tasks.h"

int main(int argc, char const *argv[]) {
	pthread_t interp_id;
	int i;

	init(0);	// without mouse integration
	srand(time(NULL));	// initialize random generator
	// ptask_init(SCHED_FIFO);
	start_task(display, 20, 20, 10, MAX_TASKS);
	interp_id = start_task(interp, 40, 40, 10, MAX_TASKS + 1);

	pthread_join(interp_id, NULL);

	allegro_exit();
	return 0;
}