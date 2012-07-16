/***************** Osso Helper functions

Functions that allow flipclock to be tied into OSSO functionality (i.e. hardware monitoring, etc).

These functions should be seamlessly disabled if osso isn't present, allowing flip to run on any other OS that has
the appropriate SDL libs. (just without as much coolness)

*/




#if defined(LIBOSSO_H_)
//Only allow to be included if libosso is present


//************** Define functions themselves ***********************//

/*******************************************************
*
* ossoFlipInit()
*
* Function to initialize all osso-related features of the app.
********************************************************/

int ossoFlipInit() {
	osso_return_t result;	//Return value from libosso calls
	osso_hw_state_t stateFlags;	//Stuct to indicate which hwStates we want to watch for
	
	
	//Setup stateFlags to monitor only inactivity
	stateFlags.system_inactivity_ind = TRUE;
	
	
		//Setup the libosso stuff for device state monitoring
	ossoAppContext = osso_initialize(DBUS_SERVICE, "1.0", FALSE, NULL);
	
	if (ossoAppContext == NULL) {
		printf("Failed to initialize libosso!\r\n");
		return 0;
	}
	
	//***************** Setup hardware state changes through libosso setup***************//
	
	/* This seems a bit redundant since we setup a more direct DBus signal later to monitor
	for battery status... which could also ready device states, but... ehn.. */
	
	//Okay got the libosso setup, so establish the callback for the hardware state changes
	result = osso_hw_set_event_cb(ossoAppContext, &stateFlags, deviceStateChanged, NULL);
	
	if (result != OSSO_OK) {
		printf("Failed to register callback!\r\n");
		return 0;
	}
	//**************** Done hardware state change setup *******************************//
	
	//Test for setting up DBUS listening
	result = osso_rpc_set_cb_f(ossoAppContext, DBUS_SERVICE, DBUS_PATH, DBUS_INTERFACE, dbus_req_handler, NULL);

	//******* Battery/Hardware status setup ******************//
	//Connect to BME DBus signal
	register_battery_info();
	
	//Send a request for battery status
	battery_info_request();
	

	//******* Done Battery/Hardware status setup *************//

	//Turn the display on (just to be safe)
	ossoDeviceScreenOn();



	return 1;
}



/*******************************************************
*
* ossoFlipDeInit()
*
* Function to de-initialize all osso-related features of the app.
********************************************************/

int ossoFlipDeInit() {

	osso_hw_state_t stateFlags;
	
	//Kill off alarmSync if it's running
	ossoKillAlarmSync();
	
	//Setup stateFlags to monitor only inactivity
	stateFlags.system_inactivity_ind = TRUE;
	
	//Clean up the osso hardware event callback
	if (ossoAppContext != NULL) {
		osso_hw_unset_event_cb(ossoAppContext, &stateFlags);
		//osso_hw_unset_event_cb(ossoAppContext, NULL);
		
		//Cleanup the osso context itself
		osso_deinitialize(ossoAppContext);
		ossoAppContext = NULL;
	}
	
	return 1;


}


/**************************************************************************
*
* ossoDeviceScreenOn()
*
* function to turn on/wake up the screen on osso devices (i.e. NITs)
**************************************************************************/

void ossoDeviceScreenOn() {

	//Get the device state pointer
	//devstat = osso.DeviceState(ossoAppContext);
	//turn the screen on
	osso_display_state_on(ossoAppContext);
	
	//Wake up from insomnia mode if applicable
	wakeFromInsomnia();
	
	//Set the "Active" mode
	if (tabletInactive == 1 ) {
		//Just returned from inactive and was not on charge before, so restart the clock display
		tabletInactive = 0;
		
		if (clockRunning == 0) {
			clockEventRedraw();
		}
		
		if (clockSecondsRunning == 0) {
			clockSecondsRedraw();
		}
	}


}

/**************************************************************************
*
* ossoUpdateAlarms(int updatedAlarms)
*
* Screw that osso stuff, let's do it with dbus directly as it makes more sense...
* (of course we'll just use the source from osso_rpc_async_run to do it, but yea)
**************************************************************************/

