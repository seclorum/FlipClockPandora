/***************** AlarmD v2 Functions

Functions that allow flipclock to be tied into AlarmD v2 Daemon.

These functions should be seamlessly disabled if osso isn't present, allowing flip to run on any other OS that has
the appropriate SDL libs. (just without as much coolness).

This version has been updated to use libalarm2 interfaces and is compatible with Maemo >= 5.x; the old alarmD interface
is implemented in the original alarmDFunc.c.  

*/




#if defined(LIBOSSO_H_)
//Only allow to be included if libosso is present

//Include alarmD API
#include <alarmd/libalarm.h>
#include <dbus/dbus-protocol.h> //For Dbus params to alarmD

#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

//Include XML library if not already present
#ifndef XMLFUNC_H
#include "xmlFunc.c"
#endif

int hasAlarmD = 1;	//Device is alarmD capable!


#define APPID "flipclock"


//************** Define functions themselves ***********************//


/****************************************************************************
*
* updateAlarmIndex(int alarmIndex) {
*
* Generic function update a single alarm and sync it with AlarmD
* This function is identical to alarmDFunc.c since it doesn't contain any direct
* calls to libalarm.
*
****************************************************************************/

int updateAlarmIndex(int alarmIndex) {
	struct tm tim, todayTest;
    time_t now, todayStart;

	int targetTime;	//Time our alarm will go off

	int todayDay;	//Which day of the week
	int targetDay;	//Day of the week that we're aiming for
	
	char testDate[100];
	char timeBlock[6];	//Temp buffer for HH MM used in conversion
	int hours =0;
	int minutes = 0;
	
	int needsUpdate = 0;	//Does alarm need to be updated?
	int alarmDResult = 0;	//Status of alarmD calls.
	
	
	if (userPreferences.userAlarms[alarmIndex] == NULL) {
		return 0;
	}
	
	
	//Get the hours and minutes
	memset(&timeBlock, 0, sizeof(timeBlock));
	sprintf(timeBlock, "%c%c", userPreferences.userAlarms[alarmIndex]->alarmHHMM[0], userPreferences.userAlarms[alarmIndex]->alarmHHMM[1]);
	hours = atoi(timeBlock);
	
	memset(&timeBlock, 0, sizeof(timeBlock));
	sprintf(timeBlock, "%c%c", userPreferences.userAlarms[alarmIndex]->alarmHHMM[2], userPreferences.userAlarms[alarmIndex]->alarmHHMM[3]);
	minutes = atoi(timeBlock);
	
	
	//Get local time
	now = time(NULL);
    tim = *(localtime(&now));

	
	
	//OKAY WRONG AGAIN! You have to calculate the actual time of the event (i.e. what day it's actually scheduled for) so that
	//the dst can be accurately determined!
	//so, try once more...
	//todayTest = myTime.gmtime(myTime.time())
	
	//#find out what day of the week it is (where 0 is Monday, Sunday is 6)
	//todayDay = todayTest.tm_wday
	todayDay = tim.tm_wday;
	
	//#modify the struct to point to 00:00:00 today
	//todayTest = (todayTest.tm_year, todayTest.tm_mon, todayTest.tm_mday, 0,0,0, todayTest.tm_wday, todayTest.tm_yday, todayTest.tm_isdst)
	memset(&todayTest, 0, sizeof(todayTest));
	todayTest.tm_year = tim.tm_year;
	todayTest.tm_mon = tim.tm_mon;
	todayTest.tm_mday = tim.tm_mday;
	todayTest.tm_isdst = tim.tm_isdst;
	todayStart = mktime(&todayTest);
	

	
	targetDay = userPreferences.userAlarms[alarmIndex]->alarmDay;

	if (targetDay - todayDay < 0) {
		//day is behind us
		targetTime = todayStart + ((7 + targetDay - todayDay) * 86400);
	} else {
		//Day is in the future still
		targetTime = todayStart + ((targetDay - todayDay) * 86400);
	}

	//Okay we've got the base point, now we need to add teh digit values to get the time
	if (hours < 13 && userPreferences.userAlarms[alarmIndex]->ampm == 1) {
		hours += 12;	//Add 12 hours to the time
	}
	if (hours == 12 && userPreferences.userAlarms[alarmIndex]->ampm == 0) {
		hours = 0;
	}
	
	targetTime += (hours *3600) + (minutes * 60);
	
	//Update the time of the alarm
	//Check to see if the alarm time changed (also need to OR alarmDisabled/Enabled
	if (userAlarmDObjects[alarmIndex]->alarmTime != targetTime && userPreferences.userAlarms[alarmIndex]->enabled == 1) {
		needsUpdate = 1;
		printf("alarm time diff\n");
	} else if (userPreferences.userAlarms[alarmIndex]->enabled == 1 && userAlarmDObjects[alarmIndex]->alarmCookie == 0) {
		//Alarm has just been turned on but time was not changed
		needsUpdate = 1;
		printf("need to enable alarm\n");
	} else if (userPreferences.userAlarms[alarmIndex]->enabled == 0 && userAlarmDObjects[alarmIndex]->alarmCookie != 0) {
		//Opposite, alarm has just been turned off but time was not changed
		needsUpdate = 1;
		printf("need to disable alarm\n");
	}
	
	if (needsUpdate) {
		printf("***** ALARM FUNC - Starting update of alarmD\n");
		userAlarmDObjects[alarmIndex]->alarmTime = targetTime;
		
		
		if (userAlarmDObjects[alarmIndex]->alarmCookie != 0) {
			//Old alarmD entry existed for this alarm, so clear it
			if (userPreferences.useAlarmD == 1 &&  hasAlarmD == 1) {
				alarmDResult = clearFlipAlarm(userAlarmDObjects[alarmIndex]->alarmCookie);
			
				printf("tried to clear alarm with cookie %i with result %i\n", userAlarmDObjects[alarmIndex]->alarmCookie, alarmDResult);
			
				if (alarmDResult) {
					userAlarmDObjects[alarmIndex]->alarmCookie = 0;
				} else {
					printf("Alarm D Clearing of event failed! aborting update!\n");
					return 0;
				}
			}
		}
		
		if (userPreferences.userAlarms[alarmIndex]->enabled == 1) {
			if (userPreferences.useAlarmD == 1 &&  hasAlarmD == 1) {
				//Set flip alarm here
				setFlipAlarmByIndex(alarmIndex);
			}
		}
		printf("***** ALARM FUNC - Done update of alarmD\n");
	}

	return needsUpdate;

}






