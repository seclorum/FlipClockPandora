/***************** Osso Helper functions Header File

Functions that allow flipclock to be tied into OSSO functionality (i.e. hardware monitoring, etc).

These functions should be seamlessly disabled if osso isn't present, allowing flip to run on any other OS that has
the appropriate SDL libs. (just without as much coolness)

*/



//************** Function Headers **************************//

int ossoFlipInit();			//Initialize the app for osso functionality

int ossoFlipDeInit();		//Clean up osso functionality

void ossoDeviceScreenOn();		//Function to turn the tablet screen on

void ossoKillAlarmSync();		//function to kill alarmDSync App

//************** Done Function Headers *********************//

//************** Constant Defs *******************************//

#if defined(LIBOSSO_H_)
#define USE_OSSO 1
#else
#define USE_OSSO 0
#endif


//************** Done constant Defs ***************************//
	
#if defined(LIBOSSO_H_)
//Only allow to be included if libosso is present

//Iclude the hildon file manager
#include <hildon/hildon-file-chooser-dialog.h>


//************** Define global osso-related vars ********************//
osso_context_t* ossoAppContext;	//Osso context of the program

//************** Done global osso-related vars **********************//

//************** Define osso-specific function headers ****************//

//***** DBUS message handlers **********
gint dbus_req_handler(const gchar * interface, const gchar * method, GArray * arguments, gpointer data, osso_rpc_t * retval);

void ossoAddUpdatedAlarms(DBusMessage *msg, gpointer updatedAlarms);

//*************** Done osso-specific functino headers ****************//

#endif
