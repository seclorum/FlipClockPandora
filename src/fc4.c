

/*  flip SDL clock
******************************************************************
  fc4.c
  Nokia N810
  
  gcc -g -Wall `sdl-config --cflags` `sdl-config --libs` -L /usr/include/SDL -lSDL_image fc4.c -o fcT
  Ciro Ippolito 2009

  Based on Ciro's code updated by Rob Williams to try to make it more into an actual clock
  again...
  
  
  ******* UPDATES/CHANGELOG ********
  07/30/09  - Rob
  	Created Makefile to allow bindings to required libraries easier to remember!
  
  
  
  ******************************************************************
*/

/*2DO
one png for the entire number sets
...
...
********************************************************************
*/

#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#include <string.h>				/* for string functions */
#include <sys/utsname.h>		/* for get hardware type */

#include <glib.h>		/*** GLib is needed for main loops, etc ***/
#if defined(HAS_OSSO)
#include <libosso.h>
#endif
/*** Libosso gives us bindings to device state; for portability we try to use dbus/etc directly, but libosso
							makes device state checks/etc easier... ***/



#include "SDL_image.h"
#include "SDL_ttf.h"
#include "SDL_rotozoom.h"

#if defined(_MSC_VER)
#include "SDL.h"
#else
#include "SDL/SDL.h"
#endif


#include <SDL/SDL_syswm.h>
#include <X11/Xutil.h>			/* Needed for stupid hildon taskmanager bug */

#include <gtk/gtk.h>			//For file chooser functions

#if defined(LIBOSSO_H_)
int hasAlarmD = 1;				//Device is alarmD capable!
#else
int hasAlarmD = 0;				//Device is alarmD capable!

#endif

//********************** STATIC DEF'S *****************************//

/********** NOTE ***********
 The following constants are defined by the Makefile!
 PROGNAME  = Name of the Application
 PROGVERSION = Version of the App
 
 DEBUGENABLED = Should debugging be on or off?
 
 MEDIAPATH = Path to media files  (images, etc).
 
****************************/


//DBUS Related settings
#define DBUS_INTERFACE "org.maemo.flipclock"
#define DBUS_SERVICE "org.maemo.flipclock"
#define DBUS_PATH "/org/maemo/flipclock/controller"

//Connection for alarmD syncing
#define DBUS_ALARMSYNC_INTERFACE "org.maemo.flipclockSync"
#define DBUS_ALARMSYNC_SERVICE "org.maemo.flipclockSync"
#define DBUS_ALARMSYNC_PATH "/org/maemo/flipclockSync/controller"

//Connection for FM Radio interface
#define DBUS_FMRADIO_INTERFACE "org.maemo.flipclockFMRadio"
#define DBUS_FMRADIO_SERVICE "org.maemo.flipclockFMRadio"
#define DBUS_FMRADIO_PATH "/org/maemo/flipclockFMRadio/controller"

//Image related helpers

enum { BACKGROUND, SECONDSBAR, SECONDSTICK, AMICON, PMICON,
		ALARMSETTINGSBACKGROUND, MOODPICKERIMG, MOODMASKIMG,
		ALARMDIGITSCROLLER, ALARMDIGITMASK, ALARMDAYOFF, ALARMDAYON,
		ALARMDAYSELECTOR, BUTTONBASE, BUTTONMASK, BUTTONAMPM, BUTTONONOFF,
		BUTTONALERTMODE, BUTTONONCELOOP, BUTTONSNOOZETIME,
		BUTTONMILITARYTIME, BUTTONALARMSTYLE, FMRADIOPICKERIMG,
		CLOCKSETTINGSBACKGROUND, SLIDERBASE, TIMERBACKGROUND,
		BUTTONFIXEDRANDOM, MAXGRAPHICELEMENTS };

enum { FIRSTHOUR, SECONDHOUR, FIRSTMINUTE, SECONDMINUTE, SECONDSBARRECT,
		AMPMSIGN };

enum { THEMECHANGETEXT, SPLASHSCREEN, MAXAPPELEMENTS };

//NOTE! ClockModeMAX should always be the last entry in this list!
enum { CLOCKMODENORMAL, CLOCKMODEWINDOWED, CLOCKMODEALARMSETTINGS, CLOCKMODECLOCKSETTINGS, CLOCKMODEMOODPICKER, CLOCKMODEHELP, CLOCKMODECHANGING, CLOCKMODEMODAL, CLOCKMODEALARMRUNNING, CLOCKMODEFMRADIOPICKER, CLOCKMODESLIDER, CLOCKMODETIMER, CLOCKMODEMAX };	//Modes for the clock to be running in...

//Types of screen transitions
enum { TRANSITIONSLIDE, TRANSITIONSLIDEUP, TRANSITIONSLIDEDOWN,
		TRANSITIONSLIDELEFT, TRANSITIONSLIDERIGHT, TRANSITIONFADE };

//Types of possible alert modes for alarms
enum { ALARMALERTMODESOUND, ALARMALERTMODEFMRADIO,
		ALARMALERTMODESOUNDFOLDER, ALARMALERTMODEMAX };

//Types of possible alarm control modes 
enum { ALARMCONTROLMODEWEEKLY, ALARMCONTROLMODESIMPLE,
		ALARMCONTROLMODEMAX };

//Font sizes; fontsizemax should always be last!
enum { FONTSIZESMALL, FONTSIZEMEDIUM, FONTSIZELARGE, FONTSIZEHUGE,
		FONTSIZEMAX };

//Text alignment
enum { TEXTALIGNLEFT, TEXTALIGNCENTER, TEXTALIGNRIGHT };

//Enums for modal buttons
enum { MODALBUTTONNOFOCUS };


//********************** DONE STATIC DEF's ************************//

//********************** Structure Definitions ***********************//

/* Structure for clock digits */
struct clockDigit {
	int lastValue;
	int xPos;
	int yPos;
	int aniFrame;
	int visible;
};





//********************** Done Struct Defs *************************//

//********************** External Header Defs ********************//

//Headers first!
#include "bme.h"
#include "userPrefs.h"
#include "themeManager.h"
//#include "alarmDFunc.h"
#include "ossoHelper.h"
#include "alarmFunc.h"
#include "mouseFunc.h"





//********************** Done external headers *******************//


//********************** Global Variables *************************//
//typedef enum {FALSE2, TRUE2} boolean;

int isTablet = 0;				//Is the app running on a tablet? 
int hasFMRadio = 0;				//Does this device have an FM radio?

int tabletInactive = 0;			//Is the tablet active or in idle mode?
int windowMode = 0;				//Current window mode. 0 = window, 1 = fullScreen

//Insomniac mode... 1 == classic/newer version, 2== new version
#if MAEMOVERSION==4
int insomniacMode = 2;
#endif
#if MAEMOVERSION==5
int insomniacMode = 1;
#endif
guint insomniacTO = 0;			//Timeout for insomniac TO 
int isInsomniacDimmed = 0;		//Is device currently in insomniac dim mode?
int originalDim = -1;			//Original dim time for insomniac mode
int originalBrightLevel = -1;	//Original brightness level for new insomniac mode
int originalSysVol = -1;		//Original System Volume for tablets...

int clockMode = CLOCKMODENORMAL;	//What "mode" is the clock running in (i.e. what screen is showing)
int lastClockMode = CLOCKMODENORMAL;	//Last clock mode, used for returning from modal displays

int clockRunning = 0;			//Is the clock currently running it's display
int clockSecondsRunning = 0;	//Are the seconds running on display

SDL_Rect lastSecondPos;			//Stores the position of the second hand on last move so that blitting is faster


int themeReady = 0;				//Is theme ready for display?

int currentAlarmNum = 0;		//The alarm to be shown in the alarm settings page

int runningAlarmIndex = -1;		//Index of the currently running alarm, or -1 if no alarm running

int alarmSnoozing = 0;			//Is an alarm snoozing right now?

int nextAlarmIndex = -1;		//Global indicator of nextAlarm
int nextAlarmTime = 0;			//Global indicator of next alarm time (since epoch)
int pendingSnoozeTime = 0;		//Global indicator of next alarm time after snooze completes

char dayShortNames[7][20] = { "SUN", "MON", "TUES", "WED", "THUR", " FRI", " SAT" };	//Name of the day of the week

//Global pointer used for modal windows
int *modalIntVar = NULL;
int modalIntMin = 0;			//Modal "int" minimum value
int modalIntMax = 100;			//Modal "int" max value
char modalStrLabel[200];

//Timer global variables
int oldTimerVal = 0;			//Value of old timer (last time)
int timerStartVal = 0;			//Start time of timer
int timerRunning = 0;			//Is the timer running?
int timerPauseTime = 0;			//Time stamp for when timer was last paused (used to do math to resume properly)


//Global vars for SDL masking of surfaces

	/* SDL interprets each pixel as a 32-bit number, so our masks must depend
	   on the endianness (byte order) of the machine */
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
Uint32 rmask = 0xff000000;
Uint32 gmask = 0x00ff0000;
Uint32 bmask = 0x0000ff00;
Uint32 amask = 0x000000ff;
#else
Uint32 rmask = 0x000000ff;
Uint32 gmask = 0x0000ff00;
Uint32 bmask = 0x00ff0000;
Uint32 amask = 0xff000000;
#endif




//Shouldn't be needed anymore
//char themePath[512] = "";         //Path for current theme

GMainLoop *mainLoopObj;			//Main GLoop of application. This is global so that it can be accessed from all places...


SDL_Surface *screen;


SDL_Surface *numbers[11];		//Number elements (tied to theme)
SDL_Surface *elements[MAXGRAPHICELEMENTS];	//clock elements (tied to theme)
SDL_Surface *digitAnimation[10];	//Statically set a max of 10 for now... we can dynamically allocate later...
int animationCount = 0;			//Number of loaded animation frames

SDL_Surface *appElements[MAXAPPELEMENTS];	//Elements for the application itself (i.e. help msg, etc). Not part of themes or anything

SDL_Surface *renderedBuffer = NULL;	//Trial, to see if rendering BG + image labels to this then blitting from this to screen for text
									//labels is faster/makes more sense?
SDL_Surface *themeBGBuffer = NULL;



struct clockDigit clockDigits[5];





//*********** Preferences *******//
//Not needed, taken care of by userPrefs module
//int militaryTime = 0;                         //Use 12 or 24 hour mode?
int isPM = 0;					//Global to indicate if it's AM or PM
SDL_Rect oldSecsPos = { 0 };	//Previous position of seconds ticker (if present)

//******************* Done Global Variables ****************************//


//********************** Function Headers *************************//
int checkForTablet();			//Use Kernel info to get the ID string of this device and determine if it's a tablet (arm) or not

int initializeApp();			//Initialize the program

int deinitializeApp();


int watchForAlarms();			//Handler for non-alarmD mode

int clockEventIdle();			//Handler to process events/etc if nothing else is going on

int clockEventRedraw();			//Handler called by timeout to redraw clock

int clockSecondsRedraw();		//Handler called by timeout to redraw seconds ticker

int updateClock();				//function to redraw the clock, should be externalized probably...

int toggleWindowMode();			//Function to toggle between window and full screen mode

void gotoSettings();			//Pseudo function to go to settings mode

void gotoNormal();				//Pseudo function to go to normal mode

void gotoClockSettings();		//Pseudo function to go to clock settings mode

void gotoTimer();				//Pseudo function to go to timer/stopwatch mode

void changeClockMode(int newClockMode);	//Clock mode changing function 

int updateTimerDigits(SDL_Surface * screenBuf, int forceUpdate);	//Function to redraw timer digits in timer mode

//***** Graphics related functions ******//
void BMPShow(SDL_Surface * screen, SDL_Surface * pic, int x, int y);

int redrawBackground();			//Function to draw the clock background

int updateSecondsBar();			//Function to draw the seconds indicator

SDL_Surface *drawStaticScreen(int newClockMode);	//Function to draw a screen for animation purposes

void drawAllLabels(SDL_Surface * screenBuff, int newClockMode, int changesOnly);	//Function to draw all the labels for a given clock mode (text and images)

void drawAllLabelsInRect(SDL_Surface * screenBuff, int newClockMode, SDL_Rect * targetRect);	//function to draw all the labels for a given rect

void drawTextLabel(SDL_Surface * screenBuff, struct textLabel *thisLabel, int newClockMode, SDL_Rect * clippingRect);	//Function to draw a text label on screen

void drawImageLabel(SDL_Surface * screenBuff, struct imageLabel *thisLabel, int newClockMode, SDL_Rect * clippingRect);	//Function to draw an image label on screen

int doRectsOverlap(SDL_Rect * src1, SDL_Rect * src2, SDL_Rect * dest);	//Handy dandy "do rects overlap" function

//***** Misc Helpers *********//
//Function to write to a file
int appendToRawFile(char *theContent, char *filePath);

//function to handle debug messages
void debugEntry(char *theContent);






//********** External function libraries *******************//

//File/Dir Utility functions
#include "fileDirFunc.c"

//XML Handling library
#include "xmlFunc.c"

//Audio playback stuff
#include "audioFunc.c"


//Misc Utility functions
#include "miscFunc.h"



//Charger/battery monitor
#include "bme.c"

//Osso helper library for tablet stuff
#include "ossoHelper.c"

//AlarmD functions (Maemo AlarmD, not flipclock specific)
//#include "alarmDFunc.c"

//Mouse handlers
#include "mouseFunc.c"

//Misc Utility functions
#include "miscFunc.c"

//Alarm functions (FlipClock, not alarmD)
#include "alarmFunc.c"

//Theme manager
#include "themeManager.c"

//User Preferences
#include "userPrefs.c"

//********************** Done Function Headers



