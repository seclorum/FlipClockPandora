/*

Flip Clock User Preferences Module/functionality.
Part of FlipClock C SDL for Maemo.

This library defines global vars and methods that are used to control user preferences for FlipClock

-Rob Williams, Aug 11, 2009.


*/

#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

//Include XML library if not already present
#ifndef XMLFUNC_H
#include "xmlFunc.c"
#endif

//********************** STATIC DEF'S *****************************//
/********** NOTE ***********
 The following constants are defined by the Makefile!
	PREFSPATH		//Path to user preferences. On tablets this should be /home/user/ 
 


*/

//********************** DONE STATIC DEF's ************************//


//********************** Structure Definitions ***********************//



//********************** Done Struct Defs *************************//


//********************** Function Headers *************************//

//********************** Done Function Headers ********************//



//********************** Function Definitions *********************//

/*******************************************************
* setDefaultPreferences()
*
* Function to set default preferences so the app will work 
* if no config file exists or if some values are missing...
*
*******************************************************/

void setDefaultPreferences() {
	int i;
	char tempBuffer[255];
	
	//Check to see if themes are defined
	if (themeCount < 1) {
		//No themes defined, so try to load em
		loadAllThemes();
	}
	
	if (themeCount < 1) {
		printf("NO themes available, failing!!\r\n");
		exit(0);
	}

	bzero(userPreferences.currentThemePath, sizeof(userPreferences.currentThemePath));
	sprintf(userPreferences.currentThemePath, "%s", availableThemes[0]->themePath);
	
	//Default to 12 hour mode
	userPreferences.militaryTime = 0;
	
	//Default to showing seconds
	userPreferences.showSeconds = 1;
	
	//Default insomniac mode off
	userPreferences.insomniacModeOn = 0;
	
	//Default mood color is red...
	userPreferences.moodRed = 255;
	userPreferences.moodGreen = 0;
	userPreferences.moodBlue = 0;
	//= SDL_MapRGB(screen->format, 255,0,0);
	
	//Default insomniac brightness
	userPreferences.insomniacDim = 3;
	
	//Default is to use insomniac lock
	userPreferences.insomniacLocked = 1;
	
	//Max alarm volume
	userPreferences.maxAlarmVol = 100;
	
	//Default to weekly alarm control
	userPreferences.alarmControlMode = ALARMCONTROLMODEWEEKLY;
	
	//Set default alarms
	//For now, let's just do one alarm for each day; others can be added later maybe...
	userPreferences.userAlarmsCount = 0;
	
	userPreferences.useAlarmD = 0;
	if (hasAlarmD) {
		//Default to using alarmD if it's present; this can be over-riden by user prefs...
		userPreferences.useAlarmD = 1;
	}
	
	for (i=0; i< 7; i++) {
		//Expand the main array
		userPreferences.userAlarms = (struct alarmObject **)realloc(userPreferences.userAlarms, (userPreferences.userAlarmsCount + 1) * sizeof(struct alarmObject *));
	
		/// allocate memory for one AlarmObject 
		userPreferences.userAlarms[userPreferences.userAlarmsCount] = (struct alarmObject *)malloc(sizeof(struct alarmObject));
		
		//Setup the defaults for this alarm obj
		initAlarmObject(userPreferences.userAlarms[userPreferences.userAlarmsCount]);
		
		userPreferences.userAlarms[userPreferences.userAlarmsCount]->alarmIndex = i;
		userPreferences.userAlarms[userPreferences.userAlarmsCount]->alarmDay = i;
		
		bzero(tempBuffer, sizeof(tempBuffer));
		sprintf(tempBuffer, "Flipclock Alarm %i", i);
		
		userPreferences.userAlarms[userPreferences.userAlarmsCount]->title = calloc(strlen(tempBuffer) +2, sizeof(char));
		sprintf(userPreferences.userAlarms[userPreferences.userAlarmsCount]->title, "%s", tempBuffer);
		
		//Increment the counter
		userPreferences.userAlarmsCount++;
	}
	


}

/*******************************************************
* getUserPreferences()
*
* Function to load the user's preferences from the current 
* preferences config file, or set default values if no
* config file is present
*
*******************************************************/

