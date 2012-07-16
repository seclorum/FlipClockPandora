

#include <stdlib.h>

#include "miscFunc.h"

/********** Misc. Functions
*
* Utilized as part of FlipClock C by Rob Williams and Ciro Ippoto
*
**********************************/



/**************************************************************************
*
* drawFadeOverlay(SDL_Surface *newScreen)
*
* Function to draw the "overlay fade" image onto the base buffer (used by fadeScreen)
**************************************************************************/

void drawFadeOverlay(SDL_Surface *newScreen) {
	Uint32 overlayColor;

	SDL_Surface *overlayBuffer; 
	
	overlayBuffer = SDL_CreateRGBSurface(SDL_SWSURFACE | SDL_SRCALPHA, screen->w, screen->h, screen->format->BitsPerPixel, 0,0,0,0);
	
	//Sneaky sneaky! Okay to make it work we have to draw the final complete screen first, THEN fade that image
	//(since it doesn't have an overall alpha value)
	
	overlayColor = SDL_MapRGBA(overlayBuffer->format, 0,0,0,0);
	SDL_FillRect(overlayBuffer, NULL, overlayColor);
	SDL_SetAlpha(overlayBuffer, SDL_SRCALPHA, 160);
	BMPShow(newScreen, overlayBuffer, 0,0);
		

	SDL_FreeSurface(overlayBuffer);

}


/**************************************************************************
*
* fadeScreenIn(SDL_Surface *newScreen)
*
* Function to fade a given screen into visibility
**************************************************************************/

void fadeScreenIn(SDL_Surface *newScreen, int useModalOverlay) {
	//the real trick here is that by setting the new screen's opacity to a low value we can just keep stacking it to make it visible
	int i;
	int frames;
	//Uint32 overlayColor;
	
	frames = 10;
	SDL_Surface *tempBuffer; //, *overlayBuffer; 
	
	
	
	tempBuffer = SDL_DisplayFormat(screen);
	
	if (useModalOverlay) {
		//Draw the black semi-transparent overlay
		drawFadeOverlay(tempBuffer);
	}
	
	/*
	overlayBuffer = SDL_CreateRGBSurface(SDL_SWSURFACE | SDL_SRCALPHA, screen->w, screen->h, screen->format->BitsPerPixel, 0,0,0,0);
	
	//Sneaky sneaky! Okay to make it work we have to draw the final complete screen first, THEN fade that image
	//(since it doesn't have an overall alpha value)
	
	if (useModalOverlay) {
		overlayColor = SDL_MapRGBA(overlayBuffer->format, 0,0,0,0);
		SDL_FillRect(overlayBuffer, NULL, overlayColor);
		SDL_SetAlpha(overlayBuffer, SDL_SRCALPHA, 160);
		BMPShow(tempBuffer, overlayBuffer, 0,0);
		
	}
	SDL_FreeSurface(overlayBuffer);
	*/
	
	
	
	//Now draw the new buffer ontop
	BMPShow(tempBuffer, newScreen, 0,0);
	
	//temp
	//SDL_SetAlpha(tempBuffer, SDL_SRCALPHA, 100);
	//BMPShow(screen, tempBuffer, 0,0);
	//return;
	
	for (i=0; i < frames + 1; i++) {

		
		SDL_SetAlpha(tempBuffer, SDL_SRCALPHA, (255 /frames) * i);
		//BMPShow(tempBuffer, newScreen, 0,0);
		
		BMPShow(screen, tempBuffer, 0,0);

		//usleep(500);
		//sleep(1);
	}
	SDL_SetAlpha(tempBuffer, SDL_SRCALPHA, 255);
	BMPShow(screen, tempBuffer, 0,0);

	
	
	SDL_FreeSurface(tempBuffer);


}

/**************************************************************************
*
* openMoodPicker()
*
* Function to open the mood picker; I didnt know where else this belonged so...
**************************************************************************/

