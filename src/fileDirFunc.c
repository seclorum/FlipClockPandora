

#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>      /* File handlers */
#include <sys/stat.h>   /* stat functions for working with files */
#include <sys/types.h>  /* More important stuff for directories */
#include <sys/dir.h>	/* Yet more important directory stuffs */
#include <ctype.h>		/* For tolower() function */


/********** File/Dir related functions
*
* Utilized as part of FlipClock C by Rob Williams and Ciro Ippoto
*
**********************************/

/* prototype std lib functions */
extern  int alphasort();

/*************************************************************************
* Touches a file... essentially all it does is open a file in w+ mode    *
* and close it again, but that should be enough.                         *
*************************************************************************/

int touchFile (char *fileName)
{
        FILE *touchedFile;

        touchedFile = fopen(fileName, "w+");

        if (touchedFile == NULL) {
                //failed
                return 0;
        } else {
                //Success
                fclose(touchedFile);
                return 1;
        }
}

/**************************************************************************
* Checks to see if a file exists; used to check if lock file is present   *
**************************************************************************/

int fileExists (char * fileName)
{
   struct stat buf;
   int i = stat ( fileName, &buf );
     if ( i == 0 )
     {
       return 1;
     }
     return 0;

}

/*************************************************************************
* Checks to see if a path is a valid directory                           *
*************************************************************************/

int isDirectory (char * path) {
   struct stat buf;
   int i = stat(path, &buf);
   
   if (i != 0) {
	//Stat failed for some reason, so we assume it's not a directory we're looking at
	return 0;
   }

   if (S_ISDIR(buf.st_mode)) {
	//Path is a valid directory
	return 1;
   } else {
	return 0;
   }
 
}

/*******************************************************************
* Function for filtering directory list                            *
*                                                                  *
*******************************************************************/

int file_select(const struct dirent *entry)
{
	if ((strcmp(entry->d_name, ".") == 0) || (strcmp(entry->d_name, "..") == 0)) {
		return (FALSE);
	} else {
		return (TRUE);
	}
}

/*******************************************************************
* Function for filtering directory list for Mp3s only                            *
*                                                                  *
*******************************************************************/

int mp3_select(const struct dirent *entry)
{
	char *tempFileName = NULL;
	char *ptr;
	
	tempFileName = calloc(strlen(entry->d_name) + 2, sizeof(char));
	sprintf(tempFileName, "%s", entry->d_name);
	
	ptr = tempFileName;

	for(ptr=tempFileName;*ptr;ptr++)
	{
		*ptr=tolower(*ptr);
	}

	if ((strstr(tempFileName, ".mp3")  != NULL) ) {
		free(tempFileName);
		return (TRUE);
	} else {
		free(tempFileName);
		return (FALSE);
	}
	
	
}

/*******************************************************************
* Function for filtering directory list                            *
*                                                                  *
*******************************************************************/

int dir_select(const struct dirent *entry)
{
	char *path;
	int retVal = 0;
	
	if ((strcmp(entry->d_name, ".") == 0) || (strcmp(entry->d_name, "..") == 0)) {
		retVal = 0;
	} else {
		path = malloc(strlen(entry->d_name) +2);
		sprintf(path, "%s", entry->d_name);
		printf("%s\r\n", path);
		if (isDirectory(path)) {
			retVal = 1;
		} else {
			retVal = 0;
		}
		free(path);
		
	}
	return retVal;
}


/*******************************************************************
* Function to get the permissions mode of a given path     *
*                                                                  *
*******************************************************************/
int getMode(char * path) {
   struct stat buf;
   int dirMask, dirMode;

   int i = stat(path, &buf);

   if (i != 0) {
        //Stat failed for some reason, so we assume it's not a path we're looking at
        return 0;
   }


   //Do some fancy binary to get file permissions only
   dirMode = buf.st_mode;

   dirMask = dirMode >>9;
   dirMask = dirMask <<9;
   dirMode = dirMode & ~dirMask;


   return dirMode;

}

/********************************************************************
* Function to get an array of files in a given directory
*
* (not used by anything...)
********************************************************************/
struct dirent **getFileList(char *path) {

	struct dirent **files = NULL;		/* Structure to contain returned results */
	int fileCount;			/* Number of files read			*/
	//int i;				/* Good old i, everyone likes i		*/

	fileCount = scandir(path, &files, file_select, alphasort);
	
	return files;
}

/*********************************************************************
* Function to get a specific file (either by index or randomly) from a given directory
*
*********************************************************************/
char *getIndexedMP3FromPath(char *path, int fileIndex) {
	
	struct dirent **files = NULL;		/* Structure to contain returned results */
	int fileCount;							/* Number of files read			*/
	char *selectedFile = NULL;		/* Selected file */
	int i;										/* Good old i, everyone likes i		*/

	fileCount = scandir(path, &files, mp3_select, alphasort);
	
	//-1 means random, otherwise it has to be a valid value
	//if (fileIndex >= fileCount) {
		//fileIndex = 0;
		
		
	//} else if (fileIndex == -1) {
	if (fileIndex == -1) {
		//-1 means random
		//Setup the random seed to the current time to try and make things more random...
		srand((unsigned)(time(NULL)));
		
		fileIndex = (int)( ((double)fileCount - 1) * rand() / ( RAND_MAX + 1.0 ) );
		printf("Random index is %i\n", fileIndex);
	}
	
	
	
	
	if (fileIndex > -1 && fileIndex < fileCount) {
		//
		selectedFile = calloc(strlen(path) + strlen(files[fileIndex]->d_name) + 5, sizeof(char));
		sprintf(selectedFile, "%s%s%s", path, "/", files[fileIndex]->d_name);
	}
	printf("File is %s\n", selectedFile);
	
	for (i=0; i< fileCount; i++) {
		free(files[i]);
	}
	free(files);
	
	return selectedFile;

}
