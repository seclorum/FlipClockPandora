
/*

Flip Clock Alarm Manager Module
Part of FlipClock C SDL for Maemo.

This is the main file for the alarm/alarm settings functions

-Rob Williams, Sept 12, 2009.


*/

//Global var to store the currently playing alarm sound object.
alarmSoundObj *myAlarmSound;

//****************** Function Definitions **************************************//

/****************************************************************************
*
* getFirstAlarmIndexByDay(int alarmDay);
*
* Get the first index of an alarm for the specific day.
****************************************************************************/

int getFirstAlarmIndexByDay(int alarmDay)
{
	int i;

	for (i = 0; i < userPreferences.userAlarmsCount; i++) {
		if (userPreferences.userAlarms[i]->alarmDay == alarmDay) {
			return i;
		}
	}

	//Not found
	return -1;
}

/****************************************************************************
*
* checkForAlarm(long alarmTime);
*
* Check to see if any alarms are occuring at the given time.
****************************************************************************/

int checkForAlarm(long alarmTime)
{
	int i;
	struct tm tim;
	char timeBlock[6];			//Temp buffer for HH MM used in conversion
	int hours = 0;
	int minutes = 0;


	tim = *(localtime(&alarmTime));


	for (i = 0; i < userPreferences.userAlarmsCount; i++) {

		//This is wrong and won't work for recurring alarms!
		//if (userPreferences.userAlarms[i]->alarmTime == alarmTime) {
		//  return i;
		//}
		//Okay instead what we do is check the alarm HHMM and day of the week to see if it matches today

		if (userPreferences.userAlarms[i]->alarmDay == tim.tm_wday) {
			//Get the actual time of this alarm based on the HH MM
			memset(&timeBlock, 0, sizeof(timeBlock));
			sprintf(timeBlock, "%c%c",
					userPreferences.userAlarms[i]->alarmHHMM[0],
					userPreferences.userAlarms[i]->alarmHHMM[1]);
			hours = atoi(timeBlock);

			if (hours < 13 && userPreferences.userAlarms[i]->ampm == 1) {
				hours += 12;	//Add 12 hours to the time
			}
			if (hours == 12 && userPreferences.userAlarms[i]->ampm == 0) {
				hours = 0;
			}

			memset(&timeBlock, 0, sizeof(timeBlock));
			sprintf(timeBlock, "%c%c",
					userPreferences.userAlarms[i]->alarmHHMM[2],
					userPreferences.userAlarms[i]->alarmHHMM[3]);
			minutes = atoi(timeBlock);

			if (tim.tm_hour == hours && tim.tm_min == minutes) {
				//Match!
				return i;

			}


		}
	}

	//Not found
	return -1;
}

/****************************************************************************
*
* handleAlarm(int alarmIndex);
*
* Run a given alarm based on it's index.
****************************************************************************/

int handleAlarm(int alarmIndex)
{

	char *soundFile = NULL;

	int fadeTime = 60000;		//time to fade alarm in... this will be set by a preference later...

	//No alarms are snoozing anymore
	alarmSnoozing = 0;
	printf("AlarmIndex is %d while count is %d\n", alarmIndex,
		   userPreferences.userAlarmsCount);

	//Quick error checking
	if (alarmIndex < 0 || alarmIndex > userPreferences.userAlarmsCount) {
		return 0;				//Fail
	}
	//Set the global flag
	runningAlarmIndex = alarmIndex;

	//Switch for the type of alert mode (right now we only have play sound)


	//Goto clock mode
	changeClockMode(CLOCKMODENORMAL);

	//Now go to alarm running
	changeClockMode(CLOCKMODEALARMRUNNING);

	printf
		("runningAlarmIndex is [%i]\nuserAlarmCount is[%i]\nAlarm mode is: [%i]\n",
		 runningAlarmIndex, userPreferences.userAlarmsCount,
		 userPreferences.userAlarms[runningAlarmIndex]->alertMode);

	printf("HandleAlarm:::\n");
	switch (userPreferences.userAlarms[runningAlarmIndex]->alertMode) {
	case 0:					//Play sound
		//do some stuff here
		//Try it crudely here..
		if (isTablet) {
			setTabletSystemVolume(0);
		}

		myAlarmSound = createPlayer("mp3");
		if (myAlarmSound->pipeline != NULL) {
			loadFile(userPreferences.userAlarms[runningAlarmIndex]->sound,
					 myAlarmSound->pipeline);
			setLoopMode(userPreferences.userAlarms[runningAlarmIndex]->
						loopSound, myAlarmSound);

			fadeSoundIn(myAlarmSound, fadeTime,
						userPreferences.maxAlarmVol);
		} else
			debugEntry("Can't play sound, pipeline is NULL\n");

		//playPipe(myAlarmSound->pipeline);

		//setPipeVolume(myAlarmSound->pipeline, 40);

		//printf("Volume is %i\n", getPipeVolume(myAlarmSound->pipeline));

		//Wake device if it's a tablet
		if (isTablet) {
			if (userPreferences.insomniacModeOn == 1 && insomniacMode == 2) {
				if (isInsomniacDimmed == 1) {
					//currently we're dimmed, so gradually fade in
					startWakeFromInsomiaStepped(fadeTime, 30);


				}
			} else {
				//No Insomniac mode, so turn screen on
				ossoDeviceScreenOn();
			}
		}
		break;
	case 1:					//Play Radio
		//Only works on tablets
		if (isTablet) {
			//Radio mode 2 = fade In
			ossoFMRadio(2,
						userPreferences.userAlarms[runningAlarmIndex]->
						fmFreq, fadeTime, userPreferences.maxAlarmVol);

			if (userPreferences.insomniacModeOn == 1 && insomniacMode == 2) {
				if (isInsomniacDimmed == 1) {
					//currently we're dimmed, so gradually fade in
					startWakeFromInsomiaStepped(fadeTime, 30);


				}
			} else {
				//No Insomniac mode, so turn screen on
				ossoDeviceScreenOn();
			}
		}
		break;
	case ALARMALERTMODESOUNDFOLDER:
		printf("Starting play folder with the file index as %i",
			   soundFileIndex);

		//Try it crudely here..
		if (isTablet) {
			setTabletSystemVolume(0);
		}
		//sprintf(soundFile, "%s%s", MEDIAPATH, "test.mp3");

		if (soundFileIndex == -5) {
			//need to set it either to 0 or -1 for random
			if (userPreferences.userAlarms[runningAlarmIndex]->loopSound ==
				0) {
				//0 means linear order
				soundFileIndex = 0;
			} else if (userPreferences.userAlarms[runningAlarmIndex]->
					   loopSound == 1) {
				soundFileIndex = -1;
			}
		}

		soundFile =
			getIndexedMP3FromPath(userPreferences.
								  userAlarms[runningAlarmIndex]->sound,
								  soundFileIndex);

		myAlarmSound = createPlayer("mp3");
		if (soundFileIndex > -1) {
			soundFileIndex++;
		}

		if (soundFile != NULL) {
			//Now we have to grab the first sound file in the folder

			loadFile(soundFile, myAlarmSound->pipeline);

			setLoopMode(2, myAlarmSound);

			fadeSoundIn(myAlarmSound, fadeTime,
						userPreferences.maxAlarmVol);
			//playPipe(myAlarmSound->pipeline);

			//setPipeVolume(myAlarmSound->pipeline, 40);

			//printf("Volume is %i\n", getPipeVolume(myAlarmSound->pipeline));

			//Wake device if it's a tablet
			if (isTablet) {
				if (userPreferences.insomniacModeOn == 1
					&& insomniacMode == 2) {
					if (isInsomniacDimmed == 1) {
						//currently we're dimmed, so gradually fade in
						startWakeFromInsomiaStepped(fadeTime, 30);


					}
				} else {
					//No Insomniac mode, so turn screen on
					ossoDeviceScreenOn();
				}
			}
			free(soundFile);
		}


		break;
	default:

		printf("Unknown alarm mode selected...\nNo action is made...\n");

	}




	return 0;
}


/****************************************************************************
*
* stopAlarm();
*
* Stop the currently running alarm
****************************************************************************/

void stopAlarm()
{

	if (userPreferences.alarmControlMode == ALARMCONTROLMODESIMPLE) {
		//Stopping an alarm in simple mode means it turns off
		userPreferences.userAlarms[runningAlarmIndex]->enabled = 0;

	} else {

		//Update the next alarm time
		getNextAlarmTime();

	}

	//Quick error checking
	if (runningAlarmIndex < 0
		|| runningAlarmIndex > userPreferences.userAlarmsCount) {
		return;					//Fail
	}
	//Reset the global soundFileIndex since we're not using it anymore
	soundFileIndex = -5;

	switch (userPreferences.userAlarms[runningAlarmIndex]->alertMode) {
	case 0:					//Play sound
	case 2:					//Play playlist 
		cleanupPlayer(myAlarmSound);
		if (isTablet) {
			//Restore original sysVol 
			setTabletSystemVolume(1);

			//Restore brightness in insomniac mode
			if (userPreferences.insomniacModeOn == 1 && insomniacMode == 2
				&& alarmSnoozing != 1) {
				wakeFromInsomnia();
			}
		}

		break;
	case 1:					//Play Radio
		//Only works on tablets
		if (isTablet) {
			//Radio mode 0 = stop
			ossoFMRadio(0, 0, 0, 0);

			//Restore brightness in insomniac mode
			if (userPreferences.insomniacModeOn == 1 && insomniacMode == 2
				&& alarmSnoozing != 1) {
				wakeFromInsomnia();
			}
		}
		break;

	}

	//Clear thecurrently running alarm index.
	runningAlarmIndex = -1;

	//Clear the pending snooze time if it's present
	if (alarmSnoozing != 1) {
		pendingSnoozeTime = 0;
	}

	if (userPreferences.alarmControlMode == ALARMCONTROLMODESIMPLE) {
		updateAlarms();
	}
	//Goto normal clock mode
	changeClockMode(CLOCKMODENORMAL);




}


/****************************************************************************
*
* snoozeAlarm();
*
* Snooze the currently running alarm
****************************************************************************/

