/**
 * Periods, relative deadlines and priorities for all tasks.
 */

#ifndef TASK_CONSTANTS_H
#define TASK_CONSTANTS_H

//-----------------------------------------------------
// GRAPHIC AND INTERPETER
//-----------------------------------------------------
#define GRAPH_PER	20			// period for graphic_task (ms)
#define GRAPH_DREL	GRAPH_PER	// relative deadline for graphic_task (ms)
#define GRAPH_PRI	10			// priority of graphic_task
//-----------------------------------------------------
#define INTERP_PER	40			// period for interp_task (ms)
#define INTERP_DREL	INTERP_PER	// relative deadline for interp_task (ms)
#define INTERP_PRI	10			// priority of interp_task
//-----------------------------------------------------
// MISSILES
//-----------------------------------------------------
#define MISSILE_PER		20			// task period (ms)
#define MISSILE_DREL	MISSILE_PER	// relative deadline (ms)
#define MISSILE_PRI		60			// task priority
//-----------------------------------------------------
// RADAR
//-----------------------------------------------------
#define RADAR_PER	2			// period for radar_task (ms)
#define RADAR_DREL	RADAR_PER	// relative deadline for radar_task (ms)
#define RADAR_PRI	30			// priority of radar_task
//-----------------------------------------------------
// TRACKERS
//-----------------------------------------------------
#define TRACKER_PER		20			// task period (ms)
#define TRACKER_DREL	TRACKER_PER	// relative deadline (ms)
#define TRACKER_PRI		50			// task priority
//-----------------------------------------------------
// ROCKET LAUNCHER
//-----------------------------------------------------
#define LAUNCHER_PER		50			// period for rocket_launcher_task (ms)
#define LAUNCHER_DREL		LAUNCHER_PER// relative deadline for rocket_launcher_task (ms)
#define LAUNCHER_PRI		50			// priority of rocket_launcher_task

#endif