void openMoodPicker() {
	SDL_Surface *tempBuffer;

	if (elements[MOODPICKERIMG] != NULL) {
		//Mood picker exists, so jump to it
		
		//Store the current clock mode
		lastClockMode = clockMode;
		
		//Go to modal mode so no other graphics/buttons over ride us
		clockMode = CLOCKMODEMOODPICKER;
	
		tempBuffer = drawStaticScreen(CLOCKMODEMOODPICKER); 
	
		fadeScreenIn(tempBuffer, 1);
		drawMoodBox(screen);
		
		if (tempBuffer != NULL) {
			SDL_FreeSurface(tempBuffer);
		}

	}
}

/**************************************************************************
*
* closeMoodPicker()
*
* Function to close the mood picker; I didnt know where else this belonged so...
**************************************************************************/

void closeMoodPicker() {
	SDL_Surface *tempBuffer;
	
	if (clockMode == CLOCKMODEMOODPICKER) {
		//Mood picker exists, so close it
		
	
		tempBuffer = drawStaticScreen(lastClockMode); 
	
		fadeScreenIn(tempBuffer, 1);
		
		//Save the current mood
		setUserPreferences();
		 
		clockMode = lastClockMode;
		
		if (tempBuffer != NULL) {
			SDL_FreeSurface(tempBuffer);
		}

	}
}

/****************************************************************************
*
* drawMoodBox(SDL_Surface *screenBuff);
*
* Draw the mood preview box on a mood picker screen
****************************************************************************/

void drawMoodBox(SDL_Surface *screenBuff) {
		SDL_Rect src, dest;
		Uint32 colorTest;


		colorTest = SDL_MapRGB(screenBuff->format, userPreferences.moodRed,userPreferences.moodGreen,userPreferences.moodBlue);
		
		//this should be handled by the theme definition, but for now jsut do statically
		dest.x = 640;
		dest.y = 100;
		dest.w = elements[MOODMASKIMG]->w;
		dest.h = elements[MOODMASKIMG]->h;
		
		src.x = 0;
		src.y = 0;
		src.w = elements[MOODMASKIMG]->w;
		src.h = elements[MOODMASKIMG]->h; 
		
		//Draw the mood picker
		SDL_FillRect(screenBuff, &dest, colorTest);
		printf("moodcolor is %i\n", colorTest);
		if (elements[MOODMASKIMG] == NULL) {
			printf("No mood mask!\n");
		}
		
		SDL_BlitSurface(elements[MOODMASKIMG], NULL, screenBuff, &dest);	
		SDL_UpdateRects(screenBuff, 1, &dest);
		
		
}


/**************************************************************************
*
* openFMRadioPicker()
*
* Function to open the FM Radio picker; again I didnt know where else this belonged so...
**************************************************************************/

void openFMRadioPicker() {
	SDL_Surface *tempBuffer;

	if (elements[FMRADIOPICKERIMG] != NULL && hasFMRadio == 1) {
		if (currentAlarmNum > -1 && currentAlarmNum <= userPreferences.userAlarmsCount) {
			if (userPreferences.userAlarms[currentAlarmNum]->alertMode == ALARMALERTMODEFMRADIO) {
				//FM picker exists, so jump to it
				
				//Store the current clock mode
				lastClockMode = clockMode;
				
				//Go to modal mode so no other graphics/buttons over ride us
				clockMode = CLOCKMODEFMRADIOPICKER;
			
				tempBuffer = drawStaticScreen(CLOCKMODEFMRADIOPICKER); 
			
				fadeScreenIn(tempBuffer, 1);
				//drawMoodBox(screen);
				
				if (tempBuffer != NULL) {
					SDL_FreeSurface(tempBuffer);
				}
			}
		}

	}
}

/**************************************************************************
*
* closeFMRadioPicker()
*
* Function to close the FM Radio picker; I didnt know where else this belonged so...
**************************************************************************/

void closeFMRadioPicker() {
	SDL_Surface *tempBuffer;
	
	if (clockMode == CLOCKMODEFMRADIOPICKER) {
		//Mood picker exists, so close it
		
	
		tempBuffer = drawStaticScreen(lastClockMode); 
	
		fadeScreenIn(tempBuffer, 1);
		
		//Save the current mood
		setUserPreferences();
		 
		clockMode = lastClockMode;
		
		if (tempBuffer != NULL) {
			SDL_FreeSurface(tempBuffer);
		}

	}
}