void snoozeAlarm()
{
	int oldAlarm, oldSoundIndex;
	int currentTime, nextTime;

	oldAlarm = runningAlarmIndex;

	//Store this for later
	oldSoundIndex = soundFileIndex;

	//Quick error checking
	if (runningAlarmIndex < 0
		|| runningAlarmIndex > userPreferences.userAlarmsCount) {
		return;					//Fail
	}

	if (userPreferences.userAlarms[oldAlarm]->snoozeTime == 0) {
		return;					//NO snoozing!
	}
	//Set the global snoozing flag
	alarmSnoozing = 1;

	currentTime = time(NULL);

	nextTime =
		((userPreferences.userAlarms[oldAlarm]->snoozeTime -
		  1) * 60 * 1000) + ((60 - (currentTime % 60)) * 1000);

	pendingSnoozeTime = currentTime + (nextTime / 1000);

	//Stop the current alarm
	stopAlarm();

	//reset sound file index to old value
	soundFileIndex = oldSoundIndex;

	//Warn user that we're snoozing somehow...

	//Go back to insomniac mode if needed
	if (userPreferences.insomniacModeOn == 1 && insomniacMode == 2) {
		set_brightness_insomnia_TO(userPreferences.insomniacDim);
	}

	//Set the timeout to call teh snoozed alarm
	g_timeout_add(nextTime, (GSourceFunc) handleAlarm, (int *) oldAlarm);

}


/****************************************************************************
*
* getNextAlarmTime();
*
* Get the next alarm time (as seconds since epoch) and assign it to the global
* vars nextAlarmTime and nextAlarmIndex.
****************************************************************************/

void getNextAlarmTime()
{

	int i, theDay;
	struct tm tim, todayTest;
	time_t now, todayStart;
	int targetTime;

	char timeBlock[6];			//Temp buffer for HH MM used in conversion
	int hours = 0;
	int minutes = 0;

	//Get local time
	now = time(NULL);
	tim = *(localtime(&now));

	//Set to -1 for now (no next alarm)
	nextAlarmIndex = -1;
	nextAlarmTime = 0;

	printf("next alarmtime as get:%i\n", nextAlarmTime);

	//First we check for alarms that are enabled in the future
	for (i = 0; i < userPreferences.userAlarmsCount; i++) {
		//Only care if the alarm is active
		if (userPreferences.userAlarms[i]->enabled == 1) {
			theDay = userPreferences.userAlarms[i]->alarmDay;

			//generate the time of the alarm
			//Get the hours and minutes
			memset(&timeBlock, 0, sizeof(timeBlock));
			sprintf(timeBlock, "%c%c",
					userPreferences.userAlarms[i]->alarmHHMM[0],
					userPreferences.userAlarms[i]->alarmHHMM[1]);
			hours = atoi(timeBlock);

			memset(&timeBlock, 0, sizeof(timeBlock));
			sprintf(timeBlock, "%c%c",
					userPreferences.userAlarms[i]->alarmHHMM[2],
					userPreferences.userAlarms[i]->alarmHHMM[3]);
			minutes = atoi(timeBlock);

			memset(&todayTest, 0, sizeof(todayTest));
			todayTest.tm_year = tim.tm_year;
			todayTest.tm_mon = tim.tm_mon;
			todayTest.tm_mday = tim.tm_mday;
			todayTest.tm_isdst = tim.tm_isdst;
			todayStart = mktime(&todayTest);

			//Okay we've got the base point, now we need to add teh digit values to get the time
			if (hours < 12 && userPreferences.userAlarms[i]->ampm == 1) {
				hours += 12;	//Add 12 hours to the time
			}
			if (hours == 12 && userPreferences.userAlarms[i]->ampm == 0) {
				hours = 0;
			}

			if (theDay - tim.tm_wday < 0) {
				//day is behind us
				targetTime =
					todayStart + ((7 + theDay - tim.tm_wday) * 86400);
			} else if (theDay - tim.tm_wday == 0) {
				//Day is today, check to see if time is still to come or if it's behind us (so meaning next week)
				targetTime = todayStart + (hours * 3600) + (minutes * 60);
				if (targetTime < now) {
					//Time was behind us, so add a week
					targetTime = todayStart + (7 * 86400);
				} else {
					//Good, so put back to normal
					targetTime = todayStart;
				}


			} else {
				//Day is in the future still
				targetTime = todayStart + ((theDay - tim.tm_wday) * 86400);
			}



			targetTime += (hours * 3600) + (minutes * 60);
			printf("Target alarm time for day %i is %i\n", theDay,
				   targetTime);



			if (targetTime > now) {
				//Only care about alarms in the future
				if (nextAlarmTime > 60) {
					if ((targetTime - now) < (nextAlarmTime - now)) {
						nextAlarmTime = targetTime;
						nextAlarmIndex = i;
					}
				} else {
					nextAlarmTime = targetTime;
					nextAlarmIndex = i;
				}
			}

		}

	}

	printf("next alarmtime as get end:%i\n", nextAlarmTime);


}


/********** HELPER/Pseudo functions for alarm digit scrollers ****************
* These functions allow for the alarm digit scrollers to operate
*
******************************************************************************/

void digitUpHH1()
{
	char currHH1Str[4];
	char currHH2Str[4];
	char newHHMM[5];
	int currHH1;
	int currHH2;
	int newHH1;
	int changed = 0;

	sprintf(currHH1Str, "%c",
			userPreferences.userAlarms[currentAlarmNum]->alarmHHMM[0]);
	currHH1 = atoi(currHH1Str);


	if (currHH1 >= 0) {
		//We're good to increment one!
		if (currHH1 == 0) {
			if (userPreferences.militaryTime == 0) {
				newHH1 = 1;
			} else {
				newHH1 = 2;
			}
			bzero(currHH2Str, sizeof(currHH2Str));
			sprintf(currHH2Str, "%c",
					userPreferences.userAlarms[currentAlarmNum]->
					alarmHHMM[1]);
			currHH2 = atoi(currHH2Str);
			if (currHH2 > 0) {
				animateAlarmScrollerDigit(screen, 1, currHH2, 0,
										  (currHH2) * 10,
										  &currentTheme.alarmDigitPos[1]);
			}
			currHH2 = 0;
			sprintf(newHHMM, "%i%i%c%c", newHH1, currHH2,
					userPreferences.userAlarms[currentAlarmNum]->
					alarmHHMM[2],
					userPreferences.userAlarms[currentAlarmNum]->
					alarmHHMM[3]);
		} else {
			newHH1 = currHH1 - 1;
			sprintf(newHHMM, "%i%c%c%c", newHH1,
					userPreferences.userAlarms[currentAlarmNum]->
					alarmHHMM[1],
					userPreferences.userAlarms[currentAlarmNum]->
					alarmHHMM[2],
					userPreferences.userAlarms[currentAlarmNum]->
					alarmHHMM[3]);
		}
		//Flag to indicate we need a redraw
		changed = 1;
	}

	if (changed == 1) {
		//Redraw the new value
		animateAlarmScrollerDigit(screen, 0, currHH1, newHH1, 10,
								  &currentTheme.alarmDigitPos[0]);

		//Save the new time
		bzero(userPreferences.userAlarms[currentAlarmNum]->alarmHHMM,
			  sizeof(userPreferences.userAlarms[currentAlarmNum]->
					 alarmHHMM));
		sprintf(userPreferences.userAlarms[currentAlarmNum]->alarmHHMM,
				"%s", newHHMM);

	}

}

void digitUpHH2()
{
	char currHH1Str[4];
	char currHH0Str[4];
	char newHHMM[5];

	int currHH1;
	int newHH1;
	int currHH0;
	int changed = 0;

	sprintf(currHH1Str, "%c",
			userPreferences.userAlarms[currentAlarmNum]->alarmHHMM[1]);
	currHH1 = atoi(currHH1Str);

	if (currHH1 >= 0) {

		//We're good to increment one!
		if (currHH1 == 0) {
			sprintf(currHH0Str, "%c",
					userPreferences.userAlarms[currentAlarmNum]->
					alarmHHMM[0]);
			currHH0 = atoi(currHH0Str);
			if (userPreferences.militaryTime == 0) {
				if (currHH0 == 0) {
					newHH1 = 9;
				} else {
					newHH1 = 2;
				}
			} else {
				if (currHH0 < 2) {
					newHH1 = 9;
				} else {
					newHH1 = 3;
				}
			}
		} else {
			newHH1 = currHH1 - 1;
		}
		sprintf(newHHMM, "%c%i%c%c",
				userPreferences.userAlarms[currentAlarmNum]->alarmHHMM[0],
				newHH1,
				userPreferences.userAlarms[currentAlarmNum]->alarmHHMM[2],
				userPreferences.userAlarms[currentAlarmNum]->alarmHHMM[3]);

		//Flag to indicate we need a redraw
		changed = 1;
	}

	if (changed == 1) {
		//Redraw the new value
		animateAlarmScrollerDigit(screen, 1, currHH1, newHH1, 10,
								  &currentTheme.alarmDigitPos[1]);

		//Save the new time
		bzero(userPreferences.userAlarms[currentAlarmNum]->alarmHHMM,
			  sizeof(userPreferences.userAlarms[currentAlarmNum]->
					 alarmHHMM));
		sprintf(userPreferences.userAlarms[currentAlarmNum]->alarmHHMM,
				"%s", newHHMM);

	}


}

void digitUpMM1()
{
	char currHH1Str[4];
	char newHHMM[6];
	int currHH1;
	int newHH1;
	int changed = 0;

	sprintf(currHH1Str, "%c",
			userPreferences.userAlarms[currentAlarmNum]->alarmHHMM[2]);
	currHH1 = atoi(currHH1Str);


	if (currHH1 >= 0) {
		//We can go up!
		currHH1--;
		if (currHH1 < 0) {
			newHH1 = 6 + currHH1;
		} else {
			newHH1 = currHH1;
		}

		sprintf(newHHMM, "%c%c%i%c",
				userPreferences.userAlarms[currentAlarmNum]->alarmHHMM[0],
				userPreferences.userAlarms[currentAlarmNum]->alarmHHMM[1],
				newHH1,
				userPreferences.userAlarms[currentAlarmNum]->alarmHHMM[3]);
		//Flag to indicate we need a redraw
		changed = 1;
	}

	if (changed == 1) {
		//Redraw the new value
		animateAlarmScrollerDigit(screen, 2, currHH1 + 1, newHH1, 10,
								  &currentTheme.alarmDigitPos[2]);

		//Save the new time
		bzero(userPreferences.userAlarms[currentAlarmNum]->alarmHHMM,
			  sizeof(userPreferences.userAlarms[currentAlarmNum]->
					 alarmHHMM));
		sprintf(userPreferences.userAlarms[currentAlarmNum]->alarmHHMM,
				"%s", newHHMM);
	}
}