void BMPShow(SDL_Surface * screen2, SDL_Surface * pic, int x, int y)
{
	SDL_Rect dest;
	dest.x = x;
	dest.y = y;
	dest.w = pic->w;
	dest.h = pic->h;

	SDL_BlitSurface(pic, NULL, screen2, &dest);

	// Update the changed portion of the screen 
	SDL_UpdateRects(screen2, 1, &dest);

	return;
}



/***********************************************************************
* loadAppGraphics()
*
* Function to load all of the graphics and images for the application itself
* (not associated with themes)
*
***********************************************************************/

int loadAppGraphics()
{

	SDL_Surface *tempBuffer;	//Surface to store a loaded image into before converting it

	char fullPath[512];			//Full path to files
	char elementsArr[MAXAPPELEMENTS][255];	//Image elements
	int i = 0;					//Good ol' i

	for (i = 0; i < MAXAPPELEMENTS; i++) {
		memset(elementsArr[i], 0, sizeof(elementsArr[i]));
	}

	sprintf(elementsArr[THEMECHANGETEXT], "themeChange.png");
	sprintf(elementsArr[SPLASHSCREEN], "splash.png");
	// sprintf(elementsArr[PMICON], "pmIcon.png");

	tempBuffer = NULL;

	//Load graphic elements (non digits)
	for (i = 0; i < MAXAPPELEMENTS; i++) {
		appElements[i] = NULL;
		bzero(fullPath, sizeof(fullPath));

		if (strlen(elementsArr[i]) > 0) {
			sprintf(fullPath, "%s%s", MEDIAPATH, elementsArr[i]);

			tempBuffer = IMG_Load(fullPath);

			if (tempBuffer != NULL) {
				printf("loaded app graphic %s\n", fullPath);
				appElements[i] = SDL_DisplayFormatAlpha(tempBuffer);
				SDL_FreeSurface(tempBuffer);
				tempBuffer = NULL;
			}
		}
	}


	return TRUE;
};


/***********************************************************************
* clearAppGraphics()
*
* Function to clean up all of the graphics and images for the application itself
* (Called when app is closing)
* (not associated with themes)
*
***********************************************************************/

int clearAppGraphics()
{
	int i;

	for (i = 0; i < MAXAPPELEMENTS; i++) {
		if (appElements[i] != NULL) {
			SDL_FreeSurface(appElements[i]);
			appElements[i] = NULL;
		}
	}

	return TRUE;
}


/***********************************************************************
* drawAllLabels(SDL_Surface *screenBuff, int newClockMode)
*
* Function draw all labels for a given clock mode
*
***********************************************************************/

void drawAllLabels(SDL_Surface * screenBuff, int newClockMode,
				   int changesOnly)
{
	int q;

	SDL_Rect labelRects[currentTheme.textLabels[newClockMode].labelCount + currentTheme.imageLabels[newClockMode].labelCount];	//Rects for labels to be updated
	int updateRectCount = 0;	//Number of label rects to update

	//Draw image labels first (that way background images can work if desired)
	if (currentTheme.imageLabels[newClockMode].labelCount > 0) {
		for (q = 0; q < currentTheme.imageLabels[newClockMode].labelCount;
			 q++) {
			if (changesOnly == 1 && runningAlarmIndex == -1) {
				if (currentTheme.imageLabels[newClockMode].labelArr[q]->
					labelKey != NULL) {
					//Redraw it!
					drawImageLabel(screenBuff,
								   currentTheme.imageLabels[newClockMode].
								   labelArr[q], newClockMode, NULL);

					labelRects[updateRectCount] =
						currentTheme.imageLabels[newClockMode].
						labelArr[q]->labelRect;
					updateRectCount++;

				}
			} else {
				drawImageLabel(screenBuff,
							   currentTheme.imageLabels[newClockMode].
							   labelArr[q], newClockMode, NULL);

				labelRects[updateRectCount] =
					currentTheme.imageLabels[newClockMode].labelArr[q]->
					labelRect;
				updateRectCount++;
			}
		}
	}

	if (changesOnly != 1) {
		if (renderedBuffer != NULL) {
			SDL_FreeSurface(renderedBuffer);
			renderedBuffer = NULL;
		}
		drawMoodOverlay(screenBuff, newClockMode, NULL);
		renderedBuffer =
			SDL_ConvertSurface(screenBuff, screenBuff->format,
							   screenBuff->flags);
	}
	//Now draw any labels that need to be drawn...
	if (currentTheme.textLabels[newClockMode].labelCount > 0) {
		for (q = 0; q < currentTheme.textLabels[newClockMode].labelCount;
			 q++) {
			if (changesOnly == 1 && runningAlarmIndex == -1) {
				//Only redraw dynamic text fields
				if (currentTheme.textLabels[newClockMode].labelArr[q]->
					labelKey != NULL) {
					//Redraw it!
					drawTextLabel(screenBuff,
								  currentTheme.textLabels[newClockMode].
								  labelArr[q], newClockMode, NULL);

					labelRects[updateRectCount] =
						currentTheme.textLabels[newClockMode].labelArr[q]->
						labelRect;
					updateRectCount++;
				}
			} else {
				drawTextLabel(screenBuff,
							  currentTheme.textLabels[newClockMode].
							  labelArr[q], newClockMode, NULL);

				labelRects[updateRectCount] =
					currentTheme.textLabels[newClockMode].labelArr[q]->
					labelRect;
				updateRectCount++;
			}
		}
	}


	if (screenBuff == screen) {
		SDL_UpdateRects(screenBuff, updateRectCount, labelRects);
	}

}

/***********************************************************************
* drawAllLabelsInRect(SDL_Surface *screenBuff, int newClockMode)
*
* Function draw all labels for a given clock mode that lie in a given rect
*
***********************************************************************/

void drawAllLabelsInRect(SDL_Surface * screenBuff, int newClockMode,
						 SDL_Rect * targetRect)
{
	int q;
	SDL_Rect overlapRect;


	SDL_Rect labelRects[currentTheme.textLabels[newClockMode].labelCount + currentTheme.imageLabels[newClockMode].labelCount];	//Rects for labels to be updated
	int updateRectCount = 0;	//Number of label rects to update

	//Draw image labels first (that way background images can work if desired)
	if (currentTheme.imageLabels[newClockMode].labelCount > 0) {
		for (q = 0; q < currentTheme.imageLabels[newClockMode].labelCount;
			 q++) {
			if (doRectsOverlap
				(&currentTheme.imageLabels[newClockMode].labelArr[q]->
				 labelRect, targetRect, &overlapRect)) {
				drawImageLabel(screenBuff,
							   currentTheme.imageLabels[newClockMode].
							   labelArr[q], newClockMode, &overlapRect);

				//labelRects[updateRectCount] = overlapRect;
				//updateRectCount++;
			}
		}
	}
	//Now draw any labels that need to be drawn...
	if (currentTheme.textLabels[newClockMode].labelCount > 0) {
		for (q = 0; q < currentTheme.textLabels[newClockMode].labelCount;
			 q++) {
			if (doRectsOverlap
				(&currentTheme.textLabels[newClockMode].labelArr[q]->
				 labelRect, targetRect, &overlapRect)) {
				drawTextLabel(screenBuff,
							  currentTheme.textLabels[newClockMode].
							  labelArr[q], newClockMode, &overlapRect);

				//labelRects[updateRectCount] = overlapRect;
				//updateRectCount++;
			}
		}
	}


	if (screenBuff == screen) {
		if (updateRectCount > 0) {
			SDL_UpdateRects(screenBuff, updateRectCount, labelRects);
		}
	}

}


/***********************************************************************
* int doRectsOverlap(SDL_Rect *firstRect, SDL_Rect *secondRect)
*
* Simple test to see if two rects overlap
***********************************************************************/

int doRectsOverlap(SDL_Rect * src1, SDL_Rect * src2, SDL_Rect * dest)
{
	int px0, py0, px1, py1;
	int cx0, cy0, cx1, cy1;
	int rx0, ry0, rx1, ry1;

	// fill in default (NULL) result rectangle
	if (dest != NULL) {
		dest->x = 0;
		dest->y = 0;
		dest->w = 0;
		dest->h = 0;
	}
	// get coordinates of the rectangles

	px0 = src1->x;
	py0 = src1->y;
	px1 = src1->x + src1->w - 1;
	py1 = src1->y + src1->h - 1;

	cx0 = src2->x;
	cy0 = src2->y;
	cx1 = src2->x + src2->w - 1;
	cy1 = src2->y + src2->h - 1;

	// check if the rectangles intersect

	if ((cx1 < px0))
		return 0;

	if ((cx0 > px1))
		return 0;

	if ((cy1 < py0))
		return 0;

	if ((cy0 > py1))
		return 0;

	// intersect x

	if (cx0 <= px0)
		rx0 = px0;
	else
		rx0 = cx0;

	if (cx1 >= px1)
		rx1 = px1;
	else
		rx1 = cx1;

	// intersect y

	if (cy0 <= py0)
		ry0 = py0;
	else
		ry0 = cy0;

	if (cy1 >= py1)
		ry1 = py1;
	else
		ry1 = cy1;

	// fill in result rect
	if (dest != NULL) {
		dest->x = rx0;
		dest->y = ry0;
		dest->w = (rx1 - rx0) + 1;
		dest->h = (ry1 - ry0) + 1;
		//printf("dest x %i, y %i, w %i h %i\n", rx0, ry0, dest->w, dest->h);
	}
	return 1;
};




/***********************************************************************
* drawTextLabel(SDL_Surface *screenBuff, struct textLabel *thisLabel, int newClockMode)
*
* Function draw a text label on the screen buffer
*
***********************************************************************/