/*******************************************************
*
* getFlipAlarms()
*
* Function to list all Flipclock alarms
* 
* Hopefully updated for AlarmDv2.
********************************************************/

int getFlipAlarms() {

	cookie_t *setAlarms, *iter;	//Container for alarm ID's
	alarm_event_t *thisAlarm;	//All alarm events
	alarm_action_t *act = 0;	//Action for this alarm event
	int i = 0;
	
	const char *dbusInterface = NULL;
	const char *dbusPath = NULL;
	
	
	int matchedAlarm =0;	//Does alarm match criteria?
	
	setAlarms = NULL;
	
	setAlarms = alarmd_event_query((time_t) 0, (time_t) INT_MAX, 0, 0, APPID);
	if (setAlarms == NULL) {
		printf("No alarms!\n");
		return 0;
	}
	
	if (setAlarms[0] == (cookie_t) 0) {
		printf("No alarms!\n");
	} else {
		//Alarms found!
		for (iter = setAlarms; *iter != (cookie_t) 0; iter++) {
			thisAlarm = alarmd_event_get(*iter);
			
			matchedAlarm = 0;
			
			
			//Shouldn't need to do this thanks to that APPID thing, but might as well just to be safe... never hurts!
			//Libalarm2 introduces multiple actions per alarm event, so we have to check them all
			for(i = 0; (act = alarm_event_get_action(thisAlarm, i)) != 0; ++i )
			{
				//printf("action%d.label = %s\n", i, alarm_action_get_label(act));

				dbusInterface = alarm_action_get_dbus_interface(act);
				dbusPath = alarm_action_get_dbus_path(act);

				if ((dbusInterface != NULL)  && (dbusPath != NULL)) {
					if (!strcmp(dbusInterface, DBUS_INTERFACE)) { 
						if (!strcmp(dbusPath, DBUS_PATH)) {	
							matchedAlarm = 1;
						}
					}
					//Should I be freeing these? guess so..
					//free(dbusInterface);
					//free(dbusPath);
				}
			}
			
			
			if (matchedAlarm) {
				printf("Found flip alarm %s\n", alarm_event_get_title(thisAlarm));
			}
			
			
			alarm_event_delete(thisAlarm);
		}
		

	}
	
	if (setAlarms != NULL) {
		free(setAlarms);
	}
	
	return 1;
	

}




