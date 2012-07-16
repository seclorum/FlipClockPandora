/*  flip SDL clock
******************************************************************
  fc3.c
  Nokia N810
  
  gcc -g -Wall `sdl-config --cflags` `sdl-config --libs` -L /usr/include/SDL -lSDL_image fc3.c -o fc
  Ciro Ippolito 2009
  ******************************************************************
*/

/*2DO
one png for the entire number sets
...
...
********************************************************************
*/

#include <stdlib.h>
#include <time.h>
#include "SDL_image.h"
#if defined(_MSC_VER)
#include "SDL.h"
#else
#include "SDL/SDL.h"
#endif

SDL_Surface *screen;
SDL_Surface *secs;
SDL_Surface *fondo;
SDL_Surface *zero;
SDL_Surface *uno;
SDL_Surface *due;
SDL_Surface *tre;
SDL_Surface *quattro;
SDL_Surface *cinque;
SDL_Surface *sei;
SDL_Surface *sette;
SDL_Surface *otto;
SDL_Surface *nove;
SDL_Surface *numbers[10];
SDL_Surface *elements[10];


void initBMP()
{
    elements[0] = IMG_Load("fondo.png");
    elements[1] = IMG_Load("barra.png");
    elements[2] = IMG_Load("secs.png");
    numbers[0]  = IMG_Load("0.png");
    numbers[1]  = IMG_Load("1.png");
    numbers[2]  = IMG_Load("2.png");
    numbers[3]  = IMG_Load("3.png");
    numbers[4]  = IMG_Load("4.png");
    numbers[5]  = IMG_Load("5.png");
    numbers[6]  = IMG_Load("6.png");
    numbers[7]  = IMG_Load("7.png");
    numbers[8]  = IMG_Load("8.png");
    numbers[9]  = IMG_Load("9.png");
}

void LoadBMP(char *file, SDL_Surface *surface)
{
 surface= SDL_LoadBMP(file);
   if ( surface == NULL ) {
        fprintf(stderr, "Couldn't load %s: %s\n", file, SDL_GetError());
        return;
    }
  return;
}

void BMPShow(SDL_Surface *screen, SDL_Surface *pic,int x, int y)
{
  SDL_Rect dest;
  dest.x = x;
  dest.y = y;
  dest.w = pic->w;
  dest.h = pic->h;
  SDL_BlitSurface(pic, NULL, screen, &dest);
    // Update the changed portion of the screen 
    SDL_UpdateRects(screen, 1, &dest);
  
  return;
}
void BMPShowArray(SDL_Surface *screen, SDL_Surface *pic,int x, int y)
{
  SDL_Rect dest;
  dest.x = x;
  dest.y = y;
  dest.w = pic->w;
  dest.h = pic->h;
  SDL_BlitSurface(pic, NULL, screen, &dest);
  SDL_UpdateRects(screen, 1, &dest);
  return;	
}
void ShowBMP(char *file, SDL_Surface *screen, int x, int y)
{
    SDL_Surface *image;
    SDL_Rect dest;
    image = SDL_LoadBMP(file);
    if ( image == NULL ) {
        fprintf(stderr, "Couldn't load %s: %s\n", file, SDL_GetError());
        return;
    }
    dest.x = x;
    dest.y = y;
    dest.w = image->w;
    dest.h = image->h;
    SDL_BlitSurface(image, NULL, screen, &dest);
    SDL_UpdateRects(screen, 1, &dest);
}

Uint32 TimerFunc( Uint32 interval, void *p )
{
    printf("ciao modno\n");
    return 1000;
}