void drawTextLabel(SDL_Surface * screenBuff, struct textLabel *thisLabel,
				   int newClockMode, SDL_Rect * clippingRect)
{
	//printf("Drawing text label %s!\n", thisLabel->labelKey);
	SDL_Surface *textSurface, *tempSurface;	//Surface for rendered text
	SDL_Color textColor, textBGColor;	//Colour of text to render
	Uint32 textBGColorPixel;
	char *textToRender;			//Text to draw
	TTF_Font *font;				//Font to use
	char *tempPointer;


	int i, offset, clipOffset = 0;

	struct tm tim;
	time_t now, timeDiff;
	SDL_Rect targetRect, textClipRect;


	textToRender = NULL;

	//Setup the targetRect
	memcpy(&targetRect, &thisLabel->labelRect,
		   sizeof(thisLabel->labelRect));

	//First render the text if necessary
	if (thisLabel->labelKey != NULL) {


		if (!strcmp(thisLabel->labelKey, "date")) {
			//Draw the date!
			now = time(NULL);
			tim = *(localtime(&now));

			textToRender = calloc(50, sizeof(char));

			strftime(textToRender, 50, thisLabel->labelValue, &tim);


		} else if (!strcmp(thisLabel->labelKey, "nextAlarmCountDown")) {
			//Not done yet...
			i = 0;

			if (alarmSnoozing == 1) {
				i = pendingSnoozeTime;

			} else if (nextAlarmTime != 0) {
				i = nextAlarmTime;

			}

			if (i > 0) {
				//Instead of time(NULL) we need to use the start of this minute...
				timeDiff = time(NULL);
				timeDiff = timeDiff - (timeDiff % 60);
				timeDiff = ((i - timeDiff) / 60);

				textToRender = calloc(50, sizeof(char));
				if ((timeDiff / 60) > 60) {
					sprintf(textToRender, "+%02ld:%02ld",
							(timeDiff / 60) % 60, (timeDiff % 60));
				} else {
					sprintf(textToRender, "%02ld:%02ld",
							(timeDiff / 60) % 60, (timeDiff % 60));
				}
			} else {
				return;
			}
		} else if (!strcmp(thisLabel->labelKey, "snoozingCountDown")) {
			//This is the same as above, but used for a "snoozing only" countdown
			i = 0;

			if (alarmSnoozing == 1) {
				i = pendingSnoozeTime;

			}

			if (i > 0) {
				//Instead of time(NULL) we need to use the start of this minute...
				timeDiff = time(NULL);
				timeDiff = timeDiff - (timeDiff % 60);
				timeDiff = ((i - timeDiff) / 60);

				textToRender = calloc(50, sizeof(char));
				if ((timeDiff / 60) > 0) {
					sprintf(textToRender, "%02ld:%02ld",
							(timeDiff / 60) % 60, (timeDiff % 60));
				} else {
					sprintf(textToRender, "%ld", (timeDiff % 60));
				}
			} else {
				return;
			}

		} else if (!strcmp(thisLabel->labelKey, "nextAlarmDate")) {
			//Date string for next alarm
			//printf("nextalarmtime: %i\n", nextAlarmTime);
			if (nextAlarmTime != 0 && alarmSnoozing != 1) {
				tim = *(localtime((time_t *) & nextAlarmTime));
				textToRender = calloc(50, sizeof(char));

				if (userPreferences.militaryTime != 0) {
					tempPointer = strstr(thisLabel->labelValue, "%l");
					if (tempPointer != NULL) {
						//Change to 24 hour instead of 12 hour display of alarm time
						i = thisLabel->labelValue - tempPointer;
						if (i < 0) {
							i = -i;
						}
						thisLabel->labelValue[i + 1] = 107;	//k

					}
				} else {
					//In military time, so convert if req.
					tempPointer = strstr(thisLabel->labelValue, "%k");
					if (tempPointer != NULL) {
						//Change to 24 hour instead of 12 hour display of alarm time
						i = thisLabel->labelValue - tempPointer;
						if (i < 0) {
							i = -i;
						}
						thisLabel->labelValue[i + 1] = 108;	//l

					}

				}

				strftime(textToRender, 50, thisLabel->labelValue, &tim);

			} else {
				return;
			}
		} else if (!strcmp(thisLabel->labelKey, "snoozeActive")) {
			if (alarmSnoozing == 1) {
				textToRender =
					calloc(strlen(thisLabel->labelValue) + 5,
						   sizeof(char));
				sprintf(textToRender, "%s", thisLabel->labelValue);

			} else {
				return;
			}


		} else if (!strcmp(thisLabel->labelKey, "selectedAlarmDate")) {



			textToRender = calloc(50, sizeof(char));

			tim.tm_wday =
				userPreferences.userAlarms[currentAlarmNum]->alarmDay;


			strftime(textToRender, 50, thisLabel->labelValue, &tim);
			//sprintf(textToRender, thisLabel->labelValue, dayShortNames[userPreferences.userAlarms[currentAlarmNum]->alarmDay]);
		} else if (!strcmp(thisLabel->labelKey, "nonMilitaryTime")) {
			//Label should only be visible if military time is off
			if (userPreferences.militaryTime != 0) {
				return;
			} else {
				textToRender =
					calloc(strlen(thisLabel->labelValue) + 2,
						   sizeof(char));
				sprintf(textToRender, "%s", thisLabel->labelValue);


			}
		} else if (!strcmp(thisLabel->labelKey, "selectedAlarmSound")) {
			//Only draw this if we;re in sound mode
			if (userPreferences.userAlarms[currentAlarmNum]->alertMode ==
				ALARMALERTMODESOUND
				|| userPreferences.userAlarms[currentAlarmNum]->
				alertMode == ALARMALERTMODESOUNDFOLDER) {

				if (userPreferences.userAlarms[currentAlarmNum]->sound ==
					NULL) {
					textToRender = calloc(50, sizeof(char));
					sprintf(textToRender, "None");
				} else {
					textToRender =
						calloc(strlen
							   (userPreferences.
								userAlarms[currentAlarmNum]->sound) + 2,
							   sizeof(char));

					//Juse show the basename, otherwise it's going to be stupid...

					sprintf(textToRender, "%s",
							baseName(userPreferences.
									 userAlarms[currentAlarmNum]->sound));
				}
			}
		} else if (!strcmp(thisLabel->labelKey, "alarmSnooze")) {
			if (userPreferences.userAlarms[runningAlarmIndex]->
				snoozeTime == 0) {
				//Abort, no snoozing allowed!
				return;
			} else {

				textToRender =
					calloc(strlen(thisLabel->labelValue) + 30,
						   sizeof(char));
				if (strstr(thisLabel->labelValue, "%i") != NULL) {
					sprintf(textToRender, thisLabel->labelValue,
							userPreferences.userAlarms[runningAlarmIndex]->
							snoozeTime);
				} else {
					sprintf(textToRender, "%s", thisLabel->labelValue);
				}
			}
		} else if (!strcmp(thisLabel->labelKey, "FMCurrentFrequency")) {
			//Label for the FM selector.
			//Get the current frequency
			if (userPreferences.userAlarms[currentAlarmNum]->alertMode ==
				ALARMALERTMODEFMRADIO) {
				i = -1;

				textToRender = calloc(15, sizeof(char));

				i = userPreferences.userAlarms[currentAlarmNum]->fmFreq;
				if (i > 88000 && i < 108000) {
					sprintf(textToRender, "%.1f", i / 1000.0);
				} else {
					sprintf(textToRender, "N/A");
				}
			} else {
				return;
			}
		} else if (!strcmp(thisLabel->labelKey, "%modalValue")) {
			//Label for slider Key.
			textToRender = calloc(15, sizeof(char));
			sprintf(textToRender, "%i", *modalIntVar);
		} else if (!strcmp(thisLabel->labelKey, "maxAlarmVol")) {
			//Label for slider Key.
			textToRender = calloc(20, sizeof(char));
			sprintf(textToRender, "%i", userPreferences.maxAlarmVol);

		} else if (!strcmp(thisLabel->labelKey, "insomniacDim")) {
			//Label for slider Key.
			textToRender = calloc(20, sizeof(char));
			sprintf(textToRender, "%i", userPreferences.insomniacDim);

		} else if (!strcmp(thisLabel->labelKey, "alarmSnoozeTime")) {
			textToRender = calloc(20, sizeof(char));
			if (userPreferences.userAlarms[currentAlarmNum]->snoozeTime >
				0) {
				sprintf(textToRender, "%i",
						userPreferences.userAlarms[currentAlarmNum]->
						snoozeTime);
			} else {
				sprintf(textToRender, "Off");
			}
		}

	} else {
		if (thisLabel->labelValue == NULL) {
			return;
		} else if (!strcmp(thisLabel->labelValue, "%modalKey")) {
			//Label for slider Key.
			textToRender = calloc(strlen(modalStrLabel) + 5, sizeof(char));
			sprintf(textToRender, "%s", modalStrLabel);
			//return;
		} else if (!strcmp(thisLabel->labelValue, "%modalMaxVal")) {
			//Label for slider Key.
			textToRender = calloc(25, sizeof(char));
			sprintf(textToRender, "%i", modalIntMax);
			//Max value should always be on the right of slider
			if (elements[SLIDERBASE] != NULL) {
				targetRect.x =
					((screen->w - elements[SLIDERBASE]->w) / 2) +
					(elements[SLIDERBASE]->w - targetRect.w);
			}

		} else if (!strcmp(thisLabel->labelValue, "%modalMinVal")) {
			//Label for slider Key.
			textToRender = calloc(25, sizeof(char));
			sprintf(textToRender, "%i", modalIntMin);
			//Min value should always be on the left of slider
			if (elements[SLIDERBASE] != NULL) {
				targetRect.x = (screen->w - elements[SLIDERBASE]->w) / 2;
			}

		} else {
			//Nothing special so use the value we have
			textToRender =
				calloc(strlen(thisLabel->labelValue) + 2, sizeof(char));
			sprintf(textToRender, "%s", thisLabel->labelValue);
		}
	}

	if (textToRender == NULL) {
		//no good, bail
		return;
	}

	if (thisLabel->usesMoodColor == 0) {
		//No mood colour, so use our colours!
		textColor.r = thisLabel->textColor[0];
		textColor.g = thisLabel->textColor[1];
		textColor.b = thisLabel->textColor[2];
	} else {
		textColor.r = userPreferences.moodRed;
		textColor.g = userPreferences.moodGreen;
		textColor.b = userPreferences.moodBlue;
	}

	if (!strcmp(thisLabel->textSize, "small")) {
		font = currentTheme.themeFonts[FONTSIZESMALL];
	} else if (!strcmp(thisLabel->textSize, "medium")) {
		font = currentTheme.themeFonts[FONTSIZEMEDIUM];
	} else if (!strcmp(thisLabel->textSize, "large")) {
		font = currentTheme.themeFonts[FONTSIZELARGE];
	} else if (!strcmp(thisLabel->textSize, "huge")) {
		font = currentTheme.themeFonts[FONTSIZEHUGE];
	} else {
		font = currentTheme.themeFonts[FONTSIZEMEDIUM];
	}

	//Okay, now off we go!

	if (thisLabel->usesBGColor == 0) {
		textSurface = NULL;
		textSurface =
			TTF_RenderUTF8_Blended(font, textToRender, textColor);

		//Clear the area behind the label
		if (renderedBuffer != NULL) {
			SDL_BlitSurface(renderedBuffer, &thisLabel->labelRect,
							screenBuff, &thisLabel->labelRect);
		}
		//drawThemeBackground(screenBuff, newClockMode, &thisLabel->labelRect);
	} else {
		//uses a background color instead
		textBGColor.r = thisLabel->textBGColor[0];
		textBGColor.g = thisLabel->textBGColor[1];
		textBGColor.b = thisLabel->textBGColor[2];

		textBGColorPixel =
			SDL_MapRGB(screenBuff->format, textBGColor.r, textBGColor.g,
					   textBGColor.b);

		textSurface = NULL;
		//Fill the space with the background color (incase the text is less than the area)
		SDL_FillRect(screenBuff, &thisLabel->labelRect, textBGColorPixel);
		textSurface =
			TTF_RenderUTF8_Shaded(font, textToRender, textColor,
								  textBGColor);

	}

	tempSurface = NULL;
	clipOffset = 5;
	if (thisLabel->textAngle > 0) {
		clipOffset = 0;
		tempSurface =
			SDL_ConvertSurface(textSurface, textSurface->format,
							   textSurface->flags);
		SDL_FreeSurface(textSurface);
		textSurface =
			rotozoomSurface(tempSurface, thisLabel->textAngle, 1.0, 1);
		SDL_FreeSurface(tempSurface);

	}

	//Finally draw the new label

	//printf("Target w is %i, actual w is %i\n", targetRect.w, textSurface->w);
	textClipRect.w = thisLabel->labelRect.w;
	textClipRect.h = thisLabel->labelRect.h - clipOffset;
	textClipRect.x = 0;
	textClipRect.y = clipOffset;

	if (thisLabel->textAlign == TEXTALIGNCENTER) {
		//We need to adjust to center the text
		offset = (thisLabel->labelRect.w - textSurface->w) / 2;
		targetRect.x += offset;
		targetRect.w -= offset;


	} else if (thisLabel->textAlign == TEXTALIGNRIGHT) {
		//Need to adjust to right align text
		offset = thisLabel->labelRect.w - textSurface->w;
		targetRect.x += offset;
		targetRect.w -= offset;
	}


	SDL_BlitSurface(textSurface, &textClipRect, screenBuff, &targetRect);

	//BMPShow(screenBuff, textSurface, thisLabel->labelRect.x, thisLabel->labelRect.y);

	if (textSurface != NULL) {
		SDL_FreeSurface(textSurface);
	}

	if (textToRender != NULL) {
		free(textToRender);
	}
}


/***********************************************************************
* drawImageLabel(SDL_Surface *screenBuff, struct imageLabel *thisLabel, int newClockMode)
*
* Function draw an image label on the screen buffer
*
***********************************************************************/

