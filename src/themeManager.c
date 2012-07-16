/*

Flip Clock Theme Manager Functionality
Part of FlipClock C SDL for Maemo.

This library defines global vars and methods that are used to control themes for FlipClock

-Rob Williams, Aug 11, 2009.


*/

#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <stdio.h>
#include <ctype.h>	//for tolower() function

//Include XML library if not already present
#ifndef XMLFUNC_H
#include "xmlFunc.c"
#endif



//********************** STATIC DEF'S *****************************//


//********************** DONE STATIC DEF's ************************//


//********************** Structure Definitions ***********************//




//********************** Done Struct Defs *************************//


//********************** Global variables *************************//






//********************** Done global variables *******************//






//********************** Function Definitions *********************//



/***********************************************************************
* preloadThemeGraphics()
*
* Function to load all of the graphics and images associated with a given clock
* screen display (digits, animations, elements)
*
***********************************************************************/ 


void preloadThemeGraphics()
{
	SDL_Surface *tempBuffer;		//Surface to store a loaded image into before converting it
	
	char fullPath[512];	//Full path to files
	char elementsArr[MAXGRAPHICELEMENTS][255];	//Image elements
	
	char moodImgsArr[CLOCKMODEMAX][255];			//Mood images for clock screens
	
	int i = 0;			//Good ol' i
	
	//Load numbers
	for (i=0; i < 11; i++) {
		bzero(fullPath, sizeof(fullPath));
		sprintf(fullPath, "%s%i.png", currentTheme.themePath, i);
		//printf("97: %s\n", fullPath);
		tempBuffer = IMG_Load(fullPath);
		if (tempBuffer != NULL) {
			//printf("loaded digit %s\n", fullPath);
			numbers[i] = SDL_DisplayFormatAlpha(tempBuffer);
			SDL_FreeSurface(tempBuffer);
		}
		//numbers[i] = IMG_Load(fullPath); 
	}

	//Try to load animations for this screen
	animationCount=0;
	bzero(fullPath, sizeof(fullPath));
	sprintf(fullPath, "%sdigitAni%i.png", currentTheme.themePath, animationCount);
	
	while (fileExists(fullPath)) {
		//Create a new surface
		tempBuffer = IMG_Load(fullPath);
		if (tempBuffer != NULL) {
			digitAnimation[animationCount] = SDL_DisplayFormatAlpha(tempBuffer);
			SDL_FreeSurface(tempBuffer);
			animationCount++;
		}
		else {
			printf ("Animation digit cannot be loaded...\n");
		}
				
		bzero(fullPath, sizeof(fullPath));
		sprintf(fullPath, "%sdigitAni%i.png", currentTheme.themePath, animationCount);
		if (animationCount > 10) break;    //Temp till we get dynamic structures involved here
	}

	//Done loading animations
	for (i=0; i < MAXGRAPHICELEMENTS; i++) {
		bzero(elementsArr[i], sizeof(elementsArr[i]));
	}
    sprintf(elementsArr[BACKGROUND], "fondo");
    sprintf(elementsArr[SECONDSBAR], "barra");
    sprintf(elementsArr[SECONDSTICK], "secs");
	sprintf(elementsArr[AMICON], "amIcon");
    sprintf(elementsArr[PMICON], "pmIcon");
	sprintf(elementsArr[ALARMSETTINGSBACKGROUND], "fondoAlarmSettings");
	sprintf(elementsArr[MOODPICKERIMG], "moodPicker");
	sprintf(elementsArr[MOODMASKIMG], "moodMask");
	sprintf(elementsArr[FMRADIOPICKERIMG], "fmRadioPicker");
	sprintf(elementsArr[CLOCKSETTINGSBACKGROUND], "fondoClockSettings");
	//
	
	sprintf(elementsArr[ALARMDIGITSCROLLER], "alarmDigitScroller");
	sprintf(elementsArr[ALARMDIGITMASK], "alarmDigitMask");
	sprintf(elementsArr[ALARMDAYOFF], "dayOff");
	sprintf(elementsArr[ALARMDAYON], "dayOn");
	sprintf(elementsArr[ALARMDAYSELECTOR], "daySelector");
	sprintf(elementsArr[BUTTONBASE], "buttonBase");
	sprintf(elementsArr[BUTTONMASK], "buttonMask");
	sprintf(elementsArr[BUTTONAMPM], "swAMPM");
	sprintf(elementsArr[BUTTONONOFF], "swOffOn");
	sprintf(elementsArr[BUTTONALERTMODE], "swAlertMode");
	sprintf(elementsArr[BUTTONONCELOOP], "swOnceLoop");
	sprintf(elementsArr[BUTTONSNOOZETIME], "swSnoozeTime");
	sprintf(elementsArr[BUTTONMILITARYTIME], "sw1224");
	sprintf(elementsArr[BUTTONALARMSTYLE], "swAlarmStyle");
	sprintf(elementsArr[BUTTONFIXEDRANDOM], "swFixedRandom");
	
	sprintf(elementsArr[SLIDERBASE], "sliderBase");
	sprintf(elementsArr[TIMERBACKGROUND], "fondoTimer");
	
	//Load graphic elements (non digits)
	for (i=0; i < MAXGRAPHICELEMENTS; i++) {
		elements[i] = NULL;
		bzero(fullPath, sizeof(fullPath));
		sprintf(fullPath, "%s%s.png", currentTheme.themePath, elementsArr[i]);
		
		tempBuffer = IMG_Load(fullPath);
		//printf("loaded %s\n", fullPath);
		if (tempBuffer != NULL) {
			elements[i] = SDL_DisplayFormatAlpha(tempBuffer);
			SDL_FreeSurface(tempBuffer);
		} else {
			//try a JPEG instead
			bzero(fullPath, sizeof(fullPath));
			sprintf(fullPath, "%s%s.jpg", currentTheme.themePath, elementsArr[i]);
			
			tempBuffer = IMG_Load(fullPath);
			printf("loaded %s\n", fullPath);
			if (tempBuffer != NULL) {
				elements[i] = SDL_DisplayFormatAlpha(tempBuffer);
				SDL_FreeSurface(tempBuffer);
			}
		
		}
		
	}
	
	
	//****** Load mood images if available
	for (i=0; i < CLOCKMODEMAX; i++) {
		bzero(moodImgsArr[i], sizeof(moodImgsArr[i]));
	}
	sprintf(moodImgsArr[CLOCKMODENORMAL], "moodNormal.png");
	sprintf(moodImgsArr[CLOCKMODEWINDOWED], "moodWindowed.png");
	sprintf(moodImgsArr[CLOCKMODEALARMSETTINGS], "moodAlarmSettings.png");
	sprintf(moodImgsArr[CLOCKMODECLOCKSETTINGS], "moodClockSettings.png");
	sprintf(moodImgsArr[CLOCKMODEMOODPICKER], "moodMoodPicker.png");
	sprintf(moodImgsArr[CLOCKMODEHELP], "moodHelp.png");	

	//Load mood images
	for (i=0; i < CLOCKMODEMAX; i++) {
		currentTheme.moodImages[i] = NULL;
		if (strlen(moodImgsArr[i]) > 0) {
			bzero(fullPath, sizeof(fullPath));
			sprintf(fullPath, "%s%s", currentTheme.themePath, moodImgsArr[i]);
			
			tempBuffer = IMG_Load(fullPath);
			if (tempBuffer != NULL) {
				currentTheme.moodImages[i] = SDL_DisplayFormatAlpha(tempBuffer);
				SDL_FreeSurface(tempBuffer);
			}
		}
	}
	
	
	//Load theme fonts if present
	bzero(fullPath, sizeof(fullPath));
	sprintf(fullPath, "%sprimaryFont.ttf", currentTheme.themePath);
	
	for (i=0; i < FONTSIZEMAX; i++) {
		currentTheme.themeFonts[i] = NULL;
	}
	
	if (fileExists(fullPath)) {
		//Load the fonts
		currentTheme.themeFonts[FONTSIZESMALL] = TTF_OpenFont(fullPath, 20);		//Small
		currentTheme.themeFonts[FONTSIZEMEDIUM] = TTF_OpenFont(fullPath, 30);	//Medium
		currentTheme.themeFonts[FONTSIZELARGE] = TTF_OpenFont(fullPath, 40);	//Large
		currentTheme.themeFonts[FONTSIZEHUGE] = TTF_OpenFont(fullPath, 50);	//Huge
	}
	

}

/********* Done preloadThemeGraphics ************************/

/***********************************************************************
* clearThemeGraphics()
*
* Function to unload all of the graphics and images associated with a given clock
* screen display (digits, animations, elements)
*
***********************************************************************/ 

void clearThemeGraphics() {
	int i, j;
	//Clean up SDL surfaces
	for (i=0; i < 11; i++) {
		if (numbers[i] != NULL) {
			SDL_FreeSurface(numbers[i]);
			numbers[i] = NULL;
		}
	}

	for (i=0; i < MAXGRAPHICELEMENTS; i++) {
		if (elements[i] != NULL) {
			SDL_FreeSurface(elements[i]);
			elements[i] = NULL;
		}
	}
	
	for (i=0; i < 10; i++) {
		if (digitAnimation[i] != NULL) {
			SDL_FreeSurface(digitAnimation[i]);
			digitAnimation[i] = NULL;
		}
	}
	
	
	for (j=0; j < CLOCKMODEMAX; j++) {
		if (currentTheme.moodImages[j] != NULL) {
			SDL_FreeSurface(currentTheme.moodImages[j]);
			currentTheme.moodImages[j] = NULL;
		}
		
		//Clean up labels
		//Text labels first
		for (i=0; i < currentTheme.textLabels[j].labelCount; i++) {
			if (currentTheme.textLabels[j].labelArr[i] != NULL) {
				//labelValue and labelKey are the only dynamic ones
				if (currentTheme.textLabels[j].labelArr[i]->labelValue != NULL) {
					free(currentTheme.textLabels[j].labelArr[i]->labelValue);
				}
				
				if (currentTheme.textLabels[j].labelArr[i]->labelKey != NULL) {
					free(currentTheme.textLabels[j].labelArr[i]->labelKey);
				}
				
				//Free the entry itself
				free(currentTheme.textLabels[j].labelArr[i]);
				currentTheme.textLabels[j].labelArr[i] = NULL;
			
			}
		}
		if (currentTheme.textLabels[j].labelArr != NULL) {
			free(currentTheme.textLabels[j].labelArr);
			currentTheme.textLabels[j].labelArr = NULL;
		}
		currentTheme.textLabels[j].labelCount = 0;
		//Done cleaning up text labels
		

		
		//Clean up image labels
		for (i=0; i < currentTheme.imageLabels[j].labelCount; i++) {
			if (currentTheme.imageLabels[j].labelArr[i] != NULL) {
				//Image obj is the only one we need to clear
				if (currentTheme.imageLabels[j].labelArr[i]->imageObj != NULL) {
					SDL_FreeSurface(currentTheme.imageLabels[j].labelArr[i]->imageObj);
					currentTheme.imageLabels[j].labelArr[i]->imageObj = NULL;
				}
				
				if (currentTheme.imageLabels[j].labelArr[i]->labelKey != NULL) {
					free(currentTheme.imageLabels[j].labelArr[i]->labelKey);
				}
				
				
				
				//Free the entry itself
				free(currentTheme.imageLabels[j].labelArr[i]);
				currentTheme.imageLabels[j].labelArr[i] = NULL;
			
			}
		}

		if (currentTheme.imageLabels[j].labelArr != NULL) {
			free(currentTheme.imageLabels[j].labelArr);
			currentTheme.imageLabels[j].labelArr = NULL;
		}
		currentTheme.imageLabels[j].labelCount = 0;
		
		//Done cleaning up image labels
		
	}
	
	//Clean up fonts
	for (i=0; i < FONTSIZEMAX; i++) {
		if (currentTheme.themeFonts[i] != NULL) {
			
			TTF_CloseFont(currentTheme.themeFonts[i]);
			currentTheme.themeFonts[i] = NULL;
			
		}
		currentTheme.themeFonts[i] = NULL;
	}


	
	
	

	
}

