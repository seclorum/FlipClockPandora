
/*

Flip Clock GStreamer based Media Playerback Module
Part of FlipClock C SDL for Maemo.

This is the media playback module for FlipClockC. It's pretty basic at the moment
but could grow in complexity if required in the future. It's loosely based
on my previous GStreamer concepts from the old Python version of Flip, mashed
together with the examples from the Maemo 5 SDK:
https://garage.maemo.org/svn/maemoexamples/trunk/maemo-examples/example_wavlaunch.c

-Rob Williams, Sept 27, 2009.


*/




#include <gtk/gtk.h>

#include <stdlib.h>
#include <gst/gst.h>

//****************** Structure defs *******************************************//

typedef struct _alarmSoundObj alarmSoundObj;


struct _alarmSoundObj {
	 GstElement *pipeline;		//Pipeline for the sound
	GstElement *source;
	GstElement *decoder;
	GstElement *converter;
	GstElement *sink;
	 char *fileType;				//Type of file, for now only MP3s
	 int looping;					//should file loop or not?
	 int stepTime;					//used for fading sound
	 int maxVol;					//used for fading sound
	 int startTime;					//Time that last play command was issued... this way we can compare against this to prevent loop lockups...

};

//****************** Done structure defs **************************************//

//****************** Global variables ******************************************//

char sinkModule[30];	//Name of sink module to use (dspmp3, pulseaudio, alsa); detected automatically

char decoder[30]; 	//Name of MP3 decoder

char playbin[30]; 	//Name of playbin 

int playerReady = 0;

int canDecodeMP3 = 0;

int bahbah = 0;

int soundFileIndex = -5;

int gstHasError = 0;

//****************** Done global vars *****************************************//



int playPipe(GstElement *playerPipe);

int playAlarm(alarmSoundObj *alarmObj);

int stopPipe(GstElement *playerPipe);

int stepVolumeUp( alarmSoundObj *alarmSound);

void cleanupPlayer(alarmSoundObj *alarmSound);

int playPipeTO(GstElement *playerPipe);

int loadFile (char *file, GstElement *playerPipe);

int getPipeVolume(GstElement *playerPipe);

int setPipeVolume(GstElement *playerPipe, int volLevel);

int fadeSoundIn( alarmSoundObj *alarmSound, int millisecondsIn, int maxVol);

//****************** Function Definitions **************************************//


/*********************************************************
* void  cb_message_error(GstBus * bus, GstMessage * message,GstElement *pipeline) ;
*
* Global function that gets called by a steam whenever an error occurs
* 
*
**********************************************************/
void cb_message_error (GstBus * bus, GstMessage * message, gpointer data) {
    GError *err = 0;
    gchar *debug;

	
	printf("Got error %s message\n", GST_MESSAGE_TYPE_NAME (message));

    gst_message_parse_error (message, &err, &debug);
    printf("Error: %s\n", err->message);
    g_error_free(err);
    g_free (debug);
	
	gstHasError = 1;
}

