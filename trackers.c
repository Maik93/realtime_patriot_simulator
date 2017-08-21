/**
 * Trackers: they're attached by the radar on every object seen in the sky.
 * Following these objects, they try to evaluate velocities and accelerations.
 */
#include "trackers.h"

#include <stdio.h>
#include <math.h>
#include <allegro.h>

#include "baseUtils.h"
#include "common.h"
#include "colors.h"
#include "missiles.h"
#include "radar.h"

// public variables
int				tracker_disp_mode = 0; // what parameter is currently shown by tracker display [0-2]
struct point	current_points_tracked[MAX_TRACKERS];	// array of all currently tracked points (abs. coord)
int				tracker_is_active[MAX_TRACKERS];		// flag activity for trackers
struct tracker
	tracked_points[MAX_TRACKERS]; // memory for trackers, containing points acquired and vel and acc evaluated

// private variables
int tracker_view[MAX_TRACKERS][TRACKER_RES][TRACKER_RES]; // image buffer for trackers

// Store pixels contained in the square centered in (x0, y0) and width TRACKER_RES.
void get_image(int tracker_i, int x0, int y0) {
	int i, j; // image matrix indexes
	int x, y; // graphical coordinates
	for (i = 0; i < TRACKER_RES; i++)
		for (j = 0; j < TRACKER_RES; j++) {
			x = x0 - TRACKER_RES / 2 + i;
			y = y0 - TRACKER_RES / 2 + j;

			// take only points in world
			if (x > WORLD_BOX_X1 && x < WORLD_BOX_X2
			        && y > WORLD_BOX_Y1 && y < WORLD_BOX_Y2)
				tracker_view[tracker_i][i][j] = getpixel(screen, x, y);
			else tracker_view[tracker_i][i][j] = BKG;
		}
}

// Evaluate centroid of element seen by tracker_i, relative to the center of image.
struct point compute_centroid(int tracker_i) {
	struct point c_rel;	// coord. of centroid
	int n = 0;			// number of points found for x and y
	int i, j;

	c_rel.x = 0;
	c_rel.y = 0;
	for (i = 0; i < TRACKER_RES; i++)
		for (j = 0; j < TRACKER_RES; j++) {
			// radar and rocket_launcher sprites are excluded, as enemy missile trails
			if (tracker_view[tracker_i][i][j] != BKG &&
			        tracker_view[tracker_i][i][j] != PATMISS_COL &&
			        tracker_view[tracker_i][i][j] != RSENSOR_COL &&
			        tracker_view[tracker_i][i][j] != LAUNCHER_COL &&
			        tracker_view[tracker_i][i][j] != PREDICTION_COL &&
			        tracker_view[tracker_i][i][j] != TCOL) {
				c_rel.x += i - TRACKER_RES / 2;
				c_rel.y += j - TRACKER_RES / 2;
				n++;
			}
		}

	// check if some point is seen, otherwise deactivate the tracker
	if (n != 0) {
		c_rel.x /= n;
		c_rel.y /= n;

		// highlight centroid with a green dot on tracker display
		tracker_view[tracker_i][c_rel.x + TRACKER_RES / 2][c_rel.y + TRACKER_RES / 2] = GREEN;
	}
	else tracker_is_active[tracker_i] = 0;

	return c_rel;
}

// Store last TSTORE points tracked.
void store_point(int i) {
	int k;

	if (i >= MAX_TRACKERS) return;

	k = tracked_points[i].top;

	// don't store same coord.
	if (current_points_tracked[i].x == tracked_points[i].x[k] ||
	        current_points_tracked[i].y == tracked_points[i].y[k])
		return;

	k = (k + 1) % TSTORE;
	clock_gettime(CLOCK_MONOTONIC, &tracked_points[i].t[k]);
	tracked_points[i].x[k] = abs2world_x(current_points_tracked[i].x);
	tracked_points[i].y[k] = abs2world_y(current_points_tracked[i].y);
	tracked_points[i].top = k;

	tracked_points[i].n_samples++;
}

// Draw predicted trajectory for the object tracked by tracker_i.
void draw_predictions(int tracker_i, float delta_t) {
	float x, y, vy;
	int k, prev_k;
	int c = 500; // max number of iterations, to avoid accidental infinite loop

	k = tracked_points[tracker_i].top;
	prev_k = (k - 1 + TSTORE) % TSTORE;

	x = tracked_points[tracker_i].x[k];
	y = tracked_points[tracker_i].y[k];
	vy = tracked_points[tracker_i].vy;

	while (y > 0 && c > 0) {
		x += tracked_points[tracker_i].vx * delta_t;
		y += (0.5 * tracked_points[tracker_i].ay * delta_t + vy) * delta_t;
		vy += tracked_points[tracker_i].ay * delta_t;

		putpixel(screen_buff, world2abs_x(x), world2abs_y(y), PREDICTION_COL);
		c--;
	}
}

