/*

Flip Clock User Preferences Header Module
Part of FlipClock C SDL for Maemo.

This is the header file for the user preferences functions

-Rob Williams, Aug 11, 2009.


*/



//********************** Structure Definitions ***********************//

/***** Move alarmCookie, alarmTime into separate struct that
will be used ONLY by fcAlarmSync for alarmD status/updates.
This makes sense so that configs won't be over-written or
get mixed up since these values aren't needed in Flipclock
directly anyways */


/* Structure for Flip Alarm */
struct alarmObject {
	long alarmTime;
	int alarmIndex;				//Unique index for this alarm, used to match dbus signal with configured alarms
	char alarmHHMM[10];			//HHMM in 12 hour format of alarm time
	int ampm;					//Am = 1, PM = 0
	int alarmDay;				//What day of the week does this alarm represent? Temporary until Ciro gives us a date picker...maybe?
	long alarmCookie;			//Unique cookie/index of this alarm
	char *title;				//Title of this alarm
	int recurrenceCount;		//How often should alarm re-occur (-1 for infinate)
	int recurrence;				//What is the time difference between occurances (Once per week, so alarm always happens on the same day)
	int enabled;				//Is the alarm enabled or not?
	char *sound;				//Sound file to play
	int fmFreq;					//FM Frequency to use for this alarm (if applicable)
	int alertMode;				//Mode of alarm. right now 0 = play sound... others could be added later...
	int loopSound;				//should the sound file be looped?
	int snoozeTime;				//How many seconds should the snooze for this alarm be?
};


/* Structure for user preferences */
struct prefObj {
	char currentThemePath[512];	//Path to current theme
	int militaryTime;			//Is clock 24 or 12 hour mode
	int showSeconds;			//Should the seconds ticker be visible if defined by theme
	int insomniacModeOn;		//Is insomniac mode on?
	int insomniacLocked;		//Does insomniac mode allow interaction, or is it dim-locked?
	int maxAlarmVol;			//Maximum alarm sound volume
	Uint8 moodRed;				//Mood colour
	Uint8 moodGreen;			//Mood colour
	Uint8 moodBlue;				//Mood colour
	struct alarmObject **userAlarms;	//Alarms
	int userAlarmsCount;		//Number of user alarms
	
	int useAlarmD;				//Do we want to use alarmD or internal alarm tracking?
	
	
	int insomniacDim;			//Dim level for insomniac mode (in mode 2)
	int alarmControlMode;		//Style of alarm control 0 - Weekly, 1 - Simple/Basic
} userPreferences;




//********************** Done Struct Defs *************************//

//********************** Function Headers *************************//

void setDefaultPreferences();			//Function to establish preference defaults

void getUserPreferences();				//Function to load preferences from XML file

void setUserPreferences();				//Function to save preferences to XML file

void initAlarmObject(struct alarmObject *thisAlarm); 	//Function to initialize an alarm struct and populate it with default values

void cleanUserPreferences();			//Function to clean up/free any assigned user preferences from memory (called at exit time)

void freeAlarmObject(struct alarmObject *thisAlarm);	//Function to free an alarm object
//********************** Done Function Headers ********************//