void drawImageLabel(SDL_Surface * screenBuff, struct imageLabel *thisLabel,
					int newClockMode, SDL_Rect * clippingRect)
{
	SDL_Rect targetRect;
	SDL_Rect *sourceRect = NULL;
	int i;						//Can't go wrong with i!
	double di;


	if (clippingRect == NULL) {
		memcpy(&targetRect, &thisLabel->labelRect,
			   sizeof(thisLabel->labelRect));
	} else {
		sourceRect = (SDL_Rect *) malloc(sizeof(SDL_Rect));
		memcpy(&targetRect, clippingRect, sizeof(clippingRect));

		if (clippingRect->x >= thisLabel->labelRect.x) {
			sourceRect->x = clippingRect->x - thisLabel->labelRect.x;
		} else {
			sourceRect->x = thisLabel->labelRect.x - clippingRect->x;
		}

		if (clippingRect->y >= thisLabel->labelRect.y) {
			sourceRect->y = clippingRect->y - thisLabel->labelRect.y;
		} else {
			sourceRect->y = thisLabel->labelRect.y - clippingRect->y;
		}
		sourceRect->w = clippingRect->w;
		sourceRect->h = clippingRect->h;

	}




	if (thisLabel->labelKey != NULL) {

		//First figure out if the image label has a value tied to it
		if (!strcmp(thisLabel->labelKey, "alarmOnOff")) {
			//Clear the area behind the label
			drawThemeBackground(screenBuff, newClockMode, &targetRect);

			//this is the label for the setAlarmOnOff button
			drawButton(screenBuff, elements[BUTTONONOFF], &targetRect,
					   userPreferences.userAlarms[currentAlarmNum]->
					   enabled);

		} else if (!strcmp(thisLabel->labelKey, "alarmAMPM")) {
			//this is the label for the setAlarmAMPM button
			if (userPreferences.militaryTime == 0) {
				//Clear the area behind the label
				drawThemeBackground(screenBuff, newClockMode, &targetRect);

				drawButton(screenBuff, elements[BUTTONAMPM], &targetRect,
						   userPreferences.userAlarms[currentAlarmNum]->
						   ampm);
			}
		} else if (!strcmp(thisLabel->labelKey, "alarmMode")) {
			//Clear the area behind the label
			drawThemeBackground(screenBuff, newClockMode, &targetRect);

			//This is the button for alarm mode (play sound, etc)
			drawButton(screenBuff, elements[BUTTONALERTMODE], &targetRect,
					   userPreferences.userAlarms[currentAlarmNum]->
					   alertMode);
		/**** Not needed anymore, handled by slider 
		} else if (!strcmp(thisLabel->labelKey, "alarmSnoozeTime")) {
			//Clear the area behind the label
			drawThemeBackground(screenBuff, newClockMode, &targetRect);
			
			//This is the button for snooze time (snooze time is in 5 min increments)
			drawButton(screenBuff, elements[BUTTONSNOOZETIME],&targetRect, (userPreferences.userAlarms[currentAlarmNum]->snoozeTime / 5));
		*****************/
		} else if (!strcmp(thisLabel->labelKey, "alarmOnceLoop")) {
			//This is the button for once/loop alarm sound setting; this should only be visible if alertMode is sound
			if (userPreferences.userAlarms[currentAlarmNum]->alertMode ==
				ALARMALERTMODESOUND) {

				//Clear the area behind the label
				drawThemeBackground(screenBuff, newClockMode, &targetRect);

				drawButton(screenBuff, elements[BUTTONONCELOOP],
						   &targetRect,
						   (userPreferences.userAlarms[currentAlarmNum]->
							loopSound));
			} else if (userPreferences.userAlarms[currentAlarmNum]->
					   alertMode == ALARMALERTMODESOUNDFOLDER) {

				//Clear the area behind the label
				drawThemeBackground(screenBuff, newClockMode, &targetRect);

				drawButton(screenBuff, elements[BUTTONFIXEDRANDOM],
						   &targetRect,
						   (userPreferences.userAlarms[currentAlarmNum]->
							loopSound));
			}

		} else if (!strcmp(thisLabel->labelKey, "currentAlertModeSound")) {
			if (userPreferences.userAlarms[currentAlarmNum]->alertMode ==
				ALARMALERTMODESOUND
				|| userPreferences.userAlarms[currentAlarmNum]->
				alertMode == ALARMALERTMODESOUNDFOLDER) {
				//Good to go!

				if (thisLabel->imageObj != NULL) {

					//Clear the area behind the label
					drawThemeBackground(screenBuff, newClockMode,
										&targetRect);

					SDL_BlitSurface(thisLabel->imageObj, sourceRect,
									screenBuff, &targetRect);

				}

			}
		} else if (!strcmp(thisLabel->labelKey, "currentAlertModeFMRadio")) {
			if (userPreferences.userAlarms[currentAlarmNum]->alertMode ==
				ALARMALERTMODEFMRADIO) {
				//Good to go!

				if (thisLabel->imageObj != NULL) {

					//Clear the area behind the label
					drawThemeBackground(screenBuff, newClockMode,
										&targetRect);

					SDL_BlitSurface(thisLabel->imageObj, sourceRect,
									screenBuff, &targetRect);

				}

			}


		} else if (!strcmp(thisLabel->labelKey, "alarmSnoozeEnabled")) {
			if (userPreferences.userAlarms[runningAlarmIndex]->
				snoozeTime != 0) {
				//Clear the area behind the label
				drawThemeBackground(screenBuff, newClockMode, &targetRect);

				//Abort, no snoozing allowed!
				SDL_BlitSurface(thisLabel->imageObj, sourceRect,
								screenBuff, &targetRect);
			}
		} else if (!strcmp(thisLabel->labelKey, "nextAlarmEnabled")) {
			if (nextAlarmTime != 0) {
				//Clear the area behind the label
				drawThemeBackground(screenBuff, newClockMode, &targetRect);

				SDL_BlitSurface(thisLabel->imageObj, sourceRect,
								screenBuff, &targetRect);
			} else {
				return;
			}

		} else if (!strcmp(thisLabel->labelKey, "militaryTime")) {
			//Clear the area behind the label
			drawThemeBackground(screenBuff, newClockMode, &targetRect);

			//This is the button for alarm mode (play sound, etc)
			drawButton(screenBuff, elements[BUTTONMILITARYTIME],
					   &targetRect, userPreferences.militaryTime);

		} else if (!strcmp(thisLabel->labelKey, "alarmStyle")) {
			//Clear the area behind the label
			drawThemeBackground(screenBuff, newClockMode, &targetRect);

			//This is the button for alarm mode (play sound, etc)
			drawButton(screenBuff, elements[BUTTONALARMSTYLE], &targetRect,
					   userPreferences.alarmControlMode);


		} else if (!strcmp(thisLabel->labelKey, "secondsVisible")) {
			//Clear the area behind the label
			drawThemeBackground(screenBuff, newClockMode, &targetRect);

			//This is the button for seconds visible
			drawButton(screenBuff, elements[BUTTONONOFF], &targetRect,
					   userPreferences.showSeconds);

		} else if (!strcmp(thisLabel->labelKey, "insomniacLock")) {
			if (isTablet) {
				//Clear the area behind the label
				drawThemeBackground(screenBuff, newClockMode, &targetRect);

				//This is the button for seconds visible
				drawButton(screenBuff, elements[BUTTONONOFF], &targetRect,
						   userPreferences.insomniacLocked);
			} else {
				return;
			}

		} else if (!strcmp(thisLabel->labelKey, "insomniacMode")) {
			//this is the label for the setInsomniacMode button
			if (isTablet) {
				//Clear the area behind the label
				drawThemeBackground(screenBuff, newClockMode, &targetRect);

				drawButton(screenBuff, elements[BUTTONONOFF], &targetRect,
						   userPreferences.insomniacModeOn);
			} else {
				return;
			}


		} else if (!strcmp(thisLabel->labelKey, "FMSelector")) {
			//Label for the FM selector.
			//Get the current frequency
			if (userPreferences.userAlarms[currentAlarmNum]->alertMode ==
				ALARMALERTMODEFMRADIO) {
				i = -1;

				//Clear the area behind the label
				drawThemeBackground(screenBuff, newClockMode, &targetRect);

				i = userPreferences.userAlarms[currentAlarmNum]->fmFreq;
				if (i > 88000 && i < 108000) {

					//Valid Frequency, so now draw at the right position
					di = i - 88100.0;
					di = di / 20000;

					i = targetRect.x + (targetRect.w * di);

					targetRect.x = i;



					SDL_BlitSurface(thisLabel->imageObj, sourceRect,
									screenBuff, &targetRect);


				} else {
					printf("invalid freq!\n");
				}

			}

		} else if (!strcmp(thisLabel->labelKey, "sliderPointer")) {
			//Label for the slider pointer

			i = -1;

			i = *modalIntVar;

			if (i >= modalIntMin && i <= modalIntMax) {

				//Valid Frequency, so now draw at the right position
				di = i - modalIntMin;
				di = di / (modalIntMax - modalIntMin);

				i = targetRect.x +
					((targetRect.w - thisLabel->imageObj->w) * di);

				targetRect.x = i;

				//Clear the area behind the label
				SDL_BlitSurface(elements[SLIDERBASE], NULL, screenBuff,
								&thisLabel->labelRect);
				SDL_BlitSurface(thisLabel->imageObj, sourceRect,
								screenBuff, &targetRect);


			} else {
				printf("invalid slider value!\n");
			}

		}
	} else {
		//Nothing special so..

		//Clear the area behind the label
		drawThemeBackground(screenBuff, newClockMode, &targetRect);

		//Check to see if we need to use mood or not
		if (thisLabel->usesMoodColor == 1) {

			//MoodColor 1 means blend alpha, just like mood images for screens normally
			//SDL_BlitSurface(thisLabel->imageObj, sourceRect, screenBuff, &targetRect);
			drawMoodOverlayByImage(screenBuff, thisLabel->imageObj,
								   &targetRect);
		} else {
			if (thisLabel->imageObj != NULL) {
				SDL_BlitSurface(thisLabel->imageObj, sourceRect,
								screenBuff, &targetRect);
			}
		}
	}

	if (sourceRect != NULL) {
		free(sourceRect);
	}

}



/***********************************************************************
* transitionScreen(int transitionMode, int transitionFrames, SDL_Surface *oldScreen, SDL_Surface *newScreen)
*
* Function to transition one screen (oldScreen) to a new one (newScreen) on
* the display surface. TransitionMode is a constant controlling the visual effect
*, and transitionFrames is the number of frames that the animation should occur
* over.
************************************************************************/

void transitionScreen(int transitionMode, int transitionFrames,
					  SDL_Surface * oldScreen, SDL_Surface * newScreen)
{
	Uint32 backColour;			//Background colour; could be defined by themes in the future, but for now it's always black
	int frames;
	SDL_Rect src, dest;
	SDL_Surface *tempBuffer;

	int i = 0;
	int halfFrames;
	float fadeVal;

	tempBuffer = NULL;

	//Setup backcolour to be black for now
	backColour = SDL_MapRGB(screen->format, 0, 0, 0);
	//Number of frames the animation will occur over
	frames = transitionFrames;

	switch (transitionMode) {
	case (TRANSITIONSLIDEUP):
		//Slide existing screen UP
		for (i = 0; i < frames + 1; i++) {
			//Fill black
			SDL_FillRect(screen, NULL, backColour);

			//Move up
			src.x = 0;
			src.y = (screen->h / frames) * i;
			src.h = oldScreen->h;
			src.w = oldScreen->w;

			dest.x = 0;
			dest.y = 0;
			dest.w = screen->w;
			dest.h = screen->h;


			//Copy partial source to screen
			SDL_BlitSurface(oldScreen, &src, screen, &dest);
			src.y = screen->h - ((screen->h / frames) * i);
			dest.w = screen->w;
			dest.h = ((screen->h / frames) * i);
			SDL_BlitSurface(newScreen, &dest, screen, &src);


			//Should copy new screen here too, but do that later...

			SDL_UpdateRect(screen, 0, 0, 0, 0);
			usleep(100);
		}
		break;
	case TRANSITIONSLIDEDOWN:
		//Slide existing screen down

		for (i = 0; i < frames + 1; i++) {

			//Fill black
			SDL_FillRect(screen, NULL, backColour);

			//Move down
			src.x = 0;
			src.y = 0;
			src.w = oldScreen->w;
			src.h = oldScreen->h;

			dest.x = 0;
			dest.y = (screen->h / frames) * i;
			dest.w = oldScreen->w;
			dest.h = oldScreen->h;
			//dest.w = pic->w;
			//dest.h = pic->h;

			//Copy partial source to screen
			SDL_BlitSurface(oldScreen, &src, screen, &dest);
			src.y = screen->h - ((screen->h / frames) * i);
			SDL_BlitSurface(newScreen, &src, screen, NULL);

			SDL_UpdateRect(screen, 0, 0, 0, 0);
			usleep(100);


		}
		break;
	case TRANSITIONSLIDERIGHT:
		//Slide existing screen to the right

		for (i = 0; i < frames + 1; i++) {

			//Fill black
			SDL_FillRect(screen, NULL, backColour);

			//Move right
			src.x = 0;
			src.y = 0;
			src.w = oldScreen->w;
			src.h = oldScreen->h;

			dest.x = (screen->w / frames) * i;;
			dest.y = 0;
			dest.w = oldScreen->w;
			dest.h = oldScreen->h;
			//dest.w = pic->w;
			//dest.h = pic->h;

			//Copy partial source to screen
			SDL_BlitSurface(oldScreen, &src, screen, &dest);
			src.x = screen->w - ((screen->w / frames) * i);
			SDL_BlitSurface(newScreen, &src, screen, NULL);

			SDL_UpdateRect(screen, 0, 0, 0, 0);
			usleep(100);


		}
		break;

	case TRANSITIONSLIDELEFT:
		//Slide existing screen to the left

		for (i = 0; i < frames + 1; i++) {
			//Fill black
			SDL_FillRect(screen, NULL, backColour);

			//Move up
			src.x = (screen->w / frames) * i;
			src.y = 0;
			src.h = oldScreen->h;
			src.w = oldScreen->w;

			dest.x = 0;
			dest.y = 0;
			dest.w = screen->w;
			dest.h = screen->h;


			//Copy partial source to screen
			SDL_BlitSurface(oldScreen, &src, screen, &dest);
			src.x = screen->w - ((screen->w / frames) * i);
			dest.h = screen->h;
			dest.w = ((screen->w / frames) * i);
			SDL_BlitSurface(newScreen, &dest, screen, &src);


			//Should copy new screen here too, but do that later...

			SDL_UpdateRect(screen, 0, 0, 0, 0);
			usleep(100);
		}
		break;


	case TRANSITIONFADE:
		//Fade from one screen to blank, then fade the new one in
		src.x = 0;
		src.y = 0;
		src.w = oldScreen->w;
		src.h = oldScreen->h;
		//Fade out existing screen
		halfFrames = frames / 4;

		for (i = 0; i < halfFrames; i++) {
			fadeVal = (255 / halfFrames) * i;
			//printf("newAlpha %i\n", (int) fadeVal);
			tempBuffer = SDL_DisplayFormatAlpha(screen);

			backColour =
				SDL_MapRGBA(tempBuffer->format, 0, 0, 0, (int) fadeVal);
			SDL_FillRect(tempBuffer, NULL, backColour);

			SDL_BlitSurface(oldScreen, &src, screen, &src);
			SDL_BlitSurface(tempBuffer, &src, screen, &src);
			SDL_UpdateRect(screen, 0, 0, 0, 0);

			SDL_FreeSurface(tempBuffer);
			//usleep(500);


		}
		usleep(100);
		//Fade in new screen
		for (i = 0; i < halfFrames; i++) {
			fadeVal = 255 - ((255 / halfFrames) * i);
			//printf("newAlpha %i\n", (int) fadeVal);
			tempBuffer = SDL_DisplayFormatAlpha(screen);

			backColour =
				SDL_MapRGBA(tempBuffer->format, 0, 0, 0, (int) fadeVal);
			SDL_FillRect(tempBuffer, NULL, backColour);

			SDL_BlitSurface(newScreen, &src, screen, &src);
			SDL_BlitSurface(tempBuffer, &src, screen, &src);
			SDL_UpdateRect(screen, 0, 0, 0, 0);

			SDL_FreeSurface(tempBuffer);


		}

		break;

	}
	src.x = 0;
	src.y = 0;
	src.w = newScreen->w;
	src.h = newScreen->h;
	SDL_BlitSurface(newScreen, &src, screen, &src);
	SDL_UpdateRect(screen, 0, 0, 0, 0);
}

/***********************************************************************
* gotoSettings()
*
* Clock mode pseudo function for use with buttons to move to setting mode
***********************************************************************/

void gotoSettings()
{
	changeClockMode(CLOCKMODEALARMSETTINGS);
}

/***********************************************************************
* gotoNormal()
*
* Clock mode pseudo function for use with buttons to move to normal mode
***********************************************************************/

void gotoNormal()
{
	changeClockMode(CLOCKMODENORMAL);
}

/***********************************************************************
* gotoClockSettings()
*
* Clock mode pseudo function for use with buttons to move to clock setting mode
***********************************************************************/
void gotoClockSettings()
{
	changeClockMode(CLOCKMODECLOCKSETTINGS);
}

/***********************************************************************
* gotoTimer()
*
* Clock mode pseudo function for use with buttons to move to clock timer/stopwatch mode
***********************************************************************/
void gotoTimer()
{
	changeClockMode(CLOCKMODETIMER);
}


