/*

Flip Clock Mouse Event Manager
Part of FlipClock C SDL for Maemo.

This library defines global vars and methods that are used for mouse/cursor interaction in Flipclock

Right now we'll start just with basic mouse events like clicks... drags and complicated stuff can come later...

-Rob Williams, Aug 13, 2009.


*/



//********************** Function Definitions *********************//

/****************************************************************
*
* initButtonCollection(struct buttonCollection *myButtonCollection)
*
* Initialize a button collection to be zero'd out
*****************************************************************/

int initButtonCollection(struct buttonCollection *myButtonCollection) {
	int j;
	
	/* Clear out the clock buttons */
	for (j=0; j < CLOCKMODEMAX; j++) {
		myButtonCollection[j].buttonCount = 0;
		myButtonCollection[j].buttonActions = NULL;
	}

	return 1;
}
 
/****************************************************************
*
* initButton(struct mouseAction *thisButton)
*
* Initialize a button to be zero'd out
*****************************************************************/
void initButton(struct mouseAction *thisButton) {

	
	thisButton->pressedOn = 0;
	thisButton->releasedOn = 0;
	thisButton->enabled = 0;
	thisButton->buttonAction = NULL;
	
	//Drag stuff
	thisButton->dragSettings.dragUpOffset = 0;
	thisButton->dragSettings.dragDownOffset = 0;
	thisButton->dragSettings.dragLeftOffset = 0;
	thisButton->dragSettings.dragRightOffset = 0;
	thisButton->dragSettings.dragUpAction = NULL;
	thisButton->dragSettings.dragDownAction = NULL;
	thisButton->dragSettings.dragLeftAction = NULL;
	thisButton->dragSettings.dragRightAction = NULL;
	

}

/****************************************************************
* checkForHit(SDL_Rect targetRect, int xPos, int yPos;)
*
* Quick helper function to see if a point is inside a given rect
*****************************************************************/

int checkForHit(SDL_Rect *destRect, int xPos, int yPos) {
	int withinX, withinY;

	withinX = 0;
	withinY = 0;

	if (xPos >= destRect->x && xPos <= (destRect->x + destRect->w)) {
		withinX = 1;
	}

	if (yPos >= destRect->y && yPos <= (destRect->y + destRect->h)) {
		withinY = 1;
	}
	
	if (withinX == 1 && withinY == 1) {
		return 1;
	} else {
		return 0;
	}
	

}

/****************************************************************
* doButtonAction(void(*buttonAction)())
*
* Runs a button action, passing reference to the button if needed
****************************************************************/

void doButtonAction(void(*buttonAction)()) {

}

/****************************************************************
* checkMouseEvents()
*
* Main function to check current clock settings and execute any
* code related to mouse events. This function gets called
* and time a mouseEvent occurs.
*****************************************************************/

void checkMouseEvents() 
{
	//Run standard clock buttons check
	checkButtonActions(clockButtonActions[clockMode].buttonActions, clockButtonActions[clockMode].buttonCount); 
	
	if (themeReady == 1) {
		//Run clock buttons check for this theme
		checkButtonActions(themeButtonActions[clockMode].buttonActions, themeButtonActions[clockMode].buttonCount);
	}
	
	

}

/****************************************************************
* checkButtonActions(struct mouseAction **modeNormalButtonActions)
*
* Main function to check current clock settings and execute any
* code related to mouse events. This function gets called
* and time a mouseEvent occurs.
*
* We use an actions array to store the function pointers, then
* iterate over it and call the actions once were done with
* the button checking code; this is important because button actions
* (such as changeTheme) might alter the button code itself, so
* we don't want to screw things up...
*****************************************************************/