/*******************************************************
*
* clearFlipAlarm(int alarmCookie)
*
* Function to clear a flip alarm from alarmD
*
* Updated for v2
********************************************************/

int clearFlipAlarm(int alarmCookie) {

	//This is pretty easy
	return alarmd_event_del((cookie_t) alarmCookie);

}

/*******************************************************
*
* clearAllFlipAlarms()
*
* Function to clear all Flipclock alarms from alarmQueue
*
* Updated for v2
********************************************************/

int clearAllFlipAlarms() {

	cookie_t *setAlarms, *iter;	//Container for alarm ID's
	alarm_event_t *thisAlarm;	//All alarm events
	alarm_action_t *act = 0;	//Action for this alarm event
	int i = 0;
	
	const char *dbusInterface = NULL;
	const char *dbusPath = NULL;
	
	int matchedAlarm =0;	//Does alarm match criteria?
	
	setAlarms = NULL;
	
	setAlarms = alarmd_event_query((time_t) 0, (time_t) INT_MAX, 0, 0, APPID);
	if (setAlarms == NULL) {
		printf("No alarms!\n");
		return 0;
	}
	
	if (setAlarms[0] == (cookie_t) 0) {
		printf("No alarms!\n");
	} else {
		//Alarms found!
		for (iter = setAlarms; *iter != (cookie_t) 0; iter++) {
			thisAlarm = alarmd_event_get(*iter);
			
			matchedAlarm = 0;
			
			//Shouldn't need to do this thanks to that APPID thing, but might as well just to be safe... never hurts!
			//Libalarm2 introduces multiple actions per alarm event, so we have to check them all
			for(i = 0; (act = alarm_event_get_action(thisAlarm, i)) != 0; ++i )
			{
				//printf("action%d.label = %s\n", i, alarm_action_get_label(act));

				dbusInterface = alarm_action_get_dbus_interface(act);
				dbusPath = alarm_action_get_dbus_path(act);

				if ((dbusInterface != NULL)  && (dbusPath != NULL)) {
					if (!strcmp(dbusInterface, DBUS_INTERFACE)) { 
						if (!strcmp(dbusPath, DBUS_PATH)) {	
							matchedAlarm = 1;
						}
					}
					//Should I be freeing these? guess so..
					//free(dbusInterface);
					//free(dbusPath);
				}
			}
			
			
			if (matchedAlarm) {
				clearFlipAlarm(*iter);
				printf("Cleared flip alarm %s\n", alarm_event_get_title(thisAlarm));
			}
			
			
			alarm_event_delete(thisAlarm);
		}
		

	}
	
	if (setAlarms != NULL) {
		free(setAlarms);
	}
	
	return 1;
	

}