/***********************************************************************
* changeClockMode(newClockMode)
*
* Function to change the mode of the clock to the given mode. This is the master
* function that can be called directly or through the pseudo-functions
* CLOCKMODEWINDOWED
* gotoSettingsMode, gotoNormalMode, gotoWindowedMode, gotoHelpMode, etc
************************************************************************/

void changeClockMode(int newClockMode)
{
	SDL_Surface *oldScreen;		//The existing frame buffer
	SDL_Surface *newScreen;		//The new frame buffer  

	int frames = 16;
	int transMode;


	if (clockMode == newClockMode) {
		//We're already at the desired mode, so don't bother doing anything
		return;
	}
	//Possible modes right now are CLOCKMODENORMAL, CLOCKMODEWINDOWED, CLOCKMODEALARMSETTINGS, CLOCKMODECLOCKSETTINGS, CLOCKMODETIMER, CLOCKMODEHELP 
	switch (clockMode) {
	case CLOCKMODENORMAL:
		switch (newClockMode) {
		case CLOCKMODEALARMSETTINGS:
			//We want to go to settings mode

			//Goto "changing mode" so nothing else happens while we're animating
			clockMode = CLOCKMODECHANGING;

			oldScreen =
				SDL_ConvertSurface(screen, screen->format, screen->flags);
			newScreen = drawStaticScreen(newClockMode);

			transMode = TRANSITIONSLIDEUP;
			switch (currentTheme.transitionMode) {
			case TRANSITIONSLIDE:
				transMode = TRANSITIONSLIDEUP;
				break;
			case TRANSITIONFADE:
				transMode = TRANSITIONFADE;
				break;
			}

			transitionScreen(transMode, frames, oldScreen, newScreen);

			SDL_FreeSurface(oldScreen);
			SDL_FreeSurface(newScreen);

			//Now go to the new clock mode since the animation is done
			clockMode = newClockMode;
			break;

		case CLOCKMODEALARMRUNNING:
			//We Want to go into alarm executing mode
			//basically all we do is draw the extra labels

			drawAllLabels(screen, newClockMode, 0);
			clockMode = newClockMode;

			break;

		case CLOCKMODECLOCKSETTINGS:
			//change to settings screen
			//Goto "changing mode" so nothing else happens while we're animating
			clockMode = CLOCKMODECHANGING;

			oldScreen =
				SDL_ConvertSurface(screen, screen->format, screen->flags);
			newScreen = drawStaticScreen(newClockMode);

			transMode = TRANSITIONSLIDEDOWN;
			switch (currentTheme.transitionMode) {
			case TRANSITIONSLIDE:
				transMode = TRANSITIONSLIDEDOWN;
				break;
			case TRANSITIONFADE:
				transMode = TRANSITIONFADE;
				break;
			}

			transitionScreen(transMode, frames, oldScreen, newScreen);

			SDL_FreeSurface(oldScreen);
			SDL_FreeSurface(newScreen);

			//Now go to the new clock mode since the animation is done
			clockMode = newClockMode;
			break;

		case CLOCKMODETIMER:
			//change to timer/stopwatch screen
			//Goto "changing mode" so nothing else happens while we're animating
			clockMode = CLOCKMODECHANGING;

			oldScreen =
				SDL_ConvertSurface(screen, screen->format, screen->flags);
			newScreen = drawStaticScreen(newClockMode);

			transMode = TRANSITIONSLIDERIGHT;
			switch (currentTheme.transitionMode) {
			case TRANSITIONSLIDE:
				transMode = TRANSITIONSLIDERIGHT;
				break;
			case TRANSITIONFADE:
				transMode = TRANSITIONFADE;
				break;
			}

			transitionScreen(transMode, frames, oldScreen, newScreen);

			SDL_FreeSurface(oldScreen);
			SDL_FreeSurface(newScreen);

			//Now go to the new clock mode since the animation is done
			clockMode = newClockMode;
			break;


		}
		break;
	case CLOCKMODEALARMSETTINGS:
		//Update alarms since were leaving the alarm settings screen
		updateAlarms();

		switch (newClockMode) {
		case CLOCKMODENORMAL:
			//We're going back to normal clock mode from the settings screen
			//Goto "changing mode" so nothing else happens while we're animating
			clockMode = CLOCKMODECHANGING;

			oldScreen =
				SDL_ConvertSurface(screen, screen->format, screen->flags);
			newScreen = drawStaticScreen(newClockMode);

			transMode = TRANSITIONSLIDEDOWN;
			switch (currentTheme.transitionMode) {
			case TRANSITIONSLIDE:
				transMode = TRANSITIONSLIDEDOWN;
				break;
			case TRANSITIONFADE:
				transMode = TRANSITIONFADE;
				break;
			}

			transitionScreen(transMode, frames, oldScreen, newScreen);

			SDL_FreeSurface(oldScreen);
			SDL_FreeSurface(newScreen);
			//Now go to the new clock mode since the animation is done
			clockMode = newClockMode;
			break;
		case CLOCKMODECLOCKSETTINGS:
			//going "Around" to the clock settings screen
			clockMode = CLOCKMODECHANGING;

			oldScreen =
				SDL_ConvertSurface(screen, screen->format, screen->flags);
			newScreen = drawStaticScreen(newClockMode);

			transMode = TRANSITIONSLIDEUP;
			switch (currentTheme.transitionMode) {
			case TRANSITIONSLIDE:
				transMode = TRANSITIONSLIDEUP;
				break;
			case TRANSITIONFADE:
				transMode = TRANSITIONFADE;
				break;
			}

			transitionScreen(transMode, frames, oldScreen, newScreen);

			SDL_FreeSurface(oldScreen);
			SDL_FreeSurface(newScreen);

			//Now go to the new clock mode since the animation is done
			clockMode = newClockMode;


			break;


		}
		break;
	case CLOCKMODEALARMRUNNING:
		//Alarm can only be running in normal mode, so change back to that

		clockMode = CLOCKMODECHANGING;

		//oldScreen = SDL_ConvertSurface(screen, screen->format, screen->flags);
		newScreen = drawStaticScreen(newClockMode);

		//Better fading as we don't want that "fade to black" effect
		fadeScreenIn(newScreen, 0);

		//transMode = TRANSITIONFADE;

		//transitionScreen(transMode, frames, oldScreen, newScreen);

		//SDL_FreeSurface(oldScreen);
		SDL_FreeSurface(newScreen);
		//Now go to the new clock mode since the animation is done
		clockMode = newClockMode;
		break;

	case CLOCKMODECLOCKSETTINGS:
		//Going back to normal screen
		switch (newClockMode) {
		case CLOCKMODENORMAL:
			transMode = TRANSITIONSLIDEUP;
			break;
		case CLOCKMODEALARMSETTINGS:
			transMode = TRANSITIONSLIDEDOWN;
			break;
		}

		//We're going back to normal clock mode from the settings screen
		//Goto "changing mode" so nothing else happens while we're animating
		clockMode = CLOCKMODECHANGING;

		oldScreen =
			SDL_ConvertSurface(screen, screen->format, screen->flags);
		newScreen = drawStaticScreen(newClockMode);

		//transMode = TRANSITIONSLIDEUP;
		switch (currentTheme.transitionMode) {
		case TRANSITIONSLIDE:
			//transMode = TRANSITIONSLIDEUP;
			break;
		case TRANSITIONFADE:
			transMode = TRANSITIONFADE;
			break;
		}

		transitionScreen(transMode, frames, oldScreen, newScreen);

		SDL_FreeSurface(oldScreen);
		SDL_FreeSurface(newScreen);
		//Now go to the new clock mode since the animation is done
		clockMode = newClockMode;

		break;

	case CLOCKMODETIMER:
		//Going back to normal clock mode from timer mode?
		transMode = TRANSITIONSLIDELEFT;
		switch (newClockMode) {
		case CLOCKMODENORMAL:
			transMode = TRANSITIONSLIDELEFT;
			break;
			//Others as required later
		}

		//We're going back to normal clock mode from the settings screen
		//Goto "changing mode" so nothing else happens while we're animating
		clockMode = CLOCKMODECHANGING;

		oldScreen =
			SDL_ConvertSurface(screen, screen->format, screen->flags);
		newScreen = drawStaticScreen(newClockMode);


		switch (currentTheme.transitionMode) {
		case TRANSITIONSLIDE:
			//transMode = TRANSITIONSLIDEUP;
			break;
		case TRANSITIONFADE:
			transMode = TRANSITIONFADE;
			break;
		}

		transitionScreen(transMode, frames, oldScreen, newScreen);

		SDL_FreeSurface(oldScreen);
		SDL_FreeSurface(newScreen);
		//Now go to the new clock mode since the animation is done
		clockMode = newClockMode;

		break;

	}

}

/***********************************************************************
* drawStaticScreen 
*
* Function to render a given clockmode screen onto a screen buffer and return it
*
************************************************************************/

SDL_Surface *drawStaticScreen(int newClockMode)
{
	SDL_Surface *screenBuffer;

	SDL_Rect srcRect;
//  int j=0;

	//Regenerate the background
	if (themeBGBuffer != NULL) {
		SDL_FreeSurface(themeBGBuffer);
		themeBGBuffer = NULL;
	}



	screenBuffer =
		SDL_ConvertSurface(screen, screen->format, screen->flags);

	switch (newClockMode) {
	case CLOCKMODENORMAL:
		//Draw the normal clock mode screen
		drawThemeBackground(screenBuffer, CLOCKMODENORMAL, NULL);

		//This should really be a label, but...
		if (userPreferences.showSeconds == 1
			&& elements[SECONDSBAR] != NULL) {
			BMPShow(screenBuffer, elements[SECONDSBAR],
					currentTheme.digitPos[SECONDSBARRECT].x,
					currentTheme.digitPos[SECONDSBARRECT].y);
		}

		drawAllLabels(screenBuffer, CLOCKMODENORMAL, 0);

		updateClock(screenBuffer, CLOCKMODENORMAL);
		updateSecondsBar(screenBuffer, 1);

		break;
	case CLOCKMODEALARMSETTINGS:
		//Draw background
		drawThemeBackground(screenBuffer, CLOCKMODEALARMSETTINGS, NULL);

		//Draw the digit scrollers... (fingers crossed)
		drawAlarmDigits(screenBuffer,
						userPreferences.userAlarms[currentAlarmNum]->
						alarmHHMM);

		//Draw the alarm day pickers
		drawAllAlarmDays(screenBuffer);

		//Draw screen labels...
		//Now draw any labels that need to be drawn...
		drawAllLabels(screenBuffer, CLOCKMODEALARMSETTINGS, 0);





		break;

	case CLOCKMODEMOODPICKER:
		//Mood picker
		SDL_FreeSurface(screenBuffer);

		//We want to use a blank buffer to overlay!
		screenBuffer =
			SDL_ConvertSurface(elements[MOODPICKERIMG],
							   elements[MOODPICKERIMG]->format,
							   elements[MOODPICKERIMG]->flags);

		//Draw any labels for this screen
		drawAllLabels(screenBuffer, CLOCKMODEMOODPICKER, 0);

		//Draw the current mood
		drawMoodBox(screenBuffer);

		break;

	case CLOCKMODEFMRADIOPICKER:
		//Mood picker
		SDL_FreeSurface(screenBuffer);

		//We want to use a blank buffer to overlay!
		screenBuffer =
			SDL_ConvertSurface(elements[FMRADIOPICKERIMG],
							   elements[FMRADIOPICKERIMG]->format,
							   elements[FMRADIOPICKERIMG]->flags);

		//Draw any labels for this screen
		drawAllLabels(screenBuffer, CLOCKMODEFMRADIOPICKER, 0);

		//Draw the current mood
		//drawMoodBox(screenBuffer);

		break;

	case CLOCKMODECLOCKSETTINGS:
		//Draw background
		drawThemeBackground(screenBuffer, CLOCKMODECLOCKSETTINGS, NULL);

		//Draw all labels/buttons
		drawAllLabels(screenBuffer, CLOCKMODECLOCKSETTINGS, 0);

		break;

	case CLOCKMODETIMER:
		//Draw background
		drawThemeBackground(screenBuffer, newClockMode, NULL);

		//test
		//for (j =0; j < 6; j++) {
		//  drawAlarmScrollerDigit(screenBuffer, j, 0, &currentTheme.timerDigitPos[j]);
		//}
		updateTimerDigits(screenBuffer, 1);


		//Draw all labels/buttons
		drawAllLabels(screenBuffer, newClockMode, 0);

		break;

	case CLOCKMODESLIDER:
		//Stuff goes here
		drawFadeOverlay(screenBuffer);

		//Center it
		srcRect.x = (screenBuffer->w - elements[SLIDERBASE]->w) / 2;
		srcRect.y = (screenBuffer->h - elements[SLIDERBASE]->h) / 2;
		SDL_BlitSurface(elements[SLIDERBASE], NULL, screenBuffer,
						&srcRect);

		//Draw any labels for this screen
		drawAllLabels(screenBuffer, CLOCKMODESLIDER, 0);


		break;
	}

	return screenBuffer;

}



/***********************************************************************
* flipDigit()
*
* Function to handle actually drawing a digit on screen.
* This function will also automatically setup "flip" animations if they're
* available.
* 
*
***********************************************************************/