/********* Done clearThemeGraphics ************************/

/****************************************************************
*
* initTextLabel(struct textLabel *thisLabel)
*
* Initialize a text label struct to be zero'd out
*****************************************************************/
void initTextLabel(struct textLabel *thisLabel) {
	int i;
	thisLabel->labelValue = NULL;
	for (i=0; i < 4; i++) {
		thisLabel->textColor[i]=0;
		thisLabel->textBGColor[i] = 0;
	}
	thisLabel->usesBGColor = 0;
	memset(&thisLabel->textSize, 0, sizeof(thisLabel->textSize));	
	thisLabel->usesMoodColor = 0;
	thisLabel->labelKey = NULL;
}

/****************************************************************
*
* initImageLabel(struct imageLabel *thisLabel)
*
* Initialize an imageLabel struct to be zero'd out
*****************************************************************/
void initImageLabel(struct imageLabel *thisLabel) {

	thisLabel->imageObj = NULL;
	thisLabel->labelKey = NULL;
	thisLabel->usesMoodColor = 0;
}


/***********************************************************************
* clearThemeButtons()
*
* Since buttons associated with a given theme are stored in dynamic arrays
* (one array per clock mode), we need to make sure these arrays are
* cleared up and free'd. This function takes care of that.
*
***********************************************************************/ 

void clearThemeButtons() {
	int j, i;
	
	/* Clear out the theme buttons */
	for (j=0; j < CLOCKMODEMAX; j++) {

		//// Clear theme-created buttons
		for (i=0; i < themeButtonActions[j].buttonCount; i++) {
			if (themeButtonActions[j].buttonActions[i] != NULL) {
				free(themeButtonActions[j].buttonActions[i]);
				themeButtonActions[j].buttonActions[i] = NULL;
			}
		}
	
		if (themeButtonActions[j].buttonActions != NULL) {
			free(themeButtonActions[j].buttonActions);
			themeButtonActions[j].buttonActions = NULL;
		}
		themeButtonActions[j].buttonCount = 0;
		
	
	}
	/* Done the theme buttons */
	
	//Clean up mouse flags
	bzero(mouseMoveEnabled, sizeof(mouseMoveEnabled));

}

/********* Done clearThemeButtons ************************/

/***********************************************************************
* setupThemeDigits()
*
* Function to setup the co-ordinates and controllers of digits for the
* current theme
*
***********************************************************************/ 

void setupThemeDigits () {
	int i;
		
	//Defaults
	for (i=0; i < 5; i++ ) {
	
		clockDigits[i].xPos = currentTheme.digitPos[i].x;
		clockDigits[i].yPos = currentTheme.digitPos[i].y;
		clockDigits[i].lastValue = 314;
		
		clockDigits[i].aniFrame = 0;

	}
	
	
	//Set specific locations of digits... should be pulled from a theme file later?	
	clockDigits[FIRSTHOUR].xPos = currentTheme.digitPos[FIRSTHOUR].x;
	clockDigits[SECONDHOUR].xPos = currentTheme.digitPos[SECONDHOUR].x; //193;
	clockDigits[FIRSTMINUTE].xPos = currentTheme.digitPos[FIRSTMINUTE].x;   //415;
	clockDigits[SECONDMINUTE].xPos = currentTheme.digitPos[SECONDMINUTE].x; //605;
	
		//printf("cleared to %i\n", clockButtonActions[CLOCKMODENORMAL].buttonCount);
}

/********* Done setupThemeDigits ************************/

/***********************************************************************
* themeChangeOverlay(show)
*
*	Display (show = 1) or hide (show = 0) theme change overlay message
************************************************************************/

void themeChangeOverlay(int show) {
	Uint32 overlayColor;
	SDL_Surface *tempBuffer;
	
	if (show == 1) {
		if (appElements[THEMECHANGETEXT] != NULL) {
			//Create a temp buffer based on the current screen
			tempBuffer = SDL_DisplayFormatAlpha(screen);
			printf("%i width %i height\n", tempBuffer->w, tempBuffer->h);
			//Draw the overlay
			overlayColor = SDL_MapRGBA(tempBuffer->format, 0,0,0, 160);
	
			SDL_FillRect(tempBuffer, NULL, overlayColor);
			
			//Draw the wait msg
			BMPShow(tempBuffer, appElements[THEMECHANGETEXT], 300, 200);
			BMPShow(screen, tempBuffer, 0,0);
			
			SDL_FreeSurface(tempBuffer);
			//SDL_UpdateRect(screen, 0,0,screen->w, screen->h);
		}
	}

}

/****************************************************************************
*
* drawMoodOverlay(SDL_Surface *screenBuff, int currClockMode);
*
* Draw the mood overlay onto the current screen
****************************************************************************/

void drawMoodOverlay(SDL_Surface *screenBuff, int currClockMode, SDL_Rect *targetPos) {
		SDL_Rect dest;
		Uint32 colorTest;
		SDL_Surface *tempBuffer, *tempBuffer2;
		
		
		//make sure a mood overlay is defined for this mode
		if (currentTheme.moodImages[currClockMode] != NULL) {
			printf("Drawing mood overlay\n");
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
			tempBuffer = SDL_DisplayFormatAlpha(screenBuff);

			//tempBuffer = SDL_CreateRGBSurface(SDL_SWSURFACE | SDL_SRCALPHA, screenBuff->w, screenBuff->h, screenBuff->format->BitsPerPixel, 0,0,0,0);
			tempBuffer2 = SDL_DisplayFormatAlpha(currentTheme.moodImages[currClockMode]);
						
			colorTest = SDL_MapRGBA(tempBuffer->format, userPreferences.moodRed,userPreferences.moodGreen,userPreferences.moodBlue, 255);
			//SDL_SetAlpha(tempBuffer, SDL_SRCALPHA, 160);
			SDL_FillRect(tempBuffer, &dest, colorTest);

			SDL_BlitSurface(tempBuffer, &dest, tempBuffer2, &dest);

			SDL_BlitSurface(tempBuffer2, &dest, screenBuff, &dest);

			//BMPShow(screenBuff, tempBuffer2, 0,0);
			
			SDL_FreeSurface(tempBuffer);
			SDL_FreeSurface(tempBuffer2);

	}
}

/***********************************************************************
* drawThemeBackground()
*
* Function to draw the theme background
*
***********************************************************************/

void drawThemeBackground(SDL_Surface *screenBuf, int newClockMode, SDL_Rect *targetPos) {
	Uint32 moodColor;
	SDL_Surface *bgImage;
	SDL_Rect target;
	
	bgImage = NULL;
	
	//No backgrounds for alarm mode
	if (newClockMode == CLOCKMODEALARMRUNNING) {
		return;
	}
	
	
	
	if (targetPos == NULL) {
		target.w = screenBuf->w;
		target.h = screenBuf->h;
		target.x = 0;
		target.y = 0;
	} else {
		target.w = targetPos->w;
		target.h = targetPos->h;
		target.x = targetPos->x;
		target.y = targetPos->y;
	}
	
	//Used the cache theme BG Surface if available
	if (themeBGBuffer != NULL) {
		SDL_BlitSurface(themeBGBuffer, &target, screenBuf, &target);
	} else {
	
	
		if (currentTheme.usesAlphaBG == 1) {
			//Draw mood colour first... let's just do a static one for now
			moodColor = SDL_MapRGB(screen->format, userPreferences.moodRed,userPreferences.moodGreen,userPreferences.moodBlue);
			SDL_FillRect(screenBuf, targetPos, moodColor);
		
		}
		
		//Draw the BG for the mode
		switch (newClockMode) {
			case CLOCKMODENORMAL:
				bgImage = elements[BACKGROUND];
				break;
			case CLOCKMODEALARMSETTINGS:
				bgImage = elements[ALARMSETTINGSBACKGROUND];
				break;
			case CLOCKMODECLOCKSETTINGS:
				bgImage = elements[CLOCKSETTINGSBACKGROUND];
				break;
			case CLOCKMODEFMRADIOPICKER:
				bgImage = elements[FMRADIOPICKERIMG];
				break;
			case CLOCKMODETIMER:
				bgImage = elements[TIMERBACKGROUND];
				break;
		}
		
		if (bgImage == NULL) {
			printf("no background to draw!\n");
			debugEntry("No background to redraw!");
		} else {
			printf("drawing background\n");
			SDL_BlitSurface(bgImage, &target, screenBuf, &target);
			//BMPShow(screenBuf, bgImage,0,0);        //Background
		}
		
		//Now draw mood overlay
		//drawMoodOverlay(screenBuf, newClockMode, &target);
	
		//Save the screenBuf as the cached BG
		themeBGBuffer = SDL_ConvertSurface(screenBuf, screenBuf->format, screenBuf->flags);
	
	
	}
	//Update the screen
	if (targetPos == NULL) {
		SDL_UpdateRects(screenBuf, 1, &target);
	}
}



/***********************************************************************
* mapButtonKeyToAction
*
* Function to convert a buttonKey (defined in theme XML file) into an
* actual function to be called for a specific action
*
***********************************************************************/

