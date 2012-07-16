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

#ifndef __RADIO_CALLS_H__
#define __RADIO_CALLS_H__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <linux/soundcard.h>
#include <linux/videodev.h>

typedef struct
{
  /* V4l2 structures */
  struct v4l2_capability vc;
  struct v4l2_tuner vt;
  struct v4l2_control vctrl;
  struct v4l2_frequency vf;

  int radio_fd;
  int mixer_fd;

  unsigned int freq;
  unsigned int vol;

  int silent;

} RadioDescriptor;

int   radio_open(RadioDescriptor* radioDesc);

void  radio_close(RadioDescriptor* radioDesc);

int   radio_setfreq(RadioDescriptor* radioDesc, unsigned int freq);

unsigned 
int   radio_getfreq(RadioDescriptor* radioDesc);

void  radio_mute(RadioDescriptor* radioDesc);

void  radio_unmute(RadioDescriptor* radioDesc);

void  radio_setvolume(RadioDescriptor* radioDesc, unsigned int vol);

unsigned 
int   radio_getvolume(RadioDescriptor* radioDesc);

int   radio_get_signal_strength(RadioDescriptor* radioDesc);

#endif /*__RADIO_CALLS_H__*/