void digitUpMM2()
{
	char currHH1Str[4];
	char newHHMM[6];
	int currHH1;
	int newHH1;
	int changed = 0;

	sprintf(currHH1Str, "%c",
			userPreferences.userAlarms[currentAlarmNum]->alarmHHMM[3]);
	currHH1 = atoi(currHH1Str);


	if (currHH1 >= 0) {
		//We can go up!
		currHH1--;
		if (currHH1 < 0) {
			newHH1 = 10 + currHH1;
		} else {
			newHH1 = currHH1;
		}

		sprintf(newHHMM, "%c%c%c%i",
				userPreferences.userAlarms[currentAlarmNum]->alarmHHMM[0],
				userPreferences.userAlarms[currentAlarmNum]->alarmHHMM[1],
				userPreferences.userAlarms[currentAlarmNum]->alarmHHMM[2],
				newHH1);
		//Flag to indicate we need a redraw
		changed = 1;
	}

	if (changed == 1) {
		//Redraw the new value
		animateAlarmScrollerDigit(screen, 3, currHH1 + 1, newHH1, 10,
								  &currentTheme.alarmDigitPos[3]);

		//Save the new time
		bzero(userPreferences.userAlarms[currentAlarmNum]->alarmHHMM,
			  sizeof(userPreferences.userAlarms[currentAlarmNum]->
					 alarmHHMM));
		sprintf(userPreferences.userAlarms[currentAlarmNum]->alarmHHMM,
				"%s", newHHMM);
	}

}

void digitDownHH1()
{
	char currHH1Str[4];
	char newHHMM[5];
	int currHH1;
	int currHH2;
	int newHH1;
	int changed = 0;

	sprintf(currHH1Str, "%c",
			userPreferences.userAlarms[currentAlarmNum]->alarmHHMM[0]);
	currHH1 = atoi(currHH1Str);

	//Check to see if we're in military time mode
	if (userPreferences.militaryTime == 0) {
		//In military time, the first HH can only be 1 or 0, so make sure we're okay

		if (currHH1 <= 1) {
			//Check to see if we need to roll the 2nd hh digit
			bzero(currHH1Str, sizeof(currHH1Str));
			sprintf(currHH1Str, "%c",
					userPreferences.userAlarms[currentAlarmNum]->
					alarmHHMM[1]);
			currHH2 = atoi(currHH1Str);

			if (currHH2 > 2) {
				//Roll to zero instead
				animateAlarmScrollerDigit(screen, 1, currHH2, 0,
										  (currHH2) * 10,
										  &currentTheme.alarmDigitPos[1]);
				currHH2 = 0;
			}


			//We're good to increment one!
			newHH1 = (currHH1 + 1) % 2;

			sprintf(newHHMM, "%i%i%c%c", newHH1, currHH2,
					userPreferences.userAlarms[currentAlarmNum]->
					alarmHHMM[2],
					userPreferences.userAlarms[currentAlarmNum]->
					alarmHHMM[3]);

			//Flag to indicate we need a redraw
			changed = 1;
		}
	} else {
		//In military time, so limit is 2... if 2nd hour is less than 4...
		bzero(currHH1Str, sizeof(currHH1Str));
		sprintf(currHH1Str, "%c",
				userPreferences.userAlarms[currentAlarmNum]->alarmHHMM[1]);

		if (currHH1 < 1 || (currHH1 <= 2 && atoi(currHH1Str) < 4)) {
			//We're good to increment one!
			newHH1 = (currHH1 + 1) % 3;

			sprintf(newHHMM, "%i%c%c%c", newHH1,
					userPreferences.userAlarms[currentAlarmNum]->
					alarmHHMM[1],
					userPreferences.userAlarms[currentAlarmNum]->
					alarmHHMM[2],
					userPreferences.userAlarms[currentAlarmNum]->
					alarmHHMM[3]);

			//Flag to indicate we need a redraw
			changed = 1;
		}

	}

	if (changed == 1) {
		//Redraw the new value
		animateAlarmScrollerDigit(screen, 0, currHH1, newHH1, 10,
								  &currentTheme.alarmDigitPos[0]);

		//Save the new time
		bzero(userPreferences.userAlarms[currentAlarmNum]->alarmHHMM,
			  sizeof(userPreferences.userAlarms[currentAlarmNum]->
					 alarmHHMM));
		sprintf(userPreferences.userAlarms[currentAlarmNum]->alarmHHMM,
				"%s", newHHMM);

	}

}

void digitDownHH2()
{
	char currHH1Str[4];
	char newHHMM[5];
	int currHH1, currHH2;
	int changed = 0;


	sprintf(currHH1Str, "%c",
			userPreferences.userAlarms[currentAlarmNum]->alarmHHMM[1]);
	currHH1 = atoi(currHH1Str);

	bzero(currHH1Str, sizeof(currHH1Str));
	sprintf(currHH1Str, "%c",
			userPreferences.userAlarms[currentAlarmNum]->alarmHHMM[0]);

	//Check to see if we're in military time mode
	if (userPreferences.militaryTime == 0) {
		//Okay, non military time is a bit tough...

		if (atoi(currHH1Str) < 1) {
			//We can go up to 9

			if (currHH1 <= 9) {
				//We're good to increment one!
				currHH2 = (currHH1 + 1) % 10;

				sprintf(newHHMM, "%c%i%c%c",
						userPreferences.userAlarms[currentAlarmNum]->
						alarmHHMM[0], currHH2,
						userPreferences.userAlarms[currentAlarmNum]->
						alarmHHMM[2],
						userPreferences.userAlarms[currentAlarmNum]->
						alarmHHMM[3]);

				//Flag to indicate we need a redraw
				changed = 1;
			}
		} else {
			//We can only go up to 2 now
			if (currHH1 <= 2) {
				//We're good to increment one!
				currHH2 = (currHH1 + 1) % 3;

				sprintf(newHHMM, "%c%i%c%c",
						userPreferences.userAlarms[currentAlarmNum]->
						alarmHHMM[0], currHH2,
						userPreferences.userAlarms[currentAlarmNum]->
						alarmHHMM[2],
						userPreferences.userAlarms[currentAlarmNum]->
						alarmHHMM[3]);

				//Flag to indicate we need a redraw
				changed = 1;
			}
		}
	} else {
		//In military time it's similar but not quite the same
		if (atoi(currHH1Str) < 2) {
			//We can go up to 9

			if (currHH1 <= 9) {
				//We're good to increment one!
				currHH2 = (currHH1 + 1) % 10;

				sprintf(newHHMM, "%c%i%c%c",
						userPreferences.userAlarms[currentAlarmNum]->
						alarmHHMM[0], currHH2,
						userPreferences.userAlarms[currentAlarmNum]->
						alarmHHMM[2],
						userPreferences.userAlarms[currentAlarmNum]->
						alarmHHMM[3]);

				//Flag to indicate we need a redraw
				changed = 1;
			}
		} else {
			//We can only go up to 3 now
			if (currHH1 <= 3) {
				//We're good to increment one!
				currHH2 = (currHH1 + 1) % 4;

				sprintf(newHHMM, "%c%i%c%c",
						userPreferences.userAlarms[currentAlarmNum]->
						alarmHHMM[0], currHH2,
						userPreferences.userAlarms[currentAlarmNum]->
						alarmHHMM[2],
						userPreferences.userAlarms[currentAlarmNum]->
						alarmHHMM[3]);

				//Flag to indicate we need a redraw
				changed = 1;
			}
		}

	}
	if (changed == 1) {
		//Redraw the new value
		animateAlarmScrollerDigit(screen, 1, currHH1, currHH2, 10,
								  &currentTheme.alarmDigitPos[1]);
		//Save the new time
		bzero(userPreferences.userAlarms[currentAlarmNum]->alarmHHMM,
			  sizeof(userPreferences.userAlarms[currentAlarmNum]->
					 alarmHHMM));
		sprintf(userPreferences.userAlarms[currentAlarmNum]->alarmHHMM,
				"%s", newHHMM);

	}

}

void digitDownMM1()
{
	char currHH1Str[4];
	char newHHMM[6];
	int currHH1;
	int changed = 0;

	sprintf(currHH1Str, "%c",
			userPreferences.userAlarms[currentAlarmNum]->alarmHHMM[2]);
	currHH1 = atoi(currHH1Str);

	if (currHH1 <= 5) {
		//We can go up!
		currHH1++;

		sprintf(newHHMM, "%c%c%i%c",
				userPreferences.userAlarms[currentAlarmNum]->alarmHHMM[0],
				userPreferences.userAlarms[currentAlarmNum]->alarmHHMM[1],
				currHH1 % 6,
				userPreferences.userAlarms[currentAlarmNum]->alarmHHMM[3]);
		//Flag to indicate we need a redraw
		changed = 1;
	}

	if (changed == 1) {
		//Redraw the new value
		animateAlarmScrollerDigit(screen, 2, currHH1 - 1, currHH1 % 6, 10,
								  &currentTheme.alarmDigitPos[2]);

		//Save the new time
		bzero(userPreferences.userAlarms[currentAlarmNum]->alarmHHMM,
			  sizeof(userPreferences.userAlarms[currentAlarmNum]->
					 alarmHHMM));
		sprintf(userPreferences.userAlarms[currentAlarmNum]->alarmHHMM,
				"%s", newHHMM);
	}

}

void digitDownMM2()
{
	char currHH1Str[4];
	char newHHMM[6];
	int currHH1;
	int changed = 0;

	sprintf(currHH1Str, "%c",
			userPreferences.userAlarms[currentAlarmNum]->alarmHHMM[3]);
	currHH1 = atoi(currHH1Str);
	if (currHH1 <= 9) {
		//We can go up!
		currHH1++;

		sprintf(newHHMM, "%c%c%c%i",
				userPreferences.userAlarms[currentAlarmNum]->alarmHHMM[0],
				userPreferences.userAlarms[currentAlarmNum]->alarmHHMM[1],
				userPreferences.userAlarms[currentAlarmNum]->alarmHHMM[2],
				currHH1 % 10);
		//Flag to indicate we need a redraw
		changed = 1;
	}

	if (changed == 1) {
		//Redraw the new value
		animateAlarmScrollerDigit(screen, 3, currHH1 - 1, currHH1 % 10, 10,
								  &currentTheme.alarmDigitPos[3]);

		//Save the new time
		bzero(userPreferences.userAlarms[currentAlarmNum]->alarmHHMM,
			  sizeof(userPreferences.userAlarms[currentAlarmNum]->
					 alarmHHMM));
		sprintf(userPreferences.userAlarms[currentAlarmNum]->alarmHHMM,
				"%s", newHHMM);
	}
}


