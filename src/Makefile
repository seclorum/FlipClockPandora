


all: fcT gtkhak

build:
	ssh -X ibi@10.1.1.2 'make -C /media/SD16/pandora/hak/flipclock/src/ fcT'

fcT:	fc4.c alarmDFunc.c bme.c fcFMRadio.c n800FMRadio.c xmlFunc.c \
	alarmDv2Func.c fc3.c fileDirFunc.c ossoHelper.c \
	alarmFunc.c	fc4.c miscFunc.c themeManager.c \
	audioFunc.c	fcAlarmSync.c mouseFunc.c userPrefs.c 
	gcc -g -Wall -DDEBUGENABLED=1 -DPREFSPATH=\"./flipclock/\" -DTHEMESPATH=\"./flipclock/themes/\" -DMEDIAPATH=\"./flipclock/media/\" -DPROGNAME=\"flipclock\" -DPROGVERSION=1.0 -DMAEMOVERSION=4 `sdl-config --cflags` `pkg-config --cflags  gstreamer-0.10 gtk+-2.0 glib-2.0` `sdl-config --libs` `pkg-config --libs  gstreamer-0.10 gtk+-2.0 glib-2.0` -L/usr/lib -I/usr/include/SDL -lSDL_image -lSDL_ttf -lSDL_gfx -lpthread -lrt \
	\
	fc4.c \
	-o fcT	 2>&1 | tee make.out

gtkhak:	gtkhak.c
	gcc -g -Wall -DDEBUGENABLED=1 -DPREFSPATH=\"./flipclock/\" -DTHEMESPATH=\"./flipclock/themes/\" -DMEDIAPATH=\"./flipclock/media/\" -DPROGNAME=\"flipclock\" -DPROGVERSION=1.0 -DMAEMOVERSION=4 `sdl-config --cflags` `pkg-config --cflags  gstreamer-0.10 gtk+-2.0 glib-2.0` `sdl-config --libs` `pkg-config --libs  gstreamer-0.10 gtk+-2.0 glib-2.0` -L/usr/lib -I/usr/include/SDL -lSDL_image -lSDL_ttf -lSDL_gfx -lpthread -lrt \
	\
	gtkhak.c \
	-o gtkhak

clean:	
	rm -rf fcT *.o *.?~


