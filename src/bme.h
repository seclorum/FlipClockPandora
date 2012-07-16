/*

Flip Clock Battery Managerment/device manager Header Module
Part of FlipClock C SDL for Maemo.

This is the header file for the battery management/device control functions

-Rob Williams, Aug 11, 2009.


*/


//********************** Structure Definitions ***********************//


struct battStat {
	int lowBattery;
	int charging;
	int onAC;
} batteryStatus;


//Global vars (because I'm lazy)
int insomniacStepCount = 0;	//What step of insomniac fading are we on?

//********************** Done Struct Defs *************************//

//********************** Function Headers *************************//


int setInsomniacMode(int turnOn);			//Function to turn insomniac mode on or off

int wakeFromInsomniaStepped(int stepTime);

void startWakeFromInsomiaStepped(int stepTime, int stepCount);


//********************** Done Function Headers ********************//