/*********************************************************
* void  eos_message_received(GstBus * bus, GstMessage * message,GstElement *pipeline) ;
*
* Global function that gets called by a steam whenever it reaches end of file (end of stream)
* 
*
**********************************************************/
void eos_message_received (GstBus * bus, GstMessage * message, alarmSoundObj *alarmObj) {
	GstState currentState = 0;
	char * soundFile = NULL;
	int oldVol = 0;

	if (message->type == GST_MESSAGE_EOS && gstHasError == 0) {
		if (alarmObj->looping == 1) {
			gst_element_get_state(alarmObj->pipeline, &currentState, NULL, 0);
			
			printf("state: %i\n", currentState);
			printf("looping\n");
			
			bahbah++;
			
			//if (bahbah < 5) {
				//g_timeout_add(5, (GSourceFunc) playPipeTO, alarmObj->pipeline);
				oldVol = getPipeVolume(alarmObj->pipeline);
				setPipeVolume(alarmObj->pipeline, 0);
			
				if (time(NULL) - alarmObj->startTime < 2) {
					printf("not enough time passed, possible runaway! delaying sound restart\n");
					//not enough time has gone by, so to prevent run away loops let's take a second before we retry
					g_timeout_add(2000, (GSourceFunc) playAlarm, alarmObj);
				} else {
			
					playAlarm(alarmObj);
				}
				
				//playPipe(alarmObj->pipeline);
				setPipeVolume(alarmObj->pipeline, oldVol);
				
				gst_element_get_state(alarmObj->pipeline, &currentState, NULL, 0);
				
				//printf("state: %i\n", currentState);
			//}
			
		} else if (alarmObj->looping == 2) {
			printf("playlsit looping!\n");
			//Loop mode 2 == playlist
			
			//this is terribly wrong and shouldn't be here, but...
			if (runningAlarmIndex != -1) {
				soundFile = getIndexedMP3FromPath(userPreferences.userAlarms[runningAlarmIndex]->sound, soundFileIndex);
			
				if (soundFile == NULL) {
					//File is too high, reset
					soundFileIndex = 0;
					
					soundFile = getIndexedMP3FromPath(userPreferences.userAlarms[runningAlarmIndex]->sound, soundFileIndex);
				}
			
				if (soundFile != NULL) {
					oldVol = getPipeVolume(alarmObj->pipeline);
					loadFile(soundFile, alarmObj->pipeline);
					setPipeVolume(alarmObj->pipeline, 0);
					
					//playPipe(alarmObj->pipeline);
					if (time(NULL) - alarmObj->startTime < 2) {
						printf("not enough time passed, possible runaway! delaying sound restart\n");
						//not enough time has gone by, so to prevent run away loops let's take a second before we retry
						g_timeout_add(2000, (GSourceFunc) playAlarm, alarmObj);
					} else {
				
						playAlarm(alarmObj);
					}
					setPipeVolume(alarmObj->pipeline, oldVol);
					//fadeSoundIn(alarmObj, 5000, 10);
					//gst_element_get_state(alarmObj->pipeline, &currentState, NULL, 0);
					
					free(soundFile);
				}
				
				if (soundFileIndex != -1) {
					soundFileIndex++;
				}
			}
			
			
		
		
		}	else {
			//Nope we're done so clean up
			//cleanupPlayer(alarmObj);
			//Stop the alarm since we're done
			stopAlarm();
			
		}
	}
	
}


/*********************************************************
* gstHasModule(char *modName)
*
* Tries to detect if gstreamer plugin of name modName is installed and available on the system
*
**********************************************************/

int gstFindBestSink(char *modName) {
	GstRegistry *thisReg;
	GList *pluginList, *current;
	
	
	const gchar *plugName;
	
	int hasDSP = 0;
	int hasFileSrc= 0;
	int hasPlaybin = 0;
	int hasPulse = 0;
	int hasAlsa = 0;
//	int hasMad = 0;


	thisReg = gst_registry_get_default(); 
	pluginList = gst_registry_get_plugin_list(thisReg);
	
	//Loop over the plugins
	current = g_list_first(pluginList);
	
	while (current != NULL) {

debugEntry("Plugin:[");
//debugEntry(plugName);
printf("plugname: %s\n", plugName);
debugEntry("]\n");

		plugName = gst_plugin_get_name(current->data);

//		if (!strcmp(plugName,"filesrc")) {
			hasFileSrc = 1;
//		}
	
#if 0
		if (!strcmp(plugName,"playbin")) {
			hasPlaybin = 1;
		}

		if (!strcmp(plugName,"dspmp3")) {
			hasDSP = 1;
		}
		if (!strcmp(plugName,"pulseaudio")) {
			hasPulse = 1;
		}
		if (!strcmp(plugName,"alsa")) {
			hasAlsa = 1;
		}
		
		if (!strcmp(plugName,"mad")) {
			canDecodeMP3 = 1;
			hasMad = 1;
		}

		// Meamo 5 mp3 decoder...
		if (!strcmp(plugName,"nokiamp3dec")) {
			canDecodeMP3 = 1;
		}
#endif
		//printf("gst plugin is %s\n", gst_plugin_get_name(current->data));
		current = g_list_next(current);
	}
	
	//Free up
	gst_plugin_list_free(pluginList);
	
canDecodeMP3 = 1;
//hasPlaybin = 1;
hasFileSrc = 1;

	bzero(sinkModule, sizeof(sinkModule));
	// Set decoder
	bzero(decoder, sizeof(decoder));

	if (hasDSP) {
		//best choice is DSP
		sprintf(sinkModule, "dspmp3sink");
	} else if (hasFileSrc) {
		sprintf(sinkModule, "alsasink");	
		sprintf(decoder, "decodebin");	
	} else if (hasPlaybin) {
		//sprintf(decoder, "playbin");
	} else if (hasPulse) {
		sprintf(sinkModule, "pulseaudio");
	} else if (hasAlsa) {
		sprintf(sinkModule, "alsa");
	}

/*
	if (canDecodeMP3)
	{
		if (hasMad)
		{
			sprintf(decoder, "mad");
		}
		else
		{
			sprintf(decoder, "decodebin2");
		}
	}
*/
	
	printf("best sink mode is %s with decoder %s...", sinkModule, decoder);
	return 1;

}