//******** Done HELPER/Pseudo functions for alarm digit scrollers ************



/****************************************************************************
*
* drawAlarmDigits(SDL_Surface *screenBuff, char *alarmHHMM );
*
* Draw the given alarm time time on the alarm digit scrollers
****************************************************************************/

void drawAlarmDigits(SDL_Surface * screenBuff, char *alarmHHMM)
{

	int i;
	char thisDigit[5];

	//make sure the digit scroller exists for this theme
	if (elements[ALARMDIGITSCROLLER] != NULL) {
		for (i = 0; i < 4; i++) {
			bzero(thisDigit, sizeof(thisDigit));
			sprintf(thisDigit, "%c", alarmHHMM[i]);
			printf("agggggggg\n");
			//Loop over each digit in the time and draw it accordingly
			drawAlarmScrollerDigit(screenBuff, i, atoi(thisDigit),
								   &currentTheme.alarmDigitPos[i]);


		}

		//Update the display when done...
		SDL_UpdateRects(screenBuff, 4, currentTheme.alarmDigitPos);
	}
}

/****************************************************************************
*
* drawAlarmScrollerDigit(SDL_Surface *screenBuff, char *alarmHHMM );
*
* Draw the given alarm time time on the alarm digit scrollers
****************************************************************************/

void drawAlarmScrollerDigit(SDL_Surface * screenBuff, int digitIndex,
							int digitValue, SDL_Rect * digitPosRect)
{

	SDL_Rect sourceRect, targetRect;
	int digitIndexRef;

	Uint32 moodColour;
	moodColour =
		SDL_MapRGB(screenBuff->format, userPreferences.moodRed,
				   userPreferences.moodGreen, userPreferences.moodBlue);

	switch (digitIndex) {
	case 0:
		digitIndexRef = FIRSTHOUR;
		break;
	case 1:
		digitIndexRef = SECONDHOUR;
		break;
	case 2:
		digitIndexRef = FIRSTMINUTE;
		break;
	case 3:
		digitIndexRef = SECONDMINUTE;
		break;
	}

	sourceRect.x = 0;
	sourceRect.y =
		(elements[ALARMDIGITSCROLLER]->h - 207) - (digitValue * 99);
	sourceRect.w = elements[ALARMDIGITMASK]->w;
	sourceRect.h = elements[ALARMDIGITMASK]->h;

	//And off we go!

	//targetRect.x = currentTheme.alarmDigitPos[digitIndexRef].x;
	//targetRect.y = currentTheme.alarmDigitPos[digitIndexRef].y;
	targetRect.x = digitPosRect->x;
	targetRect.y = digitPosRect->y;

	targetRect.w = elements[ALARMDIGITSCROLLER]->w;
	targetRect.h = elements[ALARMDIGITMASK]->h;

	SDL_FillRect(screenBuff, &targetRect, moodColour);

	//SDL_BlitSurface(elements[ALARMDIGITSCROLLER], &sourceRect, screenBuff, &currentTheme.alarmDigitPos[digitIndexRef]);
	SDL_BlitSurface(elements[ALARMDIGITSCROLLER], &sourceRect, screenBuff,
					digitPosRect);


	//Draw the mask
	if (elements[ALARMDIGITMASK] != NULL) {
		//SDL_BlitSurface(elements[ALARMDIGITMASK], NULL, screenBuff, &currentTheme.alarmDigitPos[digitIndexRef]);
		SDL_BlitSurface(elements[ALARMDIGITMASK], NULL, screenBuff,
						digitPosRect);
	}

}

/****************************************************************************
*
* animateAlarmDigits(SDL_Surface *screenBuff, char *oldAlarmHHMM, *newAlarmHHMM );
*
* Draw the given alarm time time on the alarm digit scrollers
****************************************************************************/

void animateAlarmDigits(SDL_Surface * screenBuff, char *oldAlarmHHMM,
						char *newAlarmHHMM)
{

	int i;
	char thisOldDigit[5];
	char thisNewDigit[5];

	//make sure the digit scroller exists for this theme
	if (elements[ALARMDIGITSCROLLER] != NULL) {
		for (i = 0; i < 4; i++) {
			bzero(thisOldDigit, sizeof(thisOldDigit));
			sprintf(thisOldDigit, "%c", oldAlarmHHMM[i]);

			bzero(thisNewDigit, sizeof(thisNewDigit));
			sprintf(thisNewDigit, "%c", newAlarmHHMM[i]);

			//Loop over each digit in the time and draw it accordingly
			if (atoi(thisOldDigit) - atoi(thisNewDigit) > 0) {
				animateAlarmScrollerDigit(screenBuff, i,
										  atoi(thisOldDigit),
										  atoi(thisNewDigit),
										  (atoi(thisOldDigit) -
										   atoi(thisNewDigit)) * 10,
										  &currentTheme.alarmDigitPos[i]);
			} else if (atoi(thisOldDigit) - atoi(thisNewDigit) < 0) {
				animateAlarmScrollerDigit(screenBuff, i,
										  atoi(thisOldDigit),
										  atoi(thisNewDigit),
										  (atoi(thisNewDigit) -
										   atoi(thisOldDigit)) * 10,
										  &currentTheme.alarmDigitPos[i]);
			}
			//drawAlarmScrollerDigit(screenBuff, i, atoi(thisDigit));


		}

		//Update the display when done...
		SDL_UpdateRects(screenBuff, 4, currentTheme.alarmDigitPos);
	}
}

/****************************************************************************
*
* animateAlarmScrollerDigit(SDL_Surface *screenBuff, int digitIndex, int oldDigitValue, int newDigitValue, int frames );
*
* Animate a single alarm digit moving from the old value to the new value
****************************************************************************/

void animateAlarmScrollerDigit(SDL_Surface * screenBuff, int digitIndex,
							   int oldDigitValue, int newDigitValue,
							   int frames, SDL_Rect * digitPosRect)
{

	SDL_Rect sourceRect, sourceRect2;
	SDL_Surface *tempDigit, *tempBuffer;
	int digitIndexRef;
	float fadeVal;
	Uint32 backColour;
	Uint32 moodColour;

	Uint8 red, green, blue, alpha;	//Values of colour choosen
	Uint32 colour;

	int oldOffset;
	int newOffset;
	int aniOffset;
	int i;
	int halfFrames;

	switch (digitIndex) {
	case 0:
		digitIndexRef = FIRSTHOUR;
		break;
	case 1:
		digitIndexRef = SECONDHOUR;
		break;
	case 2:
		digitIndexRef = FIRSTMINUTE;
		break;
	case 3:
		digitIndexRef = SECONDMINUTE;
		break;
	}
	newOffset =
		(elements[ALARMDIGITSCROLLER]->h - 207) - (newDigitValue * 99);

	oldOffset =
		(elements[ALARMDIGITSCROLLER]->h - 207) - (oldDigitValue * 99);

	aniOffset = (newOffset - oldOffset) / frames;

	sourceRect.x = 0;
	sourceRect.y = oldOffset;
	sourceRect.w = elements[ALARMDIGITMASK]->w;
	sourceRect.h = elements[ALARMDIGITMASK]->h;


	moodColour =
		SDL_MapRGB(screenBuff->format, userPreferences.moodRed,
				   userPreferences.moodGreen, userPreferences.moodBlue);

	switch (currentTheme.alarmDigitTransitionMode) {
	case TRANSITIONSLIDE:
		for (i = 0; i <= frames; i++) {
			sourceRect.y = oldOffset + (aniOffset * i);
			if (i == frames) {
				sourceRect.y = newOffset;
			}
			//And off we go!
			//SDL_FillRect(screenBuff, &currentTheme.alarmDigitPos[digitIndexRef], moodColour);
			SDL_FillRect(screenBuff, digitPosRect, moodColour);


			//SDL_BlitSurface(elements[ALARMDIGITSCROLLER], &sourceRect, screenBuff, &currentTheme.alarmDigitPos[digitIndexRef]);
			SDL_BlitSurface(elements[ALARMDIGITSCROLLER], &sourceRect,
							screenBuff, digitPosRect);

			//Draw the mask
			//SDL_BlitSurface(elements[ALARMDIGITMASK], NULL, screenBuff, &currentTheme.alarmDigitPos[digitIndexRef]);
			SDL_BlitSurface(elements[ALARMDIGITMASK], NULL, screenBuff,
							digitPosRect);

			//SDL_UpdateRects(screenBuff, 1, &currentTheme.alarmDigitPos[digitIndexRef]);
			SDL_UpdateRects(screenBuff, 1, digitPosRect);


		}
		break;
	case TRANSITIONFADE:

		//Override frames since fades are different like that...
		if ((oldDigitValue - newDigitValue) > 1
			|| (oldDigitValue - newDigitValue) < -1) {
			frames = 10;
		}

		printf("animating digit %i %i over %i frames\n", oldDigitValue,
			   newDigitValue, frames);

		//Start by copying the current digit
		//sourceRect2.x = currentTheme.alarmDigitPos[digitIndexRef].x;
		//sourceRect2.y = currentTheme.alarmDigitPos[digitIndexRef].y;
		sourceRect2.x = digitPosRect->x;
		sourceRect2.y = digitPosRect->y;

		sourceRect2.w = elements[ALARMDIGITMASK]->w;
		sourceRect2.h = elements[ALARMDIGITMASK]->h;

		tempDigit = SDL_DisplayFormatAlpha(elements[ALARMDIGITMASK]);
		SDL_BlitSurface(screenBuff, &sourceRect2, tempDigit, NULL);

		//Get the RGB to use from teh area directly above this digit

		//colour = getPixel(screenBuff, currentTheme.alarmDigitPos[digitIndexRef].x, currentTheme.alarmDigitPos[digitIndexRef].y -2);
		colour =
			getPixel(screenBuff, digitPosRect->x, digitPosRect->y - 2);

		SDL_GetRGBA(colour, screenBuff->format, &red, &green, &blue,
					&alpha);




		halfFrames = frames / 2;

		for (i = 0; i <= halfFrames; i++) {
			fadeVal = (255 / halfFrames) * i;
			if (i == halfFrames) {
				fadeVal = 255;
			}
			tempBuffer = SDL_DisplayFormatAlpha(tempDigit);

			backColour =
				SDL_MapRGBA(tempBuffer->format, red, green, blue,
							(int) fadeVal);
			SDL_FillRect(tempBuffer, NULL, backColour);

			//Draw mood colour first
			//SDL_FillRect(screenBuff, &currentTheme.alarmDigitPos[digitIndexRef], moodColour);
			SDL_FillRect(screenBuff, digitPosRect, moodColour);


			SDL_BlitSurface(tempDigit, NULL, screenBuff, &sourceRect2);
			SDL_BlitSurface(tempBuffer, NULL, screenBuff, &sourceRect2);
			//Draw the mask
			//SDL_BlitSurface(elements[ALARMDIGITMASK], NULL, screenBuff, &currentTheme.alarmDigitPos[digitIndexRef]);
			SDL_BlitSurface(elements[ALARMDIGITMASK], NULL, screenBuff,
							digitPosRect);

			SDL_UpdateRects(screenBuff, 1, &sourceRect2);

			SDL_FreeSurface(tempBuffer);
			usleep(200);


		}
		usleep(100);

		//Make the new digit
		sourceRect.y = newOffset;



		//Fade in the new digit
		for (i = 0; i <= halfFrames; i++) {
			fadeVal = 255 - ((255 / halfFrames) * i);
			if (i == halfFrames) {
				fadeVal = 0;
			}

			tempBuffer = SDL_DisplayFormatAlpha(tempDigit);

			backColour =
				SDL_MapRGBA(tempBuffer->format, red, green, blue,
							(int) fadeVal);
			SDL_FillRect(tempBuffer, NULL, backColour);

			//And off we go!
			//Draw mood colour first
			//SDL_FillRect(screenBuff, &currentTheme.alarmDigitPos[digitIndexRef], moodColour);
			SDL_FillRect(screenBuff, digitPosRect, moodColour);

			//SDL_BlitSurface(elements[ALARMDIGITSCROLLER], &sourceRect, screenBuff, &currentTheme.alarmDigitPos[digitIndexRef]);
			SDL_BlitSurface(elements[ALARMDIGITSCROLLER], &sourceRect,
							screenBuff, digitPosRect);



			SDL_BlitSurface(tempBuffer, NULL, screenBuff, &sourceRect2);

			//Draw the mask
			//SDL_BlitSurface(elements[ALARMDIGITMASK], NULL, screenBuff, &currentTheme.alarmDigitPos[digitIndexRef]);
			SDL_BlitSurface(elements[ALARMDIGITMASK], NULL, screenBuff,
							digitPosRect);

			SDL_UpdateRects(screenBuff, 1, &sourceRect2);

			SDL_FreeSurface(tempBuffer);
			usleep(200);


		}

		SDL_FreeSurface(tempDigit);


		break;

	}							//End switch
}