/**************************************************************************
*
* slideFMRadioPicker()
*
* Function to move the FM radio slider; I didnt know where else this belonged so...
**************************************************************************/

void slideFMRadioPicker() {
	double i;
	int newFreq;
	//printf("Sliding! Current pos is: %i %i\n",currentMouseState.xCurrPos, currentMouseState.yCurrPos); 
	
	if (fmSelector != NULL) {
		//an FM selector label is present
		
		//Recalculate the FM frequency
		i = currentMouseState.xCurrPos - fmSelector->labelRect.x;
		if (i < 0) {
			i =0;
		} else if (i > fmSelector->labelRect.w) {
			i = fmSelector->labelRect.w;
		}
		
		newFreq = ((i / fmSelector->labelRect.w) * 200) + 880;
		if (newFreq % 2 == 0) {
			newFreq++;
		}
		if (newFreq > 1079) {
			newFreq = 1079;
		}
		
		newFreq = newFreq * 100;
		
		userPreferences.userAlarms[currentAlarmNum]->fmFreq = newFreq;
		
		//drawImageLabel(screen, fmSelector, CLOCKMODEFMRADIOPICKER, NULL);
		//SDL_UpdateRects(screen, 1, &fmSelector->labelRect);
		drawAllLabels(screen, CLOCKMODEFMRADIOPICKER, 1);
		//printf("fm selector new freq %s\n", userPreferences.userAlarms[currentAlarmNum]->sound);
	}


}


/**************************************************************************
*
* tuneFMRadioUp()
*
* Function to manually incrememnt the FM frequency; I didnt know where else this belonged so...
**************************************************************************/

void tuneFMRadioUp() {
	int i;
	int newFreq;
		
	//Recalculate the FM frequency
	i = userPreferences.userAlarms[currentAlarmNum]->fmFreq;
	if (i + 200 <= 107900) {
		newFreq = i + 200;
	
		userPreferences.userAlarms[currentAlarmNum]->fmFreq = newFreq;
		
		drawAllLabels(screen, CLOCKMODEFMRADIOPICKER, 1);
		//printf("fm selector new freq %s\n", userPreferences.userAlarms[currentAlarmNum]->sound);
	}
}

/**************************************************************************
*
* tuneFMRadioUp()
*
* Function to manually incrememnt the FM frequency; I didnt know where else this belonged so...
**************************************************************************/

void tuneFMRadioDown() {
	int i;
	int newFreq;
		
	//Recalculate the FM frequency
	i = userPreferences.userAlarms[currentAlarmNum]->fmFreq;
	if (i - 200 >= 88500) {
		newFreq = i - 200;
	
		userPreferences.userAlarms[currentAlarmNum]->fmFreq = newFreq;
		
		drawAllLabels(screen, CLOCKMODEFMRADIOPICKER, 1);
		//printf("fm selector new freq %s\n", userPreferences.userAlarms[currentAlarmNum]->sound);
	}
}



/****************************************************************************
*
* drawButton(SDL_Surface *screenBuff, SDL_Surface *buttonObj, int buttonValue);
*
* Draw a button on the given screen buffer; int onOff determines whether the top part of the button obj
* is drawn (on), or the bottom part (off)
****************************************************************************/