void getUserPreferences() {
	char prefsFile[512];		//Path to preferences file
	
	FILE *inputFilePtr;           /* Pointer to input file */
	char *configContent;
	struct xmlNode *rootEle, *preferencesEle, *colorEle, *alarmObjEle, *childEle;
	
	
	int i;

	//Check to see if path exists for preferences
	if (isDirectory(PREFSPATH)) {
		
		sprintf(prefsFile, "%s.flipClockPrefs.xml", PREFSPATH);
		
		if (fileExists(prefsFile)) {
			//Preference file exists, so load the data out of it
			inputFilePtr = fopen(prefsFile, "rb"); /* Open in BINARY mode */

			//Read the file
			configContent = readFile(inputFilePtr);
	
			//Close file pointer
			fclose(inputFilePtr);
			
			
			
			rootEle = findXMLObjectTest(configContent, "flipClockPreferences", 0);
			
			if (rootEle != NULL) {
				//Settings exist
				preferencesEle = findXMLObjectTest(rootEle->value, "mainOptions", 0);
				if (preferencesEle != NULL) {
					//Read the setting values
					
					//Load desired theme
					childEle = findXMLObjectTest(preferencesEle->value, "currentThemePath", 0);
					if (childEle != NULL) {
						printf("setting new theme path %s\n", childEle->value);
						bzero(userPreferences.currentThemePath, sizeof(userPreferences.currentThemePath));
						sprintf(userPreferences.currentThemePath, "%s", childEle->value);
						freeXMLObject(childEle);
					}
					
					//Load military time setting
					childEle = findXMLObjectTest(preferencesEle->value, "militaryTime", 0);
					if (childEle != NULL) {
						userPreferences.militaryTime = atoi(childEle->value);
						freeXMLObject(childEle);
					}
					
					//Load show seconds ticker setting
					childEle = findXMLObjectTest(preferencesEle->value, "showSeconds", 0);
					if (childEle != NULL) {
						userPreferences.showSeconds = atoi(childEle->value);
						freeXMLObject(childEle);
					}
					
					//Load insomniac mode setting
					childEle = findXMLObjectTest(preferencesEle->value, "insomniacModeOn", 0);
					if (childEle != NULL) {
						userPreferences.insomniacModeOn = atoi(childEle->value);
						freeXMLObject(childEle);
					}
					
					//Load alarmD setting
					childEle = findXMLObjectTest(preferencesEle->value, "useAlarmD", 0);
					if (childEle != NULL) {
						userPreferences.useAlarmD = atoi(childEle->value);
						freeXMLObject(childEle);
					}
					
					//Load max alarm volume setting
					childEle = findXMLObjectTest(preferencesEle->value, "maxAlarmVol", 0);
					if (childEle != NULL) {
						userPreferences.maxAlarmVol = atoi(childEle->value);
						freeXMLObject(childEle);
					}
					
					//Load insomniac dim level
					childEle = findXMLObjectTest(preferencesEle->value, "insomniacDim", 0);
					if (childEle != NULL) {
						userPreferences.insomniacDim = atoi(childEle->value);
						freeXMLObject(childEle);
					}
					
					//Load insomniac lock setting
					childEle = findXMLObjectTest(preferencesEle->value, "insomniacLocked", 0);
					if (childEle != NULL) {
						userPreferences.insomniacLocked = atoi(childEle->value);
						freeXMLObject(childEle);
					}
					
					//Load alarm control mode setting
					childEle = findXMLObjectTest(preferencesEle->value, "alarmControlMode", 0);
					if (childEle != NULL) {
						userPreferences.alarmControlMode = atoi(childEle->value);
						freeXMLObject(childEle);
					}
					
					//Mood colours
					colorEle = findXMLObjectTest(preferencesEle->value, "moodColor", 0);
					if (colorEle != NULL) {
						childEle = findXMLObjectTest(colorEle->value, "red", 0);
						if (childEle != NULL) {
							userPreferences.moodRed = atoi(childEle->value);
							freeXMLObject(childEle);
						}
					
						childEle = findXMLObjectTest(colorEle->value, "green", 0);
						if (childEle != NULL) {
							userPreferences.moodGreen = atoi(childEle->value);
							freeXMLObject(childEle);
						}
					
						childEle = findXMLObjectTest(colorEle->value, "blue", 0);
						if (childEle != NULL) {
							userPreferences.moodBlue = atoi(childEle->value);
							freeXMLObject(childEle);
						}
					
					
						freeXMLObject(colorEle);
					}
					
					
					//Others to come later...
					
					freeXMLObject(preferencesEle);
				}
				
				
				//Try to load the saved alarms
				preferencesEle = findXMLObjectTest(rootEle->value, "userAlarms", 0);
				if (preferencesEle != NULL) {
					i =0;
					alarmObjEle = findXMLObjectTest(preferencesEle->value, "alarmObj", i);
					
					if (alarmObjEle != NULL) {
						//Read the setting values
						do {
							if (userPreferences.userAlarmsCount < i + 1) {
								printf("User alarm count was %i not high enough for alarm %i, adding\n", userPreferences.userAlarmsCount, i + 1);
								//Alarm object for this doesn't exist, so we need to expand to take it
								//Expand the main array
								userPreferences.userAlarms = (struct alarmObject **)realloc(userPreferences.userAlarms, (userPreferences.userAlarmsCount + 1) * sizeof(struct alarmObject *));
							
								/// allocate memory for one AlarmObject 
								userPreferences.userAlarms[userPreferences.userAlarmsCount] = (struct alarmObject *)malloc(sizeof(struct alarmObject));
								
								//Setup the defaults for this alarm obj
								initAlarmObject(userPreferences.userAlarms[userPreferences.userAlarmsCount]);
								
								//printf("User alarm count was not high enough, adding\n");
								
								userPreferences.userAlarmsCount++;
								
							}
							
							//Okay good, now the fun stuff... try to load all of the alarms
							
							childEle = findXMLObjectTest(alarmObjEle->value, "alarmTime", 0);
							if (childEle != NULL) {
								userPreferences.userAlarms[i]->alarmTime = atoi(childEle->value);
								freeXMLObject(childEle);
							}
							
							childEle = findXMLObjectTest(alarmObjEle->value, "alarmIndex", 0);
							if (childEle != NULL) {
								userPreferences.userAlarms[i]->alarmIndex = atoi(childEle->value);
								freeXMLObject(childEle);
							}
							
							childEle = findXMLObjectTest(alarmObjEle->value, "alarmHHMM", 0);
							if (childEle != NULL) {
								bzero(userPreferences.userAlarms[i]->alarmHHMM, sizeof(userPreferences.userAlarms[i]->alarmHHMM));
								sprintf(userPreferences.userAlarms[i]->alarmHHMM, "%s", childEle->value);
								freeXMLObject(childEle);
							}
							
							childEle = findXMLObjectTest(alarmObjEle->value, "ampm", 0);
							if (childEle != NULL) {
								userPreferences.userAlarms[i]->ampm = atoi(childEle->value);
								freeXMLObject(childEle);
							}
							
							childEle = findXMLObjectTest(alarmObjEle->value, "alarmDay", 0);
							if (childEle != NULL) {
								userPreferences.userAlarms[i]->alarmDay = atoi(childEle->value);
								freeXMLObject(childEle);
							}
							
							childEle = findXMLObjectTest(alarmObjEle->value, "alarmCookie", 0);
							if (childEle != NULL) {
								userPreferences.userAlarms[i]->alarmCookie = atoi(childEle->value);
								freeXMLObject(childEle);
							}
							
							childEle = findXMLObjectTest(alarmObjEle->value, "title", 0);
							if (childEle != NULL) {
								if (userPreferences.userAlarms[i]->title != NULL) {
									free(userPreferences.userAlarms[i]->title);
								}
								userPreferences.userAlarms[i]->title = calloc(strlen(childEle->value) + 2, sizeof(char));
								sprintf(userPreferences.userAlarms[i]->title, "%s", childEle->value);
								freeXMLObject(childEle);
							}
							
							childEle = findXMLObjectTest(alarmObjEle->value, "recurrenceCount", 0);
							if (childEle != NULL) {
								
								userPreferences.userAlarms[i]->recurrenceCount = atoi(childEle->value);
								freeXMLObject(childEle);
							}
							
							childEle = findXMLObjectTest(alarmObjEle->value, "recurrence", 0);
							if (childEle != NULL) {
								//printf("Loaded recurrence Pref as %s\n", childEle->value);
								userPreferences.userAlarms[i]->recurrence = atoi(childEle->value);
								freeXMLObject(childEle);
							}
							
							childEle = findXMLObjectTest(alarmObjEle->value, "enabled", 0);
							if (childEle != NULL) {
								userPreferences.userAlarms[i]->enabled = atoi(childEle->value);
								freeXMLObject(childEle);
							}
						
							childEle = findXMLObjectTest(alarmObjEle->value, "sound", 0);
							if (childEle != NULL) {
								if (userPreferences.userAlarms[i]->sound != NULL) {
									free(userPreferences.userAlarms[i]->sound);
								}
								userPreferences.userAlarms[i]->sound = calloc(strlen(childEle->value) + 2, sizeof(char));
								sprintf(userPreferences.userAlarms[i]->sound, "%s", childEle->value);
								freeXMLObject(childEle);
							}
							
							childEle = findXMLObjectTest(alarmObjEle->value, "fmFreq", 0);
							if (childEle != NULL) {
								userPreferences.userAlarms[i]->fmFreq = atoi(childEle->value);
								freeXMLObject(childEle);
							}
						
							childEle = findXMLObjectTest(alarmObjEle->value, "alertMode", 0);
							if (childEle != NULL) {
								userPreferences.userAlarms[i]->alertMode = atoi(childEle->value);
								freeXMLObject(childEle);
							}
							
							childEle = findXMLObjectTest(alarmObjEle->value, "loopSound", 0);
							if (childEle != NULL) {
								userPreferences.userAlarms[i]->loopSound = atoi(childEle->value);
								freeXMLObject(childEle);
							}
							
							childEle = findXMLObjectTest(alarmObjEle->value, "snoozeTime", 0);
							if (childEle != NULL) {
								userPreferences.userAlarms[i]->snoozeTime = atoi(childEle->value);
								freeXMLObject(childEle);
							}
							
							
					
					
							if (alarmObjEle != NULL) {
								freeXMLObject(alarmObjEle);
							}
							i++;
							alarmObjEle = findXMLObjectTest(preferencesEle->value, "alarmObj", i);
						} while (alarmObjEle != NULL);
					
						if (alarmObjEle != NULL) {
							freeXMLObject(alarmObjEle);
						}
					}
					freeXMLObject(preferencesEle);
				
				}
				
				
				
				
				freeXMLObject(rootEle);
				free(configContent);
				
			}

		}
	
	
	}
	//printf("test alarm: %i\n", userPreferences.userAlarms[2]->enabled);

}

