/*

Functions Headers that allow flipclock to be tied into AlarmD Daemon.
Part of FlipClock C SDL for Maemo.

This is the header file for the alarmD integration functions

-Rob Williams, Aug 11, 2009.


*/





//********************** Structure Definitions ***********************//


struct alarmDSettingsObject {
	long alarmTime;
	int alarmIndex;				//Unique index for this alarm, used to match dbus signal with configured alarms
	long alarmCookie;			//Unique cookie/index of this alarm
};

struct alarmDSettingsObject **userAlarmDObjects;	//Alarms
int userAlarmDObjectsCount;									//Number of alarms

//********************** Done Struct Defs *************************//

//********************** Function Headers *************************//

int getFlipAlarms();

int clearFlipAlarm(int alarmCookie);

int clearAllFlipAlarms();

//void createDefaultAlarm(alarm_event_t *thisAlarmObj);

//void freeAlarmObj(alarm_event_t thisAlarmObj);

int setFlipAlarmByIndex(int alarmIndex);

void initAlarmDSettingsObject(struct alarmDSettingsObject *thisAlarm);

//********************** Done Function Headers ********************//

