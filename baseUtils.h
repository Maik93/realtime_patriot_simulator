#ifndef UTILS_H
#define UTILS_H

#include <time.h>
#include <pthread.h>
#include <allegro.h>

#include "colors.h"

//-----------------------------------------------------
// TASKS - Here's how is organized task_param array:
// | 3x Enemy missiles | 3x Patriot missiles | disturbing object | 4x Trackers | Graphic | Radar | Rocket_launcher | Interpreter |
// ^ -> ENEMY_MISSILES_BASE_INDEX
//                     ^ -> PATRIOT_MISSILES_BASE_INDEX
//                                                               ^ -> TRACKER_BASE_INDEX
//-----------------------------------------------------
#define MAX_ENEMY_MISSILES		3
#define MAX_PATRIOT_MISSILES	5
#define MAX_MISSILES			(MAX_ENEMY_MISSILES + MAX_PATRIOT_MISSILES)
#define MAX_TRACKERS			4
#define MAX_THREADS				(MAX_ENEMY_MISSILES + MAX_PATRIOT_MISSILES + MAX_TRACKERS + 5)
//-----------------------------------------------------
#define ENEMY_MISSILES_BASE_INDEX	0
#define ENEMY_MISSILES_TOP_INDEX	(ENEMY_MISSILES_BASE_INDEX + MAX_ENEMY_MISSILES)
#define PATRIOT_MISSILES_BASE_INDEX	ENEMY_MISSILES_TOP_INDEX
#define PATRIOT_MISSILES_TOP_INDEX	(PATRIOT_MISSILES_BASE_INDEX + MAX_PATRIOT_MISSILES)
#define DISTURB_OBJECT_INDEX		PATRIOT_MISSILES_TOP_INDEX
#define TRACKER_BASE_INDEX			(DISTURB_OBJECT_INDEX + 1)
#define TRACKER_TOP_INDEX			(TRACKER_BASE_INDEX + MAX_TRACKERS)
#define GRAPHIC_INDEX				(MAX_THREADS - 4)
#define RADAR_INDEX					(MAX_THREADS - 3)
#define ROCKET_LAUNCHER_INDEX		(MAX_THREADS - 2)
#define INTERPRETER_INDEX			(MAX_THREADS - 1)
//-----------------------------------------------------
// GRAPHICS CONSTANTS
//-----------------------------------------------------
#define WIN_WIDTH	800	// width of graphic window
#define WIN_HEIGHT	600	// height of graphic window
//-----------------------------------------------------
#define CHAR_WIDTH	8	// horizontal pixels occupied by a char
#define CHAR_HEIGHT	10	// vertical pixels occupied by a char
//-----------------------------------------------------
// OTHER CONSTANTS
//-----------------------------------------------------
#define PI		3.1415926535	// 10 digit after dot
#define G0		9.8				// acceleration of gravity
//-----------------------------------------------------
// STRUCT
//-----------------------------------------------------
// See above how task_param array is organized.
struct task_param {
	int		index;			// task index. Equals to -1 for inactive tasks.
	// long	wcet;			// in microseconds
	double	response_time_sum; // sum of response times (ms)
	int		period;			// relative (ms)
	int		deadline;		// relative (ms)
	int		priority;		// in [0-99]
	int		dmiss;			// num of misses
	struct	timespec at;	// next activation time (absolute)
	struct	timespec dl;	// deadline (absolute)
	int		counts;			// number of runs for this task
};

//-----------------------------------------------------
// TASK VARIABLES
//-----------------------------------------------------
extern pthread_t			tid[MAX_THREADS];
extern pthread_attr_t		att[MAX_THREADS];
extern struct task_param	tp[MAX_THREADS];
extern struct sched_param	sp;
//-----------------------------------------------------
extern int		sigterm_tasks;	// [0-1] signal for all task to terminate
//-----------------------------------------------------
// GRAPHIC VARIABLES
//-----------------------------------------------------
extern BITMAP	*screen_buff;	// double buffer integration

//-----------------------------------------------------
// BASE UTILITY FUNCTIONS
//-----------------------------------------------------
float frand(float xmi, float xma);
fixed deg2fix(int degree);
void time_copy(struct timespec *td, struct timespec ts);
void time_add_ms(struct timespec *t, int ms);
float time_diff_ms(struct timespec t1, struct timespec t2);
int time_cmp(struct timespec t1, struct timespec t2);

//-----------------------------------------------------
// TASK HANDLING FUNCTIONS
//-----------------------------------------------------
void set_period(int index);
void wait_for_period(int index);
int deadline_miss(int index);
int find_free_slot(int min_index, int max_index);
int get_task_index(void *arg);
int get_task_period(int index);
pthread_t start_task(void *task_fun, int period, int deadline, int priority, int index);
void kill_all_task();

//-----------------------------------------------------
// ALLEGRO FUNCTIONS
//-----------------------------------------------------
void get_keycodes(char *scan, char *ascii);
int listen_scancode();
void get_string(char *str, int x, int y, int c, int b);
void make_bmp_transp(char file_name_in[30], char file_name_out[30]);
void make_bmp_half(char file_name_in[30], char file_name_out[30]);
void convert_imgs_to_transp(char *dir);
void activate_mouse();
void init(int enable_mouse);

#endif