/****************************************************************************
*
* updateAlarms() {
*
* Generic function update all alarms and sync them with AlarmD/config files
*
****************************************************************************/

void updateAlarms()
{
	int i = 0;

	if (userPreferences.alarmControlMode == ALARMCONTROLMODESIMPLE) {
		//In simple control mode every day is the same, so update all of the days to match this one
		for (i = 0; i < userPreferences.userAlarmsCount; i++) {
			if (userPreferences.militaryTime == 1) {
				//No ampm in military time!
				userPreferences.userAlarms[currentAlarmNum]->ampm = -1;
				userPreferences.userAlarms[i]->ampm = -1;
			}

			if (i == currentAlarmNum) {
				//don't update the basis alarm obviously...
				continue;
			}
			//copy alarm HHMM time
			bzero(userPreferences.userAlarms[i]->alarmHHMM,
				  sizeof(userPreferences.userAlarms[i]->alarmHHMM));
			strcpy(userPreferences.userAlarms[i]->alarmHHMM,
				   userPreferences.userAlarms[currentAlarmNum]->alarmHHMM);

			//Copy amPM setting
			userPreferences.userAlarms[i]->ampm =
				userPreferences.userAlarms[currentAlarmNum]->ampm;

			//Copy enabled setting
			userPreferences.userAlarms[i]->enabled =
				userPreferences.userAlarms[currentAlarmNum]->enabled;

			//Copy alarm sound
			if (userPreferences.userAlarms[i]->sound != NULL) {
				free(userPreferences.userAlarms[i]->sound);
			}
			userPreferences.userAlarms[i]->sound =
				calloc(strlen
					   (userPreferences.userAlarms[currentAlarmNum]->
						sound) + 5, sizeof(char));
			strcpy(userPreferences.userAlarms[i]->sound,
				   userPreferences.userAlarms[currentAlarmNum]->sound);

			//copy alert mode
			userPreferences.userAlarms[i]->alertMode =
				userPreferences.userAlarms[currentAlarmNum]->alertMode;

			//copy loop sound
			userPreferences.userAlarms[i]->loopSound =
				userPreferences.userAlarms[currentAlarmNum]->loopSound;

			//Copy snooze
			userPreferences.userAlarms[i]->snoozeTime =
				userPreferences.userAlarms[currentAlarmNum]->snoozeTime;



		}



	} else {
		if (userPreferences.militaryTime == 1) {
			for (i = 0; i < userPreferences.userAlarmsCount; i++) {
				userPreferences.userAlarms[i]->ampm = -1;
			}
		}
	}

	//Update the next alarm time
	getNextAlarmTime();

	//Save the updated config
	setUserPreferences();

#if defined(LIBOSSO_H_)
	//Call the alarmD updater if we're on the tablet
	ossoUpdateAlarms();

#endif
}


/****************************************************************************
*
* selectAlarmDay(int alarmDay) {
*
* Generic function to select a specific alarm day; pseudo functions for buttons
* follow this.
*
****************************************************************************/

void selectAlarmDay(int alarmDay)
{
	int oldAlarmNum;


	if (userPreferences.alarmControlMode == ALARMCONTROLMODEWEEKLY) {

		//Lots of stuff should go here


		oldAlarmNum = currentAlarmNum;
		currentAlarmNum = getFirstAlarmIndexByDay(alarmDay);

		//***** Update the alarm settings and activate alarm if required **********//
		//Not sure if I want to do this here, or later on when you leave the alarm settings screen...
		//or maybe even in a separate process/thread in teh background...
		// but let's try here first and see how much it slows things down

		//nope, should go somewhere where the config file won't be over-written too soon!

		//**** done actual alarm settings update ****************************//



		//**** Redraw the alarm setting screen to show the new alarm info ****//    
		drawAllAlarmDays(screen);

		//Update the alert settings rect
		//Redraw the background for the alert settings rect
		drawThemeBackground(screen, CLOCKMODEALARMSETTINGS,
							&currentTheme.alertSettingsRect);

		//Update the labels...
		drawAllLabels(screen, CLOCKMODEALARMSETTINGS, 1);

		//animate the digits to adjust them to the new value
		animateAlarmDigits(screen,
						   userPreferences.userAlarms[oldAlarmNum]->
						   alarmHHMM,
						   userPreferences.userAlarms[currentAlarmNum]->
						   alarmHHMM);

		//**** Done redrawing alarm settings screen ***//
	}

}

//*** Pseudo functions ***//
void selectSunday()
{
	selectAlarmDay(0);
}

void selectMonday()
{
	selectAlarmDay(1);
}

void selectTuesday()
{
	selectAlarmDay(2);
}

void selectWednesday()
{
	selectAlarmDay(3);
}

void selectThursday()
{
	selectAlarmDay(4);
}

void selectFriday()
{
	selectAlarmDay(5);
}

void selectSaturday()
{
	selectAlarmDay(6);
}


/****************************************************************************
*
* setupAlarmDayButtons() {
*
* Function to create the alarmDay buttons for the current theme. This function
* is called when a theme changes.
*
****************************************************************************/

void setupAlarmDayButtons()
{
	int i = 0;

	for (i = 0; i < 7; i++) {

		//Expand the theme-oriented normal mode buttons array to have space for our new button
		themeButtonActions[CLOCKMODEALARMSETTINGS].buttonActions =
			(struct mouseAction **)
			realloc(themeButtonActions[CLOCKMODEALARMSETTINGS].
					buttonActions,
					(themeButtonActions[CLOCKMODEALARMSETTINGS].
					 buttonCount + 1) * sizeof(struct mouseAction *));

		/// allocate memory for one `mouseAction` 
		themeButtonActions[CLOCKMODEALARMSETTINGS].
			buttonActions[themeButtonActions[CLOCKMODEALARMSETTINGS].
						  buttonCount] =
			(struct mouseAction *) malloc(sizeof(struct mouseAction));

		//Initialize the button entry
		initButton(themeButtonActions[CLOCKMODEALARMSETTINGS].
				   buttonActions[themeButtonActions
								 [CLOCKMODEALARMSETTINGS].buttonCount]);

		//Buttons are click type
		themeButtonActions[CLOCKMODEALARMSETTINGS].
			buttonActions[themeButtonActions[CLOCKMODEALARMSETTINGS].
						  buttonCount]->buttonMode = CLICK;

		//Setup the button position
		themeButtonActions[CLOCKMODEALARMSETTINGS].
			buttonActions[themeButtonActions[CLOCKMODEALARMSETTINGS].
						  buttonCount]->buttonRect.w =
			elements[ALARMDAYON]->w;
		themeButtonActions[CLOCKMODEALARMSETTINGS].
			buttonActions[themeButtonActions[CLOCKMODEALARMSETTINGS].
						  buttonCount]->buttonRect.h =
			elements[ALARMDAYON]->h;

		//check to see if we're using veritcal or horizontal layout
		if (currentTheme.alarmDayRect.w > currentTheme.alarmDayRect.h) {
			themeButtonActions[CLOCKMODEALARMSETTINGS].
				buttonActions[themeButtonActions[CLOCKMODEALARMSETTINGS].
							  buttonCount]->buttonRect.y =
				currentTheme.alarmDayRect.y;
			themeButtonActions[CLOCKMODEALARMSETTINGS].
				buttonActions[themeButtonActions[CLOCKMODEALARMSETTINGS].
							  buttonCount]->buttonRect.x =
				currentTheme.alarmDayRect.x +
				(currentTheme.alarmDayRect.w / 7) * i;
		} else {
			themeButtonActions[CLOCKMODEALARMSETTINGS].
				buttonActions[themeButtonActions[CLOCKMODEALARMSETTINGS].
							  buttonCount]->buttonRect.x =
				currentTheme.alarmDayRect.x;
			themeButtonActions[CLOCKMODEALARMSETTINGS].
				buttonActions[themeButtonActions[CLOCKMODEALARMSETTINGS].
							  buttonCount]->buttonRect.y =
				currentTheme.alarmDayRect.y +
				(currentTheme.alarmDayRect.h / 7) * i;
		}

		switch (i) {
		case 0:
			themeButtonActions[CLOCKMODEALARMSETTINGS].
				buttonActions[themeButtonActions[CLOCKMODEALARMSETTINGS].
							  buttonCount]->buttonAction = &selectSunday;
			break;
		case 1:
			themeButtonActions[CLOCKMODEALARMSETTINGS].
				buttonActions[themeButtonActions[CLOCKMODEALARMSETTINGS].
							  buttonCount]->buttonAction = &selectMonday;
			break;
		case 2:
			themeButtonActions[CLOCKMODEALARMSETTINGS].
				buttonActions[themeButtonActions[CLOCKMODEALARMSETTINGS].
							  buttonCount]->buttonAction = &selectTuesday;
			break;
		case 3:
			themeButtonActions[CLOCKMODEALARMSETTINGS].
				buttonActions[themeButtonActions[CLOCKMODEALARMSETTINGS].
							  buttonCount]->buttonAction =
				&selectWednesday;
			break;
		case 4:
			themeButtonActions[CLOCKMODEALARMSETTINGS].
				buttonActions[themeButtonActions[CLOCKMODEALARMSETTINGS].
							  buttonCount]->buttonAction = &selectThursday;
			break;
		case 5:
			themeButtonActions[CLOCKMODEALARMSETTINGS].
				buttonActions[themeButtonActions[CLOCKMODEALARMSETTINGS].
							  buttonCount]->buttonAction = &selectFriday;
			break;
		case 6:
			themeButtonActions[CLOCKMODEALARMSETTINGS].
				buttonActions[themeButtonActions[CLOCKMODEALARMSETTINGS].
							  buttonCount]->buttonAction = &selectSaturday;
			break;
		}


		//*** Finally enable the button (the whole enabled thing seems pretty stupid, but maybe handy later?)
		themeButtonActions[CLOCKMODEALARMSETTINGS].
			buttonActions[themeButtonActions[CLOCKMODEALARMSETTINGS].
						  buttonCount]->enabled = 1;


		//Increment the counter
		themeButtonActions[CLOCKMODEALARMSETTINGS].buttonCount++;


	}
}