/*******************************************************
*
* createDefaultAlarm(alarm_event_t *thisAlarmObj)
*
* Function to fill an alarm event object with the default
* flipclock values.
*******************************************************

void createDefaultAlarm(alarm_event_t *thisAlarmObj) {

	thisAlarmObj->flags = ALARM_EVENT_NO_DIALOG | ALARM_EVENT_BOOT | ALARM_EVENT_ACTIVATION | ALARM_EVENT_SHOW_ICON;
	thisAlarmObj->sound = NULL;
	
	//**** Setup the DBus stuff
	thisAlarmObj->dbus_service = calloc(strlen(DBUS_SERVICE) + 2, sizeof(char));
	sprintf(thisAlarmObj->dbus_service, "%s", DBUS_SERVICE);
	
	thisAlarmObj->dbus_interface = calloc(strlen(DBUS_INTERFACE) + 2, sizeof(char));
	sprintf(thisAlarmObj->dbus_interface, "%s", DBUS_INTERFACE);

	thisAlarmObj->dbus_path = calloc(strlen(DBUS_PATH) + 2, sizeof(char));
	sprintf(thisAlarmObj->dbus_path, "%s", DBUS_PATH);
	
	thisAlarmObj->dbus_name = calloc(strlen("triggerAlarm") + 2, sizeof(char));
	sprintf(thisAlarmObj->dbus_name, "triggerAlarm");
	//**** Done Dbus setup
	
	thisAlarmObj->title = calloc(50, sizeof(char));
	sprintf(thisAlarmObj->title, "FlipClockAlarm");
	
	thisAlarmObj->message = NULL;
	thisAlarmObj->icon = NULL;
	thisAlarmObj->snooze = 0;
	thisAlarmObj->recurrence = 0;
	thisAlarmObj->recurrence_count = 0;

}
*/
/*******************************************************
*
* freeAlarmObj(alarm_event_t *thisAlarmObj)
*
* Function to free the members of an alarm obj
* Not sure if this is teh same as the built in alarm_event_free, but
* whatever... that one crashed things so...
*
* This shouldn't be required anymore, so comment out for now, delete later
********************************************************

void freeAlarmObj(alarm_event_t thisAlarmObj) {
	//Not sure 
	if (thisAlarmObj.dbus_service != NULL) {
		free(thisAlarmObj.dbus_service);
	}
	
	if (thisAlarmObj.dbus_interface != NULL) {
		free(thisAlarmObj.dbus_interface);
	}
	
	if (thisAlarmObj.dbus_path != NULL) {
		free(thisAlarmObj.dbus_path);
	}
	
	if (thisAlarmObj.dbus_name != NULL) {
		free(thisAlarmObj.dbus_name);
	}
	
	if (thisAlarmObj.title != NULL) {
		free(thisAlarmObj.title);
	}
	
}
**********************/


/*******************************************************
*
* setFlipAlarm(int alarmIndex)
*
* Function to create a flip alarm for alarmD
********************************************************/

int setFlipAlarmByIndex(int alarmIndex) {
	alarm_event_t *newEvent = 0;
	alarm_action_t *act = 0;
	


	//First make sure the alarm has a time defined
	if (userAlarmDObjects[alarmIndex]->alarmTime == 0) {
		return 0;
	}
	
	//Clear event if previously existing to be safe
	if (userAlarmDObjects[alarmIndex]->alarmCookie > 0) {
		if (clearFlipAlarm(userAlarmDObjects[alarmIndex]->alarmCookie) == 0) {
			//Couldn't clear out old alarm!
			return 0;
		} else {
			userAlarmDObjects[alarmIndex]->alarmCookie = 0;
		}
	}

	//Now we create the alarm event struct based on the settings given
	// Initialize alarm to 0s
    //memset (&newEvent, 0, sizeof(alarm_event_t));
	
	newEvent = alarm_event_create();	//Create the default alarm struct.
	//Set the APP ID
	alarm_event_set_alarm_appid(newEvent, APPID);
	
	alarm_event_set_title(newEvent, "FlipClockAlarm");
	
	//Create the alarm action
	act = alarm_event_add_actions(newEvent, 1);
	//Setup the action
	newEvent->flags = ALARM_EVENT_BOOT | ALARM_EVENT_SHOW_ICON;
	
	alarm_action_set_dbus_interface(act, DBUS_INTERFACE);
	alarm_action_set_dbus_service(act, DBUS_SERVICE);
	alarm_action_set_dbus_path(act, DBUS_PATH);
	alarm_action_set_dbus_name(act, "triggerAlarm");
	act->flags = ALARM_ACTION_WHEN_TRIGGERED | ALARM_ACTION_DBUS_USE_ACTIVATION | ALARM_ACTION_TYPE_DBUS;
	
	
	
	//Setup the alarm event members
	//createDefaultAlarm(&newEvent);
	
	
	
	//Now populate it with our actual values
	//if (userPreferences.userAlarms[alarmIndex]->title != NULL) {
		//printf("Alarm title: %s %s\n", userPreferences.userAlarms[alarmIndex]->title, newEvent.title);
		//if (newEvent.title != NULL) {
			//free(newEvent.title);
		//}
		//newEvent.title = calloc(strlen(userPreferences.userAlarms[alarmIndex]->title), sizeof(char));
		//sprintf(newEvent.title, "boo %s", userPreferences.userAlarms[alarmIndex]->title); 
	//}

	newEvent->recur_secs =userPreferences.userAlarms[alarmIndex]->recurrence * 60;  // The alarm function is now in seconds...
	newEvent->recur_count =userPreferences.userAlarms[alarmIndex]->recurrenceCount;
	newEvent->alarm_time = (time_t) userAlarmDObjects[alarmIndex]->alarmTime;
	
	printf("trying to make actual alarmD call with recurrence %i %i\n", newEvent->recur_secs,userPreferences.userAlarms[alarmIndex]->recurrence );
	//userPreferences.userAlarms[alarmIndex]->alarmCookie = alarm_event_add(&newEvent);
	
	//Okay all good, let's try to add it
	userAlarmDObjects[alarmIndex]->alarmCookie = alarmd_event_add_with_dbus_params(newEvent, DBUS_TYPE_INT64, &userAlarmDObjects[alarmIndex]->alarmTime, DBUS_TYPE_INT32, &alarmIndex, DBUS_TYPE_INVALID);
	
	
	
	alarm_event_delete(newEvent);

	
	printf("AlarmD update status: %ld\n",userAlarmDObjects[alarmIndex]->alarmCookie);
	
	
	
	return 1;
}





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

