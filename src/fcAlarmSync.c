/*  flip SDL alarmD Updater
******************************************************************
  This is the alarmD sync/updater component of FlipClockC. This app is setup as
  a simple mechanism to synchronize flipclock alarms with the AlarmD daemon on Maemo.

  The reason for this app is to allow for simple background updates of AlarmD; doing it
  from the main app is only possible by waiting for the alarmD daemon to actually
  process the events, which takes time and locks up the interface; so instead we
  setup this app to listen for dbus requests for alarmD sync. It makes things much better...
  
  
  ******* UPDATES/CHANGELOG ********
  10/02/09  - Rob
		Initial build.  
  
  
  ******************************************************************
*/



#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>

#include <sys/signal.h>

#include <string.h>     /* for string functions */

#include <glib.h>		/*** GLib is needed for main loops, etc ***/
#include <libosso.h>    
/*** Libosso gives us bindings to device state; for portability we try to use dbus/etc directly, but libosso
							makes device state checks/etc easier... ***/

//******************* Work arounds for dependencies ****************************//
typedef unsigned int Uint8;	//Needed since SDL isn't here to provide this...

struct themeSettings {
	char *themePath;		//Path of current theme
};

/* Structure for theme summary (used to list available themes) */
struct themeSummary {
	char *themeName;		//Name of theme
	char *themePath;		//Path to theme
};

struct themeSummary **availableThemes = NULL;	//Dynamic array of available themes

struct themeSettings currentTheme = {NULL};				//settings for current theme

//Types of possible alarm control modes 
enum { ALARMCONTROLMODEWEEKLY, ALARMCONTROLMODESIMPLE, ALARMCONTROLMODEMAX};

//***************** Done workarounds ******************************************//



//************** Function headers ***********************************//



//Include alarmD intergration
#include "alarmDFunc.h"

//Include the user prefs module for loading/saving the config files
#include "userPrefs.h"



//DBUS Related settings
#define DBUS_INTERFACE "org.maemo.flipclock"
#define DBUS_SERVICE "org.maemo.flipclock"
#define DBUS_PATH "/org/maemo/flipclock/controller"

//Connection for alarmD syncing
#define DBUS_ALARMSYNC_INTERFACE "org.maemo.flipclockSync"
#define DBUS_ALARMSYNC_SERVICE "org.maemo.flipclockSync"
#define DBUS_ALARMSYNC_PATH "/org/maemo/flipclockSync/controller"

#define LOCKFILE "/var/run/fcAlarmSync"

//************** Define global osso-related vars ********************//

osso_context_t* ossoAppContext;	//Osso context of the program


GMainLoop *mainLoopObj;			//Main GLoop of application. This is global so that it can be accessed from all places...


int themeCount = 1;			//Used to fool userPrefs module into thinking we have themes available


//************** Done global osso-related vars **********************//



//************** Define osso-specific function headers ****************//

//***** DBUS message handlers **********
gint dbus_req_handler(const gchar * interface, const gchar * method, GArray * arguments, gpointer data, osso_rpc_t * retval);

//*************** Done osso-specific functino headers ****************//

int initializeApp();		//Setup app

int deinitializeApp(); 		//Clean up before exiting

void sig_leave(int sig);	//Signal handler for exit/abort

void loadAllThemes();		//To placate userPrefs even though this is never actually called

int handleAlarmsSync();		//Function to actually sync all alarms to alarmD

//***************** Done function headers ****************************//


//***************** Function definitions *****************************//

//INclude file handlers
#include "fileDirFunc.c"


//#ifdef ALARMD2
#if defined(ALARMD2)
//Include alarmD2 (Maemo 5+ version) stuff
#include "alarmDv2Func.c"

#else
//Include original alarmD (Maemo 4) stuff...
#include "alarmDFunc.c"

#endif

//Include the user prefs module for loading/saving the config files
#include "userPrefs.c"


void quitApp() {
	deinitializeApp();
   	exit(0);
}


/******************************************************************************
* Main
*
* Main application routine. Really all this does is sets things up and waits
* for the dbus messages...
* 
*******************************************************************************/
int main(int argc, char *argv[])
{
	
	if (argc > 2) {
		clearAllFlipAlarms();
		exit(0);
	}
	
	if (fileExists(LOCKFILE)) {
		printf("App already running, aborting\n");
		exit(0);
	} else {
		//Create the lockfile
		touchFile(LOCKFILE);
	}
	
	initializeApp();
	
	//Add the failsafe exit timeout to make sure the app doesn't run forever
	g_timeout_add(600000, (GSourceFunc) quitApp, NULL);
	
	//Setup a sync call just to be safe since Dbus doesn't always get through the first time...
	g_timeout_add(10, (GSourceFunc) handleAlarmsSync, NULL);
	
	//Run the main loop
	g_main_loop_run(mainLoopObj);
    

    return 0;



}





static void systemActivityChange(osso_hw_state_t* hwState, gpointer data) {

	if (hwState->system_inactivity_ind == 1) {
		//Tablet inactive, so quit to free memory
		deinitializeApp();
   		exit(0);

	}


}