/****************************************************************************
*
* drawAllAlarmDays(SDL_Surface *screenBuff);
*
* Draw all of the alarm days initially (or after refresh?)
****************************************************************************/

void drawAllAlarmDays(SDL_Surface * screenBuff)
{
	int i = 0;

	//Only draw alarm day pickers if control mode is Weekly
	if (userPreferences.alarmControlMode == ALARMCONTROLMODEWEEKLY) {

		//Draw the background again first
		drawThemeBackground(screenBuff, CLOCKMODEALARMSETTINGS,
							&currentTheme.alarmDayRect);

		for (i = 0; i < 7; i++) {
			drawAlarmDayPicker(screenBuff, i);
		}

		SDL_UpdateRects(screenBuff, 1, &currentTheme.alarmDayRect);
	}

}


/****************************************************************************
*
* drawAlarmDayPicker(SDL_Surface *screenBuff, int alarmDay );
*
* Draw one of the alarm day pickers. The day pickers are spread evenly accross the
* bounding box defined in the theme file; if the bounding box is wider than high the days are
* laid out horizontally; otherwise they're drawn vertically.
****************************************************************************/

void drawAlarmDayPicker(SDL_Surface * screenBuff, int alarmDay)
{
	int horizontalLayout = 0;
	int thisAlarmIndex = 0;
	SDL_Rect targetPos, boxPos;

	SDL_Surface *textSurface;	//Surface for rendered text
	SDL_Color textColor;		//Colour of text to render
	Uint32 colorTest;			//Mood color
	TTF_Font *font;				//Font to use


	//check to see if we're using veritcal or horizontal layout
	if (currentTheme.alarmDayRect.w > currentTheme.alarmDayRect.h) {
		horizontalLayout = 1;
	} else {
		horizontalLayout = 0;
	}

	//Setup the target position for this day picker
	targetPos.w = currentTheme.alarmDayRect.w;
	targetPos.h = currentTheme.alarmDayRect.h;

	if (horizontalLayout) {
		targetPos.y = currentTheme.alarmDayRect.y;
		targetPos.x =
			currentTheme.alarmDayRect.x +
			(currentTheme.alarmDayRect.w / 7) * alarmDay;

	} else {
		targetPos.x = currentTheme.alarmDayRect.x;
		targetPos.y =
			currentTheme.alarmDayRect.y +
			(currentTheme.alarmDayRect.h / 7) * alarmDay;
	}



	//First check to see if the given day matches the currently selected one
	if (alarmDay == userPreferences.userAlarms[currentAlarmNum]->alarmDay) {


		//Draw the selected day box if present
		if (elements[ALARMDAYSELECTOR] != NULL) {
			//Draw mood color first
			if (currentTheme.alarmDayUseMood == 1) {
				//Draw mood box
				boxPos.x = targetPos.x;
				boxPos.y = targetPos.y;
				boxPos.w = elements[ALARMDAYSELECTOR]->w;
				boxPos.h = elements[ALARMDAYSELECTOR]->h;
				colorTest =
					SDL_MapRGBA(screenBuff->format,
								userPreferences.moodRed,
								userPreferences.moodGreen,
								userPreferences.moodBlue, 255);

				SDL_FillRect(screenBuff, &boxPos, colorTest);
			}


			SDL_BlitSurface(elements[ALARMDAYSELECTOR], NULL, screenBuff,
							&targetPos);
		}
	}
	//Now check to see if the alarm is on or off
	thisAlarmIndex = getFirstAlarmIndexByDay(alarmDay);
	if (userPreferences.userAlarms[thisAlarmIndex]->enabled == 1) {
		//Alarm is on
		SDL_BlitSurface(elements[ALARMDAYON], NULL, screenBuff,
						&targetPos);
	} else {
		//alarm is off
		SDL_BlitSurface(elements[ALARMDAYOFF], NULL, screenBuff,
						&targetPos);
	}

	//finally, draw the name of the day of the week
	if (currentTheme.alarmDayTextUseMood == 0) {
		//No mood colour, so use our colours!
		textColor.r = currentTheme.alarmDayTextColor[0];
		textColor.g = currentTheme.alarmDayTextColor[1];
		textColor.b = currentTheme.alarmDayTextColor[2];
	} else {
		textColor.r = userPreferences.moodRed;
		textColor.g = userPreferences.moodGreen;
		textColor.b = userPreferences.moodBlue;
	}


	font = currentTheme.themeFonts[currentTheme.alarmDayTextSize];


	//Okay, now off we go!
	textSurface = NULL;
	textSurface =
		TTF_RenderUTF8_Blended(font, dayShortNames[alarmDay], textColor);

	targetPos.x += currentTheme.alarmDayTextOffset.x;
	targetPos.y += currentTheme.alarmDayTextOffset.y;

	SDL_BlitSurface(textSurface, NULL, screenBuff, &targetPos);

	if (textSurface != NULL) {
		SDL_FreeSurface(textSurface);
	}


}

/****************************************************************************
*
* toggleCurrentAlarm();
*
* Function to enable or disable the current alarm
****************************************************************************/

void toggleCurrentAlarm()
{
	SDL_Rect targetPos;
	SDL_Rect labelRects[2];
	int labelCount = 0;;
	int q;

	userPreferences.userAlarms[currentAlarmNum]->enabled =
		!userPreferences.userAlarms[currentAlarmNum]->enabled;


	if (userPreferences.alarmControlMode != ALARMCONTROLMODESIMPLE) {
		//Setup the target position for this day picker
		targetPos.w = elements[ALARMDAYOFF]->w;
		targetPos.h = elements[ALARMDAYOFF]->h;

		if (currentTheme.alarmDayRect.w > currentTheme.alarmDayRect.h) {
			targetPos.y = currentTheme.alarmDayRect.y;
			targetPos.x =
				currentTheme.alarmDayRect.x +
				(currentTheme.alarmDayRect.w / 7) *
				userPreferences.userAlarms[currentAlarmNum]->alarmDay;

		} else {
			targetPos.x = currentTheme.alarmDayRect.x;
			targetPos.y =
				currentTheme.alarmDayRect.y +
				(currentTheme.alarmDayRect.h / 7) *
				userPreferences.userAlarms[currentAlarmNum]->alarmDay;
		}


		//Update the day picker
		drawAlarmDayPicker(screen,
						   userPreferences.userAlarms[currentAlarmNum]->
						   alarmDay);

		labelRects[labelCount] = targetPos;
		labelCount++;
	}
	//Search for the ON/OFF button and redraw it only
	if (currentTheme.imageLabels[CLOCKMODEALARMSETTINGS].labelCount > 0) {
		for (q = 0;
			 q <
			 currentTheme.imageLabels[CLOCKMODEALARMSETTINGS].labelCount;
			 q++) {
			if (!strcmp
				(currentTheme.imageLabels[CLOCKMODEALARMSETTINGS].
				 labelArr[q]->labelKey, "alarmOnOff")) {
				drawImageLabel(screen,
							   currentTheme.
							   imageLabels[CLOCKMODEALARMSETTINGS].
							   labelArr[q], CLOCKMODEALARMSETTINGS, NULL);

				labelRects[labelCount] =
					currentTheme.imageLabels[CLOCKMODEALARMSETTINGS].
					labelArr[q]->labelRect;
				labelCount++;

				break;
			}
		}
	}

	SDL_UpdateRects(screen, labelCount, labelRects);


}

/****************************************************************************
*
* toggleAlarmAMPM();
*
* Function to toggle the current alarm AM or PM setting (in 12 hour mode)
****************************************************************************/

void toggleAlarmAMPM()
{
	int q;

	if (userPreferences.militaryTime == 0) {

		userPreferences.userAlarms[currentAlarmNum]->ampm =
			!userPreferences.userAlarms[currentAlarmNum]->ampm;

		//Search for the AM/PM button and redraw it only
		if (currentTheme.imageLabels[CLOCKMODEALARMSETTINGS].labelCount >
			0) {
			for (q = 0;
				 q <
				 currentTheme.imageLabels[CLOCKMODEALARMSETTINGS].
				 labelCount; q++) {
				if (!strcmp
					(currentTheme.imageLabels[CLOCKMODEALARMSETTINGS].
					 labelArr[q]->labelKey, "alarmAMPM")) {
					drawImageLabel(screen,
								   currentTheme.
								   imageLabels[CLOCKMODEALARMSETTINGS].
								   labelArr[q], CLOCKMODEALARMSETTINGS,
								   NULL);

					SDL_UpdateRects(screen, 1,
									&currentTheme.
									imageLabels[CLOCKMODEALARMSETTINGS].
									labelArr[q]->labelRect);

					break;
				}
			}
		}

	}
}