// Compute error between acquired points and estimate trajectory for tracker_i and return its value.
float evaluate_error(int tracker_i) {
	float x, y, delta_t, supposed_x, supposed_y, real_x, real_y, sumx = 0, sumy = 0;
	int x0, y0, k, count = 0;
	struct timespec t0;

	// start from last acquired point
	k = tracked_points[tracker_i].top;

	// check if there's samples
	if (tracked_points[tracker_i].n_samples == 0)
		return TRAJ_MAX_ERROR;

	// actual position is considered as our initial condition
	t0 = tracked_points[tracker_i].t[k];
	x0 = tracked_points[tracker_i].x[k];
	y0 = tracked_points[tracker_i].y[k];

	// compare each acquired point with evaluated trajectory's output in the corresponding times
	do {
		delta_t = -time_diff_ms(t0, tracked_points[tracker_i].t[k]) / 1000;

		supposed_x = x0 + tracked_points[tracker_i].vx * delta_t;
		supposed_y = y0 + (0.5 * tracked_points[tracker_i].ay * delta_t + tracked_points[tracker_i].vy) * delta_t;
		// DBG
		// printf("Pred:\tx=%2.f\ty=%2.f\n", supposed_x, supposed_y);

		real_x = tracked_points[tracker_i].x[k];
		real_y = tracked_points[tracker_i].y[k];
		// DBG
		// printf("Real:\tx=%2.f\ty=%2.f\n", real_x, real_y);

		sumx += pow(real_x - supposed_x, 2);
		sumy += pow(real_y - supposed_y, 2);
		// DBG
		// printf("Differences:\tEx=%2.f\tEy=%2.f\n", real_x - supposed_x, real_y - supposed_y);

		k = (k - 1 + TSTORE) % TSTORE; // previous k
		count++;
	} while (k != tracked_points[tracker_i].top && count < tracked_points[tracker_i].n_samples);

	// DBG
	// printf("Error:\tEx=%2.f\tEy=%2.f\n", sqrt(sumx), sqrt(sumy));
	// printf("Error:\tE=%2.f\n", sqrt(sumx + sumy));

	return sqrt(sumx + sumy);
}

// Acceleration ax is zero, therefore vx is constant, so x motion law is of the fist order.
// Evaluating the best aproximating line for x positions, I can exctract vx
// as the coefficent of the first-order term. Mean squar error minimization is used.
float evaluate_vx(int tracker_i) {
	int i, j, k, first_k; // indexes
	float a0, a1; // coefficients of the fitting line
	float x[TSTORE], sumx = 0, sumxy = 0, sumx2 = 0;
	int y[TSTORE], sumy = 0;

	first_k = (tracked_points[tracker_i].top + 1) % TSTORE;
	k = first_k; // start from older one

	// load points to fit: y is the horizontal position of missile at time x
	for (j = 0; j < TSTORE; j++) {
		x[j] = TSCALE *
		       time_diff_ms(tracked_points[tracker_i].t[k], tracked_points[tracker_i].t[first_k]) / 1000.0;
		y[j] = tracked_points[tracker_i].x[k];

		k = (k + 1) % TSTORE;
	}

	for (i = 0; i < TSTORE; i++) {
		sumx += x[i];
		sumx2 += x[i] * x[i];
		sumy += y[i];
		sumxy += x[i] * y[i];
	}

	// a0 = ((sumx2 * sumy - sumx * sumxy) * 1.0 / (TSTORE * sumx2 - sumx * sumx) * 1.0);
	a1 = ((TSTORE * sumxy - sumx * sumy) * 1.0 / (TSTORE * sumx2 - sumx * sumx) * 1.0);

	// DBG
	// printf("From fitting: Vx = %f\n", a1);
	return a1;
}