void ossoUpdateAlarms() {

	DBusMessage *msg = NULL;
	DBusConnection *db_conn;
	
	
	//printf("updating alarms!\n");
	db_conn = dbus_bus_get(DBUS_BUS_SESSION, NULL);
	
   //Create the dbus message
    msg = dbus_message_new_method_call(DBUS_ALARMSYNC_SERVICE, DBUS_ALARMSYNC_PATH, DBUS_ALARMSYNC_INTERFACE, "syncAlarmD");


	//Setup the message to be auto-launching of the target
	//dbus_message_set_auto_start(msg, TRUE);
	
	//Don't want a reply
	dbus_message_set_no_reply(msg, TRUE);

	//Add the arguments
	/*
	for(i=0; i< userPreferences.userAlarmsCount; i++) {
		if (updatedAlarms[i] == 1) {
			//Add to dbus message
			printf("adding alarm %i to args\n", i);
			dbus_message_append_args(msg, DBUS_TYPE_INT32, &i, DBUS_TYPE_INVALID);
		}
	}*/

	//Finally, send the method call msg
	dbus_connection_send(db_conn, msg, NULL);
	
	
	//Flush it out
	dbus_connection_flush(db_conn);

	//And clean up
	dbus_message_unref(msg);
 
}

/**************************************************************************
*
* ossoKillAlarmSync()
*
* Tell the alarmSync app to exit since flipclock is shutting down
* 
**************************************************************************/

void ossoKillAlarmSync() {

	DBusMessage *msg = NULL;
	DBusConnection *db_conn;
	
	
	//printf("updating alarms!\n");
	db_conn = dbus_bus_get(DBUS_BUS_SESSION, NULL);
	
   //Create the dbus message
    msg = dbus_message_new_method_call(DBUS_ALARMSYNC_SERVICE, DBUS_ALARMSYNC_PATH, DBUS_ALARMSYNC_INTERFACE, "exit");


	//Stop the message from be auto-launching if the target isn't running (no point in starting it to tell it to exit is there)
	dbus_message_set_auto_start(msg, FALSE);
	
	//Don't want a reply
	dbus_message_set_no_reply(msg, TRUE);

	//Finally, send the method call msg
	dbus_connection_send(db_conn, msg, NULL);
	
	//Flush it out
	dbus_connection_flush(db_conn);

	//And clean up
	dbus_message_unref(msg);

}


/**************************************************************************
*
* ossoFMRadio(int radioMode, int radioFreq)
*
* Generate the dbus message to turn the FM radio on/off/etc. 
* RadioMode: 0= radioOff, 1= radioOn, 2= radioFadeIn
* 
**************************************************************************/

void ossoFMRadio(int radioMode, int radioFreq, int fadeTime, int maxVolume) {

	DBusMessage *msg = NULL;
	DBusConnection *db_conn;
	
	
	db_conn = dbus_bus_get(DBUS_BUS_SESSION, NULL);
	
   //Create the dbus message
	switch (radioMode) {
    	case 0:
			//Radio Off
			msg = dbus_message_new_method_call(DBUS_FMRADIO_SERVICE, DBUS_FMRADIO_PATH, DBUS_FMRADIO_INTERFACE, "radioOff");
			break;
		case 1:
			//Radio On
			msg = dbus_message_new_method_call(DBUS_FMRADIO_SERVICE, DBUS_FMRADIO_PATH, DBUS_FMRADIO_INTERFACE, "radioOn");
			//Add the frequency argument
			dbus_message_append_args(msg, DBUS_TYPE_UINT32, &radioFreq, DBUS_TYPE_INVALID);
			break;
		case 2:
			//Fade radio in
			msg = dbus_message_new_method_call(DBUS_FMRADIO_SERVICE, DBUS_FMRADIO_PATH, DBUS_FMRADIO_INTERFACE, "radioFadeIn");
			//Add the frequency argument
			dbus_message_append_args(msg, DBUS_TYPE_UINT32, &radioFreq, DBUS_TYPE_UINT32, &fadeTime, DBUS_TYPE_UINT32, &maxVolume, DBUS_TYPE_INVALID);
			break;
	}
	
	if (msg == NULL) {
		//Failed
		return;
	}
	
	//Add the max volume setting (since N800 radio doesn't listen to master mixer volume)
	dbus_message_append_args(msg, DBUS_TYPE_INT32, &userPreferences.maxAlarmVol, DBUS_TYPE_INVALID);
	

	//Setup the message to be auto-launching of the target
	dbus_message_set_auto_start(msg, TRUE);
	
	//Don't want a reply
	//dbus_message_set_no_reply(msg, TRUE);

	
	//Finally, send the method call msg
	dbus_connection_send_with_reply(db_conn, msg, NULL, -1);
	
	
	//Flush it out
	dbus_connection_flush(db_conn);

	//And clean up
	dbus_message_unref(msg);
	
	return;
 
}