int flipDigit(int digitIndex)
{
	struct clockDigit *thisDigit;
	SDL_Surface *tempBuffer = NULL;
	int redraw = 0;


	thisDigit = &clockDigits[digitIndex];

	Uint32 moodColor;
	SDL_Rect colorBox;

	if (numbers[thisDigit->lastValue] != NULL) {
		colorBox.x = thisDigit->xPos;
		colorBox.y = thisDigit->yPos;
		colorBox.w = numbers[thisDigit->lastValue]->w;
		colorBox.h = numbers[thisDigit->lastValue]->h;

	}

	if (currentTheme.usesAlphaBG == 1) {
		if (numbers[thisDigit->lastValue] != NULL) {
			moodColor =
				SDL_MapRGB(screen->format, userPreferences.moodRed,
						   userPreferences.moodGreen,
						   userPreferences.moodBlue);

			SDL_FillRect(screen, &colorBox, moodColor);
		}

	}
	//Animation based on image files...
	if (currentTheme.digitTransMode == 0) {

		if (thisDigit->aniFrame < animationCount
			&& digitAnimation[thisDigit->aniFrame] != NULL) {
			//printf("animating frame %i on digit %i\n", thisDigit->aniFrame, digitIndex);
			//Some animation to occur

			//Render the animation frame
			//BMPShow(screen, digitAnimation[thisDigit->aniFrame], thisDigit->xPos, thisDigit->yPos);
			SDL_BlitSurface(digitAnimation[thisDigit->aniFrame], NULL,
							screen, &colorBox);


			//Increment the animation frame
			thisDigit->aniFrame++;
			//Setup timeout
			g_timeout_add(50, (GSourceFunc) flipDigit,
						  (gpointer) digitIndex);
		} else {
			thisDigit->aniFrame = 0;
			redraw = 1;
		}
	} else if (currentTheme.digitTransMode == 1) {
		//Animation is fading out/in
		if (thisDigit->aniFrame < 5) {
			//Still fading out

			//The overlay is the existing screen
			tempBuffer = SDL_DisplayFormat(screen);
			drawThemeBackground(tempBuffer, CLOCKMODENORMAL, &colorBox);
			SDL_SetAlpha(tempBuffer, SDL_SRCALPHA,
						 (255 / 5) * thisDigit->aniFrame);

			SDL_BlitSurface(tempBuffer, &colorBox, screen, &colorBox);

			//Done, clean up
			if (tempBuffer != NULL) {
				SDL_FreeSurface(tempBuffer);
			}
			//Increment the animation frame
			thisDigit->aniFrame++;

			//Setup timeout
			g_timeout_add(20, (GSourceFunc) flipDigit, (int *) digitIndex);
		} else if (thisDigit->aniFrame < 10) {
			//Still fading new digit in

			//The overlay is the new screen
			tempBuffer = SDL_DisplayFormat(screen);
			drawThemeBackground(tempBuffer, CLOCKMODENORMAL, &colorBox);
			if (numbers[thisDigit->lastValue] != NULL) {
				SDL_BlitSurface(numbers[thisDigit->lastValue], NULL,
								tempBuffer, &colorBox);
			}
			//printf("new frame %i alpha %i\n", thisDigit->aniFrame, (255 /5) * (thisDigit->aniFrame - 5));
			SDL_SetAlpha(tempBuffer, SDL_SRCALPHA,
						 (255 / 5) * (thisDigit->aniFrame - 5));

			SDL_BlitSurface(tempBuffer, &colorBox, screen, &colorBox);

			//Done, clean up
			if (tempBuffer != NULL) {
				SDL_FreeSurface(tempBuffer);
			}
			//Increment the animation frame
			thisDigit->aniFrame++;

			//Setup timeout
			g_timeout_add(20, (GSourceFunc) flipDigit, (int *) digitIndex);
		} else {
			//The overlay is the new screen
			tempBuffer = SDL_DisplayFormat(screen);
			drawThemeBackground(tempBuffer, CLOCKMODENORMAL, &colorBox);

			SDL_BlitSurface(tempBuffer, &colorBox, screen, &colorBox);

			//Done, clean up
			if (tempBuffer != NULL) {
				SDL_FreeSurface(tempBuffer);
			}

			//Done animating
			thisDigit->aniFrame = 0;
			redraw = 1;

		}

	}


	if (redraw) {
		thisDigit->aniFrame = 0;
		//Just update the damn thing
		if (numbers[thisDigit->lastValue] != NULL) {
			//BMPShow(screen, numbers[thisDigit->lastValue], thisDigit->xPos, thisDigit->yPos);
			SDL_BlitSurface(numbers[thisDigit->lastValue], NULL, screen,
							&colorBox);
		}
		//Display AM or PM Icon if this digit is firstHour and we're in 12 hour mode
		if (digitIndex == 0 && userPreferences.militaryTime == 0) {
			if (isPM) {
				//Show PM Icon
				if (elements[PMICON] != NULL) {

					BMPShow(screen, elements[PMICON],
							currentTheme.digitPos[AMPMSIGN].x,
							currentTheme.digitPos[AMPMSIGN].y);
				}
			} else {
				//Show AM Icon
				if (elements[AMICON] != NULL) {
					BMPShow(screen, elements[AMICON],
							currentTheme.digitPos[AMPMSIGN].x,
							currentTheme.digitPos[AMPMSIGN].y);
				}
			}

		}
	}

	if (clockMode == CLOCKMODEALARMRUNNING) {
		colorBox.x = thisDigit->xPos;
		colorBox.y = thisDigit->yPos;
		colorBox.w = numbers[thisDigit->lastValue]->w;
		colorBox.h = numbers[thisDigit->lastValue]->h;
		drawAllLabelsInRect(screen, clockMode, &colorBox);
	}

	if (numbers[thisDigit->lastValue] != NULL) {
		SDL_UpdateRects(screen, 1, &colorBox);
	}
	return FALSE;

}

/***********************************************************************
* drawDigit()
*
* Function to handle actually drawing a digit on a buffer; this method is 
* very similar to flipDigit, but is designed to be used against a non-display
* surface to quickly render the clock screen digits.
* 
*
***********************************************************************/

int drawDigit(int digitIndex, SDL_Surface * screenBuf)
{
	struct clockDigit *thisDigit;

	thisDigit = &clockDigits[digitIndex];

	Uint32 moodColor;
	SDL_Rect colorBox;

	if (currentTheme.usesAlphaBG == 1) {
		if (numbers[thisDigit->lastValue] != NULL) {
			moodColor =
				SDL_MapRGB(screen->format, userPreferences.moodRed,
						   userPreferences.moodGreen,
						   userPreferences.moodBlue);

			colorBox.x = thisDigit->xPos;
			colorBox.y = thisDigit->yPos;
			colorBox.w = numbers[thisDigit->lastValue]->w;
			colorBox.h = numbers[thisDigit->lastValue]->h;
			SDL_FillRect(screenBuf, &colorBox, moodColor);
		}

	}


	//Just update the damn thing
	if (numbers[thisDigit->lastValue] != NULL) {
		BMPShow(screenBuf, numbers[thisDigit->lastValue], thisDigit->xPos,
				thisDigit->yPos);
	}
	//Display AM or PM Icon if this digit is firstHour and we're in 12 hour mode
	if (digitIndex == 0 && userPreferences.militaryTime == 0) {
		if (isPM) {
			//Show PM Icon
			if (elements[PMICON] != NULL) {

				BMPShow(screenBuf, elements[PMICON],
						currentTheme.digitPos[AMPMSIGN].x,
						currentTheme.digitPos[AMPMSIGN].y);
			}
		} else {
			//Show AM Icon
			if (elements[AMICON] != NULL) {
				BMPShow(screenBuf, elements[AMICON],
						currentTheme.digitPos[AMPMSIGN].x,
						currentTheme.digitPos[AMPMSIGN].y);
			}
		}

	}
	return FALSE;

}




int updateClock(SDL_Surface * screenBuf, int targetClockMode)
{
	int q, currValue;
	char holder[5];
	char s[30];
	size_t i;
	struct tm tim;
	time_t now;
	int oldPM;

	SDL_Rect labelRects[currentTheme.textLabels[targetClockMode].labelCount];	//Rects for labels to be updated
	int updateRectCount = 0;	//Number of label rects to update

	if (themeReady != 1
		|| (targetClockMode != CLOCKMODENORMAL
			&& targetClockMode != CLOCKMODEALARMRUNNING)) {
		//Theme not ready for display yet, so ignore
		return FALSE;
	}



	int firstHour, secondHour, firstMin, secondMin;
	int theTime[4];

	now = time(NULL);
	tim = *(localtime(&now));

	oldPM = isPM;
	if (userPreferences.militaryTime == 1) {
		i = strftime(s, 4, "%H", &tim);
	} else {
		//Set AM or PM first
		i = strftime(s, 4, "%P", &tim);
		printf("date ampm is %s\n", s);
		if (strcmp(s, "PM") == 0 || strcmp(s, "pm") == 0) {
			isPM = 1;
		} else {
			isPM = 0;
		}
		bzero(s, sizeof(s));

		i = strftime(s, 4, "%I", &tim);
	}



	if (strlen(s) == 1) {
		//Only 1 digit hour so pad first hour with nothing
		//showFirstHour = 0;
		firstHour = 10;			//10 = Null, no digit blank
		secondHour = atoi(s);
	} else {
		//2 digit hour
		bzero(holder, sizeof(holder));
		sprintf(holder, "%c", s[0]);
		firstHour = atoi(holder);

		//Special case, check to see if the first digit is 0 since that looks silly
		if (firstHour == 0) {
			firstHour = 10;		//10 = Null, no digit blank
		}

		bzero(holder, sizeof(holder));
		sprintf(holder, "%c", s[1]);
		secondHour = atoi(holder);
	}


	bzero(s, sizeof(s));
	i = strftime(s, 4, "%M", &tim);

	if (strlen(s) == 1) {
		//Only 1 minute digit so pad first with 0
		firstMin = 0;
		secondMin = atoi(s);
	} else {
		//2 digit hour
		bzero(holder, sizeof(holder));
		sprintf(holder, "%c", s[0]);
		firstMin = atoi(holder);

		bzero(holder, sizeof(holder));
		sprintf(holder, "%c", s[1]);
		secondMin = atoi(holder);
	}

	theTime[0] = firstHour;
	theTime[1] = secondHour;
	theTime[2] = firstMin;
	theTime[3] = secondMin;

	//check time
	for (q = 0; q < 4; q++) {

		currValue = theTime[q];
		if (screenBuf == screen) {
			if (clockDigits[q].lastValue != currValue
				|| (q == 0 && oldPM != isPM)) {
				clockDigits[q].lastValue = currValue;
				//Use flip to draw depending on whether we're dealing with the screen or a buffer
				//int * whateverq;
				//whateverq = g_new0(int,1 );
				//&whateverq = q;
				g_timeout_add(100, (GSourceFunc) flipDigit, (gpointer) q);
				//drawDigit(q, screenBuf);
				flipDigit(q);
			}
		} else {
			clockDigits[q].lastValue = currValue;
			drawDigit(q, screenBuf);

		}
	}

	//Now draw any labels that need to be drawn...
	//this means clock mode, and alarm mode if alarm mode is active...

	if (currentTheme.textLabels[targetClockMode].labelCount > 0) {
		for (q = 0;
			 q < currentTheme.textLabels[targetClockMode].labelCount;
			 q++) {
			if (currentTheme.textLabels[targetClockMode].labelArr[q]->
				labelKey != NULL) {
				printf("label key%s\n",
					   currentTheme.textLabels[targetClockMode].
					   labelArr[q]->labelKey);
				if (!strcmp
					(currentTheme.textLabels[targetClockMode].labelArr[q]->
					 labelKey, "date")
					|| !strcmp(currentTheme.textLabels[targetClockMode].
							   labelArr[q]->labelKey, "nextAlarmCountDown")
					|| !strcmp(currentTheme.textLabels[targetClockMode].
							   labelArr[q]->labelKey,
							   "snoozingCountDown")) {
					//Label needs to be redrawn
					drawTextLabel(screenBuf,
								  currentTheme.textLabels[targetClockMode].
								  labelArr[q], targetClockMode, NULL);

					labelRects[updateRectCount] =
						currentTheme.textLabels[targetClockMode].
						labelArr[q]->labelRect;
					updateRectCount++;
				}
			}
		}

	}
	//Update the labels
	if (screenBuf == screen) {
		SDL_UpdateRects(screenBuf, updateRectCount, labelRects);
	}

	/*
	   if (targetClockMode == CLOCKMODENORMAL) {
	   //Draw only what's changed
	   drawAllLabels(screenBuf, targetClockMode, 1);
	   } else if (targetClockMode == CLOCKMODEALARMRUNNING) {
	   //Draw everything
	   drawAllLabels(screenBuf, targetClockMode, 0);
	   }
	 */


	return TRUE;
}