void (*mapButtonKeyToAction(char *buttonKey)) ()
{
	/* Possible values for buttons keys are:
		"quit" - Exits the app
		"nextTheme" - Rolls through themes incrementally
		
		"gotoSettings" - Change to the settings screen
		"pickMood" - Open the mood picker
		"selectMoodColor" - The area that clicking on will sample the mood colour from
		"closeMoodPicker" - Close the mood picker 
		
		
		
		more to come...
	
	
	
	*/
	
	if (!strcmp(buttonKey, "quit")) {
		return &quitApp;
	} else if (!strcmp(buttonKey, "nextTheme")) {
		return &cycleThemeNext;
	} else if (!strcmp(buttonKey, "previousTheme")) {
		return &cycleThemePrevious;
	} else if (!strcmp(buttonKey, "gotoSettings")) {
		return &gotoSettings;
	} else if (!strcmp(buttonKey, "gotoNormal")) {
		return &gotoNormal;
	} else if (!strcmp(buttonKey, "gotoClockSettings")) {
		return &gotoClockSettings;
	} else if (!strcmp(buttonKey, "gotoTimer")) {
		return &gotoTimer;	
		
	
	} else if (!strcmp(buttonKey, "setMilitaryTime")) {
		return &toggleMilitaryTime;
	} else if (!strcmp(buttonKey, "setAlarmStyle")) {
		return &toggleAlarmStyle;
	} else if (!strcmp(buttonKey, "setInsomniacMode")) {
		return &toggleInsomniacModeButton;
	
	} else if (!strcmp(buttonKey, "setSecondsVisible")) {
		return &toggleSecondsVisible;
	} else if (!strcmp(buttonKey, "setInsomniacLock")) {
		return &toggleInsomniacLock;
	} else if (!strcmp(buttonKey, "pickMood")) {
		return &openMoodPicker;
	} else if (!strcmp(buttonKey, "selectMoodColor")) {
		return &selectMoodColor;
	} else if (!strcmp(buttonKey, "closeMoodPicker")) {
		return &closeMoodPicker;
	} else if (!strcmp(buttonKey, "setAlarmOnOff")) {
		return &toggleCurrentAlarm;
	} else if (!strcmp(buttonKey, "setAlarmAMPM")) {
		return &toggleAlarmAMPM;
	} else if (!strcmp(buttonKey, "setAlarmMode")) {
		return &cycleAlarmMode;
	} else if (!strcmp(buttonKey, "setAlarmSnoozeTime")) {
		return &openSnoozeTimeSlider;
	} else if (!strcmp(buttonKey, "setAlarmOnceLoop")) {
		return &toggleAlarmSoundLoop;
	} else if (!strcmp(buttonKey, "setAlarmSound")) {
		return &pickAlarmSoundFile;
	} else if (!strcmp(buttonKey, "stopAlarm")) {
		return &stopAlarm;
	} else if (!strcmp(buttonKey, "doSnooze")) {
		return &snoozeAlarm;
	} else if (!strcmp(buttonKey, "openFMPicker")) {
		return &openFMRadioPicker;
	} else if (!strcmp(buttonKey, "closeFMPicker")) {
		return &closeFMRadioPicker;
	} else if (!strcmp(buttonKey, "slideFMPicker")) {
		return &slideFMRadioPicker;
	} else if (!strcmp(buttonKey, "tuneFMRadioUp")) {
		return &tuneFMRadioUp;
	} else if (!strcmp(buttonKey, "tuneFMRadioDown")) {
		return &tuneFMRadioDown;
	} else if (!strcmp(buttonKey, "setInsomniacLevel")) {
		return &openInsomniacLevelSlider;
	} else if (!strcmp(buttonKey, "setAlarmMaxVol")) {
		return &openAlarmMaxVolSlider;
		
	} else if (!strcmp(buttonKey, "startTimer")) {
		return &startTimer;
	} else if (!strcmp(buttonKey, "stopTimer")) {
		return &stopTimer;
	} else if (!strcmp(buttonKey, "pauseTimer")) {
		return &pauseToggleTimer;
		
	} else {
		return NULL;
	}
	

}

/***********************************************************************
*
* loadThemeLabels(struct xmlNode *baseEle) 
*
* Attempt to load and process all theme labels defined by a given XML
* element
***********************************************************************/