void drawButton(SDL_Surface *screenBuff, SDL_Surface *buttonObj, SDL_Rect *targetRect, int buttonValue) {
	SDL_Rect buttonPos;
	
	//Draw the button base
	if (elements[BUTTONBASE] != NULL) {
		SDL_BlitSurface(elements[BUTTONBASE], NULL, screenBuff, targetRect);
	} 
	
	/* this is stupid, let's just assume all buttons are stacked vertically for now */
	/*
	if (buttonObj->w > buttonObj->h) {
		buttonPos.w = buttonObj->w /2;
		buttonPos.h = buttonObj->h;
		
		buttonPos.y = 0;
		if (onOff == 1) {
			buttonPos.x = 0;
		} else {
			buttonPos.x = buttonPos.w;
		}
	} else {*/
	
		buttonPos.w = buttonObj->w;
		if (elements[BUTTONBASE] != NULL) {
			buttonPos.h = elements[BUTTONBASE]->h;
		} else {
			//guess...
			buttonPos.h =buttonObj->h / 2;
		}
		
		buttonPos.x = 0;
		
		
		if (buttonValue * buttonPos.h < buttonObj->h) {
			buttonPos.y = buttonValue * buttonPos.h;
		} else {
			//Range is outside of button size so use the last one instead
			buttonPos.y =  buttonObj->h - buttonPos.h;
		}

	//}


	//Draw the button
	SDL_BlitSurface(buttonObj, &buttonPos, screenBuff, targetRect);
	
	//finally draw the mask if it exists
	if (elements[BUTTONMASK] != NULL) {
		SDL_BlitSurface(elements[BUTTONMASK], NULL, screenBuff, targetRect);
	}

}


/******************** MODAL OVERLAY/WINDOW FUNCS ************************************/

/****************************************************************
*
* createNoFocusButton(struct mouseAction *thisButton)
*
* Initialize a button to be a no-focus button
*****************************************************************/
void createNoFocusButton(struct mouseAction *thisButton, void (*closeFunc)()) {

	thisButton->enabled = 1;
	thisButton->buttonAction = closeFunc;
	thisButton->buttonMode = OUTSIDECLICK;
	
}



/******************** SLIDER INTERFACE *********************************************/
/* These functions are used to define a generic full screen modal slider, used
to allow a finger-friendly user to pick a value from a given range. The value
is updated in real time, and simply needs to be a pointer to whatever value you want to change;
in this way the code can be used for sliders to control any value using the same
generic interface
*/

/**************************************************************************
*
* setupSliderButtons()
*
* Setup all the buttons for a slider modal display. This function
* gets called once at theme setup/init
**************************************************************************/