void on_pad_added (GstElement *t_pad, GstPad * pad, void *data) {
        GstPad * sinkpad;
        GstElement * sink = (GstElement *) data;

        /* We can now link this pad with the converter sink pad */
        sinkpad = gst_element_get_static_pad (sink, "sink");
        gst_pad_link (pad, sinkpad);
        gst_object_unref (sinkpad);
}

/*********************************************************
* GstElement *createGstPipe(char *outputType);
*
* Creates a new GStreamer pipeline for the given playback type
*
**********************************************************/

alarmSoundObj *createPlayer(char *outputType) {
	
	GError *error = 0;
	GstBus *bus;
	alarmSoundObj *alarmSound;
	char launchStr[255];
	bzero(launchStr, sizeof(launchStr));
	alarmSound = g_new0(alarmSoundObj, 1);
	alarmSound->pipeline = NULL;	
	alarmSound->looping = 0;	//No loop by default
	//Right now only mp3s, but ogg/etc could be added later theoretically...
	if (strcmp(outputType, "mp3") == 0)
	{
//		if (strcmp(sinkModule, "filesrc") == 0)
		{
		// gst-launch filesrc location=/tmp/ringer.mp3 ! decodebin ! audioconvert ! alsasink 
//	filesrc name=player location=/tmp/ringer.mp3 ! ! audioconvert ! alsasink name=sink
//		sprintf(launchStr, "filesrc name=player location=/tmp/ringer.mp3 ! %s! audioconvert ! %s name=sink", decoder, sinkModule);
		sprintf(launchStr, "filesrc name=launch-source location=/tmp/ringer.mp3 ! %s! audioconvert ! %s name=sink", decoder, sinkModule);

		printf("The launch string is [%s]", launchStr);
		alarmSound->pipeline = gst_parse_launch (launchStr, &error);
/*
		alarmSound->source    = gst_element_factory_make ("filesrc",       "file-source");
		alarmSound->decoder   = gst_element_factory_make ("decodebin",     "decoder-bin");
		alarmSound->converter = gst_element_factory_make ("audioconvert", "converter");
		alarmSound->sink      = gst_element_factory_make ("alsasink", "audio-output");

		alarmSound->pipeline = gst_pipeline_new("audio-player");
        gst_bin_add_many (GST_BIN (alarmSound->pipeline), alarmSound->source, alarmSound->decoder, alarmSound->converter, alarmSound->sink, NULL);
        gst_element_link (alarmSound->source, alarmSound->decoder);
        g_signal_connect (alarmSound->decoder, "pad-added", G_CALLBACK (on_pad_added), alarmSound->converter);
        gst_element_link (alarmSound->converter, alarmSound->sink);
*/
		}
	
		if(alarmSound->pipeline == NULL)
		{
			printf("Error couldn't create pipeline for whatever reason...\nError is [%s]\n", error->message);
		}
	}

	if (alarmSound->pipeline != NULL) {
	
		//Setup end of file handler
		/* setup message handling */
		bus = gst_pipeline_get_bus (GST_PIPELINE (alarmSound->pipeline));
		gst_bus_add_signal_watch_full (bus, G_PRIORITY_HIGH);
		g_signal_connect (bus, "message::error", (GCallback) (cb_message_error), NULL);
		g_signal_connect (bus, "message::eos", (GCallback) eos_message_received, alarmSound);

		gst_object_unref (GST_OBJECT(bus));
		playerReady = 1;
		printf("Audio player is ready\n");
	}
	else
	{
		printf("Error occured we got no pipe...\n");
	}
	
	return alarmSound;
}