void loadThemeLabels(struct xmlNode *labelsEle) {
	int i, j, labelMode;	//Good ol' i
	
	char tempHex[10];
	
	SDL_Surface *tempBuffer;		//Surface to store a loaded image into before converting it	
	struct xmlNode  *positionEle, *childEle, *childEle2; //XML node structures
	char fullPath[512];	//Full path to files
	

	if (labelsEle != NULL) {
		//Process button defintions for this theme
		i =0;
		
		
		positionEle = findXMLObjectTest(labelsEle->value, "labelDef", i);
		do {
			//Find out what mode this label applies to
			childEle = findXMLObjectTest(positionEle->value, "clockMode", 0);
			if (childEle != NULL) {
				if (strcmp(childEle->value, "normal") == 0) {
					//Label applies to normal clock mode!
					
					labelMode = CLOCKMODENORMAL;
				} else if (strcmp(childEle->value, "settings") == 0) {
					labelMode = CLOCKMODEALARMSETTINGS;
				
				} else if (strcmp(childEle->value, "clockSettings") == 0) {
					labelMode = CLOCKMODECLOCKSETTINGS;
				
				} else if (strcmp(childEle->value, "timer") == 0) {
					labelMode = CLOCKMODETIMER;
				
				} else if (strcmp(childEle->value, "moodPicker") == 0) {
					labelMode = CLOCKMODEMOODPICKER;
				
				} else if (strcmp(childEle->value, "alarmActive") == 0) {
					labelMode = CLOCKMODEALARMRUNNING;	//Alarm is running
				
				} else if (strcmp(childEle->value, "FMRadioPicker") == 0) {
					labelMode = CLOCKMODEFMRADIOPICKER;
				
				} else if (strcmp(childEle->value, "slider") == 0) {
					labelMode = CLOCKMODESLIDER;
				
				
				} //Done if statements to see which mode button applies to
				freeXMLObject(childEle);
				
				
				//Find out what type of button this is
				childEle = findXMLObjectTest(positionEle->value, "type", 0);
				if (childEle == NULL) {
					freeXMLObject(positionEle);
					i++;
					positionEle = findXMLObjectTest(labelsEle->value, "labelDef", i);
					continue;
				}
				
				if (strcmp(childEle->value, "text") == 0) {
					//Text label
					//Expand the text label array to have space for our new label
					currentTheme.textLabels[labelMode].labelArr = (struct textLabel **)realloc(currentTheme.textLabels[labelMode].labelArr, (currentTheme.textLabels[labelMode].labelCount + 1) * sizeof(struct textLabel *));
					
					/// allocate memory for one `mouseAction` 
					currentTheme.textLabels[labelMode].labelArr[currentTheme.textLabels[labelMode].labelCount] = (struct textLabel *)malloc(sizeof(struct textLabel));

					//Init the button to be blank
					initTextLabel(currentTheme.textLabels[labelMode].labelArr[currentTheme.textLabels[labelMode].labelCount]);

					// copy the data into the new element (structure) 
				
					//*** Now onto label rect...eeee
					childEle2 = findXMLObjectTest(positionEle->value, "top", 0);
					currentTheme.textLabels[labelMode].labelArr[currentTheme.textLabels[labelMode].labelCount]->labelRect.y = atoi(childEle2->value);
					freeXMLObject(childEle2);
					
					childEle2 = findXMLObjectTest(positionEle->value, "left", 0);
					currentTheme.textLabels[labelMode].labelArr[currentTheme.textLabels[labelMode].labelCount]->labelRect.x = atoi(childEle2->value);
					freeXMLObject(childEle2);
					
					childEle2 = findXMLObjectTest(positionEle->value, "width", 0);
					currentTheme.textLabels[labelMode].labelArr[currentTheme.textLabels[labelMode].labelCount]->labelRect.w = atoi(childEle2->value);
					freeXMLObject(childEle2);
					
					childEle2 = findXMLObjectTest(positionEle->value, "height", 0);
					currentTheme.textLabels[labelMode].labelArr[currentTheme.textLabels[labelMode].labelCount]->labelRect.h = atoi(childEle2->value);
					freeXMLObject(childEle2);
					
					//*** Done label rect!
					
					//Grab label value
					childEle2 = findXMLObjectTest(positionEle->value, "labelValue", 0);
					if (childEle2 != NULL) {
						currentTheme.textLabels[labelMode].labelArr[currentTheme.textLabels[labelMode].labelCount]->labelValue = calloc(strlen(childEle2->value) +1, sizeof(char));
						sprintf(currentTheme.textLabels[labelMode].labelArr[currentTheme.textLabels[labelMode].labelCount]->labelValue, "%s", childEle2->value);
						freeXMLObject(childEle2);
					}
					
					//Grab label key
					childEle2 = findXMLObjectTest(positionEle->value, "labelKey", 0);
					if (childEle2 != NULL) {
						currentTheme.textLabels[labelMode].labelArr[currentTheme.textLabels[labelMode].labelCount]->labelKey = calloc(strlen(childEle2->value) +1, sizeof(char));
						sprintf(currentTheme.textLabels[labelMode].labelArr[currentTheme.textLabels[labelMode].labelCount]->labelKey, "%s", childEle2->value);
						freeXMLObject(childEle2);
					}
					
					//Grab text angle
					currentTheme.textLabels[labelMode].labelArr[currentTheme.textLabels[labelMode].labelCount]->textAngle = 0;
					childEle2 = findXMLObjectTest(positionEle->value, "textAngle", 0);
					if (childEle2 != NULL) {
						currentTheme.textLabels[labelMode].labelArr[currentTheme.textLabels[labelMode].labelCount]->textAngle = atoi(childEle2->value);
						freeXMLObject(childEle2);
					}
					
					//Grab text alignment
					currentTheme.textLabels[labelMode].labelArr[currentTheme.textLabels[labelMode].labelCount]->textAlign = TEXTALIGNLEFT;
					childEle2 = findXMLObjectTest(positionEle->value, "textAlign", 0);
					if (childEle2 != NULL) {
						if (!strcmp(childEle2->value, "left")) {
							currentTheme.textLabels[labelMode].labelArr[currentTheme.textLabels[labelMode].labelCount]->textAlign = TEXTALIGNLEFT;
						} else if (!strcmp(childEle2->value, "center")) {
							currentTheme.textLabels[labelMode].labelArr[currentTheme.textLabels[labelMode].labelCount]->textAlign = TEXTALIGNCENTER;
						} else if (!strcmp(childEle2->value, "right")) {
							currentTheme.textLabels[labelMode].labelArr[currentTheme.textLabels[labelMode].labelCount]->textAlign = TEXTALIGNRIGHT;
						}
						freeXMLObject(childEle2);
					}
					
					
					//Text colour
					for(j=0; j < 3; j++) {
						//Set colour to defaults
						currentTheme.textLabels[labelMode].labelArr[currentTheme.textLabels[labelMode].labelCount]->textColor[j] = 0;
					}
					currentTheme.textLabels[labelMode].labelArr[currentTheme.textLabels[labelMode].labelCount]->textColor[3] = 255;
					currentTheme.textLabels[labelMode].labelArr[currentTheme.textLabels[labelMode].labelCount]->usesMoodColor = 0;
					
					childEle2 = findXMLObjectTest(positionEle->value, "textColor", 0);
					if (childEle2 != NULL) {
						if(!strcmp(childEle2->value, "mood")) {
							//Use mood colour!
							currentTheme.textLabels[labelMode].labelArr[currentTheme.textLabels[labelMode].labelCount]->usesMoodColor = 1;

						} else if (childEle2->value - strchr(childEle2->value, 35) == 0) {
							//Colour is a hex value (char 35 = #)
							for (j=0; j< 4; j++) {
							
								if (strlen(childEle2->value) > (2 * j) +2) {
									//Clear the temp hex buffer
									memset(tempHex, 0, sizeof(tempHex));
									//Fill the temp hex buffer with our chars
									sprintf(tempHex, "%c%c", childEle2->value[(2 *j)+1],childEle2->value[(2 *j)+2]);
									
									//Convert our hex chars into an int value
									sscanf(tempHex, "%x", (unsigned int *) &currentTheme.textLabels[labelMode].labelArr[currentTheme.textLabels[labelMode].labelCount]->textColor[j]);
								
								}
							}
							
						}
						freeXMLObject(childEle2);
					}
					//Done Text Colour!
					
					//Finally grab the text size!
					childEle2 = findXMLObjectTest(positionEle->value, "textSize", 0);
					if (childEle2 != NULL) {
						
						sprintf(currentTheme.textLabels[labelMode].labelArr[currentTheme.textLabels[labelMode].labelCount]->textSize, "%s", childEle2->value);
						
						
						freeXMLObject(childEle2);
					}
					
					//check for text BG color
					currentTheme.textLabels[labelMode].labelArr[currentTheme.textLabels[labelMode].labelCount]->usesBGColor = 0;
					childEle2 = findXMLObjectTest(positionEle->value, "textBGColor", 0);
					if (childEle2 != NULL) {
						
						if (childEle2->value - strchr(childEle2->value, 35) == 0) {
							currentTheme.textLabels[labelMode].labelArr[currentTheme.textLabels[labelMode].labelCount]->usesBGColor = 1;
						
							//Colour is a hex value (char 35 = #)
							for (j=0; j< 4; j++) {
							
								if (strlen(childEle2->value) > (2 * j) +2) {
									//Clear the temp hex buffer
									memset(tempHex, 0, sizeof(tempHex));
									//Fill the temp hex buffer with our chars
									sprintf(tempHex, "%c%c", childEle2->value[(2 *j)+1],childEle2->value[(2 *j)+2]);
									
									//Convert our hex chars into an int value
									sscanf(tempHex, "%x", (unsigned int *) &currentTheme.textLabels[labelMode].labelArr[currentTheme.textLabels[labelMode].labelCount]->textBGColor[j]);
									
								
								
								}
							}
							
						}
				
						freeXMLObject(childEle2);
					}
					//Done text bG color
					
					
					
					//Increment the label counter
					currentTheme.textLabels[labelMode].labelCount++;
					
				} else if (strcmp(childEle->value, "image") == 0 || strcmp(childEle->value, "moodAlphaImage") == 0) {
					//Image label... do this later?
					
					//Expand the text label array to have space for our new label
					currentTheme.imageLabels[labelMode].labelArr = (struct imageLabel **)realloc(currentTheme.imageLabels[labelMode].labelArr, (currentTheme.imageLabels[labelMode].labelCount + 1) * sizeof(struct imageLabel *));
					
					/// allocate memory for one `mouseAction` 
					currentTheme.imageLabels[labelMode].labelArr[currentTheme.imageLabels[labelMode].labelCount] = (struct imageLabel *)malloc(sizeof(struct imageLabel));

					//Init the button to be blank
					initImageLabel(currentTheme.imageLabels[labelMode].labelArr[currentTheme.imageLabels[labelMode].labelCount]);

					// copy the data into the new element (structure) 
				
					//*** Now onto label rect...eeee
					childEle2 = findXMLObjectTest(positionEle->value, "top", 0);
					currentTheme.imageLabels[labelMode].labelArr[currentTheme.imageLabels[labelMode].labelCount]->labelRect.y = atoi(childEle2->value);
					freeXMLObject(childEle2);
					
					childEle2 = findXMLObjectTest(positionEle->value, "left", 0);
					currentTheme.imageLabels[labelMode].labelArr[currentTheme.imageLabels[labelMode].labelCount]->labelRect.x = atoi(childEle2->value);
					freeXMLObject(childEle2);
					
					childEle2 = findXMLObjectTest(positionEle->value, "width", 0);
					currentTheme.imageLabels[labelMode].labelArr[currentTheme.imageLabels[labelMode].labelCount]->labelRect.w = atoi(childEle2->value);
					freeXMLObject(childEle2);
					
					childEle2 = findXMLObjectTest(positionEle->value, "height", 0);
					currentTheme.imageLabels[labelMode].labelArr[currentTheme.imageLabels[labelMode].labelCount]->labelRect.h = atoi(childEle2->value);
					freeXMLObject(childEle2);
					
					//*** Done label rect!
					
					//Grab label image
					childEle2 = findXMLObjectTest(positionEle->value, "imageFile", 0);
					if (childEle2 != NULL) {
						bzero(fullPath, sizeof(fullPath));
						sprintf(fullPath, "%s%s", currentTheme.themePath, childEle2->value);
						
						tempBuffer = IMG_Load(fullPath);
						
						
						if (tempBuffer != NULL) {
							
							currentTheme.imageLabels[labelMode].labelArr[currentTheme.imageLabels[labelMode].labelCount]->imageObj = SDL_DisplayFormatAlpha(tempBuffer);
							SDL_FreeSurface(tempBuffer);
						} else {
							printf("Couldn't load %s!!\n", fullPath);
						}
						
						
						freeXMLObject(childEle2);
					}
					
					currentTheme.imageLabels[labelMode].labelArr[currentTheme.imageLabels[labelMode].labelCount]->usesMoodColor = 0;
					if (!strcmp(childEle->value, "moodAlphaImage")) {
						currentTheme.imageLabels[labelMode].labelArr[currentTheme.imageLabels[labelMode].labelCount]->usesMoodColor = 1;
					}
					
					//Grab label key
					childEle2 = findXMLObjectTest(positionEle->value, "labelKey", 0);
					if (childEle2 != NULL) {
						currentTheme.imageLabels[labelMode].labelArr[currentTheme.imageLabels[labelMode].labelCount]->labelKey = calloc(strlen(childEle2->value) +1, sizeof(char));
						sprintf(currentTheme.imageLabels[labelMode].labelArr[currentTheme.imageLabels[labelMode].labelCount]->labelKey, "%s", childEle2->value);
						freeXMLObject(childEle2);
					}
					
					//check for FM selector
					
					if (currentTheme.imageLabels[labelMode].labelArr[currentTheme.imageLabels[labelMode].labelCount]->labelKey != NULL) {
						if (!strcmp(currentTheme.imageLabels[labelMode].labelArr[currentTheme.imageLabels[labelMode].labelCount]->labelKey, "FMSelector")) {
							
							fmSelector = currentTheme.imageLabels[labelMode].labelArr[currentTheme.imageLabels[labelMode].labelCount];
							
						} else if (!strcmp(currentTheme.imageLabels[labelMode].labelArr[currentTheme.imageLabels[labelMode].labelCount]->labelKey, "sliderPointer")) {
						
							sliderPointerLabel = currentTheme.imageLabels[labelMode].labelArr[currentTheme.imageLabels[labelMode].labelCount];
						}
					}
					//Increment the label counter
					currentTheme.imageLabels[labelMode].labelCount++;
					
				
				}
				
				freeXMLObject(childEle);
				
				


		
			} // end if (childEle != NULL)
			
			freeXMLObject(positionEle);
			i++;
			positionEle = findXMLObjectTest(labelsEle->value, "labelDef", i);
		} while (positionEle != NULL);

	}

}

/***********************************************************************
*
* loadThemeButtons(struct xmlNode *buttonsEle) 
*
* Attempt to load all theme buttons defined by the given XML node
***********************************************************************/