/****************************************************************************
*
* toggleMilitaryTime();
*
* Function to toggle the current alarm AM or PM setting (in 12 hour mode)
****************************************************************************/

void toggleMilitaryTime()
{
	int q;
	int alarmHH = 0;
	int alarmMM = 0;
	char timeBlock[6];

	userPreferences.militaryTime = !userPreferences.militaryTime;

	//We need to change the alarm times to reflect the new setting
	for (q = 0; q < userPreferences.userAlarmsCount; q++) {
		memset(&timeBlock, 0, sizeof(timeBlock));
		sprintf(timeBlock, "%c%c",
				userPreferences.userAlarms[q]->alarmHHMM[0],
				userPreferences.userAlarms[q]->alarmHHMM[1]);
		alarmHH = atoi(timeBlock);

		memset(&timeBlock, 0, sizeof(timeBlock));
		sprintf(timeBlock, "%c%c",
				userPreferences.userAlarms[q]->alarmHHMM[2],
				userPreferences.userAlarms[q]->alarmHHMM[3]);
		alarmMM = atoi(timeBlock);

		if (userPreferences.militaryTime == 0) {
			//Going from 24 hour to 12 hour mode, so update all alarms to match
			if (alarmHH == 0) {
				//00 in 24 hour mode is 12 AM
				alarmHH = 12;


				memset(&userPreferences.userAlarms[q]->alarmHHMM, 0,
					   sizeof(userPreferences.userAlarms[q]->alarmHHMM));
				sprintf(userPreferences.userAlarms[q]->alarmHHMM,
						"%02i%02i", alarmHH, alarmMM);
				userPreferences.userAlarms[q]->ampm = 0;
			} else if (alarmHH == 12) {
				//12 in 24hr mode is 12 pm
				userPreferences.userAlarms[q]->ampm = 1;

			} else if (alarmHH > 12) {
				//Can't be greater than 12 anymore
				alarmHH = alarmHH - 12;

				memset(&userPreferences.userAlarms[q]->alarmHHMM, 0,
					   sizeof(userPreferences.userAlarms[q]->alarmHHMM));
				sprintf(userPreferences.userAlarms[q]->alarmHHMM,
						"%02i%02i", alarmHH, alarmMM);
				userPreferences.userAlarms[q]->ampm = 1;
			} else {
				//Less than 12 so must be AM
				userPreferences.userAlarms[q]->ampm = 0;
			}

		} else if (userPreferences.militaryTime == 1) {
			//Going from 12 hour to 24 hour mode, so update all alarms to match
			if (alarmHH == 12 && userPreferences.userAlarms[q]->ampm == 0) {
				//12 AM in 12hr mode is 00
				alarmHH = 0;

				memset(&userPreferences.userAlarms[q]->alarmHHMM, 0,
					   sizeof(userPreferences.userAlarms[q]->alarmHHMM));
				sprintf(userPreferences.userAlarms[q]->alarmHHMM,
						"%02i%02i", alarmHH, alarmMM);
				printf("setting alarm to Midnight (00hr))\n\n");
			} else if (userPreferences.userAlarms[q]->ampm == 1
					   && alarmHH != 12) {
				alarmHH = alarmHH + 12;

				memset(&userPreferences.userAlarms[q]->alarmHHMM, 0,
					   sizeof(userPreferences.userAlarms[q]->alarmHHMM));
				sprintf(userPreferences.userAlarms[q]->alarmHHMM,
						"%02i%02i", alarmHH, alarmMM);

			}
			userPreferences.userAlarms[q]->ampm = -1;

		}


	}


	//Search for the Military time button and redraw it only
	if (currentTheme.imageLabels[CLOCKMODECLOCKSETTINGS].labelCount > 0) {
		for (q = 0;
			 q <
			 currentTheme.imageLabels[CLOCKMODECLOCKSETTINGS].labelCount;
			 q++) {
			if (!strcmp
				(currentTheme.imageLabels[CLOCKMODECLOCKSETTINGS].
				 labelArr[q]->labelKey, "militaryTime")) {
				drawImageLabel(screen,
							   currentTheme.
							   imageLabels[CLOCKMODECLOCKSETTINGS].
							   labelArr[q], CLOCKMODECLOCKSETTINGS, NULL);

				SDL_UpdateRects(screen, 1,
								&currentTheme.
								imageLabels[CLOCKMODECLOCKSETTINGS].
								labelArr[q]->labelRect);

				break;
			}
		}
	}
}

/****************************************************************************
*
* toggleAlarmStyle();
*
* Function to toggle the alarm setting style (simple or normal)
****************************************************************************/

void toggleAlarmStyle()
{
	int q;

	userPreferences.alarmControlMode++;
	if (userPreferences.alarmControlMode == ALARMCONTROLMODEMAX) {
		userPreferences.alarmControlMode = 0;
	}


	//Search for the AM/PM button and redraw it only
	if (currentTheme.imageLabels[CLOCKMODECLOCKSETTINGS].labelCount > 0) {
		for (q = 0;
			 q <
			 currentTheme.imageLabels[CLOCKMODECLOCKSETTINGS].labelCount;
			 q++) {
			if (!strcmp
				(currentTheme.imageLabels[CLOCKMODECLOCKSETTINGS].
				 labelArr[q]->labelKey, "alarmStyle")) {
				drawImageLabel(screen,
							   currentTheme.
							   imageLabels[CLOCKMODECLOCKSETTINGS].
							   labelArr[q], CLOCKMODECLOCKSETTINGS, NULL);

				SDL_UpdateRects(screen, 1,
								&currentTheme.
								imageLabels[CLOCKMODECLOCKSETTINGS].
								labelArr[q]->labelRect);

				break;
			}
		}
	}

}


/****************************************************************************
*
* toggleInsomniacModeButton();
*
* Function to toggle insomniac mode
****************************************************************************/

void toggleInsomniacModeButton()
{
	int q;

	if (isTablet) {
		setInsomniacMode(-1);

		if (clockMode == CLOCKMODECLOCKSETTINGS) {
			//Search for the Insomniac button and redraw it only
			if (currentTheme.imageLabels[CLOCKMODECLOCKSETTINGS].
				labelCount > 0) {
				for (q = 0;
					 q <
					 currentTheme.imageLabels[CLOCKMODECLOCKSETTINGS].
					 labelCount; q++) {
					if (!strcmp
						(currentTheme.imageLabels[CLOCKMODECLOCKSETTINGS].
						 labelArr[q]->labelKey, "insomniacMode")) {
						drawImageLabel(screen,
									   currentTheme.
									   imageLabels[CLOCKMODECLOCKSETTINGS].
									   labelArr[q], CLOCKMODECLOCKSETTINGS,
									   NULL);

						SDL_UpdateRects(screen, 1,
										&currentTheme.
										imageLabels
										[CLOCKMODECLOCKSETTINGS].
										labelArr[q]->labelRect);

						break;
					}
				}
			}
		}
	}

}


/****************************************************************************
*
* toggleSecondsVisible();
*
* Function to toggle whether seconds are shown or not
****************************************************************************/

void toggleSecondsVisible()
{
	int q;

	userPreferences.showSeconds = !userPreferences.showSeconds;


	if (clockMode == CLOCKMODECLOCKSETTINGS) {
		//Search for the Insomniac button and redraw it only
		if (currentTheme.imageLabels[CLOCKMODECLOCKSETTINGS].labelCount >
			0) {
			for (q = 0;
				 q <
				 currentTheme.imageLabels[CLOCKMODECLOCKSETTINGS].
				 labelCount; q++) {
				if (!strcmp
					(currentTheme.imageLabels[CLOCKMODECLOCKSETTINGS].
					 labelArr[q]->labelKey, "secondsVisible")) {
					drawImageLabel(screen,
								   currentTheme.
								   imageLabels[CLOCKMODECLOCKSETTINGS].
								   labelArr[q], CLOCKMODECLOCKSETTINGS,
								   NULL);

					SDL_UpdateRects(screen, 1,
									&currentTheme.
									imageLabels[CLOCKMODECLOCKSETTINGS].
									labelArr[q]->labelRect);

					break;
				}
			}
		}
	}


}

/****************************************************************************
*
* toggleInsomniacLock();
*
* Function to toggle whether insomniac mode is screen dim locked or not
****************************************************************************/

void toggleInsomniacLock()
{
	int q;

	userPreferences.insomniacLocked = !userPreferences.insomniacLocked;


	if (clockMode == CLOCKMODECLOCKSETTINGS) {
		//Search for the Insomniac button and redraw it only
		if (currentTheme.imageLabels[CLOCKMODECLOCKSETTINGS].labelCount >
			0) {
			for (q = 0;
				 q <
				 currentTheme.imageLabels[CLOCKMODECLOCKSETTINGS].
				 labelCount; q++) {
				if (!strcmp
					(currentTheme.imageLabels[CLOCKMODECLOCKSETTINGS].
					 labelArr[q]->labelKey, "insomniacLock")) {
					drawImageLabel(screen,
								   currentTheme.
								   imageLabels[CLOCKMODECLOCKSETTINGS].
								   labelArr[q], CLOCKMODECLOCKSETTINGS,
								   NULL);

					SDL_UpdateRects(screen, 1,
									&currentTheme.
									imageLabels[CLOCKMODECLOCKSETTINGS].
									labelArr[q]->labelRect);

					break;
				}
			}
		}
	}


}


/****************************************************************************
*
* cycleAlarmMode();
*
* Function to cycle through possible alarm alert modes
****************************************************************************/

void cycleAlarmMode()
{


	userPreferences.userAlarms[currentAlarmNum]->alertMode += 1;
	if (hasFMRadio == 0
		&& userPreferences.userAlarms[currentAlarmNum]->alertMode ==
		ALARMALERTMODEFMRADIO) {
		//Device doesn't have an FM radio, so skip
		userPreferences.userAlarms[currentAlarmNum]->alertMode += 1;
	}

	if (userPreferences.userAlarms[currentAlarmNum]->alertMode ==
		ALARMALERTMODEMAX) {
		userPreferences.userAlarms[currentAlarmNum]->alertMode = 0;
	}
	//Redraw the background for the alert settings rect
	drawThemeBackground(screen, CLOCKMODEALARMSETTINGS,
						&currentTheme.alertSettingsRect);


	//Update all the labels since some may depend on the alert mode...
	drawAllLabels(screen, CLOCKMODEALARMSETTINGS, 1);

}