int updateSecondsBar(SDL_Surface * screenBuf, int forceUpdate)
{
	struct tm tim;
	time_t now;
	SDL_Rect src;


	SDL_Rect updateRects[2];	//Max rects to update is 2, one for old pos, one for new pos
	int updateRectCount = 0;

	if (themeReady != 1 || userPreferences.showSeconds == 0) {
		//Theme not ready for display yet, or user doesn't want to see seconds so ignore
		return FALSE;
	}
	//check if in the right clock mode
	if (screenBuf == screen && clockMode != CLOCKMODENORMAL
		&& clockMode != CLOCKMODEALARMRUNNING && forceUpdate != 1) {
		//Not in clock mode, so ignore
		return FALSE;
	}


	//Make sure that seconds bar and ticker exist for this theme (or there's no point)
	if (elements[SECONDSBAR] != NULL && elements[SECONDSTICK] != NULL
		&& currentTheme.digitPos[SECONDSBARRECT].x != 9999
		&& currentTheme.digitPos[SECONDSBARRECT].y != 9999) {


		now = time(NULL);
		tim = *(localtime(&now));

		if (tabletInactive == 0) {
			if (tim.tm_sec % currentTheme.secondsBarIncrement == 0
				|| forceUpdate == 1) {
				if (oldSecsPos.w != 0 && oldSecsPos.h != 0) {
					//Cover up the old draw position
					src.x =
						oldSecsPos.x -
						currentTheme.digitPos[SECONDSBARRECT].x;
					src.y =
						oldSecsPos.y -
						currentTheme.digitPos[SECONDSBARRECT].y;
					src.w = oldSecsPos.w;
					src.h = oldSecsPos.h;


					drawThemeBackground(screenBuf, CLOCKMODENORMAL,
										&oldSecsPos);
					SDL_BlitSurface(elements[SECONDSBAR], &src, screenBuf,
									&oldSecsPos);
					updateRects[updateRectCount] = oldSecsPos;
					updateRectCount++;

					oldSecsPos.w = 0;
					oldSecsPos.h = 0;
					oldSecsPos.x = 0;
					oldSecsPos.y = 0;

				}

				if (tim.tm_sec == 99) {
					//we're at 0, so just redraw the whole damn empty bar
					drawThemeBackground(screenBuf, CLOCKMODENORMAL,
										&currentTheme.
										digitPos[SECONDSBARRECT]);
					BMPShow(screenBuf, elements[SECONDSBAR],
							currentTheme.digitPos[SECONDSBARRECT].x,
							currentTheme.digitPos[SECONDSBARRECT].y);
					//printf("seconds at 0!\n");
				} else {



					//check if seconds ticker is a ticker, or a progress bar
					//printf("Size comparison: w: %i %i, h: %i %i\n", elements[SECONDSTICK]->w, elements[SECONDSBAR]->w, elements[SECONDSTICK]->h, elements[SECONDSBAR]->h);
					if (elements[SECONDSTICK]->w == elements[SECONDSBAR]->w
						&& elements[SECONDSTICK]->h ==
						elements[SECONDSBAR]->h) {
						//This is a seconds progress bar, not a ticker!
						printf("progress bar!\n");
						if (elements[SECONDSBAR]->w >
							elements[SECONDSBAR]->h) {
							src.w =
								(tim.tm_sec / 60.0000) *
								elements[SECONDSBAR]->w;
							src.h = elements[SECONDSTICK]->h;
						} else {
							src.h =
								(tim.tm_sec / 60.0000) *
								elements[SECONDSBAR]->h;
							src.w = elements[SECONDSTICK]->w;
						}
						src.x = 0;
						src.y = 0;

						//Draw it now

						SDL_BlitSurface(elements[SECONDSTICK], &src,
										screenBuf,
										&currentTheme.
										digitPos[SECONDSBARRECT]);

						oldSecsPos.w = src.w;
						oldSecsPos.h = src.h;
						oldSecsPos.x =
							currentTheme.digitPos[SECONDSBARRECT].x;
						oldSecsPos.y =
							currentTheme.digitPos[SECONDSBARRECT].y;

						updateRects[updateRectCount] =
							currentTheme.digitPos[SECONDSBARRECT];
						updateRectCount++;

					} else {
						//This is a ticker, not progress bar so only redraw as required
						if (elements[SECONDSBAR]->w >
							elements[SECONDSBAR]->h) {

							//printf("time says %f, pos says %f\n", tim.tm_sec / 60.0000, ((tim.tm_sec / 60.0000) * elements[SECONDSBAR]->w) - elements[SECONDSTICK]->w);
							src.x =
								(((tim.tm_sec / 60.0000) *
								  elements[SECONDSBAR]->w) -
								 elements[SECONDSTICK]->w) +
								currentTheme.digitPos[SECONDSBARRECT].x;
							if (src.x < 0) {
								src.x = 0;
							}
							if (src.x -
								currentTheme.digitPos[SECONDSBARRECT].x <
								0) {
								printf("too low, %i\n", src.x);
								src.x =
									currentTheme.digitPos[SECONDSBARRECT].
									x;
							}

							src.y =
								currentTheme.digitPos[SECONDSBARRECT].y;
							//src.x = src.x 
						} else {
							src.y =
								(((tim.tm_sec / 60.0000) *
								  elements[SECONDSBAR]->h) -
								 elements[SECONDSTICK]->h) +
								currentTheme.digitPos[SECONDSBARRECT].y;
							if (src.y < 0) {
								src.y = 0;
							}
							if (src.y -
								currentTheme.digitPos[SECONDSBARRECT].y <
								elements[SECONDSTICK]->h) {
								src.y = 0;
							}

							src.x =
								currentTheme.digitPos[SECONDSBARRECT].x;
						}
						src.w = (elements[SECONDSTICK]->w);
						src.h = elements[SECONDSTICK]->h;
						//printf("Target for ticker: %i x %i y %i w %i h\n", src.x, src.y, src.w, src.h);
						//Draw the blank bar to cover the previous seconds ticker
						SDL_BlitSurface(elements[SECONDSTICK], NULL,
										screenBuf, &src);

						//Draw the new seconds ticker
						oldSecsPos.w = src.w;
						oldSecsPos.h = src.h;
						oldSecsPos.x = src.x;
						oldSecsPos.y = src.y;

						updateRects[updateRectCount] = src;
						updateRectCount++;

					}

				}

			}



		} else {
			if (clockMode != CLOCKMODEALARMRUNNING) {

				//Tablet just went dark, so draw the whole bar blank so that when we wake up things will be ok
				BMPShow(screenBuf, elements[SECONDSBAR],
						currentTheme.digitPos[SECONDSBARRECT].x,
						currentTheme.digitPos[SECONDSBARRECT].y);
			}

		}

	}

	if (updateRectCount > 0) {
		if (clockMode == CLOCKMODEALARMRUNNING) {
			//check for overlaps
			drawAllLabelsInRect(screenBuf, CLOCKMODEALARMRUNNING,
								&updateRects[0]);
			if (updateRectCount > 1) {
				drawAllLabelsInRect(screenBuf, CLOCKMODEALARMRUNNING,
									&updateRects[1]);
			}

		}

	}



	if (screenBuf == screen) {
		SDL_UpdateRects(screenBuf, updateRectCount, updateRects);
	}

	return TRUE;

}

/******************************************************************************
*
* updateTimerDigits(screenBuff) 
*
* Function called once per second to update the timer display 
*
*******************************************************************************/

int updateTimerDigits(SDL_Surface * screenBuf, int forceUpdate)
{

	int i = 0;
	//struct tm tim;
	//time_t now; 
	int now, counter, newVal;
	//char s[30];   
	int secs, mins, oldMins, oldSecs = 0;
//  int oldDiff, oldVal = 0;
	int s1, s2, m1, m2, h1, h2 = 0;
	int oldS1, oldS2, oldM1, oldM2, oldH1, oldH2 = 0;

	//i = strftime(s,4,"%H",&tim);
	if (timerRunning == 1) {
		now = time(NULL);
	} else {
		now = timerPauseTime;
	}



	counter = timerStartVal;
	//counter = 1270304383;

	if (now >= counter) {
		newVal = now - counter;
	} else if (now < counter) {
		newVal = counter - now;
	}
	//if (oldTimerVal == 0) {
	//oldTimerVal = newVal - 1;
	//}

	if ((oldTimerVal == newVal || timerRunning != 1) && forceUpdate != 1) {
		//Time hasn't changed, so don't bother
		return 0;
	}

	if (counter != 0) {

		secs = newVal % 60;
		mins = ((newVal - secs) / 60) % 60;

		oldSecs = oldTimerVal % 60;
		oldMins = ((oldTimerVal - oldSecs) / 60) % 60;

		s2 = secs % 10;
		s1 = (secs - (s2)) / 10;

		m2 = mins % 10;
		m1 = (mins - m2) / 10;

		h2 = ((newVal - mins - secs) / 3600) % 3600 % 10;
		h1 = (((newVal - mins - secs) / 3600) % 3600 - h2) / 10;

		oldS2 = (oldTimerVal % 60) % 10;
		oldS1 = ((oldTimerVal % 60) - oldS2) / 10;

		oldM2 = ((oldTimerVal - (oldTimerVal % 60)) / 60) % 60 % 10;
		oldM1 =
			(((oldTimerVal - (oldTimerVal % 60)) / 60) % 60 - oldM2) / 10;

		oldH2 = ((oldTimerVal - oldMins - oldSecs) / 3600) % 3600 % 10;
		oldH1 =
			(((oldTimerVal - oldMins - oldSecs) / 3600) % 3600 -
			 oldH2) / 10;


		if (h1 > 9) {
			h1 = 9;
		}
		if (oldH1 > 9) {
			oldH1 = 9;
		}
	} else {
		s2 = 0;
		s1 = 0;
		h2 = 0;
		h1 = 0;
		m2 = 0;
		m1 = 0;
	}

	//Calculate old values (for animation)


	//printf("h1 is %i, old h1 is %i, h2 is %i, old h2 is %i\n", h1, oldH1, h2, oldH2);
	if (forceUpdate != 1) {
		if (s1 != oldS1) {
			animateAlarmScrollerDigit(screenBuf, i, oldS1, s1, 10,
									  &currentTheme.timerDigitPos[4]);
		}

		if (s2 != oldS2) {
			animateAlarmScrollerDigit(screenBuf, i, oldS2, s2, 10,
									  &currentTheme.timerDigitPos[5]);
		}

		if (m1 != oldM1) {
			animateAlarmScrollerDigit(screenBuf, i, oldM1, m1, 10,
									  &currentTheme.timerDigitPos[2]);
		}

		if (m2 != oldM2) {
			animateAlarmScrollerDigit(screenBuf, i, oldM2, m2, 10,
									  &currentTheme.timerDigitPos[3]);
		}

		if (h1 != oldH1) {
			animateAlarmScrollerDigit(screenBuf, i, oldH1, h1, 10,
									  &currentTheme.timerDigitPos[0]);
		}

		if (h2 != oldH2) {
			animateAlarmScrollerDigit(screenBuf, i, oldH2, h2, 10,
									  &currentTheme.timerDigitPos[1]);
		}
	} else {

		drawAlarmScrollerDigit(screenBuf, i, h1,
							   &currentTheme.timerDigitPos[0]);
		drawAlarmScrollerDigit(screenBuf, i, h2,
							   &currentTheme.timerDigitPos[1]);
		drawAlarmScrollerDigit(screenBuf, i, m1,
							   &currentTheme.timerDigitPos[2]);
		drawAlarmScrollerDigit(screenBuf, i, m2,
							   &currentTheme.timerDigitPos[3]);
		drawAlarmScrollerDigit(screenBuf, i, s1,
							   &currentTheme.timerDigitPos[4]);
		drawAlarmScrollerDigit(screenBuf, i, s2,
							   &currentTheme.timerDigitPos[5]);

		if (screenBuf == screen) {
			SDL_UpdateRects(screenBuf, 6, currentTheme.timerDigitPos);
		}
	}




	//if (screenBuf == screen) {
	//SDL_UpdateRects(screenBuf, 6, currentTheme.timerDigitPos);
	//}

	oldTimerVal = newVal;

	return TRUE;

}






/* ----------------------------------------------------------------------------
MAIN ()
 ----------------------------------------------------------------------------*/
int main(int argc, char *argv[])
{

	//************ Variable definitions **************/
	int error = 0;				//General error flag
	int digiti = 0;
	for (digiti = 0; digiti < 10; digiti++) {
		digitAnimation[digiti] = NULL;
	}

	//*********** Done Var Definitions **************/

	//*********** Hardware detection ***************//
	isTablet = checkForTablet();
	debugEntry("Starting up");

	//*********** Done Hardware detection ***********//




	//Setup SDL window class
	putenv("SDL_VIDEO_X11_WMCLASS=flipclock");

	//*********** Initial SDL setup ****************//
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_NOPARACHUTE) < 0) {
		fprintf(stderr, "Unable to init SDL: %s\n", SDL_GetError());
		exit(1);
	}
	//Init the font library
	TTF_Init();

	/* Initialize the GTK and GStreamer. */
	g_thread_init(NULL);
//  gdk_threads_init();

	gtk_init(&argc, &argv);

	gst_init(&argc, &argv);

	//Find teh best available sink on the device and store it for later
	//gstFindBestSink("dspmp3sink");
	gstFindBestSink("filesrc");

	// Register SDL_Quit to be called at exit; makes sure things are
	atexit(SDL_Quit);			// cleaned up when we quit.

	//Setup the window caption/etc
	//As per http://www.mail-archive.com/maemo-developers@maemo.org/msg18713.html
	SDL_SysWMinfo info;
	SDL_VERSION(&info.version);
	if (SDL_GetWMInfo(&info)) {

		Display *dpy = info.info.x11.display;
		Window win;
		if (dpy) {
			win = info.info.x11.fswindow;
			if (win)
				XStoreName(dpy, win, PROGNAME);
			win = info.info.x11.wmwindow;
			if (win)
				XStoreName(dpy, win, PROGNAME);
		}
	}
	//Done setting window title/task navigation 
	debugEntry("Setting up video mode");

	if (isTablet) {
		screen = SDL_SetVideoMode(800, 480, 0, SDL_SWSURFACE | SDL_FULLSCREEN);	// Full screen Tablet
		windowMode = 1;
		//hide cursor since tablet uses the touch screen
		SDL_ShowCursor(SDL_DISABLE);

	} else {
		screen = SDL_SetVideoMode(800, 480, 0, SDL_SWSURFACE);	// Window PC
		windowMode = 0;
	}

	debugEntry("Done setting up video mode");

	if (screen == NULL)			// If fail, return error
	{
		fprintf(stderr, "Unable to set video mode: %s\n", SDL_GetError());
		exit(1);
	}
	//******* Load main graphics for application (not theme related) *************//
	loadAppGraphics();


	//Draw splash screen if it's present
	if (appElements[SPLASHSCREEN] != NULL) {
		BMPShow(screen, appElements[SPLASHSCREEN], 0, 0);
	}
	//******* Done main graphics for application (not theme related) *************//


	//Setup buttons
	initButtonCollection(clockButtonActions);
	initButtonCollection(themeButtonActions);


	//Load the images
	loadAllThemes();

	//loadTheme("themes/defaultFlip/");
	debugEntry("Done loading themes");

	//Load user preferences
	setDefaultPreferences();
	getUserPreferences();

	//Setup the next alarm time vars
	getNextAlarmTime();

	//Load the default theme
	debugEntry(userPreferences.currentThemePath);

	loadTheme(userPreferences.currentThemePath);

	//printf("cleared to %i\n", clockButtonActions[CLOCKMODENORMAL].buttonCount);
	//*********** Initialize Application ***********//
	error = initializeApp();

	debugEntry("Done initialializing App");

