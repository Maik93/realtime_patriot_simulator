#include <stdlib.h>
#include <stdio.h>
#include <allegro.h>
#include <time.h>

#include "baseUtils.h"
#include "task_constants.h"
#include "common.h"

int main(int argc, char const *argv[]) {
	pthread_t interp_id;
	int i;

	init(0);			// without mouse integration
	srand(time(NULL));	// initialize random generator

	// start graphic task. It will take care of other tasks too.
	start_task(graphic_task, GRAPH_PER, GRAPH_PER, GRAPH_PRI, GRAPHIC_INDEX);

	// keybord listener.
	interp_id = start_task(interp_task, INTERP_PER, INTERP_PER, INTERP_PRI, INTERPRETER_INDEX);

	// wait for return of interp_task, caused by ESC_KEY
	pthread_join(interp_id, NULL);

	allegro_exit();
	return 0;
}