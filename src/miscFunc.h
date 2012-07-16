/********** Misc. Functions Headers
*
* Utilized as part of FlipClock C by Rob Williams and Ciro Ippoto
*
**********************************/

#ifndef MISCFUNC_H
#define MISCFUNC_H

//********************** Global vars *********************************//

struct imageLabel *sliderPointerLabel = NULL; 

//********************** Done global vars ***************************//
#define MAEMOVERSION 4
#if MAEMOVERSION==4
#define MIN_BRIGHTNESS_TOGGLE 3
#endif
#if MAEMOVERSION==5
#define MIN_BRIGHTNESS_TOGGLE 2
#endif

//********************** Structure Definitions ***********************//




//********************** Done Struct Defs *************************//

//********************** Function Headers *************************//

void drawMoodBox(SDL_Surface *screenBuff); //Function headers

Uint32 getPixel(SDL_Surface *surface, int x, int y);

void closeSlider();

void slideSlider();

//********************** Done Function Headers ********************//
#endif