//borked_fselector();


	//test!
	//getFlipAlarms();

	//Run the main loop
	g_main_loop_run(mainLoopObj);


	return 0;
}


/*let's check is I can still fly  */



/***************************************************************************
* Function to determine if we're running on a tablet or not (checks 
* kernel info to see if the system is ARM based or not
*        					 												
***************************************************************************/

int checkForTablet()
{
	struct utsname kernelInfo;
	int result;

	result = 0;					//assume not tablet by default

	if (uname(&kernelInfo) < 0) {
		return 0;
	}
	//printf("sysname: %s, release: %s, version: %s, machine: %s\r\n", kernelInfo.sysname, kernelInfo.release, kernelInfo.version, kernelInfo.machine);

	if (strstr(kernelInfo.machine, "arm") != NULL) {
		result = 1;
	}

	return result;

}



/************************************************************************
* initializeApp()
*
* Function to get the ball rolling and setup the application.
*
*************************************************************************/

int initializeApp()
{

	struct tm tim;
	time_t now;
	char tempTime[4];
	int i;

	//****************** GObject loop setup ***************//

	mainLoopObj = g_main_loop_new(NULL, FALSE);	//Create the main loop for the app

	//****************** Done GObject stuff ***************//

	if (USE_OSSO) {
		//Initialize the osso stuff for tablets
		ossoFlipInit();
	}

	if (isTablet == 1) {
		//wait a moment so that we don't interrupt our nice "flipping out"
		g_timeout_add(2000, (GSourceFunc) setInsomniacMode,
					  (int *) userPreferences.insomniacModeOn);
		//setInsomniacMode(userPreferences.insomniacModeOn);
	}
	//Check for FM Radio
	hasFMRadio = ossoHasFMRadio();



	//Register event handlers for main loop
	//The input event handler
	g_idle_add(clockEventIdle, NULL);


	//done loading, so put clock into normal mode
	clockMode = CLOCKMODENORMAL;


	//Start the clock redrawing timeout
	clockEventRedraw();

	//Start the seconds bar timeout
	clockSecondsRedraw();

	//Get the current day for the alarm settings screen

	now = time(NULL);
	tim = *(localtime(&now));

	strftime(tempTime, 4, "%w", &tim);
	i = getFirstAlarmIndexByDay(atoi(tempTime));

	if (i < 0) {
		i = 0;
	}
	currentAlarmNum = i;

	//setup the timeout handler for non-alarmD mode
	watchForAlarms();


	//Success!
	return 1;

}



/************************************************************************
* deinitializeApp()
*
* Function to clean up anything that was setup the application before we exit
*
*************************************************************************/

int deinitializeApp()
{

	if (isTablet == 1) {
		//We're exiting, so turn Insomniac mode off!
		setInsomniacMode(0);
		//Restore original sysVol just to be safe incase this didn't happen already...
		setTabletSystemVolume(1);

	}

	if (USE_OSSO) {
		//De-Initialize the osso stuff for tablets
		ossoFlipDeInit();
	}
	//Cleanup the gobject mainloop
	if (mainLoopObj != NULL) {
		g_main_loop_unref(mainLoopObj);
		mainLoopObj = NULL;
	}
	//Clean up preferences/user objects
	cleanUserPreferences();

	//clear out themes
	cleanupThemes();


	//Clear out loaded theme buttons
	clearThemeButtons();


	clearThemeGraphics();


	//Clean up graphics from main application
	clearAppGraphics();


	//Clear up clock buttons
	clearClockButtons();

	//Clean up screen buffers
	if (renderedBuffer != NULL) {
		SDL_FreeSurface(renderedBuffer);
		renderedBuffer = NULL;
	}

	if (themeBGBuffer != NULL) {
		SDL_FreeSurface(themeBGBuffer);
		themeBGBuffer = NULL;
	}
	//Close up the TTF lib
	TTF_Quit();

	return 1;



}



/************************************************************************
* clockEventIdle()
*
* Function to be called whenever nothing else is going on to check for events
* and handle them as required
* 
*************************************************************************/

int clockEventIdle()
{
	SDL_Event event = { 0 };	// Poll for events
	int error = 0;
	int runEvents = 1;

	runEvents = 1;
	while (SDL_PollEvent(&event)) {
		//if (event != NULL) { //Just to be safe...
		//printf("event of type %i occurred\r\n", event.type); 
		if (isTablet) {
			if (userPreferences.insomniacModeOn == 1 && insomniacMode == 2) {
				if (isInsomniacDimmed == 1) {

					//Some event occured, check to see if we need to do manual insomniac mode handling
					wakeFromInsomnia();
					if (userPreferences.insomniacLocked == 1) {
						runEvents = 0;
					}
				}
			}
		}
		if (runEvents) {
			switch (event.type) {
			case SDL_VIDEORESIZE:
				//printf("resize occured\r\n");
				break;

			case SDL_ACTIVEEVENT:
				//printf("some activity event occured %i %i\r\n", event.active.gain, event.active.state);
				break;

			case SDL_MOUSEBUTTONDOWN:
				currentMouseState.xDownPos = event.button.x;
				currentMouseState.yDownPos = event.button.y;
				currentMouseState.pressed = 1;
				currentMouseState.motion = 0;

				checkMouseEvents();
				break;

			case SDL_MOUSEBUTTONUP:

				currentMouseState.xUpPos = event.button.x;
				currentMouseState.yUpPos = event.button.y;
				currentMouseState.pressed = 0;
				currentMouseState.motion = 0;

				checkMouseEvents();
				break;

			case SDL_MOUSEMOTION:
				//Only run if motion tracking is needed for this clock mode
				if (mouseMoveEnabled[clockMode]) {
					currentMouseState.xCurrPos = event.button.x;
					currentMouseState.yCurrPos = event.button.y;
					currentMouseState.pressed = 1;
					currentMouseState.motion = 1;

					checkMouseEvents();
				}

				break;


			case SDL_KEYDOWN:
				//printf("Key pressed: %i\r\n", event.key.keysym.sym);
				if (event.key.keysym.sym == SDLK_a) {
					printf("premuto A");
				} else if (event.key.keysym.sym == SDLK_s) {
					printf("premuto S");
				} else if (event.key.keysym.sym == SDLK_d) {
					printf("premuto D");
				} else if (event.key.keysym.sym == SDLK_f) {
					printf("premuto F");
				} else if (event.key.keysym.sym == SDLK_g) {
					BMPShow(screen, elements[0], 0, 0);
				}
				break;
			case SDL_KEYUP:
				printf("keyup! %i\r\n", event.key.keysym.sym);
				//288 = +. 289 = - on tablet?
				if (event.key.keysym.sym == 287
					|| event.key.keysym.sym == SDLK_F2) {
					//Key 287 = Fullscreen button on tablets
					toggleWindowMode();
				} else if (event.key.keysym.sym == 27) {
					//Escape key on tablet is our good old insomniac toggle for now...
					toggleInsomniacModeButton();

				} else if (event.key.keysym.sym == 288) {
					//+ sign, which will be volume + for alarm
					setTabletSystemVolume(2);

				} else if (event.key.keysym.sym == 289) {
					//+ sign, which will be volume + for alarm
					setTabletSystemVolume(3);

				}				/*else {

								   printf("exit!!\r\n");
								   //Just quit on any key for now
								   g_main_loop_quit(mainLoopObj);

								   error = deinitializeApp();

								   return(0);
								   //if (event.key.keysym.sym == SDLK_q)      // If Q is pressed, return (and zen quit)
								   // return 0;
								   // break; 
								   } */
				break;
			case SDL_QUIT:
				printf("exit!!\r\n");
				//Just quit on any key for now
				g_main_loop_quit(mainLoopObj);

				error = deinitializeApp();

				exit(0);
				//return(0);
				//}



			}
		}
		//Some event occured, check to see if we need to do manual insomniac mode handling
		if (isTablet) {
			if (userPreferences.insomniacModeOn == 1 && insomniacMode == 2) {

				//Set the timeout
				//printf("ins dim: %i at time %i\n", userPreferences.insomniacDim, originalDim);
				//Clear the old inactivity timeout
				if (insomniacTO > 0) {
					g_source_remove(insomniacTO);
					insomniacTO = 0;
				}

				//Set new inactivity timeout to start
				if (userPreferences.insomniacDim != -1
					&& originalDim != -1) {
					insomniacTO =
						g_timeout_add(originalDim * 1000,
									  (GSourceFunc)
									  set_brightness_insomnia_TO,
									  (int *) userPreferences.
									  insomniacDim);


				}

			}
		}

	}
	//printf("done event check\r\n");

	usleep(10000);

	return TRUE;

}

/************************************************************************
* clockEventRedraw()
*
* Function will be called to redraw the clock. It's initially triggered by a timeout
* and will automatically set itself up to be called again at the next minute change
* if the system is active; if the tablet is inactive then the event does not get fired
* again until the system resumes use. 
* 
*************************************************************************/
int clockEventRedraw()
{
	int currentTime = 0;
	int nextTime = 0;
	time_t nowT;

	//check if in the right clock mode
	if (clockMode == CLOCKMODENORMAL || clockMode == CLOCKMODEALARMRUNNING) {
		//In clock mode, so redraw
		updateClock(screen, clockMode);
	}


	if (tabletInactive == 0 || batteryStatus.charging == 1) {
		//now = NULL;

		currentTime = time(&nowT);
		//printf("time: %s %i \r\n", ctime(&nowT), currentTime % 60);
		nextTime = ((60 - (currentTime % 60)) * 1000);

		g_timeout_add(nextTime, clockEventRedraw, NULL);
		clockRunning = 1;
	} else {
		clockRunning = 0;
	}
	//printf("clock redraw at %i!\r\n", nextTime);
	return FALSE;				//REturn 0 to clear this timeout
}

/************************************************************************
* clockSecondsRedraw()
*
* Function will be called to redraw the seconds ticker of the clock. It's initially triggered by a timeout
* and will automatically set itself up to be called again at the next second change
* if the system is active; if the tablet is inactive then the event does not get fired
* again until the system resumes use. 
* 
*************************************************************************/
int clockSecondsRedraw()
{

	//check if in the right clock mode
	if (clockMode == CLOCKMODENORMAL || clockMode == CLOCKMODEALARMRUNNING) {
		//IN clock mode, so redraw
		updateSecondsBar(screen, 0);
	} else if (clockMode == CLOCKMODETIMER) {
		//Redraw timer digits!
		updateTimerDigits(screen, 0);

	}

	if (tabletInactive == 0 || batteryStatus.charging == 1) {
		//g_timeout_add(1000, clockSecondsRedraw, NULL);
		if (clockMode == CLOCKMODENORMAL
			|| clockMode == CLOCKMODEALARMRUNNING) {
			//g_timeout_add(currentTheme.secondsBarIncrement * 1000, clockSecondsRedraw, NULL);
			g_timeout_add(1000, clockSecondsRedraw, NULL);
		} else {
			g_timeout_add(250, clockSecondsRedraw, NULL);
		}
		clockSecondsRunning = 1;
	} else {
		clockSecondsRunning = 0;
	}
	return FALSE;				//REturn 0 to clear this timeout
}

/************************************************************************
* watchForAlarms()
*
* Function to be called once per minute when in non-alarmD mode to check to see if
* an alarm is occuring or not
* 
*************************************************************************/
int watchForAlarms()
{
	int currentTime = 0;
	int nextTime = 0;
	time_t nowT;
	int alarmIndex = -1;

	if (userPreferences.useAlarmD == 0) {
		//now = NULL;

		currentTime = time(&nowT);
		//printf("time: %s %i \r\n", ctime(&nowT), currentTime % 60);
		nextTime = ((60 - (currentTime % 60)) * 1000);

		//check to alarm
		alarmIndex = checkForAlarm(currentTime - (currentTime % 60));

		if (alarmIndex != -1) {
			handleAlarm(alarmIndex);
		}


		g_timeout_add(nextTime, watchForAlarms, NULL);
	}

	return FALSE;				//REturn 0 to clear this timeout
}

/************************************************************************
* toggleWindowMode()
*
* Function to toggle between full screen and window mode
* 
*************************************************************************/
int toggleWindowMode()
{
	if (windowMode == 1) {
		//Fullscreen right now, so go to window'd mode instead
		SDL_WM_ToggleFullScreen(screen);
		windowMode = 0;

	} else if (windowMode == 0) {
		//Window'd so go back to full screen
		SDL_WM_ToggleFullScreen(screen);
		windowMode = 1;
	}

	return 1;
}


/*************************************************************************
* Function to add append text to a file                                  *
*************************************************************************/

int appendToRawFile(char *theContent, char *filePath)
{
	FILE *fp;					/* File pointer */

	//attempt to open file
	fp = fopen(filePath, "a");

	if (fp) {
		fwrite(theContent, 1, strlen(theContent), fp);
		fflush(fp);
		//Close
		fclose(fp);

		return 1;
	}

	return 0;

}

/*************************************************************************
* Function to generate debug content for a log file 
*
* Controlled by the global DEBUGENABLED constant that should be defined by the
* makefile.
*************************************************************************/

void debugEntry(char *theContent)
{
	if (DEBUGENABLED) {
		appendToRawFile(theContent, "/tmp/flip.log");
		appendToRawFile("\r\n", "/tmp/flip.log");
	}
}