void setupSliderButtons() {
	int firstBC = 0;
	
	int i;
	

	//Setup the basic button structs
	firstBC = themeButtonActions[CLOCKMODESLIDER].buttonCount;
	
		
	//Expand the theme-oriented normal mode buttons array to have space for our new buttons (3 of 'em)
	themeButtonActions[CLOCKMODESLIDER].buttonActions = (struct mouseAction **)realloc(themeButtonActions[CLOCKMODESLIDER].buttonActions, (themeButtonActions[CLOCKMODESLIDER].buttonCount + 3) * sizeof(struct mouseAction *));
	for (i =0; i < 3; i++) {
		/// allocate memory for one `mouseAction` 
		themeButtonActions[CLOCKMODESLIDER].buttonActions[themeButtonActions[CLOCKMODESLIDER].buttonCount] = (struct mouseAction *)malloc(sizeof(struct mouseAction));
		
		//Initialize the button entry
		initButton(themeButtonActions[CLOCKMODESLIDER].buttonActions[themeButtonActions[CLOCKMODESLIDER].buttonCount]);
	
		themeButtonActions[CLOCKMODESLIDER].buttonCount++;
		
	}

	//Setup the nofocus button, which is always button 0 for modal modes
	createNoFocusButton(themeButtonActions[CLOCKMODESLIDER].buttonActions[firstBC], &closeSlider);

	//No focus button is based on the size of SLIDERBASE element, which is centered on screen
	if (elements[SLIDERBASE] != NULL) {
		themeButtonActions[CLOCKMODESLIDER].buttonActions[firstBC]->buttonRect.x = (screen->w - elements[SLIDERBASE]->w) /2;
		themeButtonActions[CLOCKMODESLIDER].buttonActions[firstBC]->buttonRect.y = (screen->h - elements[SLIDERBASE]->h) /2;
		themeButtonActions[CLOCKMODESLIDER].buttonActions[firstBC]->buttonRect.w = elements[SLIDERBASE]->w;
		themeButtonActions[CLOCKMODESLIDER].buttonActions[firstBC]->buttonRect.h = elements[SLIDERBASE]->h;
		//printf("outside click is x %i, y %i, w %i, h%i\n", (screen->w - elements[SLIDERBASE]->w) /2, (screen->h - elements[SLIDERBASE]->h) /2, elements[SLIDERBASE]->w,elements[SLIDERBASE]->h); 
	
	}
	
	//Next setup the slider button itself
	if (sliderPointerLabel != NULL) {
		//Track mosue movements in this mode
		mouseMoveEnabled[CLOCKMODESLIDER]=1;
	
		themeButtonActions[CLOCKMODESLIDER].buttonActions[firstBC + 1]->buttonMode = MOUSESLIDE;
		//The sliding area is the size of the slider base
		themeButtonActions[CLOCKMODESLIDER].buttonActions[firstBC + 1]->buttonRect.x = (screen->w - elements[SLIDERBASE]->w) /2;
		themeButtonActions[CLOCKMODESLIDER].buttonActions[firstBC + 1]->buttonRect.y = (screen->h - elements[SLIDERBASE]->h) /2;
		themeButtonActions[CLOCKMODESLIDER].buttonActions[firstBC + 1]->buttonRect.w = elements[SLIDERBASE]->w;
		themeButtonActions[CLOCKMODESLIDER].buttonActions[firstBC + 1]->buttonRect.h = elements[SLIDERBASE]->h;
		
		themeButtonActions[CLOCKMODESLIDER].buttonActions[firstBC + 1]->enabled = 1;
		themeButtonActions[CLOCKMODESLIDER].buttonActions[firstBC + 1]->buttonAction = &slideSlider;
		
	}
	
	
	
	
	
	//******** Done buttons, now setup labels 
	
	//The slider picker is the most important label here
	
	//Setup image labels first
	if (sliderPointerLabel != NULL) {
		memcpy(&sliderPointerLabel->labelRect, &themeButtonActions[CLOCKMODESLIDER].buttonActions[firstBC + 1]->buttonRect, sizeof(themeButtonActions[CLOCKMODESLIDER].buttonActions[firstBC + 1]->buttonRect)); 
	}
	
	//******** Now setup Text Labels *****************//
		

}


/**************************************************************************
*
* openSlider(int *dataField, int minVal, int maxVal)
*
* Function to open the slider and set it up
**************************************************************************/

void openSlider(char *modalLabel, int *dataField, int minVal, int maxVal) {
	SDL_Surface *tempBuffer;

	if (elements[SLIDERBASE] != NULL && sliderPointerLabel != NULL && clockMode != CLOCKMODESLIDER) {
		//To do this we use the special "CLOCKMODESLIDER" clock mode
				
		//Store a reference to the target variable
		bzero(modalStrLabel, sizeof(modalStrLabel));
		sprintf(modalStrLabel, "%s", modalLabel);
		modalIntVar = dataField;
		modalIntMin = minVal;
		modalIntMax = maxVal;
				
				
		//Store the current clock mode
		lastClockMode = clockMode;
				
		//Go to modal mode so no other graphics/buttons over ride us
		clockMode = CLOCKMODESLIDER;
			
		tempBuffer = drawStaticScreen(CLOCKMODESLIDER);
		
		
		
		 
	
		fadeScreenIn(tempBuffer, 0);
		
		if (tempBuffer != NULL) {
			SDL_FreeSurface(tempBuffer);
		}


	}
}

/**************************************************************************
*
* closeSlider()
*
* Function to close the slider picker; I didnt know where else this belonged so...
**************************************************************************/

void closeSlider() {
	SDL_Surface *tempBuffer;
	
	if (clockMode == CLOCKMODESLIDER) {
		//Reset modal globals
		modalIntVar = NULL;
		modalIntMin = 0;
		modalIntMax = 100;
		
		
		//Slider is open, so close it
		tempBuffer = drawStaticScreen(lastClockMode); 
	
		fadeScreenIn(tempBuffer, 1);
		
		//Save the new value
		setUserPreferences();
		 
		clockMode = lastClockMode;
		
		if (tempBuffer != NULL) {
			SDL_FreeSurface(tempBuffer);
		}

	}
}

