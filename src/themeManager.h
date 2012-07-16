/*

Flip Clock Theme Manager Header Module
Part of FlipClock C SDL for Maemo.

This is the header file for the theme manager functions

-Rob Williams, Aug 11, 2009.


*/


//********************** Structure Definitions ***********************//

/* Structure for an image label (element) */
struct imageLabel {
	SDL_Rect labelRect;		//Position of the label
	SDL_Surface *imageObj;	//The image itself
	int usesMoodColor;		//Uses mood colouring?
	char *labelKey;		//Key to function to generate the label value
};

/* Structure for a collection of images */
struct imageLabelCollection {
	int labelCount;
	struct imageLabel **labelArr;
};

/* Structure for a text label (element) */
struct textLabel {
	SDL_Rect labelRect;	//Position of the label
	char *labelValue;	//label content (visible text)
	unsigned int textColor[4];	//Array of R,G,B, A for text
	int usesBGColor;	//Use background colour, or show previous background?
	unsigned int textBGColor[5];	//Array of R,G,B, A for text background
	char textSize[20];	//Size of text (small, medium, large)
	int usesMoodColor;	//Does this label use mood colour instead?
	char *labelKey;		//Key to function to generate the label value
	int textAngle;		//Angle to render text at
	int textAlign;		//Alignment of text
};

/* Structure for a collection of text labels */
struct textLabelCollection {
	int labelCount;
	struct textLabel **labelArr;
};

/* Structure for theme summary (used to list available themes) */
struct themeSummary {
	char *themeName;		//Name of theme
	char *themePath;		//Path to theme
};

/* Detailed information about a theme */
struct themeSettings {
	char *themePath;		//Path of current theme
	char *themeName;		//current theme name
	Uint32 themeBackColor;	//Background colour of theme (defaults to black). Used in transitions and things
	int transitionMode;		//Mode for screen transitions (either slide or fade)
	int digitTransMode;		//Mode for main digits to change (0 = animation files, 1 = fade)
	int alarmDigitTransitionMode;	//How should alarm scroller digits change (either slide up/down, or fade)
	int themeIndex;			//index of current theme
	int usesAlphaBG;			//does this theme use transparency on the background for moods
	SDL_Rect digitPos[6];	//Position of clock digits
	SDL_Rect alarmDigitPos[4];	//Position of digit scrollers on alarm settings screen
	SDL_Rect timerDigitPos[6];	//Position of digit scrollers on timer screen
	SDL_Rect alarmDayRect;	//Bounding box for alarm day pickers
	Uint8 alarmDayTextColor[4];	//Color of alarm day text
	int alarmDayTextUseMood;		//Should alarm day text use the mood colour?
	SDL_Rect alarmDayTextOffset;	//Position of alarm day text relative to day image
	int alarmDayUseMood;			//do the alarm day images use mood colour backings?
	int alarmDayTextSize;			//Font size to render text
	
	int secondsBarIncrement;			//Increment for seconds bar (1 = every second, 10 = every 10 seconds, etc)
	
	SDL_Rect alertSettingsRect;		//Bounding box for alert mode settings; needed to make sure BG is replaced when changing alert modes
	
	//struct imageLabelCollection moodImages[CLOCKMODEMAX]; Don't need this, just make one "overall mood mask" per screen...
	
	struct textLabelCollection textLabels[CLOCKMODEMAX];	//Text labels for this theme
	struct imageLabelCollection imageLabels[CLOCKMODEMAX];	//Image labels for this theme (not used yet)
	
	TTF_Font *themeFonts[FONTSIZEMAX];		//Fonts for this theme (different sizes);
	
	SDL_Surface *moodImages[CLOCKMODEMAX];
};



//********************** Done Struct Defs *************************//

//********************** Global Variables ***************************//

struct themeSummary **availableThemes = NULL;	//Dynamic array of available themes
int themeCount = 0;							//Total number of themes
int currentThemeIndex;							//Index of the current theme in the availableThemes array
struct themeSettings currentTheme;				//settings for current theme

struct imageLabel *fmSelector = NULL;		//Reference to the FM selector label if it exists

//********************** Done Global Variables ***********************//

//********************** Function Headers *************************//

void loadTheme(char *newThemePath);

void cycleThemeNext();

void cycleThemePrevious();

void drawThemeBackground(SDL_Surface *screenBuf, int clockMode, SDL_Rect *targetPos);

void initImageLabel(struct imageLabel *thisLabel);

void loadUIElements();

//********************** Done Function Headers ********************//