/****************************************************************************
*
* cycleSnoozeTime();
*
* Function to cycle the snooze time (5 min increments)

REDUNDANT AND NOT USED ANYMORE... SLIDER IS BETTER (see below)
**************************************************************************

void cycleSnoozeTime() {
	int q;
	
	if (userPreferences.userAlarms[currentAlarmNum]->snoozeTime < 15) {
		userPreferences.userAlarms[currentAlarmNum]->snoozeTime += 5;
	} else {
		userPreferences.userAlarms[currentAlarmNum]->snoozeTime = 0;
	}

	//Search for the snooze time and redraw it only
	if (currentTheme.imageLabels[CLOCKMODEALARMSETTINGS].labelCount > 0) {
		for (q =0; q < currentTheme.imageLabels[CLOCKMODEALARMSETTINGS].labelCount; q++) {
			if (!strcmp(currentTheme.imageLabels[CLOCKMODEALARMSETTINGS].labelArr[q]->labelKey, "alarmSnoozeTime")) {			
				drawImageLabel(screen, currentTheme.imageLabels[CLOCKMODEALARMSETTINGS].labelArr[q], CLOCKMODEALARMSETTINGS, NULL);
				
				SDL_UpdateRects(screen, 1, &currentTheme.imageLabels[CLOCKMODEALARMSETTINGS].labelArr[q]->labelRect);
				
				break;
			}
		}
	}

}
**/

/****************************************************************************
*
* openSnoozeTimeSlider()
*
* Function to open the slider interface to choose snooze time
*
****************************************************************************/

void openSnoozeTimeSlider()
{
	openSlider("Snooze Time (mins)",
			   &userPreferences.userAlarms[currentAlarmNum]->snoozeTime, 0,
			   60);

}


/****************************************************************************
*
* toggleAlarmSoundLoop();
*
* Function to toggle the current alarm loop sound
****************************************************************************/

void toggleAlarmSoundLoop()
{
	int q;

	userPreferences.userAlarms[currentAlarmNum]->loopSound =
		!userPreferences.userAlarms[currentAlarmNum]->loopSound;

	//Search for the AM/PM button and redraw it only
	if (currentTheme.imageLabels[CLOCKMODEALARMSETTINGS].labelCount > 0) {
		for (q = 0;
			 q <
			 currentTheme.imageLabels[CLOCKMODEALARMSETTINGS].labelCount;
			 q++) {
			if (!strcmp
				(currentTheme.imageLabels[CLOCKMODEALARMSETTINGS].
				 labelArr[q]->labelKey, "alarmOnceLoop")) {
				drawImageLabel(screen,
							   currentTheme.
							   imageLabels[CLOCKMODEALARMSETTINGS].
							   labelArr[q], CLOCKMODEALARMSETTINGS, NULL);

				SDL_UpdateRects(screen, 1,
								&currentTheme.
								imageLabels[CLOCKMODEALARMSETTINGS].
								labelArr[q]->labelRect);

				break;
			}
		}
	}

}



void borked_fselector()
{
	GtkWidget *dialog;
	dialog = gtk_file_chooser_dialog_new("Open File",
										 NULL,
										 GTK_FILE_CHOOSER_ACTION_OPEN,
										 GTK_STOCK_CANCEL,
										 GTK_RESPONSE_CANCEL,
										 GTK_STOCK_OPEN,
										 GTK_RESPONSE_ACCEPT, NULL);

debugEntry("Running dialog..\n");
	gtk_dialog_run(GTK_DIALOG(dialog)); 
debugEntry ("Ended dialog..\n");

}


#ifdef USE_FLTK_FILE_CHOOSER_METHOD
void ChooseFile()
{
	// Create the file chooser, and show it
	Fl_File_Chooser chooser(".",	// directory
							"*",	// filter
							Fl_File_Chooser::MULTI,	// chooser type
							"Title Of Chooser");	// title
	chooser.show();

	// Block until user picks something.
	//     (The other way to do this is to use a callback())
	//
	while (chooser.shown())
		Fl::wait();

	// User hit cancel?
	if (chooser.value() == NULL) {
		cout << "(User hit 'Cancel')\n";
		return;
	}
	// Print what the user picked
	cout << "--------------------\n";
	cout << "DIRECTORY: " << chooser.directory() << endl;
	cout << "    VALUE: " << chooser.value() << endl;
	cout << "    COUNT: " << chooser.count() << " files selected\n";

	// Multiple files? Show all of them
	if (chooser.count() > 1) {
		for (int t = 1; t <= chooser.count(); t++)
			cout << " VALUE[" << t << "]: " << chooser.value(t) << endl;
	}
	// Hide the File Chooser
	chooser.hide();
	Fl::flush();
}

#endif

/****************************************************************************
*
* pickAlarmSoundFile();
*
* Function to choose an alarm sound file from the filesystem; uses GTK file browser,
* based on http://www.gnu.org/software/guile-gnome/docs/gtk/html/GtkFileChooserDialog.html
****************************************************************************/

void pickAlarmSoundFile()
{
	int q;
	GtkWidget *dialog;
	GtkFileFilter *filter;
	guint response;
	GtkWindow *window;
	char *filename;

	debugEntry("pickAlarmSoundFile() start\n");
	window = (GtkWindow *)gtk_window_new(GTK_WINDOW_POPUP);

	if (userPreferences.userAlarms[currentAlarmNum]->alertMode ==
		ALARMALERTMODESOUND
		|| userPreferences.userAlarms[currentAlarmNum]->alertMode ==
		ALARMALERTMODESOUNDFOLDER) {
		//Only open it if we're in sound mode

		debugEntry("== > a");

		//temporarily disable insomniac mode (it'll auto start again once file manager closes)
		if (isTablet) {
			if (userPreferences.insomniacModeOn == 1 && insomniacMode == 2) {
				if (insomniacTO > 0) {
					g_source_remove(insomniacTO);
					insomniacTO = 0;
					debugEntry("== > b");
				}
			}
		}

		debugEntry("== > c");

		//Setup the file filter
		filter = gtk_file_filter_new();
		gtk_file_filter_add_pattern(filter, "*.mp3");
		debugEntry("== > d");
		gtk_file_filter_set_name(filter, "Mp3 Files");

		debugEntry("== > e");

		if (userPreferences.userAlarms[currentAlarmNum]->alertMode ==
			ALARMALERTMODESOUND) {
			debugEntry("== > f");
			dialog = gtk_file_chooser_dialog_new("Open File", window,	// parent window?
												 GTK_FILE_CHOOSER_ACTION_OPEN,
												 GTK_STOCK_CANCEL,
												 GTK_RESPONSE_CANCEL,
												 GTK_STOCK_OPEN,
												 GTK_RESPONSE_ACCEPT,
												 NULL);
		} else if (userPreferences.userAlarms[currentAlarmNum]->
				   alertMode == ALARMALERTMODESOUNDFOLDER) {
			debugEntry("== > g");
			dialog = gtk_file_chooser_dialog_new("Open File",
												 NULL,
												 GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
												 GTK_STOCK_CANCEL,
												 GTK_RESPONSE_CANCEL,
												 GTK_STOCK_OPEN,
												 GTK_RESPONSE_ACCEPT,
												 NULL);
		}

		debugEntry("== > h");
		if (userPreferences.userAlarms[currentAlarmNum]->sound != NULL) {
			debugEntry("== > i");
//          gtk_file_chooser_select_filename(GTK_FILE_CHOOSER (dialog), userPreferences.userAlarms[currentAlarmNum]->sound);
			debugEntry(userPreferences.userAlarms[currentAlarmNum]->sound);
			debugEntry("== > j");
		} else {
			printf("searching for a path!\n");
			//Try to find a sound that's not null
			debugEntry("== > k");
			for (q = 0; q < userPreferences.userAlarmsCount; q++) {
				if (userPreferences.userAlarms[q]->sound != NULL) {
					printf("found a path! %s\n",
						   userPreferences.userAlarms[q]->sound);
					gtk_file_chooser_select_filename(GTK_FILE_CHOOSER
													 (dialog),
													 userPreferences.
													 userAlarms[q]->sound);
					break;
				}
			}

		}
		debugEntry("== > l");



//gdk_threads_enter();

		//apply filter set
		gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);
		debugEntry("== > m");
		response = gtk_dialog_run(GTK_DIALOG(dialog));

//gdk_threads_leave();
		debugEntry("== > n");

		if (response == GTK_RESPONSE_ACCEPT || response == GTK_RESPONSE_OK) {

			debugEntry("== > o");
			filename =
				gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
			printf("Selected file is %s\n", filename);

			debugEntry("== > q");
			if (userPreferences.userAlarms[currentAlarmNum]->sound != NULL) {
				free(userPreferences.userAlarms[currentAlarmNum]->sound);
			}
			debugEntry("== > r");
			userPreferences.userAlarms[currentAlarmNum]->sound =
				calloc(strlen(filename) + 2, sizeof(char));
			sprintf(userPreferences.userAlarms[currentAlarmNum]->sound,
					"%s", filename);

			debugEntry("== > s");
			//Search for the file text and redraw it only
			if (currentTheme.textLabels[CLOCKMODEALARMSETTINGS].
				labelCount > 0) {
				debugEntry("== > t");
				for (q = 0;
					 q <
					 currentTheme.textLabels[CLOCKMODEALARMSETTINGS].
					 labelCount; q++) {
					debugEntry("== > u");
					if (!strcmp
						(currentTheme.textLabels[CLOCKMODEALARMSETTINGS].
						 labelArr[q]->labelKey, "selectedAlarmSound")) {

						debugEntry("== > v");
						drawTextLabel(screen,
									  currentTheme.
									  textLabels[CLOCKMODEALARMSETTINGS].
									  labelArr[q], CLOCKMODEALARMSETTINGS,
									  NULL);
						debugEntry("== > w");

						SDL_UpdateRects(screen, 1,
										&currentTheme.
										textLabels[CLOCKMODEALARMSETTINGS].
										labelArr[q]->labelRect);
						debugEntry("== > x");

						break;
					}
				}
			}

			debugEntry("== > y");
			g_free(filename);
		}

		debugEntry("== > z");
		gtk_widget_destroy(dialog);
	}
	debugEntry("pickAlarmSoundFile() end");
}