/**************************************************************************
*
* slideSlider()
*
* Function to move the generic slider; I didnt know where else this belonged so...
**************************************************************************/

void slideSlider() {
	double i;
	int newFreq;
	//printf("Sliding! Current pos is: %i %i\n",currentMouseState.xCurrPos, currentMouseState.yCurrPos); 
	
	if (sliderPointerLabel != NULL) {
		//a slider pointer label is present
		
		//Recalculate the slider value
		i = currentMouseState.xCurrPos - sliderPointerLabel->labelRect.x - (sliderPointerLabel->imageObj->w /2);
		if (i < 0) {
			i =0;
		} else if (i > sliderPointerLabel->labelRect.w) {
			i = sliderPointerLabel->labelRect.w;
		}
		
		newFreq = ((i / (sliderPointerLabel->labelRect.w - sliderPointerLabel->imageObj->w)) * (modalIntMax - modalIntMin)) + modalIntMin ;

		if (newFreq > modalIntMax) {
			newFreq = modalIntMax;
		} else if (newFreq < modalIntMin) {
			newFreq = modalIntMin;
		}

		*modalIntVar = newFreq;

		//drawImageLabel(screen, fmSelector, CLOCKMODEFMRADIOPICKER, NULL);
		//SDL_UpdateRects(screen, 1, &fmSelector->labelRect);
		drawAllLabels(screen, CLOCKMODESLIDER, 1);
		//printf("new slider freq %i, modalval %i, mainval %i\n", newFreq, *modalIntVar, userPreferences.insomniacDim);
	}
}


//*********** Specific slider implementations ***************//

/**************************************************************************
*
* openAlarmMaxVolSlider()
*
* pseudo function to open the alarm max vol picker slider
**************************************************************************/

void openAlarmMaxVolSlider() {
	openSlider("Max Alarm Vol", &userPreferences.maxAlarmVol, 0, 100);

}


/**************************************************************************
*
* openInsomniacLevelSlider()
*
* pseudo function to open the insomniac picker slider
**************************************************************************/

void openInsomniacLevelSlider() {
	openSlider("Insomniac Brightness", &userPreferences.insomniacDim, MIN_BRIGHTNESS_TOGGLE, 255);

}


/******************** DONE SLIDER INTERFACE *****************************************/


/******************** TIMER MODE RELATED FUNCTIONS **********************************/
/* These functions are used by "timer mode" to control count downs and timing
 and are fairly independent of the clock itself
 */
 
/**************************************************************************
*
* startTimer()
*
* Function to start the timer running (sets starting point as NOW)
**************************************************************************/

void startTimer() {
	
	timerStartVal= time(NULL);
	oldTimerVal = timerStartVal;
	timerRunning = 1;
	updateTimerDigits(screen, 1);
}

/**************************************************************************
*
* stopTimer()
*
* Function to stop the timer running (sets starting point as NOW)
**************************************************************************/

void stopTimer() {
	timerPauseTime = time(NULL);
	timerRunning = 0;
}

/**************************************************************************
*
* pauseToggleTimer()
*
* Function to toggle the timer running (sets starting point as NOW)
**************************************************************************/

void pauseToggleTimer() {
	int timeOffset = 0;
	
	if (timerRunning == 1) {
		//Stop the timer
		stopTimer();
	} else {
		//Adjust time based on pause time
		timeOffset = time(NULL) - timerPauseTime;
		
		if (time(NULL) > timerStartVal) {
			timerStartVal += timeOffset;
		} else {
			timerStartVal -= timeOffset;
		}
		timerRunning = 1;
		
	}
}


/***************** DONE TIMER MODE RELATED FUNCTIONS ************************/


/****************************************************************************
*
* void drawMoodOverlayByImage(SDL_Surface *screenBuff, SDL_Surface *overlayImage, SDL_Rect *targetPos);
*
* Draw the mood overlay onto the current screen
****************************************************************************/

