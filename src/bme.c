


#if defined(LIBOSSO_H_)
//Only allow to be included if libosso is present

#include <dbus/dbus.h> /* Pull in all of D-Bus headers. */
#include <stdio.h>     /* printf, fprintf, stderr */
#include <dbus/dbus-glib-lowlevel.h>


#include <gconf/gconf-client.h> //Use gConf for the "insomniac" stuff

#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/un.h>


#define BME_SIG_PATH "/com/nokia/bme/signal"
#define BME_SIG_IFC "com.nokia.bme.signal"
#define BME_REQ_PATH "/com/nokia/bme/request"
#define BME_REQ_IFC "com.nokia.bme.request"




/* DSME message blob */
struct dsme_message
{
  int len;
  int request;
  void *value;
};




//************** Function Headers **************************//

void register_battery_info_get();			//Function to connect to dbus and register the signal handler

void battery_info_request();				//Function to actually poll for battery status

static DBusHandlerResult bme_message_handler(DBusConnection *_conn, DBusMessage *_msg, gpointer *ctx);

static void deviceStateChanged(osso_hw_state_t* hwState, gpointer data);		//Callback to handle device state changes

int setInsomniacModeNew(int turnOn);

//************** Done Function Headers *********************//



/**************************************************************************
* dsme_set_brightness(int desiredVal)
*
* Function to manually set a brightness level using DSME (used on chinook/diablo)
* This function will need to be replaced/updated for Fremantle!
*
* This function is based off of the code in Advanced Backlight.. Thanks guys!
* http://adv-backlight.garage.maemo.org/
*
***************************************************************************/
static void dsme_set_brightness(int desiredVal)
{
	struct dsme_message message;
	struct sockaddr_un addr;
	int dsmesock;

	if (desiredVal < 0 || desiredVal > 254) {
		//Nope, invalid value!
		return;
	}

	printf("Setting brightness to %d\n", desiredVal);

	//value = (desiredVal * 2) + 1;
	dsmesock = socket(PF_UNIX, SOCK_STREAM, 0);
	/* thanks to Austin Che for DSME code. */
	if (dsmesock < 0) {
		printf("dsme socket failed");
		return;	
	}

	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	strcpy(addr.sun_path, "/tmp/dsmesock");
  
	if (connect(dsmesock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
	{
		close(dsmesock);
		printf("couldn't connect to dsme");
		return;
	}
                                                                
	message.len = 0x0c;
	message.request = 0x289; //Aparently this means set brightness, though I couldn't find it documented anywhere or in any source...
	message.value = (void *) desiredVal;
	send(dsmesock, (void *) &message, sizeof(message), 0);
	close(dsmesock);
}

/**************************************************************************
* set_brightness_TO(int desiredVal)
*
* Helper function to set brightness to a given level after a timeout
**************************************************************************/

int set_brightness_TO(int desiredVal) {
	dsme_set_brightness(desiredVal);
//	The Proper way to do it.. pending on a daemon...	
//	sprintf (cmd, "echo %d > /sys/class/backlight/acx565akm/brightness", 2);	
//	execcl (cmd, (char *) 0);

	//Only need to do this once to abort timeout
	return 0;

}

/**************************************************************************
* set_brightness_insomnia_TO(int desiredVal)
*
* Helper function to set brightness to a given level after a timeout
**************************************************************************/

int set_brightness_insomnia_TO(int desiredVal) {
	//printf("ooga ooga ooga\n");
#if MAEMOVERSION == 4
	dsme_set_brightness(desiredVal);
#endif
/*	
#if MAEMOVERSION == 5
	char cmd[100];
	// go crazy
	sprintf (cmd, "echo %d > /sys/class/backlight/acx565akm/brightness", desiredVal);	
	exec (cmd);
#endif
*/
	isInsomniacDimmed = 1;
	//Only need to do this once to abort timeout
	return 0;

}



void register_battery_info()
{
	DBusMessage *db_msg;
	DBusConnection *db_conn;
	DBusError dberror;
	
	dbus_error_init(&dberror);
	db_conn = dbus_bus_get(DBUS_BUS_SYSTEM, &dberror);
	if(db_conn == NULL)
	{
		printf("Failed to open connection to system message bus: %s\n",
		dberror.message);
		dbus_error_free(&dberror);
		return;
	}
	
	dbus_bus_add_match(db_conn,"type='signal',path='"BME_SIG_PATH"',interface='"BME_SIG_IFC"'",NULL);
	dbus_connection_add_filter(db_conn, (DBusHandleMessageFunction)bme_message_handler, NULL, NULL);
	db_msg = dbus_message_new_signal(BME_REQ_PATH, BME_REQ_IFC, "status_info_req");
	dbus_connection_send(db_conn, db_msg, NULL);
	dbus_connection_flush(db_conn);
	dbus_message_unref(db_msg);
	dbus_connection_setup_with_g_main(db_conn, NULL);
}


/**************************************************************************
* battery_info_request()
*
* Function to actually send out a DBus Battery status request signal 
* (incase we want to poll without waiting for an actual change event
***************************************************************************/

void battery_info_request() {
	DBusConnection *db_conn;
	DBusError dberror;
	DBusMessage *db_msg;
	
	dbus_error_init(&dberror);
	db_conn = dbus_bus_get(DBUS_BUS_SYSTEM, &dberror);
	if(db_conn == NULL)
	{
		printf("Failed to open connection to system message bus: %s\n",
		dberror.message);
		dbus_error_free(&dberror);
		return;
	}
	
	db_msg = dbus_message_new_signal(BME_REQ_PATH, BME_REQ_IFC, "status_info_req");
	dbus_connection_send(db_conn, db_msg, NULL);
	dbus_connection_flush(db_conn);
	dbus_message_unref(db_msg);
}


static DBusHandlerResult bme_message_handler(DBusConnection *_conn, DBusMessage *_msg, gpointer *ctx)
{
	//DBusMessageIter args;
	//int type;
	//char debugMsg[100];

	if(dbus_message_get_type(_msg) == DBUS_MESSAGE_TYPE_SIGNAL)
	{
		const char *member;
		member = dbus_message_get_member(_msg);
		if(!strcmp(member, "battery_low"))
		{
			batteryStatus.lowBattery = TRUE;
			/* Do something */
		} else if(!strcmp(member, "charger_charging_on") || !strcmp(member, "charger_connected") || !strcmp(member, "battery_full")) {
			batteryStatus.charging = TRUE;
		//} else if(!strcmp(member, "charger_charging_off")) {
		} else if (!strcmp(member, "charger_disconnected")) {
			batteryStatus.charging = FALSE;
		}
		
		/*dbus_message_iter_init(_msg, &args);
		type = dbus_message_iter_get_arg_type(&args);
		
		printf("type of %i args\n", type);
		*/
		
		
		//printf("dbus-printed %s\n", member);
		//sprintf(debugMsg, "charger says: %i\n", batteryStatus.charging);
		//debugEntry(debugMsg);
		//printf("%s", debugMsg);
		//printf("dbus-raw %s\n", _msg);
	}
	return DBUS_HANDLER_RESULT_HANDLED;
} 


/************************************************************************
* deviceStateChanged()
*
* Function to be called by libosso when the device changes it's state
*
*************************************************************************/

static void deviceStateChanged(osso_hw_state_t* hwState, gpointer data) {

	if (hwState->system_inactivity_ind == 1) {
		printf("Device is now inactive!\r\n");
		tabletInactive = 1;
	} else {
		printf("Device is now active!\r\n");
		if (tabletInactive == 1 ) {
			//Just returned from inactive and was not on charge before, so restart the clock display
			tabletInactive = 0;
			
			if (clockRunning == 0) {
				clockEventRedraw();
			//	clockSecondsRedraw();
			}
			
			if (clockSecondsRunning == 0) {
				clockSecondsRedraw();
			}
		}
		
		
	}


}

/************************************************************************
* setInsomniacMode(int turnOn)
*
* Function to enable or disable "insomniac mode" where display stays at half
* light all night!
*
*************************************************************************/
int setInsomniacMode(int turnOn) {
	if (insomniacMode == 2) {
		setInsomniacModeNew(turnOn);
		return 0;
	}

	char newMsg[255];			//Msg to display to user
	GConfValue* gcValue = NULL;
	//Set things up
	g_type_init();

	GConfClient* gconfClient = gconf_client_get_default();
    g_assert(GCONF_IS_CLIENT(gconfClient));

	if (originalDim == -1) {
		////Original DIM has not been set, so grab it from gconf    
    	gcValue = gconf_client_get(gconfClient, "/system/osso/dsm/display/display_blank_timeout", NULL);
//    	gcValue = gconf_client_get_without_default(gconfClient, "/system/osso/dsm/display/display_blank_timeout", NULL);
		
		if(gcValue == NULL) {
			printf("No Gconf key for display timeout found!\n");
			g_object_unref(gconfClient);
      		return 0;
		}
		//Check if value type is integer
		if(gcValue->type == GCONF_VALUE_INT) {
			originalDim = gconf_value_get_int(gcValue);
			if (originalDim > 20 * 3600) {
				//That ain't right, let's stick with a default!
				originalDim = 60;
			}
		}
		
		printf("got original dim: %i\n", originalDim);
		
		//Free up value
		gconf_value_free(gcValue);
	}
	
	if (turnOn == -1) {
		//Toggle current mode
		if (userPreferences.insomniacModeOn == 0) {
			turnOn = 1;
		} else {
			turnOn = 0;
		}
	
		userPreferences.insomniacModeOn = turnOn;
		
		setUserPreferences();
	}
	
	if (turnOn == 1) {
		//Turn insomniac mode on!
		//Set the dimtime to 1 day... that should do the trick!
		//and we then don't have to worry about chargers or anything since inactivity flag is only raised if screen actually goes off!
		gconf_client_set_int(gconfClient, "/system/osso/dsm/display/display_blank_timeout", 24 * 3600, NULL);
		printf("Set dim\n");
		sprintf(newMsg, "Insomniac Mode On...");
	
	} else {
		//Turn the damn thing back off!
		gconf_client_set_int(gconfClient, "/system/osso/dsm/display/display_blank_timeout", originalDim, NULL);
		printf("Cleared dim\n");
		sprintf(newMsg, "Insomniac Mode Off...");
		
	}
	
	osso_system_note_infoprint(ossoAppContext, newMsg, NULL);
	
	
	
	/* release GConf client */
    g_object_unref(gconfClient);
	
	return 0;

}


/************************************************************************
* setInsomniacModeNew(int turnOn)
*
* Function to enable or disable "insomniac mode" where display stays at 
* desired brightness while inactive. 
*
* I don't like doing it this way as it overrides the built in brightness
* controls that users might be accustomed to by re-implementing them manually
* but I can't find any way to control the "dim"'d brightness level which seems to 
* be hard coded
*
*************************************************************************/
int setInsomniacModeNew(int turnOn) {
	int handyInt = 0;
	int notify = 1;
	char newMsg[255];			//Msg to display to user
	GConfValue* gcValue = NULL;
	//Set things up
	g_type_init();

	GConfClient* gconfClient = gconf_client_get_default();
    g_assert(GCONF_IS_CLIENT(gconfClient));

	if (originalBrightLevel == -1) {
		////Original DIM has not been set, so grab it from gconf    
    	gcValue = gconf_client_get(gconfClient, "/system/osso/dsm/display/display_brightness", NULL);
//    	gcValue = gconf_client_get_without_default(gconfClient, "/system/osso/dsm/display/display_brightness", NULL);
		
		if(gcValue == NULL) {
			printf("No Gconf key for display brightness found!\n");
			g_object_unref(gconfClient);
      		return 0;
		}
		//Check if value type is integer
		if(gcValue->type == GCONF_VALUE_INT) {
			originalBrightLevel = gconf_value_get_int(gcValue);
			if (originalBrightLevel > 5 || originalBrightLevel < 1) {
				//That ain't right, let's stick with a default!
				originalBrightLevel = 3;
			}
		}
		
		//Convert it to a useful number 
		originalBrightLevel = originalBrightLevel * 50; 	//This is what Diablo uses... 1-5 at steps of 50
		
		//Free up value
		gconf_value_free(gcValue);
		
		//Now check to see if brightness value for advanced backlight is set (since this should be used instead if available)
		gcValue = gconf_client_get(gconfClient, "/apps/adv-backlight/brightness", NULL);
		//gcValue = gconf_client_get_without_default(gconfClient, "/apps/adv-backlight/brightness", NULL);
		
		if(gcValue != NULL) {

			//Check if value type is integer
			if(gcValue->type == GCONF_VALUE_INT) {
				originalBrightLevel = gconf_value_get_int(gcValue);
				originalBrightLevel = originalBrightLevel * 2; // Advanced Backlight uses 1-127 for soem reason?
			}
			
			//Free up value
			gconf_value_free(gcValue);
		}
		printf("got originalBrightLevel dim: %i\n", originalBrightLevel);
		
	}
	
	if (originalDim == -1) {
		////Original DIM has not been set, so grab it from gconf    
    	gcValue = gconf_client_get(gconfClient, "/system/osso/dsm/display/display_dim_timeout", NULL);
    	//gcValue = gconf_client_get_without_default(gconfClient, "/system/osso/dsm/display/display_dim_timeout", NULL);
		
		if(gcValue == NULL) {
			printf("No Gconf key for display timeout found!\n");
			g_object_unref(gconfClient);
      		return 0;
		}
		//Check if value type is integer
		if(gcValue->type == GCONF_VALUE_INT) {
			originalDim = gconf_value_get_int(gcValue);
			if (originalDim > 20 * 3600) {
				//That ain't right, let's stick with a default!
				originalDim = 60;
			}
		}
		
		//Free up value
		gconf_value_free(gcValue);
		
		
		
		printf("got original dim: %i\n", originalDim);
		

	}
	
	if (userPreferences.insomniacModeOn == 0 && turnOn == 0) {
		notify = 0;
	}
	
	if (turnOn == -1) {
		//Toggle current mode
		if (userPreferences.insomniacModeOn == 0) {
			turnOn = 1;
		} else {
			turnOn = 0;
		}
	
		userPreferences.insomniacModeOn = turnOn;
		
		setUserPreferences();
	}
	
	
	if (turnOn == 1) {
		//Turn insomniac mode on!
		//Set the dimtime to 1 day... that should do the trick!
		//and we then don't have to worry about chargers or anything since inactivity flag is only raised if screen actually goes off!
		gconf_client_set_int(gconfClient, "/system/osso/dsm/display/display_dim_timeout", 24 * 3600, NULL);
		
		printf("Set insomnia TO %i with value %i\n", insomniacTO, userPreferences.insomniacDim);
		
		if (insomniacTO == 0) { 
			if (userPreferences.insomniacDim != -1) {
				insomniacTO = g_timeout_add(originalDim * 1000, (GSourceFunc) set_brightness_insomnia_TO, (int *)userPreferences.insomniacDim);
				printf("Set insomnia timeout for %i with value %i\n", originalDim * 1000, userPreferences.insomniacDim);
			}
		}
		
		
		//Now set the actual brightness level 
		printf("Set dim\n");
		sprintf(newMsg, "Insomniac Mode On...");
	
	} else {
		//Clear the idle timeout if it exists
		if (insomniacTO > 0) {
			g_source_remove(insomniacTO);
			insomniacTO = 0;
		}
		
		////Original DIM has not been set, so grab it from gconf    
    	gcValue = gconf_client_get(gconfClient, "/system/osso/dsm/display/display_blank_timeout", NULL);
    	//gcValue = gconf_client_get_without_default(gconfClient, "/system/osso/dsm/display/display_blank_timeout", NULL);
		
		if(gcValue == NULL) {
			printf("No Gconf key for display brightness found!\n");
			g_object_unref(gconfClient);
      		return 0;
		}
		//Check if value type is integer
		if(gcValue->type == GCONF_VALUE_INT) {
			handyInt = gconf_value_get_int(gcValue);
			
			
		}
		
		
		dsme_set_brightness(originalBrightLevel);
		
		//Turn the damn thing back off!
		gconf_client_set_int(gconfClient, "/system/osso/dsm/display/display_dim_timeout", originalDim, NULL);
		//Set brightness back to original (just in case, though technically it shouldn't be possible to not be at this level right now...)
		
		//And for some bizarre reason we need to change the value of display_blank_timeout in order to update it and make it work...
		gconf_client_set_int(gconfClient, "/system/osso/dsm/display/display_blank_timeout", handyInt + 1, NULL);
		gconf_client_set_int(gconfClient, "/system/osso/dsm/display/display_blank_timeout", handyInt, NULL);
		
		
		
		printf("Cleared dim\n");
		sprintf(newMsg, "Insomniac Mode Off...");
		
	}
	
	if (notify) {
		osso_system_note_infoprint(ossoAppContext, newMsg, NULL);
	}
	
	
	/* release GConf client */
    g_object_unref(gconfClient);
	
	return 0;

}

/************************************************************************
* wakeFromInsomia()
*
* Function to return display to normal on activity/alarm if in insomniac mode 2
*
*************************************************************************/
void wakeFromInsomnia() {
	if (userPreferences.insomniacModeOn == 1 && insomniacMode == 2) {
		//Wake up the display
		if (originalBrightLevel != -1) {
			set_brightness_TO(originalBrightLevel);
			isInsomniacDimmed = 0;
		}
		
		//Clear the old TO
		if (insomniacTO > 0) {
			g_source_remove(insomniacTO);
			insomniacTO = 0;
		}
	
	}
}


/************************************************************************
* startWakeFromInsomiaStepped()
*
* Function to return display to normal on activity/alarm if in insomniac mode 2
* over a given step period... this function starts the process
*
*************************************************************************/
void startWakeFromInsomiaStepped(int stepTime, int stepCount) {
	int stepValue;
	
	if (userPreferences.insomniacModeOn == 1 && insomniacMode == 2) {
		//Wake up the display
		if (originalBrightLevel != -1 && userPreferences.insomniacDim != -1) {
			
			//Calculate the actual step amount to increase by
			stepValue = (originalBrightLevel - userPreferences.insomniacDim) / stepCount;
			
			insomniacStepCount = 0;
			
			//Start the process
			insomniacTO = g_timeout_add((stepTime / stepCount), (GSourceFunc) wakeFromInsomniaStepped, (int *)stepValue);
		}
	}
}

/************************************************************************
* wakeFromInsomiaStepped()
*
* Function to return display to normal on activity/alarm if in insomniac mode 2
* over a given step period... hmmm not working yet, no idea how to do this right...
*
*************************************************************************/
int wakeFromInsomniaStepped(int stepTime) {
	int stepValue;
	int steps = 20;	//Number of steps

	int recurr = 0;	//Should timeout continue or not?

	if (userPreferences.insomniacModeOn == 1 && insomniacMode == 2) {
		//Wake up the display
		if (originalBrightLevel != -1 && userPreferences.insomniacDim != -1) {
			insomniacStepCount++;
			stepValue = stepTime * insomniacStepCount;

			if (userPreferences.insomniacDim + stepValue < originalBrightLevel) {
				set_brightness_insomnia_TO(stepValue + userPreferences.insomniacDim);
			
				recurr = 1;
				//insomniacTO = g_timeout_add((stepTime / steps), (GSourceFunc) wakeFromInsomniaStepped, (int *)stepTime);
				//printf("Set insomnia timeout for %i with value %i\n", originalDim * 1000, userPreferences.insomniacDim);
			}
		}
		
		
		
			//set_brightness_TO(originalBrightLevel);
			//isInsomniacDimmed = 0;
		//}
		
		//Clear the old TO
		//if (insomniacTO > 0) {
			//g_source_remove(insomniacTO);
			//insomniacTO = 0;
		//}
	
	}
	
	if (recurr == 0) {
		//We're done
		isInsomniacDimmed = 0;
	}
	
	return recurr;
}



/************************************************************************
* setTabletSystemVolume(int restoreOriginal)
*
* Function to set the tablet system volume so alarms will always be audible; original volume level
* is stored and reset once alarm sound is complete.
*
*************************************************************************/
int setTabletSystemVolume(int restoreOriginal) {

	GConfValue* gcValue = NULL;
	int currVol = -1;
	
	//Set things up
	g_type_init();

	GConfClient* gconfClient = gconf_client_get_default();
    g_assert(GCONF_IS_CLIENT(gconfClient));

	//if (restoreOriginal != 0) {
		////Original Sysvol has not been set, so grab it from gconf    
    	gcValue = gconf_client_get(gconfClient, "/apps/osso/sound/master_volume", NULL);
    	//gcValue = gconf_client_get_without_default(gconfClient, "/apps/osso/sound/master_volume", NULL);
		
		if(gcValue == NULL) {
			printf("No Gconf key for master volume found!\n");
			g_object_unref(gconfClient);
      		return 0;
		}
		//Check if value type is integer
		if(gcValue->type == GCONF_VALUE_INT) {
			currVol = gconf_value_get_int(gcValue);
		}
		
		printf("got original sys vole: %i\n", originalSysVol);
		
		//Free up value
		gconf_value_free(gcValue);
	//}
	
	if (originalSysVol == -1) {
		originalSysVol = currVol;
	}
	
	
	if (restoreOriginal == 1) {
		//Restore the original volume
		gconf_client_set_int(gconfClient, "/apps/osso/sound/master_volume", originalSysVol, NULL);
		
	
	} else if (restoreOriginal == 0) {
		//Set it to 100
		gconf_client_set_int(gconfClient, "/apps/osso/sound/master_volume", 100, NULL);
		
	} else if (restoreOriginal == 2) {
		//Increment
		if (currVol + 5 < 100) {
			gconf_client_set_int(gconfClient, "/apps/osso/sound/master_volume", currVol + 5, NULL);
		}
	} else if (restoreOriginal == 3) {
		//Decrement
		if (currVol - 5 > 0) {
			gconf_client_set_int(gconfClient, "/apps/osso/sound/master_volume", currVol - 5, NULL);
		}
		
	}
	
	/* release GConf client */
    g_object_unref(gconfClient);
	
	return 0;

}



#else

int setInsomniacMode(int turnOn) {
	return 0;
}

int setInsomniacModeNew(int turnOn) {
	return 0;
}

int setTabletSystemVolume(int restoreOriginal) {
	return 0;
}

int set_brightness_TO(int desiredVal) {
	return 0;
}

int set_brightness_insomnia_TO(int desiredVal) {
	return 0;
}

void wakeFromInsomnia() {
	return;
}

void startWakeFromInsomiaStepped(int stepTime, int stepCount) {
	return;
}
#endif