/*********************************************************
* void cleanupPlayer(alarmSoundObj *alarmSound);
*
* Cleans up and frees a previously created alarmSound object
*
**********************************************************/

void cleanupPlayer(alarmSoundObj *alarmSound) {
	if (playerReady != 0) {
		if (alarmSound != NULL) {
			//Clean up the player pipe
			if (alarmSound->pipeline != NULL) {
				stopPipe(alarmSound->pipeline);
				
				gst_object_unref(GST_OBJECT(alarmSound->pipeline));
				alarmSound->pipeline = NULL;
			}
	
			free(alarmSound);
	
			alarmSound = NULL;
		}
		playerReady = 0;
	}
}


/*********************************************************
* int loadFile createGstPipe(char *file, GstElement *playerPipe);
*
* Tries to load a file into the given playerPipe
*
**********************************************************/

int loadFile (char *file, GstElement *playerPipe) {

	GstElement *audioSrc = NULL;

	printf("loadFile: file is %s\n", file);

	if (playerPipe->current_state == GST_STATE_PLAYING) {
		//We need to stop the pipe from playing
		stopPipe(playerPipe);
		
	}

	audioSrc = gst_bin_get_by_name (GST_BIN (playerPipe), "launch-source");
	if (!audioSrc)
	{
		printf("bin_get_by_name failed\n");
		printf("Couldn't load file %s for playback.\n", file);
	}
	else
	{
    	g_object_set (G_OBJECT (audioSrc), "location", file, NULL);
//	g_object_set(G_OBJECT(audioSrc), "uri", file, NULL);	
//	g_object_set(G_OBJECT(playerPipe), "uri", file, NULL);

		printf("set uri as %s\n", file);
	}


	return 1;
}

/*********************************************************
* int setLoopMode(struct alarmSoundObj, int loopMode);
*
* Sets looping on or off for the given alarmSoundObj
*
**********************************************************/

int setLoopMode(int loopMode, alarmSoundObj *alarmSound) {
	alarmSound->looping = loopMode;
	printf("looping %i\n", loopMode);
	return 1;
}


/*********************************************************
* int playPipe(GstElement *playerPipe);
*
* Tries to play the given pipe 
*
**********************************************************/

int playPipe(GstElement *playerPipe) {
	GstStateChangeReturn result;

	//Turn off the global error flag
	gstHasError = 0;

	gst_element_set_state (playerPipe, GST_STATE_READY);
	result = gst_element_set_state (playerPipe, GST_STATE_PLAYING);
	printf("play attempt result %i\n", result);
	

	return 1;
}

/*********************************************************
* int playPipe(GstElement *playerPipe);
*
* Tries to play the given pipe (from a timeout)
*
**********************************************************/

int playPipeTO(GstElement *playerPipe) {
	playPipe(playerPipe);
	return FALSE;
}


/*********************************************************
* int stopPipe(GstElement *playerPipe);
*
* Tries to stop the given pipe 
*
**********************************************************/

int stopPipe(GstElement *playerPipe) {
	GstState currentState = 0;
	if (playerReady == 1) {
		gst_element_get_state(playerPipe, &currentState, NULL, 0);
		if (currentState != GST_STATE_PAUSED) {
			gst_element_set_state (playerPipe, GST_STATE_PAUSED);
		}
		gst_element_get_state(playerPipe, &currentState, NULL, 0);
		if (currentState != GST_STATE_NULL) {
			gst_element_set_state (playerPipe, GST_STATE_NULL);
		}
	}
	return 1;
}

/*********************************************************
* int playAlarm(GstElement *playerPipe);
*
* Tries to play the given alarm object
*
**********************************************************/