void loadThemeButtons(struct xmlNode *buttonsEle) {
	int i, buttonMode, labelMode;
	
	char *stringPos = NULL;			//Used for searches
	
	struct xmlNode  *positionEle, *childEle, *dragChildEle; //XML node structures
	
	
	if (buttonsEle != NULL) {
		//Process button defintions for this theme
		i =0;
		
		/*********** There's got to be a better way of handling all of this to 
		reduce all of the code duplication between modes!! but this will work for
		now until I can figure it out I guess...
		*/
		
		positionEle = findXMLObjectTest(buttonsEle->value, "buttonDef", i);
		if (positionEle == NULL) {
			//Abort
			return;
		}
		
		do {
			//Find out what mode this button applies to
			childEle = findXMLObjectTest(positionEle->value, "clockMode", 0);
			if (childEle != NULL) {
				if (strcmp(childEle->value, "normal") == 0) {
					//Button applies to normal clock mode!
					
					
					buttonMode = CLOCKMODENORMAL;
				} else if (strcmp(childEle->value, "settings") == 0) {
					buttonMode = CLOCKMODEALARMSETTINGS;
				
				} else if (strcmp(childEle->value, "clockSettings") == 0) {
					buttonMode = CLOCKMODECLOCKSETTINGS;
				
				} else if (strcmp(childEle->value, "timer") == 0) {
					buttonMode = CLOCKMODETIMER;
				
				} else if (strcmp(childEle->value, "moodPicker") == 0) {
					buttonMode = CLOCKMODEMOODPICKER;
				
				} else if (strcmp(childEle->value, "alarmActive") == 0) {
					buttonMode = CLOCKMODEALARMRUNNING;	//Alarm is running
				
				} else if (strcmp(childEle->value, "FMRadioPicker") == 0) {
					buttonMode = CLOCKMODEFMRADIOPICKER;	//Alarm is running
				
				} else if (strcmp(childEle->value, "slider") == 0) {
					buttonMode = CLOCKMODESLIDER;	//Alarm is running
					
					
				} //Done if statements to see which mode button applies to
				freeXMLObject(childEle);
				
				
				//Expand the theme-oriented normal mode buttons array to have space for our new button
				themeButtonActions[buttonMode].buttonActions = (struct mouseAction **)realloc(themeButtonActions[buttonMode].buttonActions, (themeButtonActions[buttonMode].buttonCount + 1) * sizeof(struct mouseAction *));
				
				/// allocate memory for one `mouseAction` 
				themeButtonActions[buttonMode].buttonActions[themeButtonActions[buttonMode].buttonCount] = (struct mouseAction *)malloc(sizeof(struct mouseAction));
				
				//Initialize the button entry
				initButton(themeButtonActions[buttonMode].buttonActions[themeButtonActions[buttonMode].buttonCount]);
				
				
				// copy the data into the new element (structure) 
				labelMode = 0;		//Does a label for this button need to be generated (switch)
				
				//*** Start by grabbing the type of button
				childEle = findXMLObjectTest(positionEle->value, "type", 0);
				if (strcmp(childEle->value, "mouseDown") == 0) {
					themeButtonActions[buttonMode].buttonActions[themeButtonActions[buttonMode].buttonCount]->buttonMode = MOUSEDOWN;
				} else if (strcmp(childEle->value, "mouseUp") == 0) {
					themeButtonActions[buttonMode].buttonActions[themeButtonActions[buttonMode].buttonCount]->buttonMode = MOUSEUP;
				} else if (strcmp(childEle->value, "click") == 0) {
					themeButtonActions[buttonMode].buttonActions[themeButtonActions[buttonMode].buttonCount]->buttonMode = CLICK;
				} else if (strcmp(childEle->value, "switch") == 0) {
					//Switches are special types of buttons that also create graphics for themselves... to do this we auto generate a label in a moment...
					themeButtonActions[buttonMode].buttonActions[themeButtonActions[buttonMode].buttonCount]->buttonMode = CLICK;
					
					labelMode = 1;
				} else if (strcmp(childEle->value, "slider") == 0) {
					//Sliders are special types of buttons that have a "follow the finger" graphic associated with them
					//To use sliders, we need to make sure mouseMovement tracking is on for this clock mode
					mouseMoveEnabled[buttonMode] =1;
					
					themeButtonActions[buttonMode].buttonActions[themeButtonActions[buttonMode].buttonCount]->buttonMode = MOUSESLIDE;
					labelMode = 2;
					
				} else if (strcmp(childEle->value, "outsideClick") == 0) {
					themeButtonActions[buttonMode].buttonActions[themeButtonActions[buttonMode].buttonCount]->buttonMode = OUTSIDECLICK;
				} else if (strcmp(childEle->value, "mouseDrag") == 0) {
					themeButtonActions[buttonMode].buttonActions[themeButtonActions[buttonMode].buttonCount]->buttonMode = MOUSEDRAG;
					
					//Setup drag settings
					//themeButtonActions[buttonMode].buttonActions[themeButtonActions[buttonMode].buttonCount]->dragSettings = {0,0,0,0,NULL, NULL, NULL, NULL};
					
					//*** Special checks for drag buttons ***
					dragChildEle = findXMLObjectTest(positionEle->value, "dragUpOffset", 0);
					if (dragChildEle != NULL) {
						themeButtonActions[buttonMode].buttonActions[themeButtonActions[buttonMode].buttonCount]->dragSettings.dragUpOffset = atoi(dragChildEle->value);
						freeXMLObject(dragChildEle);
					}
					dragChildEle = findXMLObjectTest(positionEle->value, "dragDownOffset", 0);
					if (dragChildEle != NULL) {
						themeButtonActions[buttonMode].buttonActions[themeButtonActions[buttonMode].buttonCount]->dragSettings.dragDownOffset = atoi(dragChildEle->value);
						freeXMLObject(dragChildEle);
					}
					dragChildEle = findXMLObjectTest(positionEle->value, "dragLeftOffset", 0);
					if (dragChildEle != NULL) {
						themeButtonActions[buttonMode].buttonActions[themeButtonActions[buttonMode].buttonCount]->dragSettings.dragLeftOffset = atoi(dragChildEle->value);
						freeXMLObject(dragChildEle);
					}
					dragChildEle = findXMLObjectTest(positionEle->value, "dragRightOffset", 0);
					if (dragChildEle != NULL) {
						themeButtonActions[buttonMode].buttonActions[themeButtonActions[buttonMode].buttonCount]->dragSettings.dragRightOffset = atoi(dragChildEle->value);
						freeXMLObject(dragChildEle);
					}
				
					dragChildEle = findXMLObjectTest(positionEle->value, "dragUpKey", 0);
					if (dragChildEle != NULL) {
						themeButtonActions[buttonMode].buttonActions[themeButtonActions[buttonMode].buttonCount]->dragSettings.dragUpAction = mapButtonKeyToAction(dragChildEle->value);
						freeXMLObject(dragChildEle);
					}
					
					dragChildEle = findXMLObjectTest(positionEle->value, "dragDownKey", 0);
					if (dragChildEle != NULL) {
						themeButtonActions[buttonMode].buttonActions[themeButtonActions[buttonMode].buttonCount]->dragSettings.dragDownAction = mapButtonKeyToAction(dragChildEle->value);
						freeXMLObject(dragChildEle);
					}
					dragChildEle = findXMLObjectTest(positionEle->value, "dragLeftKey", 0);
					if (dragChildEle != NULL) {
						themeButtonActions[buttonMode].buttonActions[themeButtonActions[buttonMode].buttonCount]->dragSettings.dragLeftAction = mapButtonKeyToAction(dragChildEle->value);
						freeXMLObject(dragChildEle);
					}
					dragChildEle = findXMLObjectTest(positionEle->value, "dragRightKey", 0);
					if (dragChildEle != NULL) {
						themeButtonActions[buttonMode].buttonActions[themeButtonActions[buttonMode].buttonCount]->dragSettings.dragRightAction = mapButtonKeyToAction(dragChildEle->value);
						freeXMLObject(dragChildEle);
					}
				
					//*** Done drag button setup ***
					
					
					
				} 
				freeXMLObject(childEle);
				//*** Done button type! 
				
				//*** Now onto button rect...eeee
				childEle = findXMLObjectTest(positionEle->value, "top", 0);
				themeButtonActions[buttonMode].buttonActions[themeButtonActions[buttonMode].buttonCount]->buttonRect.y = atoi(childEle->value);
				freeXMLObject(childEle);
				
				childEle = findXMLObjectTest(positionEle->value, "left", 0);
				themeButtonActions[buttonMode].buttonActions[themeButtonActions[buttonMode].buttonCount]->buttonRect.x = atoi(childEle->value);
				freeXMLObject(childEle);
				
				childEle = findXMLObjectTest(positionEle->value, "width", 0);
				themeButtonActions[buttonMode].buttonActions[themeButtonActions[buttonMode].buttonCount]->buttonRect.w = atoi(childEle->value);
				freeXMLObject(childEle);
				
				childEle = findXMLObjectTest(positionEle->value, "height", 0);
				themeButtonActions[buttonMode].buttonActions[themeButtonActions[buttonMode].buttonCount]->buttonRect.h = atoi(childEle->value);
				freeXMLObject(childEle);
				
				//*** Done button rect!
				
				//*** Finally the tricky bit! the button key/actual action!
				///* Possible values for buttons keys are listed ion the mapButtonKeyToAction Function Defintion!!
				
				childEle = findXMLObjectTest(positionEle->value, "buttonKey", 0);
				if (childEle != NULL) {
					themeButtonActions[buttonMode].buttonActions[themeButtonActions[buttonMode].buttonCount]->buttonAction = mapButtonKeyToAction(childEle->value);
					//freeXMLObject(childEle);   //Moved to after label setup so button key is accessible
				}
				
				
				
				//*** Done button key/actual action
				
				//*** Finally enable the button (the whole enabled thing seems pretty stupid, but maybe handy later?)
				themeButtonActions[buttonMode].buttonActions[themeButtonActions[buttonMode].buttonCount]->enabled = 1;


				//**** Switch setup
				// Check to see if this button needs a label generated for it (i.e. it's a switch)
				
				if (labelMode == 1) {
					//labelMode 1 = Standard switch
					
					//Expand the image label array to have space for our new label
					currentTheme.imageLabels[buttonMode].labelArr = (struct imageLabel **)realloc(currentTheme.imageLabels[buttonMode].labelArr, (currentTheme.imageLabels[buttonMode].labelCount + 1) * sizeof(struct imageLabel *));
					
					/// allocate memory for one `mouseAction` 
					currentTheme.imageLabels[buttonMode].labelArr[currentTheme.imageLabels[buttonMode].labelCount] = (struct imageLabel *)malloc(sizeof(struct imageLabel));

					//Init the button to be blank
					initImageLabel(currentTheme.imageLabels[buttonMode].labelArr[currentTheme.imageLabels[buttonMode].labelCount]);

					// copy the data into the new element (structure) 
				
					//*** Now onto label rect... (we just copy the button rect here)
					currentTheme.imageLabels[buttonMode].labelArr[currentTheme.imageLabels[buttonMode].labelCount]->labelRect.x = themeButtonActions[buttonMode].buttonActions[themeButtonActions[buttonMode].buttonCount]->buttonRect.x;
					
					currentTheme.imageLabels[buttonMode].labelArr[currentTheme.imageLabels[buttonMode].labelCount]->labelRect.y = themeButtonActions[buttonMode].buttonActions[themeButtonActions[buttonMode].buttonCount]->buttonRect.y;
					currentTheme.imageLabels[buttonMode].labelArr[currentTheme.imageLabels[buttonMode].labelCount]->labelRect.w = themeButtonActions[buttonMode].buttonActions[themeButtonActions[buttonMode].buttonCount]->buttonRect.w;
					currentTheme.imageLabels[buttonMode].labelArr[currentTheme.imageLabels[buttonMode].labelCount]->labelRect.h = themeButtonActions[buttonMode].buttonActions[themeButtonActions[buttonMode].buttonCount]->buttonRect.h;
					
					
					
					
					
					//*** Since the label is tied to an action we just leave the actual graphic as empty
					currentTheme.imageLabels[buttonMode].labelArr[currentTheme.imageLabels[buttonMode].labelCount]->imageObj = NULL;
					
					
					//*** Now figure out the label key based on the button key ***
					stringPos = strstr(childEle->value, "set");
					if (stringPos != NULL) {
						if (childEle->value - stringPos == 0) {
							currentTheme.imageLabels[buttonMode].labelArr[currentTheme.imageLabels[buttonMode].labelCount]->labelKey = calloc(strlen(childEle->value), sizeof(char));
							
							//First assign the lower case first letter
							sprintf(currentTheme.imageLabels[buttonMode].labelArr[currentTheme.imageLabels[buttonMode].labelCount]->labelKey, "%c", tolower(childEle->value[3]));
							
							//Jump ahead a few chars
							childEle->value += 4;
							strcat(currentTheme.imageLabels[buttonMode].labelArr[currentTheme.imageLabels[buttonMode].labelCount]->labelKey, childEle->value);
							
							//And back up again so we don't loose 'em in memory limbo...
							childEle->value -= 4;
							
							printf("new label key is %s\n",currentTheme.imageLabels[buttonMode].labelArr[currentTheme.imageLabels[buttonMode].labelCount]->labelKey); 

						}
					}
					
					currentTheme.imageLabels[buttonMode].labelCount++;
					
				}


				if (childEle != NULL) {
					freeXMLObject(childEle);
				}


				//Increment the counter
				themeButtonActions[buttonMode].buttonCount++;

		
			} // end if (childEle != NULL)
			
			freeXMLObject(positionEle);
			i++;
			positionEle = findXMLObjectTest(buttonsEle->value, "buttonDef", i);
		} while (positionEle != NULL);
		


	}





}


