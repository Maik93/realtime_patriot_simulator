#ifndef COLORS_H
#define COLORS_H

// CHOOSE ONE OF THIS:
// #define DARK_MODE
#define LIGHT_MODE

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
#define LRED		41
#define ORANGE		42
#define LORANGE		43

//-----------------------------------------------------
// LIGHT_MODE OBJECTS DEFINITIONS
//-----------------------------------------------------
#ifdef LIGHT_MODE
	#define TEXT_TITL_COL		LORANGE	// color for text titles
	#define TEXT_COL			WHITE	// color for all non-warning texts
	#define TEXT_ALERT_COL		ORANGE	// color for all non-warning texts
	#define BORDER_COL			ORANGE	// for every box in the window
	#define ENEMY_COL			RED		// for an enemy missile
	#define PATMISS_COL			LORANGE	// for a Patriot missile
	#define TCOL				WHITE	// trail color
	#define RSENSOR_COL			LBLU	// for radar sensor (the sprite in the center of the ground)
	#define RDISPLAY_DOT_COL	GREEN	// for dots shown in radar display
	#define LAUNCHER_COL		LRED	// for rochet launcher
	#define PREDICTION_COL		BLU		// for trajectories predicted
	//-----------------------------------------------------
	#define GND					23		// window background
	#define BKG					24		// world background
	//-----------------------------------------------------
#endif

//-----------------------------------------------------
// DARK_MODE OBJECTS DEFINITIONS
//-----------------------------------------------------
#ifdef DARK_MODE
	#define TEXT_TITL_COL		LBLU	// color for text titles
	#define TEXT_COL			WHITE	// color for all non-warning texts
	#define TEXT_ALERT_COL		RED	// color for all non-warning texts
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
	//-----------------------------------------------------
#endif

#endif