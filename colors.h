#ifndef COLORS_H
#define COLORS_H
// Dark mode

// #include "baseUtils.h"
//-----------------------------------------------------
// BASIC COLORS DEFINITIONS
//-----------------------------------------------------
#define COLOR_DEPTH	8
#define TRANSP		0
#define BLACK		16
#define WHITE		15
#define GREEN		10
#define LGREY		8
#define GREY		20
#define DGREY		19
#define BLU			9
#define LBLU		54
#define RED			4
//-----------------------------------------------------
// COLOR OBJECTS DEFINITIONS
//-----------------------------------------------------
#define BORDER_COL			LBLU	// for every box in the window
#define ENEMY_COL			RED		// for an enemy missile
#define PATMISS_COL			BLU		// for a Patriot missile
#define TCOL				LGREY	// trail color
#define RSENSOR_COL			BLU		// for radar sensor (the sprite in the center of the ground)
#define RDISPLAY_DOT_COL	GREEN	// for dots shown in radar display
#define LAUNCHER_COL		BLU		// for rochet launcher
#define PREDICTION_COL		BLU		// for trajectories predicted
//-----------------------------------------------------
#define GND					DGREY	// window background
#define BKG					GREY	// world background
// #define MCOL				14		// menu color
// #define NCOL				7		// numbers color
//-----------------------------------------------------

#endif