/***********************************************************************
* cycleNextThemeNext
*
* Function to cycle through to the next theme 
*
***********************************************************************/

void cycleThemeNext() {
	if (currentTheme.themeIndex + 1 < themeCount) {
		//printf("%s new theme\n", availableThemes[currentTheme.themeIndex + 1]->themePath);
		 loadTheme(availableThemes[currentTheme.themeIndex + 1]->themePath);
	} else {
		//printf("%s new theme\n", availableThemes[0]->themePath);
		loadTheme(availableThemes[0]->themePath);
	}
}

/***********************************************************************
* cycleThemePrevious
*
* Function to cycle through to the previous theme 
*
***********************************************************************/

void cycleThemePrevious() {
	if (currentTheme.themeIndex -1 > -1) {
		//printf("%s new theme\n", availableThemes[currentTheme.themeIndex + 1]->themePath);
		 loadTheme(availableThemes[currentTheme.themeIndex - 1]->themePath);
	} else {
		//printf("%s new theme\n", availableThemes[0]->themePath);
		loadTheme(availableThemes[themeCount - 1]->themePath);
	}
}


/***********************************************************************
* loadTheme()
*
* Function to load the current theme and make it appear on the clock
*
***********************************************************************/

void loadTheme(char *newThemePath) {
	char themeConfigFile[1024];
	
	SDL_Surface *tempScreen;
	
	FILE *inputFilePtr;           /* Pointer to input file */
	char *configContent;
	char tempHex[10];
	struct xmlNode *rootEle, *positionEle, *buttonsEle, *labelsEle, *settingsEle, *childEle, *childEle2;
	
	
	int i,j;
	int newTheme;
	int buttonMode;
	
	//SDL_Surface *tempBuffer;		//Surface to store a loaded image into before converting it
	
	
	newTheme = 1;
	
	//Array of function pointers for alarm digit scrollers
	void (*digitScrollerUpArr[4])() = {NULL};
	void (*digitScrollerDownArr[4])() = {NULL};
	
	/* initialize our pointers to NULL */
	configContent = NULL;
	rootEle = NULL;
	if (currentTheme.themePath != NULL) {
		if (strcmp(newThemePath, currentTheme.themePath) == 0) {
			newTheme = 0;
		}
	}
	if (newTheme) {
		//Time to load a new theme!
		
		
		//Let's try and grab some info about it first
		if (!isDirectory(newThemePath)) {
			//Theme doesnt exist, so abort
			return;
		}
	
		//Set the flag to indicate theme is being reloaded and not ready for use
		themeReady = 0;
	
		//Show the "wait changing" message
		themeChangeOverlay(1);
	
		//find the information from the availableThemes list
		for(i = 0; i < themeCount; i++) {
			//printf("%i theme counter\n", i);
			if (strcmp(availableThemes[i]->themePath, newThemePath) == 0) {
				currentTheme.themeIndex = i;
				currentTheme.themeName = availableThemes[i]->themeName;
				currentTheme.themePath = availableThemes[i]->themePath;
				break;
			}
		}
		
		//************** Clean up old/existng theme first ********//
		//Unload/clear current stores first
		clearThemeGraphics();
		
		//********* Clear up/clean up old theme buttons ******//
		clearThemeButtons();
		
		//************** Done cleaning old theme *****************//
		
		
		//Look for theme config file
		sprintf(themeConfigFile,"%sthemeDef.xml", newThemePath);
		if (fileExists(themeConfigFile)) {
			//Load the config file since it exists
			inputFilePtr = fopen(themeConfigFile, "rb"); /* Open in BINARY mode */

			//Read the file
			configContent = readFile(inputFilePtr);
	
			//Close file pointer
			fclose(inputFilePtr);
			
			rootEle = findXMLObjectTest(configContent, "flipClockTheme", 0);
			
			
			settingsEle = findXMLObjectTest(rootEle->value, "settings", 0);
			
			//Let's just go after the info we care about first
			childEle = findXMLObjectTest(settingsEle->value, "usesAlphaBG", 0);
			
			currentTheme.usesAlphaBG = atoi(childEle->value);
			freeXMLObject(childEle);

			//************ Get theme transition settings **********************//
			//Default to slide if not mode defined
			currentTheme.transitionMode = TRANSITIONSLIDE;
			childEle = findXMLObjectTest(settingsEle->value, "transitionMode", 0);
			if (childEle != NULL) {
				if (!strcmp(childEle->value, "fade")) {
					currentTheme.transitionMode = TRANSITIONFADE;
				} else if (!strcmp(childEle->value, "slide")) {
					currentTheme.transitionMode = TRANSITIONSLIDE;
				}
				freeXMLObject(childEle);
			}
			
			//*********** Get digit transition mode *************************//
			//Default to animation sequence if not defined
			currentTheme.digitTransMode = 0;
			childEle = findXMLObjectTest(settingsEle->value, "digitMode", 0);
			if (childEle != NULL) {
				if (!strcmp(childEle->value, "fade")) {
					currentTheme.digitTransMode = 1;
				} else if (!strcmp(childEle->value, "sequence")) {
					currentTheme.digitTransMode = 0;
				}
				freeXMLObject(childEle);
			}

			printf("Theme begin preloading graphics\n");
			//Load the graphics
			preloadThemeGraphics();
			printf("Theme finished preloading graphics\n");
			

			/***** Get the clock co-ordinates from the theme file ******/
			positionEle = findXMLObjectTest(settingsEle->value, "ampmSign", 0);
			currentTheme.digitPos[AMPMSIGN].x = 0;
			currentTheme.digitPos[AMPMSIGN].y = 0;
			if (positionEle != NULL) {
				childEle = findXMLObjectTest(positionEle->value, "top", 0);
				currentTheme.digitPos[AMPMSIGN].y = atoi(childEle->value);
				freeXMLObject(childEle);
				
				
				childEle = findXMLObjectTest(positionEle->value, "left", 0);
				currentTheme.digitPos[AMPMSIGN].x = atoi(childEle->value);
				freeXMLObject(childEle);
				freeXMLObject(positionEle);
				
			}
			
			positionEle = findXMLObjectTest(settingsEle->value, "firstHour", 0);
			if (positionEle != NULL) {
				childEle = findXMLObjectTest(positionEle->value, "top", 0);
				currentTheme.digitPos[FIRSTHOUR].y = atoi(childEle->value);
				freeXMLObject(childEle);
				
				childEle = findXMLObjectTest(positionEle->value, "left", 0);
				currentTheme.digitPos[FIRSTHOUR].x = atoi(childEle->value);
				freeXMLObject(childEle);
				freeXMLObject(positionEle);
			}
			
			positionEle = findXMLObjectTest(settingsEle->value, "secondHour", 0);
			if (positionEle != NULL) {
				childEle = findXMLObjectTest(positionEle->value, "top", 0);
				currentTheme.digitPos[SECONDHOUR].y = atoi(childEle->value);
				freeXMLObject(childEle);
				
				childEle = findXMLObjectTest(positionEle->value, "left", 0);
				currentTheme.digitPos[SECONDHOUR].x = atoi(childEle->value);
				freeXMLObject(childEle);
				freeXMLObject(positionEle);
			}
			
			positionEle = findXMLObjectTest(settingsEle->value, "firstMinute", 0);
			if (positionEle != NULL) {
				childEle = findXMLObjectTest(positionEle->value, "top", 0);
				currentTheme.digitPos[FIRSTMINUTE].y = atoi(childEle->value);
				freeXMLObject(childEle);
				
				childEle = findXMLObjectTest(positionEle->value, "left", 0);
				currentTheme.digitPos[FIRSTMINUTE].x = atoi(childEle->value);
				freeXMLObject(childEle);
				freeXMLObject(positionEle);
			}
			
			positionEle = findXMLObjectTest(settingsEle->value, "secondMinute", 0);
			if (positionEle != NULL) {
				childEle = findXMLObjectTest(positionEle->value, "top", 0);
				currentTheme.digitPos[SECONDMINUTE].y = atoi(childEle->value);
				freeXMLObject(childEle);
				
				childEle = findXMLObjectTest(positionEle->value, "left", 0);
				currentTheme.digitPos[SECONDMINUTE].x = atoi(childEle->value);
				freeXMLObject(childEle);
				freeXMLObject(positionEle);
			}
			
			//Load second ticker information if present
			positionEle = findXMLObjectTest(settingsEle->value, "secondsBar", 0);
			currentTheme.digitPos[SECONDSBARRECT].x = 9999;
			currentTheme.digitPos[SECONDSBARRECT].y = 9999;
			currentTheme.secondsBarIncrement = 1;
			if (positionEle != NULL) {
				childEle = findXMLObjectTest(positionEle->value, "top", 0);
				currentTheme.digitPos[SECONDSBARRECT].y = atoi(childEle->value);
				freeXMLObject(childEle);
				
				childEle = findXMLObjectTest(positionEle->value, "left", 0);
				currentTheme.digitPos[SECONDSBARRECT].x = atoi(childEle->value);
				freeXMLObject(childEle);
				
				childEle = findXMLObjectTest(positionEle->value, "increment", 0);
				currentTheme.secondsBarIncrement = atoi(childEle->value);
				if (currentTheme.secondsBarIncrement < 0) {
					currentTheme.secondsBarIncrement = 1;
				}
				freeXMLObject(childEle);
				
				freeXMLObject(positionEle);
			}
			
			//***** Done clock co-ordinates from the theme file ******//
			
			
			//********************** Now load clock labels from theme definition file *****//
			labelsEle = findXMLObjectTest(rootEle->value, "labels", 0);
			
			if (labelsEle != NULL) {
				loadThemeLabels(labelsEle);
				freeXMLObject(labelsEle);
			}
			
			

			//********************** Done loading clock labels from theme definition file *****//



			//********* Load Buttons defined by this theme
			buttonsEle = findXMLObjectTest(rootEle->value, "buttons", 0);
			if (buttonsEle != NULL) {
				loadThemeButtons(buttonsEle);
				freeXMLObject(buttonsEle);
			}

			printf("done theme buttons\n");
			//********* Done buttons defined by this theme



			//**** Now load the rest of the config information for the theme ****//
			
			//**** Load the digit positions for the alarm digit scrollers ****//
			freeXMLObject(settingsEle);
			
			settingsEle = findXMLObjectTest(rootEle->value, "alarmScreenSettings", 0);
			
			if (settingsEle != NULL) {
				//Default to slide if not mode defined
				currentTheme.alarmDigitTransitionMode = TRANSITIONSLIDE;
				childEle = findXMLObjectTest(settingsEle->value, "alarmDigitTransitionMode", 0);
				if (childEle != NULL) {
					if (!strcmp(childEle->value, "fade")) {
						currentTheme.alarmDigitTransitionMode = TRANSITIONFADE;
					} else if (!strcmp(childEle->value, "slide")) {
						currentTheme.alarmDigitTransitionMode = TRANSITIONSLIDE;
					}
					freeXMLObject(childEle);
				}
			
			
				positionEle = findXMLObjectTest(settingsEle->value, "firstHour", 0);
				if (positionEle != NULL) {
					childEle = findXMLObjectTest(positionEle->value, "top", 0);
					currentTheme.alarmDigitPos[FIRSTHOUR].y = atoi(childEle->value);
					freeXMLObject(childEle);
					
					childEle = findXMLObjectTest(positionEle->value, "left", 0);
					currentTheme.alarmDigitPos[FIRSTHOUR].x = atoi(childEle->value);
					freeXMLObject(childEle);
					freeXMLObject(positionEle);
				}
				
				positionEle = findXMLObjectTest(settingsEle->value, "secondHour", 0);
				if (positionEle != NULL) {
					childEle = findXMLObjectTest(positionEle->value, "top", 0);
					currentTheme.alarmDigitPos[SECONDHOUR].y = atoi(childEle->value);
					freeXMLObject(childEle);
					
					childEle = findXMLObjectTest(positionEle->value, "left", 0);
					currentTheme.alarmDigitPos[SECONDHOUR].x = atoi(childEle->value);
					freeXMLObject(childEle);
					freeXMLObject(positionEle);
				}
				
				positionEle = findXMLObjectTest(settingsEle->value, "firstMinute", 0);
				if (positionEle != NULL) {
					childEle = findXMLObjectTest(positionEle->value, "top", 0);
					currentTheme.alarmDigitPos[FIRSTMINUTE].y = atoi(childEle->value);
					freeXMLObject(childEle);
					
					childEle = findXMLObjectTest(positionEle->value, "left", 0);
					currentTheme.alarmDigitPos[FIRSTMINUTE].x = atoi(childEle->value);
					freeXMLObject(childEle);
					freeXMLObject(positionEle);
				}
				
				positionEle = findXMLObjectTest(settingsEle->value, "secondMinute", 0);
				if (positionEle != NULL) {
					childEle = findXMLObjectTest(positionEle->value, "top", 0);
					currentTheme.alarmDigitPos[SECONDMINUTE].y = atoi(childEle->value);
					freeXMLObject(childEle);
					
					childEle = findXMLObjectTest(positionEle->value, "left", 0);
					currentTheme.alarmDigitPos[SECONDMINUTE].x = atoi(childEle->value);
					freeXMLObject(childEle);
					freeXMLObject(positionEle);
				}
				
				//Define an array of functions for the digits (this is kind of silly, but hey what can you do)
				digitScrollerUpArr[0] = &digitUpHH1;
				digitScrollerUpArr[1] = &digitUpHH2;
				digitScrollerUpArr[2] = &digitUpMM1;
				digitScrollerUpArr[3] = &digitUpMM2;
				
				digitScrollerDownArr[0] = &digitDownHH1;
				digitScrollerDownArr[1] = &digitDownHH2;
				digitScrollerDownArr[2] = &digitDownMM1;
				digitScrollerDownArr[3] = &digitDownMM2;
				
				
				
				//Create the buttons for our alarm scrollers
				for (i=0; i < 4; i++) {
					buttonMode = CLOCKMODEALARMSETTINGS;
				
					//Expand the theme-oriented normal mode buttons array to have space for our new button
					themeButtonActions[buttonMode].buttonActions = (struct mouseAction **)realloc(themeButtonActions[buttonMode].buttonActions, (themeButtonActions[buttonMode].buttonCount + 1) * sizeof(struct mouseAction *));
					
					/// allocate memory for one `mouseAction` 
					themeButtonActions[buttonMode].buttonActions[themeButtonActions[buttonMode].buttonCount] = (struct mouseAction *)malloc(sizeof(struct mouseAction));
			
					//It's a two way drag... up to increment, down to decrement
					themeButtonActions[buttonMode].buttonActions[themeButtonActions[buttonMode].buttonCount]->buttonMode = MOUSEDRAG;
					themeButtonActions[buttonMode].buttonActions[themeButtonActions[buttonMode].buttonCount]->dragSettings.dragUpOffset = 10;
					themeButtonActions[buttonMode].buttonActions[themeButtonActions[buttonMode].buttonCount]->dragSettings.dragDownOffset = 10;
					
					themeButtonActions[buttonMode].buttonActions[themeButtonActions[buttonMode].buttonCount]->dragSettings.dragUpAction = digitScrollerUpArr[i];
					themeButtonActions[buttonMode].buttonActions[themeButtonActions[buttonMode].buttonCount]->dragSettings.dragDownAction = digitScrollerDownArr[i];
			
					themeButtonActions[buttonMode].buttonActions[themeButtonActions[buttonMode].buttonCount]->buttonRect.y = currentTheme.alarmDigitPos[i].y - 20;
					themeButtonActions[buttonMode].buttonActions[themeButtonActions[buttonMode].buttonCount]->buttonRect.x = currentTheme.alarmDigitPos[i].x;
					themeButtonActions[buttonMode].buttonActions[themeButtonActions[buttonMode].buttonCount]->buttonRect.w = elements[ALARMDIGITMASK]->w;
					themeButtonActions[buttonMode].buttonActions[themeButtonActions[buttonMode].buttonCount]->buttonRect.h = elements[ALARMDIGITMASK]->h + 20;
			
			
					//*** Finally enable the button (the whole enabled thing seems pretty stupid, but maybe handy later?)
					themeButtonActions[buttonMode].buttonActions[themeButtonActions[buttonMode].buttonCount]->enabled = 1;


					//Increment the counter
					themeButtonActions[buttonMode].buttonCount++;
				
				}
				
				//**** Load the bounding box for the alarm day pickers ****//
				positionEle = findXMLObjectTest(settingsEle->value, "alarmDays", 0);
				if (positionEle != NULL) {
					childEle = findXMLObjectTest(positionEle->value, "top", 0);
					currentTheme.alarmDayRect.y = atoi(childEle->value);
					freeXMLObject(childEle);
					
					childEle = findXMLObjectTest(positionEle->value, "left", 0);
					currentTheme.alarmDayRect.x = atoi(childEle->value);
					freeXMLObject(childEle);
					
					childEle = findXMLObjectTest(positionEle->value, "width", 0);
					currentTheme.alarmDayRect.w = atoi(childEle->value);
					freeXMLObject(childEle);
					
					childEle = findXMLObjectTest(positionEle->value, "height", 0);
					currentTheme.alarmDayRect.h = atoi(childEle->value);
					freeXMLObject(childEle);
					
					childEle2 = findXMLObjectTest(positionEle->value, "textColor", 0);
					for(j=0; j < 3; j++) {
						//Set colour to defaults
						currentTheme.alarmDayTextColor[j] = 0;
					}
					currentTheme.alarmDayTextColor[3] = 255;
					currentTheme.alarmDayTextUseMood = 0;
					if (childEle2 != NULL) {
						if(!strcmp(childEle2->value, "mood")) {
							//Use mood colour!
							currentTheme.alarmDayTextUseMood = 1;

						} else if (childEle2->value - strchr(childEle2->value, 35) == 0) {
							//Colour is a hex value (char 35 = #)
							for (j=0; j< 4; j++) {
							
								if (strlen(childEle2->value) > (2 * j) +2) {
									//Clear the temp hex buffer
									memset(tempHex, 0, sizeof(tempHex));
									//Fill the temp hex buffer with our chars
									sprintf(tempHex, "%c%c", childEle2->value[(2 *j)+1],childEle2->value[(2 *j)+2]);
									
									//Convert our hex chars into an int value
									sscanf(tempHex, "%x", (unsigned int *) &currentTheme.alarmDayTextColor[j]);
								
								}
							}
							
						}
						freeXMLObject(childEle2);
					}
					//Done Text Colour!
					
					//finally text offset
					//defaults
					currentTheme.alarmDayTextOffset.y = 100;
					currentTheme.alarmDayTextOffset.x = 10;
					
					childEle = findXMLObjectTest(positionEle->value, "textTop", 0);
					currentTheme.alarmDayTextOffset.y = atoi(childEle->value);
					freeXMLObject(childEle);
					
					childEle = findXMLObjectTest(positionEle->value, "textLeft", 0);
					currentTheme.alarmDayTextOffset.x = atoi(childEle->value);
					freeXMLObject(childEle);
					
					currentTheme.alarmDayUseMood = 0;
					childEle = findXMLObjectTest(positionEle->value, "useAlphaBG", 0);
					if (childEle != NULL) {
						currentTheme.alarmDayUseMood = atoi(childEle->value);
						freeXMLObject(childEle);
					}
					
					currentTheme.alarmDayTextSize = FONTSIZESMALL;
					childEle = findXMLObjectTest(positionEle->value, "textSize", 0);
					if (childEle != NULL) {
						if (!strcmp(childEle->value, "small")) {
							currentTheme.alarmDayTextSize = FONTSIZESMALL;
						} else if (!strcmp(childEle->value, "medium")) {
							currentTheme.alarmDayTextSize = FONTSIZEMEDIUM;
						} else if (!strcmp(childEle->value, "large")) {
							currentTheme.alarmDayTextSize = FONTSIZELARGE;
						} else if (!strcmp(childEle->value, "huge")) {
							currentTheme.alarmDayTextSize = FONTSIZEHUGE;
						} else {
							currentTheme.alarmDayTextSize = FONTSIZEMEDIUM;
						}

						freeXMLObject(childEle);
					}				
					
					freeXMLObject(positionEle);
				}
				
				//Load the alert settings rect if present
				positionEle = findXMLObjectTest(settingsEle->value, "alertSettingsRect", 0);
				if (positionEle != NULL) {
					childEle = findXMLObjectTest(positionEle->value, "top", 0);
					currentTheme.alertSettingsRect.y = atoi(childEle->value);
					freeXMLObject(childEle);
					
					childEle = findXMLObjectTest(positionEle->value, "left", 0);
					currentTheme.alertSettingsRect.x = atoi(childEle->value);
					freeXMLObject(childEle);
					
					childEle = findXMLObjectTest(positionEle->value, "width", 0);
					currentTheme.alertSettingsRect.w = atoi(childEle->value);
					freeXMLObject(childEle);
					
					childEle = findXMLObjectTest(positionEle->value, "height", 0);
					currentTheme.alertSettingsRect.h = atoi(childEle->value);
					freeXMLObject(childEle);
					
					freeXMLObject(positionEle);
				}
				
				freeXMLObject(settingsEle);
				
			}
			
			
			//**** Now load the settins for the timer screen, if defined **********//
			
			//**** Load the digit positions for the alarm digit scrollers ****//
			
			
			settingsEle = findXMLObjectTest(rootEle->value, "timerScreenSettings", 0);
			
			if (settingsEle != NULL) {		
				
				positionEle = findXMLObjectTest(settingsEle->value, "firstHour", 0);
				if (positionEle != NULL) {
					childEle = findXMLObjectTest(positionEle->value, "top", 0);
					if (childEle != NULL) {
						currentTheme.timerDigitPos[FIRSTHOUR].y = atoi(childEle->value);
						freeXMLObject(childEle);
					}
					
					childEle = findXMLObjectTest(positionEle->value, "left", 0);
					if (childEle != NULL) {
						currentTheme.timerDigitPos[FIRSTHOUR].x = atoi(childEle->value);
						freeXMLObject(childEle);
					}
					
					freeXMLObject(positionEle);
				}
				
				positionEle = findXMLObjectTest(settingsEle->value, "secondHour", 0);
				if (positionEle != NULL) {
					childEle = findXMLObjectTest(positionEle->value, "top", 0);
					if (childEle != NULL) {
						currentTheme.timerDigitPos[SECONDHOUR].y = atoi(childEle->value);
						freeXMLObject(childEle);
					}
					
					childEle = findXMLObjectTest(positionEle->value, "left", 0);
					if (childEle != NULL) {
						currentTheme.timerDigitPos[SECONDHOUR].x = atoi(childEle->value);
						freeXMLObject(childEle);
					}
					freeXMLObject(positionEle);
				}
				
				positionEle = findXMLObjectTest(settingsEle->value, "firstMinute", 0);
				if (positionEle != NULL) {
					childEle = findXMLObjectTest(positionEle->value, "top", 0);
					if (childEle != NULL) {
						currentTheme.timerDigitPos[FIRSTMINUTE].y = atoi(childEle->value);
						freeXMLObject(childEle);
					}
					
					childEle = findXMLObjectTest(positionEle->value, "left", 0);
					if (childEle != NULL) {
						currentTheme.timerDigitPos[FIRSTMINUTE].x = atoi(childEle->value);
						freeXMLObject(childEle);
					}
					
					freeXMLObject(positionEle);
				}
				
				positionEle = findXMLObjectTest(settingsEle->value, "secondMinute", 0);
				if (positionEle != NULL) {
					childEle = findXMLObjectTest(positionEle->value, "top", 0);
					if (childEle != NULL) {
						currentTheme.timerDigitPos[SECONDMINUTE].y = atoi(childEle->value);
						freeXMLObject(childEle);
					}
					
					childEle = findXMLObjectTest(positionEle->value, "left", 0);
					if (childEle != NULL) {
						currentTheme.timerDigitPos[SECONDMINUTE].x = atoi(childEle->value);
						freeXMLObject(childEle);
					}
					freeXMLObject(positionEle);
				}
				
				positionEle = findXMLObjectTest(settingsEle->value, "firstSecond", 0);
				if (positionEle != NULL) {
					childEle = findXMLObjectTest(positionEle->value, "top", 0);
					if (childEle != NULL) {
						currentTheme.timerDigitPos[4].y = atoi(childEle->value);
						freeXMLObject(childEle);
					}
					
					childEle = findXMLObjectTest(positionEle->value, "left", 0);
					if (childEle != NULL) {
						currentTheme.timerDigitPos[4].x = atoi(childEle->value);
						freeXMLObject(childEle);
					}
					freeXMLObject(positionEle);
				}
				
				positionEle = findXMLObjectTest(settingsEle->value, "secondSecond", 0);
				if (positionEle != NULL) {
					childEle = findXMLObjectTest(positionEle->value, "top", 0);
					if (childEle != NULL) {
						currentTheme.timerDigitPos[5].y = atoi(childEle->value);
						freeXMLObject(childEle);
					}
					
					childEle = findXMLObjectTest(positionEle->value, "left", 0);
					if (childEle != NULL) {
						currentTheme.timerDigitPos[5].x = atoi(childEle->value);
						freeXMLObject(childEle);
					}
					freeXMLObject(positionEle);
				}
				
				freeXMLObject(settingsEle);


			}
			//**** Done loading timer screen settings ****************************//
			
			
			//Setup alarm day buttons
			setupAlarmDayButtons();
			//**** Done Loading the digit positions for the alarm digit scrollers ****//			
			
			
			//**** Display theme graphics now that everything is loaded and ready to go ***//
		



	
			//Load the digit settings
			setupThemeDigits();
	
			//Done loads so return flag
			themeReady = 1;
			
			tempScreen = drawStaticScreen(clockMode);
			
			BMPShow(screen, tempScreen, 0,0);
			SDL_FreeSurface(tempScreen);
			

			//**** Done theme graphics initial display ****//




	   

			

			/***** Done clock */
			//Other XML properties to come later if needed...

			
			freeXMLObject(rootEle);
			
			free(configContent);
			
		}
		

		
		//Load UI config and definitions
		loadUIElements();
		
		//Setup slider screen/buttons
		setupSliderButtons();
		

		
		
		//Save this new theme to the user prefs
		
		setUserPreferences();

		//Then, 
		//More to come...
	}
}