void setDefaultUserAlarmDObjects() {
	int i;
	char tempBuffer[255];
	
	//Set default alarms
	//For now, let's just do one alarm for each day; others can be added later maybe...
	userAlarmDObjectsCount = 0;
	
	for (i=0; i< userPreferences.userAlarmsCount; i++) {
			userAlarmDObjects = (struct alarmDSettingsObject **)realloc(userAlarmDObjects, (userAlarmDObjectsCount + 1) * sizeof(struct alarmDSettingsObject *));
							
			/// allocate memory for one AlarmObject 
			userAlarmDObjects[userAlarmDObjectsCount] = (struct alarmDSettingsObject *)malloc(sizeof(struct alarmDSettingsObject));
			
			//Setup the defaults for this alarm obj
			initAlarmDSettingsObject(userAlarmDObjects[userAlarmDObjectsCount]);
			
			userAlarmDObjects[userAlarmDObjectsCount]->alarmIndex = i;
			
			userAlarmDObjectsCount++;						
	}
	printf("useralarmDObjCount is now %i\n", userAlarmDObjectsCount);

}

/*******************************************************
* getUserPreferences()
*
* Function to load the user's preferences from the current 
* preferences config file, or set default values if no
* config file is present
*
*******************************************************/

void getAlarmDSettings() {
	char prefsFile[512];		//Path to preferences file
	
	FILE *inputFilePtr;           /* Pointer to input file */
	char *configContent;
	struct xmlNode *rootEle, *preferencesEle, *colorEle, *alarmObjEle, *childEle;
	
	
	int i;

	//Check to see if path exists for preferences
	if (isDirectory(PREFSPATH)) {
		
		sprintf(prefsFile, "%s.flipClockAlarmD.xml", PREFSPATH);
		
		if (fileExists(prefsFile)) {
			//Preference file exists, so load the data out of it
			inputFilePtr = fopen(prefsFile, "rb"); /* Open in BINARY mode */

			//Read the file
			configContent = readFile(inputFilePtr);
	
			//Close file pointer
			fclose(inputFilePtr);
			
			
			
			rootEle = findXMLObjectTest(configContent, "flipClockAlarmD", 0);
			
			if (rootEle != NULL) {
				//Settings exist
				
				//Try to load the saved alarms
				preferencesEle = findXMLObjectTest(rootEle->value, "userAlarms", 0);
				if (preferencesEle != NULL) {
					i =0;
					alarmObjEle = findXMLObjectTest(preferencesEle->value, "alarmObj", i);
					
					if (alarmObjEle != NULL) {
						//Read the setting values
						do {
							if (userAlarmDObjectsCount < i + 1) {
								printf("User alarm count was %i not high enough for alarm %i, adding\n", userAlarmDObjectsCount, i + 1);
								//Alarm object for this doesn't exist, so we need to expand to take it
								//Expand the main array
								userAlarmDObjects = (struct alarmDSettingsObject **)realloc(userAlarmDObjects, (userAlarmDObjectsCount + 1) * sizeof(struct alarmDSettingsObject *));
							
								/// allocate memory for one AlarmObject 
								userAlarmDObjects[userAlarmDObjectsCount] = (struct alarmDSettingsObject *)malloc(sizeof(struct alarmDSettingsObject));
								
								//Setup the defaults for this alarm obj
								initAlarmDSettingsObject(userAlarmDObjects[userAlarmDObjectsCount]);
								
								printf("User alarm count was not high enough, adding\n");
								
								userAlarmDObjectsCount++;
								
							}
							
							//Okay good, now the fun stuff... try to load all of the alarms
							
							childEle = findXMLObjectTest(alarmObjEle->value, "alarmTime", 0);
							if (childEle != NULL) {
								userAlarmDObjects[i]->alarmTime = atoi(childEle->value);
								freeXMLObject(childEle);
							}
							
							childEle = findXMLObjectTest(alarmObjEle->value, "alarmIndex", 0);
							if (childEle != NULL) {
								userAlarmDObjects[i]->alarmIndex = atoi(childEle->value);
								freeXMLObject(childEle);
							}
							
							childEle = findXMLObjectTest(alarmObjEle->value, "alarmCookie", 0);
							if (childEle != NULL) {
								userAlarmDObjects[i]->alarmCookie = atoi(childEle->value);
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

void setAlarmDSettings() {
	char prefsFile[512];		//Path to preferences file
	
	FILE *inputFilePtr;           /* Pointer to input file */
	char configContent[8096];
	char configContentTemp[4096];
	
	int i;
	



	sprintf(configContent, "<?xml version=\"1.0\" encoding=\"iso-8859-1\"?> \r\n\
<!-- flipclockC alarmD mappings --> \r\n\
<flipClockAlarmD> \r\n"); 

	//********** Save Alarms
	bzero(configContentTemp, sizeof(configContentTemp));
	sprintf(configContentTemp, "	<userAlarms>\r\n");
	strcat(configContent, configContentTemp);
	
	for (i=0; i < userAlarmDObjectsCount; i++) {
		bzero(configContentTemp, sizeof(configContentTemp));
		sprintf(configContentTemp, "\t\t<alarmObj>\r\n\
			<alarmTime>%ld</alarmTime> \r\n\
			<alarmIndex>%i</alarmIndex> \r\n\
			<alarmCookie>%ld</alarmCookie>\r\n\
		</alarmObj>\r\n", userAlarmDObjects[i]->alarmTime, userAlarmDObjects[i]->alarmIndex, userAlarmDObjects[i]->alarmCookie);
	
		strcat(configContent, configContentTemp);
	}
	bzero(configContentTemp, sizeof(configContentTemp));
	sprintf(configContentTemp, "	</userAlarms>\r\n");
	
	strcat(configContent, configContentTemp);
	//********** Done saving alarms
	bzero(configContentTemp, sizeof(configContentTemp));
	sprintf(configContentTemp, "\r\n </flipClockAlarmD>");
	
	strcat(configContent, configContentTemp);
	
	//Check to see if path exists for preferences
	if (isDirectory(PREFSPATH)) {
		sprintf(prefsFile, "%s.flipClockAlarmD.xml", PREFSPATH);
	
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
* void cleanAlarmDSettingsObjects() 
*
* Function to cleanup alarmDSettingsObjects
* memory/etc
* might not be needed, but good cleanup stuff for when the app is done...
*
*******************************************************/

void cleanAlarmDSettingsObjects() {
	
	//Right now the only thing we have to clean up is the alarms array...
	
	int i;

	for(i = 0; i < userAlarmDObjectsCount; i++) {
		free(userAlarmDObjects[i]);
	}
	
	free(userAlarmDObjects);
	userAlarmDObjectsCount = 0;

}

/*******************************************************
* initAlarmDSettingsObject()
*
* Function to initialize an alarmD settings object and populate it with default values
*
*******************************************************/

void initAlarmDSettingsObject(struct alarmDSettingsObject *thisAlarm) {
	
	thisAlarm->alarmTime = 0;
	thisAlarm->alarmIndex = 0;
	thisAlarm->alarmCookie = 0;
}




#else 
int hasAlarmD = 0;	//Device is alarmD capable!

int clearFlipAlarm(int alarmCookie) {
	return 0;
}

int setFlipAlarmByIndex(int alarmIndex) {
	return 0;
}


#endif