/*******************************************************
* setUserPreferences()
*
* Function to save the current preferences to the config file
* 
* Okay this is pretty crude I admit,but then this HAS to be one
* of the fastest and most efficient ways of updating the prefs...
*
*******************************************************/

void setUserPreferences() {
	char prefsFile[512];		//Path to preferences file
	
	FILE *inputFilePtr;           /* Pointer to input file */
	char configContent[8096];
	char configContentTemp[4096];
	
	int i;
	
	//mode_t fileMode;	//temp

	//First we update the user preferences, then save them
	if (currentTheme.themePath != NULL) {
		bzero(userPreferences.currentThemePath, sizeof(userPreferences.currentThemePath));
		sprintf(userPreferences.currentThemePath, "%s", currentTheme.themePath);
	}



	sprintf(configContent, "<?xml version=\"1.0\" encoding=\"iso-8859-1\"?> \r\n\
<!-- flipclockC preferences file --> \r\n\
<flipClockPreferences> \r\n\
	<mainOptions> \r\n\
		<currentThemePath>%s</currentThemePath> \r\n\
		<militaryTime>%i</militaryTime> \r\n\
		<showSeconds>%i</showSeconds> \r\n\
		<insomniacModeOn>%i</insomniacModeOn> \r\n\
		<useAlarmD>%i</useAlarmD> \r\n\
		<maxAlarmVol>%i</maxAlarmVol> \r\n\
		<insomniacDim>%i</insomniacDim> \r\n\
		<insomniacLocked>%i</insomniacLocked> \r\n\
		<alarmControlMode>%i</alarmControlMode> \r\n\
		<moodColor>\r\n\
			<red>%i</red> \r\n\
			<green>%i</green> \r\n\
			<blue>%i</blue> \r\n\
		</moodColor> \r\n\
	</mainOptions> \r\n", userPreferences.currentThemePath, userPreferences.militaryTime, userPreferences.showSeconds,userPreferences.insomniacModeOn, userPreferences.useAlarmD, userPreferences.maxAlarmVol, userPreferences.insomniacDim, userPreferences.insomniacLocked, userPreferences.alarmControlMode, userPreferences.moodRed, userPreferences.moodGreen, userPreferences.moodBlue); 

	//********** Save Alarms
	bzero(configContentTemp, sizeof(configContentTemp));
	sprintf(configContentTemp, "	<userAlarms>\r\n");
	strcat(configContent, configContentTemp);
	
	for (i=0; i < userPreferences.userAlarmsCount; i++) {
		bzero(configContentTemp, sizeof(configContentTemp));
		sprintf(configContentTemp, "\t\t<alarmObj>\r\n\
			<alarmTime>%ld</alarmTime> \r\n\
			<alarmIndex>%i</alarmIndex> \r\n\
			<alarmHHMM>%s</alarmHHMM> \r\n\
			<ampm>%i</ampm> \r\n\
			<alarmDay>%i</alarmDay> \r\n\
			<alarmCookie>%ld</alarmCookie>\r\n\
			<title>%s</title>\r\n\
			<recurrenceCount>%i</recurrenceCount>\r\n\
			<recurrence>%i</recurrence>\r\n\
			<enabled>%i</enabled>\r\n\
			<sound>%s</sound>\r\n\
			<fmFreq>%i</fmFreq> \r\n\
			<alertMode>%i</alertMode>\r\n\
			<loopSound>%i</loopSound>\r\n\
			<snoozeTime>%i</snoozeTime>\r\n\
		</alarmObj>\r\n", userPreferences.userAlarms[i]->alarmTime, userPreferences.userAlarms[i]->alarmIndex, userPreferences.userAlarms[i]->alarmHHMM,
		userPreferences.userAlarms[i]->ampm, userPreferences.userAlarms[i]->alarmDay, userPreferences.userAlarms[i]->alarmCookie, userPreferences.userAlarms[i]->title,
		userPreferences.userAlarms[i]->recurrenceCount, userPreferences.userAlarms[i]->recurrence, userPreferences.userAlarms[i]->enabled,
		userPreferences.userAlarms[i]->sound, userPreferences.userAlarms[i]->fmFreq,  userPreferences.userAlarms[i]->alertMode, userPreferences.userAlarms[i]->loopSound, userPreferences.userAlarms[i]->snoozeTime);
	
		//sprintf(configContent, "%s", configContentTemp);
		strcat(configContent, configContentTemp);
	}
	bzero(configContentTemp, sizeof(configContentTemp));
	sprintf(configContentTemp, "	</userAlarms>\r\n");
	
	strcat(configContent, configContentTemp);
	//********** Done saving alarms
	bzero(configContentTemp, sizeof(configContentTemp));
	sprintf(configContentTemp, "\r\n </flipClockPreferences>");
	
	strcat(configContent, configContentTemp);
	
	//Check to see if path exists for preferences
	if (isDirectory(PREFSPATH)) {
		sprintf(prefsFile, "%s.flipClockPrefs.xml", PREFSPATH);
	
		inputFilePtr = fopen(prefsFile, "w");
	
		if (inputFilePtr) {
			fwrite(configContent, 1, strlen(configContent), inputFilePtr);
	
			//Close
			fclose(inputFilePtr);
		}
		
		//Set prefs file to globally writeable in case we goof and run flip by a different user later or something (ie root from the command line)
		//fileMode = getMode(prefsFile);
		//printf("file mode %i\n", fileMode);
		chmod(prefsFile, 511);
	}
	

}