/***********************************************************************
* loadUIElements()
*
* Function to load the UI definitions into memory (called on startup only)
* Uses the same label/button functionality as loadTheme
*
***********************************************************************/

void loadUIElements() {
	char uiConfigFile[1024];
	
	FILE *inputFilePtr;           /* Pointer to input file */
	char *configContent;

	struct xmlNode *rootEle, *childEle;


	//Look for UI config file
	sprintf(uiConfigFile,"%suiDef.xml", MEDIAPATH);
	
	if (fileExists(uiConfigFile)) {
		//Load the config file since it exists
		inputFilePtr = fopen(uiConfigFile, "rb"); /* Open in BINARY mode */

		//Read the file
		configContent = readFile(inputFilePtr);
	
		//Close file pointer
		fclose(inputFilePtr);
			
		rootEle = findXMLObjectTest(configContent, "flipClockUI", 0);

		if (rootEle != NULL) {
			childEle = findXMLObjectTest(rootEle->value, "labels", 0);
			
			if (childEle != NULL) {
				loadThemeLabels(childEle);
				freeXMLObject(childEle);
			}
			
			childEle = findXMLObjectTest(rootEle->value, "buttons", 0);
			
			if (childEle != NULL) {
				loadThemeButtons(childEle);
				freeXMLObject(childEle);
			}

			freeXMLObject(rootEle);
		}
		if (configContent != NULL) {		
			free(configContent);
		}
		
	}
	
	
}



