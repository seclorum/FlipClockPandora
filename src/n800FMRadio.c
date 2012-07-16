/*
 * Copyright 2009 Danilo F. S. Santos <danilo.santos@signove.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <string.h>
#include <math.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <assert.h>

#include "n800FMRadio.h"

#define DEFAULT_DEVICE "/dev/radio0"
#define DEFAULT_MIXER  "/dev/mixer"

/*******************************************************************************/
#define 	_(String) 	gettext(String)
#define	FREQUENCY_MIN	87500
#define	FREQUENCY_MAX	108000
#define	FREQUENCY_STEP	5
#define 	FORMAT		"%06.2f"
#define 	FREQ_FRAC	16     /* For Nokia N800 only */
#define 	STATION_NAME_MAX_LENGTH	15
#define 	STATION_FREQ_MAX_LENGTH	6
#define 	SAMPLEDELAY     	15000
#define 	THRESHOLD 	65535.0   /* maximum acceptable signal %    */
#define 	ACCEPTLEVEL 	0.5
/*******************************************************************************/

int radio_open(RadioDescriptor *radioDesc)
{          
	// Open the Radio device.
          if ((radioDesc->radio_fd = open(DEFAULT_DEVICE, O_RDONLY))< 0)
          {
                  goto error;
          }
      	// Query Radio device capabilities.
          if (ioctl(radioDesc->radio_fd, VIDIOC_QUERYCAP, &(radioDesc->vc))<0)
          { 
              goto error;
          }
      	// Set tuner index. Must be 0.
          (radioDesc->vt).index = 0;
          ioctl(radioDesc->radio_fd, VIDIOC_S_TUNER, &(radioDesc->vt));
          
          if (!((radioDesc->vc).capabilities & V4L2_CAP_TUNER))
          {
              goto error;
          }
      	// Open the Mixer device.  
          radioDesc->mixer_fd = open(DEFAULT_MIXER, O_RDWR);
          if (radioDesc->mixer_fd < 0)
          {
                  goto error;
          }
          return 1;

error:
         if (radioDesc->radio_fd >= 0)
         {
                 close(radioDesc->radio_fd);
                 radioDesc->radio_fd = -1;
         }
         return 0;
}

void radio_close(RadioDescriptor *radioDesc)
{
	
	radio_mute(radioDesc);
	if (radioDesc->mixer_fd >=0)
	{
	  close(radioDesc->mixer_fd);
	  radioDesc->mixer_fd = -1;
	}
	if (radioDesc->radio_fd >= 0)
	{
	  close(radioDesc->radio_fd);
	  radioDesc->radio_fd = -1;
	}
}

int radio_setfreq(RadioDescriptor* radioDesc, unsigned int freq)
{	
	int res;
	
	if (radioDesc->radio_fd < 0)
		return -1;
	(radioDesc->vf).tuner = 0;
	(radioDesc->vf).frequency = (freq*FREQ_FRAC);
 
	res = ioctl(radioDesc->radio_fd, VIDIOC_S_FREQUENCY, &(radioDesc->vf));
	
	if(res > 0)
		radioDesc->freq = freq;
	
	return res;
}

unsigned int radio_getfreq(RadioDescriptor* radioDesc)
{
	int res;
	unsigned long freq;
	
	if (radioDesc->radio_fd < 0)
		return -1;
	
	res = ioctl(radioDesc->radio_fd, VIDIOC_G_FREQUENCY, &(radioDesc->vf));
	
	if(res < 0)
		return -1;
	
	freq = (radioDesc->vf).frequency;
	freq /= FREQ_FRAC;
	
	radioDesc->freq = freq;
	
	return freq;
}

void radio_mute(RadioDescriptor* radioDesc)
{
	int res;
	(radioDesc->vctrl).id = V4L2_CID_AUDIO_MUTE;
	(radioDesc->vctrl).value = 1;
	res = ioctl(radioDesc->radio_fd, VIDIOC_S_CTRL, &(radioDesc->vctrl));
	if( res > 0 )
	{
		radioDesc->silent = 1;
	}
}

void radio_unmute(RadioDescriptor* radioDesc)
{
	int res;
	(radioDesc->vctrl).id = V4L2_CID_AUDIO_MUTE;
	(radioDesc->vctrl).value = 0;
	res = ioctl(radioDesc->radio_fd, VIDIOC_S_CTRL, &(radioDesc->vctrl));
	if( res > 0 )
	{
		radioDesc->silent = 0;
	}
}

void radio_setvolume(RadioDescriptor* radioDesc, unsigned int vol)
{
	int res;

	vol |= (vol << 8);
	res = ioctl(radioDesc->mixer_fd, SOUND_MIXER_WRITE_LINE, &vol);
	if(res > 0)
	{
		radioDesc->vol = vol;
	}
}

unsigned int radio_getvolume(RadioDescriptor* radioDesc)
{
	unsigned int vol = 99999;
	int res;
	
	if (radioDesc->mixer_fd < 0)
	{	
		goto error;
	}
	printf("before got vol %u\n", vol);
	res = ioctl(radioDesc->mixer_fd, SOUND_MIXER_READ_LINE, &vol);
	vol = vol >> 8;
	if(vol !=  99999)
	{
		radioDesc->vol = vol;
		return vol;
	}

error:
	return -1;
}

int radio_get_signal_strength(RadioDescriptor* radioDesc)
{
        if (ioctl(radioDesc->radio_fd, VIDIOC_G_TUNER, &(radioDesc->vt)) < 0)
        {
                return -1;
        }
        usleep(SAMPLEDELAY);
        return radioDesc->vt.signal;
}