/************************************************************************
* initializeApp()
*
* Function to get the ball rolling and setup the application.
*
*************************************************************************/

int initializeApp() {
	osso_return_t result;	//Return value from libosso calls
	
	osso_hw_state_t stateFlags;	//Stuct to indicate which hwStates we want to watch for
	
	
	//Setup stateFlags to monitor only inactivity (since we want to exit and free up memory if system goes inactive)
	stateFlags.system_inactivity_ind = TRUE;
	
	
	//Define signal handlers for abort/kill to clean things up incase somethign bad happens
    (void) signal(SIGINT,sig_leave);
    (void) signal(SIGTERM, sig_leave);



	
	//****************** GObject loop setup ***************//
	
	mainLoopObj = g_main_loop_new(NULL, FALSE);		//Create the main loop for the app
	
	//****************** Done GObject loop setup **********//
	
	ossoAppContext = osso_initialize(DBUS_ALARMSYNC_SERVICE, "1.0", FALSE, NULL);
	
	if (ossoAppContext == NULL) {
		printf("Failed to initialize libosso!\r\n");
		return 0;
	}
	
	//Set up DBUS listening
	result = osso_rpc_set_cb_f(ossoAppContext, DBUS_ALARMSYNC_SERVICE, DBUS_ALARMSYNC_PATH, DBUS_ALARMSYNC_INTERFACE, dbus_req_handler, NULL);
	
	//Setup inactivity monitor
	//Okay got the libosso setup, so establish the callback for the hardware state changes
	result = osso_hw_set_event_cb(ossoAppContext, &stateFlags, systemActivityChange, NULL);
	
	return 1;
}


/************************************************************************
* deinitializeApp()
*
* Function to clean up anything that was setup the application before we exit
*
*************************************************************************/

int deinitializeApp() {
	
	//Cleanup the gobject mainloop
	if (mainLoopObj != NULL) {
		g_main_loop_unref(mainLoopObj);
		mainLoopObj = NULL;
	}
	
	//Clean up the osso stuff
	if (ossoAppContext != NULL) {
		osso_rpc_unset_cb_f(ossoAppContext, DBUS_ALARMSYNC_SERVICE, DBUS_ALARMSYNC_PATH, DBUS_ALARMSYNC_INTERFACE, dbus_req_handler, NULL);
		
		//Cleanup the osso context itself
		osso_deinitialize(ossoAppContext);
		ossoAppContext = NULL;
	}
	
	//Clean up preferences/user objects
	cleanUserPreferences();

	if(fileExists(LOCKFILE)) {
		remove(LOCKFILE);
	}

	return 1;
}


/**************************************************************************
* int handleAlarmsSync()
*
* Function to handle the sync'ing of all alarms
*
**************************************************************************/
int handleAlarmsSync() {
	int updated =0;			//has the config been updated?
	int i;
		
	printf("Starting alarmD sync\n");
	//Load the user prefs from the config file
	getUserPreferences();
	
	//Setup the default alarmD stuff
	setDefaultUserAlarmDObjects();
	
	//Load the saved alarmD stuff if its there
	getAlarmDSettings();
	
	updated = 0;
	
	
	for(i=0; i< userPreferences.userAlarmsCount; i++) {
		if (updateAlarmIndex(i)) {
			updated++;
		}
	}

	
	
	//Save updated user prefs
	if (updated > 0) {
		osso_system_note_infoprint(ossoAppContext, "AlarmD Sync'd!", NULL);
		setAlarmDSettings();
	}
	
	
	
	//cleanup
	//cleanAlarmDSettingsObjects() ;
	 
	//cleanUserPreferences(); 
	printf("Done, updated %i alarms\n", updated);
	
	return FALSE;
}


/**************************************************************************
* D-Bus callback handler...
*
**************************************************************************/
gint dbus_req_handler(const gchar * interface, const gchar * method, GArray *arguments, gpointer data, osso_rpc_t * retval)
{
	osso_rpc_t argValue;
	long alarmTime = 0;	//Used for triggerAlarm call
	int alarmIndex = -1;	//Used for triggerAlarm call (-1 means no index passed)
	int alarmDResult = 0;	//Status of alarmD calls.


	//char test[100];
	//sprintf(test, "AlarmSync:Received %s\r\n", method);

	//osso_system_note_infoprint(ossoAppContext, test, NULL);

	//Handle dbus calls here!
	if (strcmp(method, "syncAlarmD") == 0) {
		handleAlarmsSync();
		
	} else if (strcmp(method, "exit") == 0) {
		//Request to exit
		osso_rpc_free_val(retval);
		deinitializeApp();
   		exit(0);
	} 
	
	
	//Cleanup
	osso_rpc_free_val(retval);

	return OSSO_OK;
}



/************************************************************************
* Handles signals for interrupt/kill to remove lock file for this device*
* if present                                                            *
************************************************************************/

void sig_leave(int sig) {
	deinitializeApp();
    exit(sig);
}

/************************************************************************
* loadAllThemes() 
*
* Workaround function to satisfy user prefs module requirements
************************************************************************/
void loadAllThemes() {
	return;
}