// Acceleration ay is -G0, constant, therefore vy is linear and y motion law is of the second order.
// Evaluating the best aproximating parabola for y positions, I can exctract vy
// as the coefficent of the first-order term. Mean squar error minimization is used.
float evaluate_vy(int tracker_i) {
	int i, j, k, first_k; // indexes
	float a0, a1, a2; // coefficients of the fitting parabole
	float x[TSTORE], sumx = 0, sumxy = 0, sumx2 = 0, sumx3 = 0, p, q;
	int y[TSTORE], sumy = 0;

	first_k = (tracked_points[tracker_i].top + 1) % TSTORE;
	k = first_k; // start from older one

	// load points to fit: y is the vertical position of missile at time x
	for (j = 0; j < TSTORE; j++) {
		x[j] = TSCALE *
		       time_diff_ms(tracked_points[tracker_i].t[k], tracked_points[tracker_i].t[first_k]) / 1000.0;
		y[j] = tracked_points[tracker_i].y[k];

		k = (k + 1) % TSTORE;
	}

	for (i = 0; i < TSTORE; i++) {
		sumx += x[i];
		sumx2 += x[i] * x[i];
		sumx3 += x[i] * x[i] * x[i];
		sumy += y[i];
		sumxy += x[i] * y[i];
	}
	a2 = -G0 / 2;
	p = sumy - a2 * sumx2;
	q = sumxy - a2 * sumx3;

	// a0 = ((sumx2 * p - sumx * q) * 1.0 / (TSTORE * sumx2 - sumx * sumx) * 1.0);
	a1 = ((TSTORE * q - sumx * p) * 1.0 / (TSTORE * sumx2 - sumx * sumx) * 1.0);

	// DBG
	// printf("From fitting: Vy = %f\n", a1);
	return a1;
}

// Compute velocities along x and y for the tracked object.
void evaluate_v_and_a(int task_i, int tracker_i) {
	int k, prev_k, prev_prev_k;	// previous index point and its previous
	float vx_prev, vy_prev;		// previous velocity, needed to evaluate acc
	float dt1, dt2;				// delta t from k to prev_k and from prev_k to prev_prev_k

	if (tracked_points[tracker_i].n_samples < 2) return; // not enough points

	k = tracked_points[tracker_i].top;
	prev_k = (k - 1 + TSTORE) % TSTORE;
	prev_prev_k = (prev_k - 1 + TSTORE) % TSTORE;

	float dt = TSCALE * time_diff_ms(tracked_points[tracker_i].t[k], tracked_points[tracker_i].t[prev_k]) / 1000.0;

	tracked_points[tracker_i].ax = 0;
	tracked_points[tracker_i].vx = evaluate_vx(tracker_i);

	tracked_points[tracker_i].ay = -G0;
	tracked_points[tracker_i].vy = evaluate_vy(tracker_i);

	// DBG
	// printf("Predicted v = (%f; %f)\n", tracked_points[tracker_i].vx, tracked_points[tracker_i].vy);
}

// Destroy the struct content of tracker_i.
void clear_tracker_struct(int task_i, int tracker_i) {
	tracked_points[tracker_i].vx = 0;
	tracked_points[tracker_i].vy = 0;
	tracked_points[tracker_i].ax = 0;
	tracked_points[tracker_i].ay = 0;
	tracked_points[tracker_i].traj_error = TRAJ_MAX_ERROR;
	tracked_points[tracker_i].n_samples = 0; // there's no need to clear circular buffer, just n_samples
	tracker_is_active[tracker_i] = 0;
	tracked_points[tracker_i].time_to_shoot = -1;
	tp[task_i].index = -1;
}

void *tracker_task(void* arg) {
	int task_i, tracker_i, other_i; // indexes
	float util_perc;	// percentage of utilization, given by task response time and its period
	struct point c;		// current relative centroid

	task_i = get_task_index(arg);
	tracker_i = task_i - TRACKER_BASE_INDEX;

	tracked_points[tracker_i].n_samples = 0;
	tracked_points[tracker_i].traj_error = TRAJ_MAX_ERROR;
	tracked_points[tracker_i].time_to_shoot = -1;
	tracker_is_active[tracker_i] = 1;
	// printf("Tracker %d activated.\n", tracker_i);

	// DBG
	// printf("Start tracking (%d, %d)\n", current_points_tracked[tracker_i].x, current_points_tracked[tracker_i].y);

	set_period(task_i);
	while (!sigterm_tasks && tracker_is_active[tracker_i]) {
		get_image(tracker_i, current_points_tracked[tracker_i].x, current_points_tracked[tracker_i].y);

		c = compute_centroid(tracker_i); // relative from center of tracking box

		// updating the center point to follow
		current_points_tracked[tracker_i].x += c.x;
		current_points_tracked[tracker_i].y += c.y;

		// if inside a rectangle of any other active tracker, then I'm overlapped with someone else
		for (other_i = 0; other_i < MAX_TRACKERS; other_i++) {
			if (other_i != tracker_i && tracker_is_active[other_i] &&
			        abs(current_points_tracked[other_i].x - current_points_tracked[tracker_i].x) <= TRACKER_RES / 2 &&
			        abs(current_points_tracked[other_i].y - current_points_tracked[tracker_i].y) <= TRACKER_RES / 2) {
				tracker_is_active[tracker_i] = 0;
			}
		}

		if (tracker_is_active[tracker_i]) {
			store_point(tracker_i);

			// try to evaluate velocities of the tracked object assuming it has ballistic motion
			evaluate_v_and_a(task_i, tracker_i);

			// evaluate error between predicted and real positions
			tracked_points[tracker_i].traj_error = evaluate_error(tracker_i);

			wait_for_period(task_i);
		}
	}

	if (tp[task_i].counts > 0) {
		util_perc = tp[task_i].response_time_sum / (tp[task_i].period * tp[task_i].counts) * 100.0;
		printf("Tracker %d detached. Missed %d deadlines on %d runs. %d%% of utilization.\n",
		       tracker_i, tp[task_i].dmiss, tp[task_i].counts, (int)round(util_perc));
	}

	clear_tracker_struct(task_i, tracker_i);
}