/******************************************************************************
* loadAllThemes()
*
* Function to scan the current theme path and load all theme summaries into
* memory.
******************************************************************************/

void loadAllThemes() {
	
	//Try to load all themes from the theme directory
	
	struct dirent **files;		/* Structure to contain returned results */
	int fileCount;			/* Number of files read			*/
	int i;				/* Good old i, everyone likes i		*/
	
	char themeDefFile[255];	/* full path to theme definition */
	
	FILE *inputFilePtr;			/* Pointer for file */
	char *configContent;		/* Raw text content of theme config file */
	
	struct xmlNode *rootEle, *infoEle, *childEle;
	

	fileCount = scandir(THEMESPATH, &files, file_select, alphasort);
printf("THEMEPATH: [%s] fileCount: %d\n", THEMESPATH, fileCount);
	
	if (fileCount > 0) {
		//Files were found
		for (i=0; i< fileCount; i++) {
		
			bzero(themeDefFile, sizeof(themeDefFile));
			sprintf(themeDefFile, "%s%s/themeDef.xml", THEMESPATH, files[i]->d_name);

printf("ThemeDefFile: %s\n", themeDefFile);
			if (fileExists(themeDefFile)) {
				printf ("theme %s found!\r\n", themeDefFile);
				
				//Load the theme info
				inputFilePtr = fopen(themeDefFile, "rb"); /* Open in BINARY mode */

				//Read the file
				configContent = readFile(inputFilePtr);
		
				//Close file pointer
				fclose(inputFilePtr);
				
				
				
				rootEle = findXMLObjectTest(configContent, "flipClockTheme", 0);
				
				infoEle = findXMLObjectTest(rootEle->value, "summary", 0);
				
				//Expand the main themes list to have enough space for our new theme
				availableThemes = (struct themeSummary **)realloc(availableThemes, (themeCount + 1) * sizeof(struct themeSummary *));
				
				/* allocate memory for one `struct node` */
				availableThemes[themeCount] = (struct themeSummary *)malloc(sizeof(struct themeSummary));
				
				/* copy the data into the new element (structure) */
				childEle = findXMLObjectTest(infoEle->value, "name", 0);
				
				
				
				availableThemes[themeCount]->themeName = calloc(strlen(childEle->value) + 2, sizeof(char));
				sprintf(availableThemes[themeCount]->themeName, "%s", childEle->value);
				 
				freeXMLObject(childEle);
				//Other XML fields might be parsed later, but for now just the name is good
				freeXMLObject(infoEle);
				freeXMLObject(rootEle);
				free(configContent);
				
				
				 
				availableThemes[themeCount]->themePath = calloc(strlen(THEMESPATH) + strlen(files[i]->d_name) + 2, sizeof(char));
				sprintf(availableThemes[themeCount]->themePath, "%s%s/", THEMESPATH, 	files[i]->d_name);		
				
				//Increment the theme counter
				themeCount++;
				
			}
		
		}
		
		for (i=0; i< fileCount; i++) {
			free(files[i]);
		}
		free(files);
	}
	
	
	

}


/******************************************************************************
* cleanupThemes()
*
* Function to clear any loaded themes out of memory.
*
******************************************************************************/

void cleanupThemes()
{
	int i;
	
	
	for(i = 0; i < themeCount; i++) {

	  	free(availableThemes[i]->themeName);
		free(availableThemes[i]->themePath);
	  	free(availableThemes[i]);

	}
	
	free(availableThemes);


}