int checknumbers(char number,int x, int y){
    switch (number){
        case '0':
            BMPShow (screen,numbers[0] , x   ,y);
            break;
        case '1':
            BMPShow (screen,numbers[1] , x   ,y);
            break;
        case '2':
            BMPShow (screen,numbers[2] , x   ,y);
            break;
        case '3':
            BMPShow (screen,numbers[3] , x   ,y);
            break;
        case '4':
            BMPShow (screen,numbers[4] , x   ,y);
            break;
        case '5':
            BMPShow (screen,numbers[5] , x   ,y);
            break;
        case '6':
            BMPShow (screen,numbers[6] , x   ,y);
            break;
        case '7':
            BMPShow (screen,numbers[7] , x   ,y);
            break;
        case '8':
            BMPShow (screen,numbers[8] , x   ,y);
            break;
        case '9':
            BMPShow (screen,numbers[9] , x   ,y);
            break;
    }    
return 1;
    
}

Uint32 UpdateClock (Uint32 interval, void *p)
  {
    char s[30];
    size_t i;
    struct tm tim;
    time_t now;
    now = time(NULL);
    tim = *(localtime(&now));
    i = strftime(s,30,"%b %d, %Y; %H:%M:%S\n",&tim);
    checknumbers(s[14],  2,20);                             //crappier time routine
    checknumbers(s[15],193,20);                             //crappier time routine 2
    checknumbers(s[17],415,20);                             //crappier time routine  3
    checknumbers(s[18],605,20);                             //crappier time routine   4
    BMPShow (screen,elements[1] ,0   ,0);                   //Redraw the top minute bar
    BMPShow (screen,elements[2] , tim.tm_sec*13+10 ,0);     //Seconds arrow
    return 1000;
  }
/* ----------------------------------------------------------------------------
MAIN ()
 ----------------------------------------------------------------------------*/
int main(int argc, char *argv[])
{
    if ( SDL_Init(SDL_INIT_EVERYTHING) < 0 ) 
    {
        fprintf(stderr, "Unable to init SDL: %s\n", SDL_GetError());
        exit(1);
    }

    SDL_Init(SDL_INIT_TIMER);
    // Register SDL_Quit to be called at exit; makes sure things are
    atexit(SDL_Quit); 								// cleaned up when we quit.

    //screen = SDL_SetVideoMode(800, 480, 16, SDL_SWSURFACE|SDL_FULLSCREEN);  // Full screen Tablet
    screen = SDL_SetVideoMode(800, 480, 16, SDL_SWSURFACE);                   // Window PC
    if ( screen == NULL )     							                      // If fail, return error
    {
        fprintf(stderr, "Unable to set 800x480 video: %s\n", SDL_GetError());
        exit(1);
    }
    initBMP();
    BMPShow (screen, elements[0] ,0   ,0);        //Fondo
    BMPShow (screen, elements[1] ,0   ,0);        //barra minuti
        char s[30];
        size_t i;
        struct tm tim;
        time_t now;
        now = time(NULL);
        tim = *(localtime(&now));
        i = strftime(s,30,"%b %d, %Y; %H:%M:%S\n",&tim);
        checknumbers(s[14],  2,20);
        checknumbers(s[15],193,20);
        checknumbers(s[17],415,20);
        checknumbers(s[18],605,20);
    SDL_AddTimer (1000,UpdateClock,NULL); ///////////////////////////////////////////// ////
    typedef enum { FALSE, TRUE } boolean;
    boolean clockticking = TRUE;
    while (clockticking)					        // Main loop: loop forever
    {
        SDL_Event event;			                // Poll for events
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_KEYDOWN:
                if (event.key.keysym.sym == SDLK_a)	{
                printf("premuto A");
            }
            else if (event.key.keysym.sym == SDLK_s){
                printf("premuto S");
            }
            else if (event.key.keysym.sym == SDLK_d){
                printf("premuto D");
            }
            else if (event.key.keysym.sym == SDLK_f){
                printf("premuto F");
            }
            else if (event.key.keysym.sym == SDLK_g)
                BMPShow (screen, elements[0]  ,0    ,0);
            case SDL_KEYUP:
            if (event.key.keysym.sym == SDLK_q)		// If Q is pressed, return (and zen quit)
                return 0;
                break;     
            case SDL_QUIT:
                return(0);
            }
        }
    }
    return 0;
}


/*let's check is I can still fly  */