// Shows what each tracker sees in the right side of the screen.
void tracker_display(int tracker_i) {
	int i, j;	// image matrix indexes
	int x0, y0;	// center to draw display
	int x, y;	// graphical coordinates
	char str[8];// for text outputs

	// find out where to draw this display
	switch (tracker_i) {
	case 0:
		x0 = TRACK_D0_X;
		y0 = TRACK_D0_Y;
		break;
	case 1:
		x0 = TRACK_D1_X;
		y0 = TRACK_D1_Y;
		break;
	case 2:
		x0 = TRACK_D2_X;
		y0 = TRACK_D2_Y;
		break;
	case 3:
		x0 = TRACK_D3_X;
		y0 = TRACK_D3_Y;
		break;
	default: return;
	}

	// background
	rectfill(screen_buff, x0 - TRACKER_RES * TRACK_DSCALE / 2, y0 - TRACKER_RES * TRACK_DSCALE / 2,
	         x0 + TRACKER_RES * TRACK_DSCALE / 2, y0 + TRACKER_RES * TRACK_DSCALE / 2, BKG);

	// draws what's seen by tracker, magnified by factor TRACK_DSCALE
	for (i = 0; i < TRACKER_RES; i++)
		for (j = 0; j < TRACKER_RES; j++) {
			x = (int)x0 - TRACKER_RES * TRACK_DSCALE / 2 + i * TRACK_DSCALE;
			y = (int)y0 - TRACKER_RES * TRACK_DSCALE / 2 + j * TRACK_DSCALE;
			putpixel(screen_buff, x, y, tracker_view[tracker_i][i][j]);
		}

	// border
	rect(screen_buff, x0 - TRACKER_RES * TRACK_DSCALE / 2, y0 - TRACKER_RES * TRACK_DSCALE / 2,
	     x0 + TRACKER_RES * TRACK_DSCALE / 2, y0 + TRACKER_RES * TRACK_DSCALE / 2, BORDER_COL);

	switch (tracker_disp_mode) {
	case 0: // prints trajectory's Mean Square Error
		if (tracked_points[tracker_i].traj_error > 0) {
			sprintf(str, "MSE: %.0f", tracked_points[tracker_i].traj_error);
			textout_centre_ex(screen_buff, font, str,
			                  x0, y0 + TRACKER_RES * TRACK_DSCALE / 2 - CHAR_HEIGHT,
			                  TEXT_TITL_COL, -1);
		}
		break;
	case 1: // prints trajectory's Relative Mean Square Error
		if (tracked_points[tracker_i].traj_error > 0 && tracked_points[tracker_i].n_samples > 0) {
			sprintf(str, "RMSE: %.2f", tracked_points[tracker_i].traj_error / tracked_points[tracker_i].n_samples);
			textout_centre_ex(screen_buff, font, str,
			                  x0, y0 + TRACKER_RES * TRACK_DSCALE / 2 - CHAR_HEIGHT,
			                  TEXT_TITL_COL, -1);
		}
		break;
	case 2: // prints time to shoot
		if (tracked_points[tracker_i].time_to_shoot > 0 &&
		        tracked_points[tracker_i].time_to_shoot < 100 &&
		        tracked_points[tracker_i].traj_error < TRAJ_MAX_ERROR) {
			sprintf(str, "%.2f s", tracked_points[tracker_i].time_to_shoot);
			textout_centre_ex(screen_buff, font, str,
			                  x0, y0 + TRACKER_RES * TRACK_DSCALE / 2 - CHAR_HEIGHT,
			                  TEXT_TITL_COL, -1);
		}
		break;
	default: return;
	}
}