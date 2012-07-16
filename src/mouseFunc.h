/*

Flip Clock Mouse Event Manager Headers
Part of FlipClock C SDL for Maemo.

This library defines global vars and methods that are used for mouse/cursor interaction in Flipclock

Right now we'll start just with basic mouse events like clicks... drags and complicated stuff can come later...

-Rob Williams, Aug 13, 2009.


*/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>


//********************** STATIC DEF'S *****************************//

enum { MOUSEDOWN, MOUSEUP, CLICK, OUTSIDECLICK, MOUSEDRAG, MOUSESLIDE };

//********************** DONE STATIC DEF's ************************//


//********************** Structure Definitions ***********************//

/* Structure for current mouse position */
struct mouseState {
	int xDownPos;
	int yDownPos; 
	int xUpPos;
	int yUpPos;
	int xCurrPos;
	int yCurrPos;
	int pressed;
	int motion;
};

/* Structure for drag actions (buttons) */
struct dragAction {
	int dragUpOffset;
	int dragDownOffset;
	int dragLeftOffset;
	int dragRightOffset;
	void (*dragUpAction) ();	//Callback to be fired when drag up occurs
	void (*dragDownAction) ();	//Callback to be fired when drag down occurs
	void (*dragLeftAction) ();	//Callback to be fired when drag left occurs
	void (*dragRightAction) ();	//Callback to be fired when drag right occurs
};

/* Structure for mouse actions (buttons) */
struct mouseAction {
	SDL_Rect buttonRect;		//Bounding box of button
	int buttonMode;			//Is button press, release, click, drag, etc?
	int pressedOn;			//Was this button pressed down on?
	int releasedOn;			//Was this button released on?
	int enabled;			//Is this button enabled or not?
	void (*buttonAction) ();	//Callback to be fired when action is triggered
	struct dragAction dragSettings;	//Stuff for drag buttons
};

/* Structure for button collection */
struct buttonCollection {
	int buttonCount;
	struct mouseAction **buttonActions;
};


//********************** Done Struct Defs *************************//


//********************** Global variables *************************//

struct mouseState currentMouseState = {0,0,0,0,0,0,0, 0};


struct buttonCollection clockButtonActions[CLOCKMODEMAX];			//Collection of button actions defined by clock itself
struct buttonCollection themeButtonActions[CLOCKMODEMAX];			//Collection of button actions defined by theme

int mouseMoveEnabled[CLOCKMODEMAX] = {0};							//Flag array to indicate whether we need to track mouse movements

//********************** Done global variables *******************//


//********************** Function Headers *************************//

void checkButtonActions(struct mouseAction **buttonActions, int actionsCount);

//********************** Done Function Headers ********************//