void drawMoodOverlayByImage(SDL_Surface *screenBuff, SDL_Surface *overlayImage, SDL_Rect *targetPos) {
		SDL_Rect dest;
		Uint32 colorTest;
		SDL_Surface *tempBuffer, *tempBuffer2;
		
		
		//make sure a mood overlay is defined for this mode
		if (overlayImage != NULL) {

			//this should be handled by the theme definition, but for now jsut do statically
			if (targetPos == NULL) {
				dest.x = 0;
				dest.y = 0;
				dest.w = screenBuff->w;
				dest.h = screenBuff->h;
			} else {
				dest.x = targetPos->x;
				dest.y = targetPos->y;
				dest.w = targetPos->w;
				dest.h = targetPos->h;
			}
			
			//Create a temp buffer with RGBA format. Then we blit the mood overlay which should also be RGBA, meaning the overlay will act as a mask...
			tempBuffer = SDL_DisplayFormatAlpha(overlayImage);

			//tempBuffer = SDL_CreateRGBSurface(SDL_SWSURFACE | SDL_SRCALPHA, screenBuff->w, screenBuff->h, screenBuff->format->BitsPerPixel, 0,0,0,0);
			tempBuffer2 = SDL_DisplayFormatAlpha(overlayImage);
						
			colorTest = SDL_MapRGBA(tempBuffer->format, userPreferences.moodRed,userPreferences.moodGreen,userPreferences.moodBlue, 255);
			//SDL_SetAlpha(tempBuffer, SDL_SRCALPHA, 160);
			SDL_FillRect(tempBuffer, NULL, colorTest);

			SDL_BlitSurface(tempBuffer, NULL, tempBuffer2, NULL);

			SDL_BlitSurface(tempBuffer2, NULL, screenBuff, &dest);

			//BMPShow(screenBuff, tempBuffer2, 0,0);
			
			SDL_FreeSurface(tempBuffer);
			SDL_FreeSurface(tempBuffer2);

	}
}


/**************************************************************************
* getPixel(SDL_Surface *surface, int x, int y)
*
* Function to grab a pixel from the given surface that lies at the specificed co-ordinates. The pixel
* can then be read/worked with using the SDL_getRGB/SDL_getRGBA functions.
*
* as per http://www.libsdl.org/cgi/docwiki.cgi/Pixel_Access
*
**************************************************************************/

Uint32 getPixel(SDL_Surface *surface, int x, int y)
{
    //Lock the surface first!
	SDL_LockSurface(surface);
	
	int bpp = surface->format->BytesPerPixel;
    /* Here p is the address to the pixel we want to retrieve */
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

	SDL_UnlockSurface(surface);
	
    switch(bpp) {
    case 1:
        return *p;

    case 2:
        return *(Uint16 *)p;

    case 3:
        if(SDL_BYTEORDER == SDL_BIG_ENDIAN)
            return p[0] << 16 | p[1] << 8 | p[2];
        else
            return p[0] | p[1] << 8 | p[2] << 16;

    case 4:
        return *(Uint32 *)p;

    default:
        return 0;       /* shouldn't happen, but avoids warnings */
    }
}


/*************************************************************************
* Get the basename of a path                                             *
*************************************************************************/

char *baseName (char * path) {

        int separator;  /* Default path separator */
        char *subPath;
        //char *newPath;

        separator = '/';

		//newPath = calloc(strlen(path) + 2, sizeof(char));

        subPath = strrchr(path, separator);

		if (subPath == NULL) {
			return path;			
		} else {
			subPath++;
			return subPath;
		}

		/*
        //Check to see if subPath is empty
        if (strlen(subPath) < 2) {
                //Do it again
                //sprintf(path, "%s", path - strlen(subPath));


                strncpy(newPath, path, strlen(path) - strlen(subPath));

                newPath[strlen(path) - strlen(subPath)] = '\0';

                subPath = strrchr(newPath, separator);

        } else {
			subPath = path;
		}
        subPath++;

		free(newPath);
        //printf("basename: %s\n", subPath);

        return subPath;*/
}