int playAlarm(alarmSoundObj *alarmObj) {

	if (alarmObj->pipeline != NULL) {
		playPipe(alarmObj->pipeline);
	}
	alarmObj->startTime = time(NULL);
	
	return FALSE;
}

/*********************************************************
* int setPipeVolume(GstElement *playerPipe);
*
* Tries to set the volume of the given pipe
**********************************************************/

int setPipeVolume(GstElement *playerPipe, int volLevel) {
	gdouble realVol = 0;
	int useSinkVol = 0;			//Are we controlling volume element vol, or sink element volume? (i.e. tablet)
	GstElement *audioSink = NULL;
	
	audioSink = gst_bin_get_by_name (GST_BIN (playerPipe), "volume");
	if (!audioSink) {
		//No volume plugin, so grab the sink instead
		audioSink = gst_bin_get_by_name (GST_BIN (playerPipe), "sink");
	
		if (!audioSink) {
	    	printf ("Parse error: volume not possible, no sink\n");
			return 0;
		} else {
			useSinkVol = 1;
		}
	}
	
	if (0 <= volLevel  && volLevel <= 100) {
		realVol = volLevel / 100.0;
		if (useSinkVol == 0) {
			g_object_set(G_OBJECT (audioSink), "volume", realVol, NULL);
		} else {
			g_object_set(G_OBJECT (audioSink), "fvolume", realVol, NULL);
		}
		return 1;
	}
	
	return 0;
}

/*********************************************************
* int getPipeVolume(GstElement *playerPipe);
*
* Tries to get the volume of the given pipe
**********************************************************/

int getPipeVolume(GstElement *playerPipe) {
	
	int realVol = 0;
	gdouble rawVol	=0;
	int useSinkVol = 0;			//Are we controlling volume element vol, or sink element volume? (i.e. tablet)

	GstElement *audioSink = NULL;
	
	audioSink = gst_bin_get_by_name (GST_BIN (playerPipe), "vol");
	if (!audioSink) {
		//No volume plugin, so grab the sink instead
		audioSink = gst_bin_get_by_name (GST_BIN (playerPipe), "sink");
	
		if (!audioSink) {
	    	printf ("Parse error: no sink\n");
			return 0;
		} else {
			useSinkVol = 1;
		}
	}

	if (useSinkVol == 0) {
		g_object_get(G_OBJECT (audioSink), "volume", &rawVol, NULL);	
			
	} else {
		g_object_get(G_OBJECT (audioSink), "fvolume", &rawVol, NULL);	
		//realVol = (rawVol / 65535.0) * 100;
	}
	realVol = rawVol * 100;
	
	return realVol;

}


/*********************************************************
* int fadeSoundIn( alarmSoundObj *alarmSound, int millisecondsIn, int maxVol);
*
* Begins fading a sound in over millisecondsIn to maxVol
**********************************************************/

int fadeSoundIn( alarmSoundObj *alarmSound, int millisecondsIn, int maxVol) {
	
	//Start by setting vol low
	setPipeVolume(alarmSound->pipeline, 5);
	
	//Start playing
	//playPipe(alarmSound->pipeline);
	playAlarm(alarmSound);
	
	alarmSound->stepTime = millisecondsIn / 20;		//5 steps by default
	alarmSound->maxVol = maxVol;
	
	g_timeout_add(alarmSound->stepTime, (GSourceFunc) stepVolumeUp, alarmSound);

	return 1;
}

/*********************************************************
* int stepVolumeUp(alarmSoundObj *alarmSound);
*
*Fades an alarm sound up
**********************************************************/

int stepVolumeUp( alarmSoundObj *alarmSound) {
	int currVol = 0;
	
	
	if (alarmSound != NULL) {
		if (playerReady == 1) {
			//Get the current volume
			currVol = getPipeVolume(alarmSound->pipeline);
			
			if (currVol < alarmSound->maxVol) {
				//Start by setting vol low
				setPipeVolume(alarmSound->pipeline, currVol + 5);
				
						
				g_timeout_add(alarmSound->stepTime, (GSourceFunc) stepVolumeUp, alarmSound);
			}
		}
	}
	return FALSE;

}