void checkButtonActions(struct mouseAction **buttonActions, int actionsCount) 
{
	int i=0;
	int hit=0;
	
	int yDelta;
	int xDelta;
	
	//Define the array of actions to be taken
	void (*actionsArr[30]) () = {NULL};
	int actionsFuncCount = 0;
	
	//Do something
	//printf("actionsCount is %i\n", actionsCount);
	for (i=0; i < actionsCount; i++) {
		//printf("checking %i\n", i);
		if (buttonActions[i] != NULL ) {
			
			if (currentMouseState.pressed == 1) {
				
				if (currentMouseState.motion != 1) {
					hit	= 0;
					hit = checkForHit(&buttonActions[i]->buttonRect, currentMouseState.xDownPos, currentMouseState.yDownPos);
					if (buttonActions[i]->buttonMode == OUTSIDECLICK) {
						printf("outside click pressed? %i\n", hit);
					}
					buttonActions[i]->pressedOn = hit;
				}
				
				//Since mousedown's occur right away, do this now
				//if (buttonActions[i]->buttonMode == MOUSEDOWN && hit == 1) {
				switch (buttonActions[i]->buttonMode) {
					case MOUSEDOWN:
					case MOUSESLIDE:
						if (buttonActions[i]->pressedOn == 1) {
							//This button was pressed down on, so run it's event
							if (buttonActions[i]->buttonAction != NULL && buttonActions[i]->enabled == 1) {
								actionsArr[actionsFuncCount] = buttonActions[i]->buttonAction;
								actionsFuncCount++;
							}
						}
						break;
				}
			} else {
				
				hit	= 0;
				hit = checkForHit(&buttonActions[i]->buttonRect, currentMouseState.xUpPos, currentMouseState.yUpPos);
				buttonActions[i]->releasedOn = hit;
				
				//Everything else happens on mouse up
				switch (buttonActions[i]->buttonMode) {
					case MOUSEUP:
						if (hit == 1) {
							//This button was released on, so run it's event
							if (buttonActions[i]->buttonAction != NULL && buttonActions[i]->enabled == 1) {
								actionsArr[actionsFuncCount] = buttonActions[i]->buttonAction;
								actionsFuncCount++;
							}
						}
						break;
					case CLICK:
						if (hit == 1 && buttonActions[i]->pressedOn == 1) {
							//printf("button is good to go!\n");
							//Button was clicked on, so run it's event
							if (buttonActions[i]->buttonAction != NULL && buttonActions[i]->enabled == 1) {
								actionsArr[actionsFuncCount] = buttonActions[i]->buttonAction;
								actionsFuncCount++;
							}
							
						}
						break;
					case OUTSIDECLICK:
						//Outside click is easy, we just do the opposite of click
						/// eEEE this causes a crash!!!???? 
						//printf("checking outside\n");
						if (hit == 0 && buttonActions[i]->pressedOn == 0) {
							printf("outside click is good to go!\n");
							//Button was clicked on, so run it's event
							if (buttonActions[i]->buttonAction != NULL && buttonActions[i]->enabled == 1) {
								actionsArr[actionsFuncCount] = buttonActions[i]->buttonAction;
								actionsFuncCount++;
							}
							
						}
						break;
					//Drag to come here...
					case MOUSEDRAG:
						//printf("mousedrag event! pressed: %i, released: %i\n", buttonActions[i]->pressedOn, hit );
						//printf("hitYPos: %i, releaseYPos: %i\n", currentMouseState.yDownPos, currentMouseState.yUpPos);
						//if (hit == 1 && buttonActions[i]->pressedOn == 1) {
						//Drags shouldn't care about when you let go, only where you press
						
						if (buttonActions[i]->pressedOn == 1) {
							//printf("checking drag now!\n");
							//Okay, we were dragged on, but let's see if we were dragged over the threshold
							if (buttonActions[i]->dragSettings.dragUpOffset > 0) {
								yDelta = currentMouseState.yDownPos - currentMouseState.yUpPos;
								//printf("got drag up delta %i !\n", yDelta);
								if (yDelta >= buttonActions[i]->dragSettings.dragUpOffset) {
									if (buttonActions[i]->dragSettings.dragUpAction != NULL && buttonActions[i]->enabled == 1) {
										actionsArr[actionsFuncCount] = buttonActions[i]->dragSettings.dragUpAction;
										actionsFuncCount++;
									}
								}
							
							}
							
							//Handle drag down
							if (buttonActions[i]->dragSettings.dragDownOffset > 0) {
								yDelta = currentMouseState.yUpPos - currentMouseState.yDownPos;
								//printf("got drag down delta %i VS %i!\n", yDelta, buttonActions[i]->dragSettings.dragDownOffset);
								if (yDelta >= buttonActions[i]->dragSettings.dragDownOffset) {
									
									if (buttonActions[i]->dragSettings.dragDownAction != NULL && buttonActions[i]->enabled == 1) {
										actionsArr[actionsFuncCount] = buttonActions[i]->dragSettings.dragDownAction;
										actionsFuncCount++;
									}
								}
							
							}
						
							//Handle drag left
							if (buttonActions[i]->dragSettings.dragLeftOffset > 0) {
								xDelta = currentMouseState.xUpPos - currentMouseState.xDownPos;
								
								//printf("got drag down delta %i VS %i!\n", yDelta, buttonActions[i]->dragSettings.dragDownOffset);
								if (xDelta >= buttonActions[i]->dragSettings.dragLeftOffset) {
									
									if (buttonActions[i]->dragSettings.dragLeftAction != NULL && buttonActions[i]->enabled == 1) {
										actionsArr[actionsFuncCount] = buttonActions[i]->dragSettings.dragLeftAction;
										actionsFuncCount++;
									}
								}
							
							}
							
							//Handle drag right
							if (buttonActions[i]->dragSettings.dragRightOffset > 0) {
								xDelta = currentMouseState.xDownPos - currentMouseState.xUpPos;
								//printf("got drag down delta %i VS %i!\n", yDelta, buttonActions[i]->dragSettings.dragDownOffset);
								if (xDelta >= buttonActions[i]->dragSettings.dragRightOffset) {
									
									if (buttonActions[i]->dragSettings.dragRightAction != NULL && buttonActions[i]->enabled == 1) {
										actionsArr[actionsFuncCount] = buttonActions[i]->dragSettings.dragRightAction;
										actionsFuncCount++;
									}
								}
							
							}
						
						}
						break;
						
				}
				
				//Reset all button flags on up
				buttonActions[i]->pressedOn = 0;
				buttonActions[i]->releasedOn = 0;
			}
		
		
		}
	}
	
	
	//Now run the actions
	for (i=0; i < actionsFuncCount; i++) {
		if (actionsArr[i] != NULL) {
			//run the action
			actionsArr[i]();
		}
	}
}


