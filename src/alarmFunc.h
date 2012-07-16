/*

Flip Clock Alarm Manager Header Module
Part of FlipClock C SDL for Maemo.

This is the header file for the alarm/alarm settings functions

-Rob Williams, Aug 11, 2009.


*/





//********************** Structure Definitions ***********************//




//********************** Done Struct Defs *************************//

//********************** Function Headers *************************//

int getFirstAlarmIndexByDay(int alarmDay);				//get the first alarm for a specific day (Sunday = 0)

int checkForAlarm(long alarmTime);						//Check to see if an alarm is defined for the given time

int handleAlarm(int alarmIndex);							//execute an alarm

void stopAlarm();

void digitUpHH1();

void digitUpHH2();

void digitUpMM1();

void digitUpMM2();

void digitDownHH1();

void digitDownHH2();

void digitDownMM1();

void digitDownMM2();

void drawAlarmDigits(SDL_Surface *screenBuff, char *alarmHHMM);

void drawAlarmScrollerDigit(SDL_Surface *screenBuff, int digitIndex, int digitValue, SDL_Rect *digitPosRect);

void animateAlarmScrollerDigit(SDL_Surface *screenBuff, int digitIndex, int oldDigitValue, int newDigitValue, int frames, SDL_Rect *digitPosRect);

void drawAllAlarmDays(SDL_Surface *screenBuff);

void drawAlarmDayPicker(SDL_Surface *screenBuff, int alarmDay);

void getNextAlarmTime();

void updateAlarms();


//********************** Done Function Headers ********************//