/**************************************************************************
*
* int ossoHasFMRadio();
*
* General function to test to see if the current device has an FM radio or not
* 
**************************************************************************/

int ossoHasFMRadio() {

	if (fileExists("/dev/radio0")) {
		//Has radio!
		printf("Has FM Radio\n");
		return 1;
	} 
	
	return 0;

}



/**************************************************************************
* D-Bus callback test...
*
**************************************************************************/
gint dbus_req_handler(const gchar * interface, const gchar * method, GArray *arguments, gpointer data, osso_rpc_t * retval)
{
	osso_rpc_t argValue;
	long alarmTime = 0;	//Used for triggerAlarm call
	int alarmIndex = -1;	//Used for triggerAlarm call (-1 means no index passed)
	

	printf("Received D-BUS message %s\r\n", method);
	/* Here's how to get arguments... for future reference */
	printf("arg len: %i\r\n", arguments->len);
	if (arguments->len > 0) {
		argValue = g_array_index(arguments, osso_rpc_t, 0);
		if (argValue.type == DBUS_TYPE_STRING) {
			printf("RPC arg1: %s\r\n", argValue.value.s);
		}
	}
	/* Done argument demo */

	if (strcmp(method, "toggle") == 0) {
		toggleWindowMode();
	}

	if (strcmp(method, "switchTheme") == 0) {
		//printf("theme index:%i theme count: %i", currentTheme.themeIndex, themeCount);
		cycleThemeNext();
	}
	
	if (strcmp(method, "gotoSettings") == 0) {
		gotoSettings();
		
	}
	
	if (strcmp(method, "gotoNormal") == 0) {
		changeClockMode(CLOCKMODENORMAL);
	}
	
	if (strcmp(method, "insomniacOn") == 0) {
		setInsomniacMode(1);
	}
	
	if (strcmp(method, "insomniacOff") == 0) {
		setInsomniacMode(0);
	}
	
	if (strcmp(method, "insomniacToggle") == 0) {
		setInsomniacMode(-1);
	}
	
	if (strcmp(method, "triggerAlarm") == 0) {
		//We're triggering an alarm!
		alarmTime = 0;	//default
		alarmIndex = -1;	//default
		if (arguments->len > 0) {
			argValue = g_array_index(arguments, osso_rpc_t, 0);
			
			if (argValue.type == DBUS_TYPE_INT64) {
				alarmTime = argValue.value.u;
			}
			
			argValue = g_array_index(arguments, osso_rpc_t, 1);
			
			if (argValue.type == DBUS_TYPE_INT32) {
				alarmIndex = argValue.value.i;
			}
			printf("alarm index is %i\n", alarmIndex);
		}
		
		
		if (alarmIndex == -1) {
			//Try to find the alarm based on the time and run it
			alarmIndex = checkForAlarm(alarmTime);
		} 
		
		if (alarmIndex != -1) {
			//things are better so we can run the alarm directly without the extra overhead of checking
			//Run the alarm
			handleAlarm(alarmIndex);
		}
	}

	//Cleanup
	osso_rpc_free_val(retval);

	return OSSO_OK;

}



//************ Done function definitions ****************************//

#else

void ossoDeviceScreenOn() {
	return;
}

void ossoFMRadio(int radioMode, int radioFreq, int fadeTime, int maxVolume) {
	return;
}

int ossoHasFMRadio() {
	return 0;
}


#endif
