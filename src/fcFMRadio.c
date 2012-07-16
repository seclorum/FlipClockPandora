/*  flip SDL FM Radio Module
******************************************************************
  This is the N800 FM radio component of FlipClockC. This app is setup as
  a simple mechanism to allow FlipClock to trigger the FM radio receiver on or off
  at alarm times.

  By being a separate process this app ensures modularity and keeps things consistent
  between flip itself and various hardware. For now this only works on the N800.
  
  The N800 code is loosely based on:
  http://wiki.forum.nokia.com/index.php/Getting_Started_with_FM_Radio_on_Maemo
  
  
  
  ******* UPDATES/CHANGELOG ********
  10/02/09  - Rob
		Initial build for N800 only.  
  
  
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

//Include FM Radio stuff for N800
#include "n800FMRadio.h"


//************** Define structures **********************************//



//************** Done structure definition **************************//


//************** Function headers ***********************************//


//DBUS Related settings
#define DBUS_INTERFACE "org.maemo.flipclock"
#define DBUS_SERVICE "org.maemo.flipclock"
#define DBUS_PATH "/org/maemo/flipclock/controller"

//Connection for alarmD syncing
#define DBUS_FMRADIO_INTERFACE "org.maemo.flipclockFMRadio"
#define DBUS_FMRADIO_SERVICE "org.maemo.flipclockFMRadio"
#define DBUS_FMRADIO_PATH "/org/maemo/flipclockFMRadio/controller"

#define LOCKFILE "/var/run/fcFMRadio"

#define DEFAULT_DEVICE "/dev/radio0"
#define DEFAULT_MIXER  "/dev/mixer"

//************** Define global osso-related vars ********************//

osso_context_t* ossoAppContext;	//Osso context of the program

RadioDescriptor* radioObj = NULL;		//Radio control struct	

GMainLoop *mainLoopObj;			//Main GLoop of application. This is global so that it can be accessed from all places...

unsigned int maxVol = 100;		//Max vol of radio

unsigned int fadeTime = 60000;	//Default time to fade in

//************** Done global osso-related vars **********************//



//************** Define osso-specific function headers ****************//

//***** DBUS message handlers **********
gint dbus_req_handler(const gchar * interface, const gchar * method, GArray * arguments, gpointer data, osso_rpc_t * retval);

//*************** Done osso-specific functino headers ****************//

int initializeApp();		//Setup app

int deinitializeApp(); 		//Clean up before exiting

void sig_leave(int sig);	//Signal handler for exit/abort

//***************** Done function headers ****************************//


//***************** Function definitions *****************************//

//INclude file handlers
#include "fileDirFunc.c"

//Include FM RAdio stuff for N800
#include "n800FMRadio.c"


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
	//g_timeout_add(10, (GSourceFunc) handleAlarmsSync, NULL);
	
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


/*********************************************************
* int stepVolumeUp(int stepTime);
*
*Fades an alarm sound up
**********************************************************/

int stepVolumeUp(int stepTime) {
	unsigned int currVol = 0;
	
	
	if (radioObj->radio_fd > -1) {
		if (radioObj->silent != 1) {
			//Get the current volume
			currVol = radio_getvolume(radioObj);
			//printf("Step up currvol %i\n", currVol);
			//printf("RadioObj %i %i\n", radioObj->vol, radioObj->mixer_fd);
			if (currVol + 5 < maxVol) {
				//Start by setting vol low
				radio_setvolume(radioObj, currVol + 5);
				
						
				g_timeout_add(stepTime, (GSourceFunc) stepVolumeUp, (int *) stepTime);
			}
		}
	}
	return FALSE;

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
	
	ossoAppContext = osso_initialize(DBUS_FMRADIO_SERVICE, "1.0", FALSE, NULL);
	
	if (ossoAppContext == NULL) {
		printf("Failed to initialize libosso!\r\n");
		return 0;
	}
	
	//Set up DBUS listening
	result = osso_rpc_set_cb_f(ossoAppContext, DBUS_FMRADIO_SERVICE, DBUS_FMRADIO_PATH, DBUS_FMRADIO_INTERFACE, dbus_req_handler, NULL);
	
	//Setup inactivity monitor
	//Okay got the libosso setup, so establish the callback for the hardware state changes
	result = osso_hw_set_event_cb(ossoAppContext, &stateFlags, systemActivityChange, NULL);
	
	//Create the radio Obj
	radioObj = (RadioDescriptor *) malloc(sizeof(RadioDescriptor));
	radioObj->radio_fd = -1;
	radio_open(radioObj);
	
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
		osso_rpc_unset_cb_f(ossoAppContext, DBUS_FMRADIO_SERVICE, DBUS_FMRADIO_PATH, DBUS_FMRADIO_INTERFACE, dbus_req_handler, NULL);
		
		//Cleanup the osso context itself
		osso_deinitialize(ossoAppContext);
		ossoAppContext = NULL;
	}
	
	if (radioObj != NULL) {
		radio_close(radioObj);
		free(radioObj);
	}
	
	if(fileExists(LOCKFILE)) {
		remove(LOCKFILE);
	}

	return 1;
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
	unsigned int frequency = 107100;
	

	//char test[100];
	

	//osso_system_note_infoprint(ossoAppContext, test, NULL);
	
	if (arguments->len > 0) {
		argValue = g_array_index(arguments, osso_rpc_t, 0);
		
		if (argValue.type == DBUS_TYPE_UINT32) {
			frequency = argValue.value.u;
		}
		
		//printf("args len: %i\n", arguments->len);
		
		//check for fadeIn time
		if (arguments->len > 1) {
			argValue = g_array_index(arguments, osso_rpc_t, 1);
			
			if (argValue.type == DBUS_TYPE_UINT32) {
				//printf("new max vol is %u\n", argValue.value.u);
				fadeTime = argValue.value.u;
			}
		}
		
		//check for max volume
		if (arguments->len > 2) {
			argValue = g_array_index(arguments, osso_rpc_t, 2);
			
			if (argValue.type == DBUS_TYPE_UINT32) {
				//printf("new max vol is %u\n", argValue.value.u);
				maxVol = argValue.value.u;
			}
		}
	}
	
	

	//Handle dbus calls here!
	if (strcmp(method, "radioOn") == 0) {
		printf("Turn radio on!\n");
		//Just turn the radio on
		radio_unmute(radioObj);
		radio_setfreq(radioObj, frequency);	
		radio_setvolume(radioObj, maxVol);
	} else if (strcmp(method, "radioFadeIn") == 0) {
		//Fade the radio in like a regular alarm
		radio_unmute(radioObj);
		radio_setfreq(radioObj, frequency);	
		radio_setvolume(radioObj, 0);
		
		//Start the fade in
		stepVolumeUp(fadeTime / 20);
		
		
	} else if (strcmp(method, "radioOff") == 0) {
		printf("turn radio off!\n");
		//Just turn the radio on
		radio_mute(radioObj);
		
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



