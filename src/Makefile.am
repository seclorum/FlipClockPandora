#
# FlipClock - Full screen finger friendly alarm clock.
# 
# Copyright (C) 2008-2010 Rob Williams, Spencer Lai
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
MAINTAINERCLEANFILES = Makefile.in

bin_PROGRAMS=flipclock flipAlarmSync flipFMRadio

flipclock_SOURCES=fc4.c
flipclock_LDADD=${GTK_LIBS} ${OSSO_LIBS} ${GLIB_LIBS} ${GCONF_LIBS} ${GSTREAMER_LIBS} ${HILDON_LIBS} ${SDL_LIBS}
flipclock_CFLAGS=${GTK_CFLAGS} ${OSSO_CFLAGS} ${GLIB_CFLAGS} ${GCONF_CFLAGS} ${GSTREAMER_CFLAGS} ${HILDON_CFLAGS} ${SDL_CFLAGS} ${FC_OPTIONS_CFLAGS} ${FC_PATHS_CFLAGS} ${OS_VERSION_CFLAGS}

flipAlarmSync_SOURCES=fcAlarmSync.c
flipAlarmSync_LDADD=${GLIB_LIBS} ${OSSO_LIBS} ${ALARM_LIBS}
flipAlarmSync_CFLAGS=${GLIB_CFLAGS} ${OSSO_CFLAGS} ${ALARM_CFLAGS} ${FC_PREFSPATH_CFLAGS} ${OS_VERSION_CFLAGS} 

flipFMRadio_SOURCES=fcFMRadio.c 
flipFMRadio_LDADD=${GLIB_LIBS} ${OSSO_LIBS} 
flipFMRadio_CFLAGS=${GLIB_CFLAGS} ${OSSO_CFLAGS} ${OS_VERSION_CFLAGS}