/***********************************************************************
* clearClockButtons()
*
* Clock buttons are defined by an external XML file and dynamically loaded
* at startup, so when we shut down we need to clean those up
*
***********************************************************************/ 

void clearClockButtons() {
	int j, i;
	
	/* Clear out the clock buttons */
	for (j=0; j < CLOCKMODEMAX; j++) {
		//// Clear main clock buttons
		for (i=0; i < clockButtonActions[j].buttonCount; i++) {
			if (clockButtonActions[j].buttonActions[i] != NULL) {
				free(clockButtonActions[j].buttonActions[i]);
				clockButtonActions[j].buttonActions[i] = NULL;
			}
		}
	
		if (clockButtonActions[j].buttonActions != NULL) {
			free(clockButtonActions[j].buttonActions);
			clockButtonActions[j].buttonActions = NULL;
		}
		clockButtonActions[j].buttonCount = 0;
		//// Done main clock buttons
	}
}


//************************** BUTTON HANDLER DEFINTIONS *******************************************//
/* These should be documented... */

/****************************************************************
* quitApp()
*
* Function to cause the program to exit by generating an SDL_Quit event
*****************************************************************/

void quitApp() {
	SDL_Event fakeQuit;
	
	fakeQuit.type = SDL_QUIT;
	
	//Push it onto the event cue
	SDL_PushEvent(&fakeQuit);
	

}

/******************************************************************
* selectMoodColor() 
* 
* Function to select a mood colour. This will probably become a two step process where
* the larger colour picker is opened first...
*******************************************************************/

void selectMoodColor() {
	Uint8 red, green, blue, alpha;		//Values of colour choosen
	Uint32 colour;								//Final colour value
	
	colour = getPixel(screen, currentMouseState.xCurrPos, currentMouseState.yCurrPos);
	SDL_GetRGBA(colour, screen->format, &red, &green, &blue, &alpha);

	//userPreferences.moodColor = SDL_MapRGBA(screen->format, red, green, blue, alpha);	
	userPreferences.moodRed = red;
	userPreferences.moodGreen = green;
	userPreferences.moodBlue = blue;
	
	if (clockMode == CLOCKMODEMOODPICKER) {
		//Update the display if needed
		drawMoodBox(screen);
	
	}
	//printf("user has selected colour: r:%i, g:%i, b:%i, a:%i\n", red, green, blue, alpha);

}