/*******************************************************
* cleanUserPreferences()
*
* Function to cleanup user preferences and free associated
* memory/etc
*
*******************************************************/

void cleanUserPreferences() {
	
	//Right now the only thing we have to clean up is the alarms array...
	
	int i;

	for(i = 0; i < userPreferences.userAlarmsCount; i++) {
		freeAlarmObject(userPreferences.userAlarms[i]);
		free(userPreferences.userAlarms[i]);
	}
	
	free(userPreferences.userAlarms);
	userPreferences.userAlarmsCount = 0;

}

/*******************************************************
* initAlarmObject()
*
* Function to initialize an alarm object and populate it with default values
*
*******************************************************/

void initAlarmObject(struct alarmObject *thisAlarm) {
	
	thisAlarm->alarmTime = 0;
	thisAlarm->alarmIndex = 0;
	sprintf(thisAlarm->alarmHHMM, "0000");
	thisAlarm->title = NULL;
	thisAlarm->sound = NULL;
	thisAlarm->ampm = 1;   //default is AM
	thisAlarm->alarmDay = 0;
	thisAlarm->alarmCookie = 0;
	thisAlarm->recurrenceCount = -1;
	thisAlarm->recurrence = (60 * 24 * 7);			//Once per week
	thisAlarm->enabled = 0;
	thisAlarm->alertMode = 0;
	thisAlarm->loopSound = 0;
	thisAlarm->snoozeTime = 10;
	thisAlarm->fmFreq = 99900;
	
}

/*********************************************************
* freeAlarmObject(struct alarmObject thisAlarm)
*
* Function to cleanup an alarm object
**********************************************************/

void freeAlarmObject(struct alarmObject *thisAlarm) {
	if (thisAlarm->title != NULL) {
		free(thisAlarm->title);
	}
	if (thisAlarm->sound != NULL) {
		free(thisAlarm->sound);
	}

}

