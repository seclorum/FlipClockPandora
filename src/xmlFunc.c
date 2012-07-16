#ifndef XMLFUNC_H
#define XMLFUNC_H


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define CR 13            /* Decimal code of Carriage Return char */
#define LF 10            /* Decimal code of Line Feed char */
#define EOF_MARKER 26    /* Decimal code of DOS end-of-file marker */
#define MAX_REC_LEN 4096 /* Maximum size of input buffer */


#define xmlFuncVersion "2.0"

/********************* XML string parser **********

This library was the basis for a lot of XML work, used primarily
as a very basic and simple hack to try to search through a string for a
specific XML tag and return the tag's content. With the migration of all of the
WPP system to C though the demands on this library grew, so
eventually it's now gotten to the point where I've taken the code
and principles from the phpSerializeParser and applied them here
to build this into a more functional and dynamic "true" XML handling system.

(Ironically the phpSerializeParser was based on this code, evolved, and is
now being ported from there back over to improvements here!).

For legacy support, just as with the PHP library, the old findTag function is still
around and untouched, but new more useful options like the findTagObject are
now present and can be used for walking through XML structures without being
aware of key values/etc.


Todo:
 - Keep an eye on it and debug/improve in the future as required...
 - Add more features/conversion options/etc?

Changelog:
Version 2.0, November 2008
 - Built object support to convert XML elements into structures. This allows for dynamic
 document traversal
 - Added findXMLObject, findXMLObjectTest, freeXMLObject functions to allow for dynamic traversals and
 better memory management.
 - Increased MAX_REC_SIZE to avoid crashes due to long single lines of message


Version 1.0, Way back when (say 2006-2007ish?)
 - Initial code build.
 - Wow, turns out, it works! And it's pretty fancy dancy amazing isn't it?
 - Provides findTag function which allows you to search an XML string and pull out the 
 value of the appropriate tag.





**************************************************************/

enum xmlType
        { XML_VALUE = 0, XML_PARENT };


/******* XML Node structure definition. 
XML nodes are much simpler than JSON or PHP ones in that they
always store values and keys in the same format. Sure they could have
attributes, any maybe there's a reason to parse and deal with those,
but I can't think of it right now so stick to the basics...
*/

struct xmlNode
{
	char *key;		/* Node Key */
	char *value;		/* Node Value */
	char *refKey;		/* Reference for entire raw XML */
	int type;			/* Type of node... always either Value or Parent */
	//struct xmlNode **children	/* Array of child elements */ 
	//int childCount;		/* How many children are in the children array */
};


//***************** Function header definitions ******************************

char *readFile(FILE *input);
char *findTag(char *buffer, char *searchTag, int matchOffset);

/* experimental version of findTag that will eventually replace findTag once we confirm it
does all that it's expected to */
struct xmlNode *findXMLObject(char *buffer, char *searchTag);

/* Used to find a sequenced XML node */
struct xmlNode *findXMLObjectTest(char *buffer, char *searchTag, int matchOffset);

/* Function to free an xmlNode element that was created/allocd by findXMLObject */
void freeXMLObject(struct xmlNode *xmlEle);

/* Function to create a new XML string */
char * createXMLElement(char *arrayName);

/* Function to add a value/key to an existing XML string */
char * appendToXMLElement(char * inBuffer, char *key, char *value);

//***************** Function header definitions ******************************


//***************** Library Functions *****************************************

//Function to read a text file into a character array!
// (and it works!)
char *readFile(FILE *input) {
  char *cFile;                  /* Dynamically allocated buffer (entire file) */
  long  lFileLen;               /* Length of file */


  fseek(input, 0L, SEEK_END);  /* Position to end of file */
  lFileLen = ftell(input);     /* Get file length */
  rewind(input);               /* Back to start of file */


  //Read the entire file to a string
	
  cFile = calloc(lFileLen + 1, sizeof(char));

  if(cFile == NULL )
  {
    printf("\nInsufficient memory to read file.\n");
    return 0;
  }

  fread(cFile, lFileLen, 1, input); /* Read the entire file into cFile */

//  printf("file contains %s", cFile);
  return(cFile);
	
}




//Okay, now Rob's custom function to read through a block of text and try and pull out the contents
//of an XML tag
// This can probably be reduced/trimmed down, but it works as it is for now...

struct xmlNode *findXMLObject(char *rawBuffer, char *searchTag) {

	char *tagStart, *tagEnd, *tagEnd2;
	char *tempStringHolder;
	
	char *rawStartPos, *rawEndPos; 
	struct xmlNode *thisElement;	/* Define the XML node to be returned */
	
	char *headStart, *headEnd, *tempBuffer, *tempBuffer2;
	char *buffer;
	
	char tagMode[20];
	char openTag[50];
	char closeTag[50];
	//char tagBuffer[90000];
	char *tagBuffer = NULL;
	
	
	//char newstring[10000];
	char *newstring = NULL;
	//static char newstring2[10000];
	char *newstring2 = NULL;
	int index;
	


	//Initialize the XML node
	thisElement = malloc(sizeof(struct xmlNode));
	
	newstring = calloc(strlen(rawBuffer), sizeof(char));
	newstring2 = calloc(strlen(rawBuffer), sizeof(char));

	//Default is a string key
	thisElement->key = NULL;
	thisElement->value = NULL;
	thisElement->refKey = NULL;
	thisElement->type = XML_VALUE;
	//thisElement->children = NULL;
	//thisElement->childCount = 0;

	//strcpy(newstring2, "");  
	
	strcpy(tagMode,"none");
	
	if (strlen(searchTag) > 0) 
	{
		thisElement->key = calloc(strlen(searchTag) + 2, sizeof(char));
		sprintf(thisElement->key, "%s", searchTag);
	}
	
	//Define opening tag (with attribs)
	sprintf(openTag, "<%s ", searchTag);
	
	//Define closing tag
	sprintf(closeTag, "</%s>", searchTag);
	
	//Clear current buffer
	//strcpy(tagBuffer, "");
	
	//Allocate the buffer
	buffer = calloc(strlen(rawBuffer) +2, sizeof(char));
	sprintf(buffer, "%s", rawBuffer);
	
	//Strip off the XML header if it's in the buffer
	headEnd = strstr(buffer, "?>");
	if (headEnd != NULL) {
		//Get this junk out of the buffer!
		
		
		headStart = strstr(buffer, "<?");
		do {
			headEnd = strstr(buffer, "?>");
			
			//headLen = headEnd - headStart;
			tempBuffer = calloc(strlen(buffer) +2, sizeof(char));
			sprintf(tempBuffer, "%s", headEnd+2);
			
			bzero(buffer, sizeof(buffer));
			sprintf(buffer, "%s", tempBuffer);
			
			free(tempBuffer);
			
			headStart = strstr(buffer, "<?");
		
		} while (headStart != NULL);
	}
	
	headStart = strstr(buffer, "<!");
	if (headStart != NULL) {
		do {
			headEnd = strstr(buffer, "->");
			
			
			tempBuffer2 = calloc(headStart - buffer + 2, sizeof(char));
			strncpy(tempBuffer2, buffer, headStart - buffer);
			//printf("top half: %s\n", tempBuffer2);
			
			
			//headLen = headEnd - headStart;
			tempBuffer = calloc(strlen(buffer) +2, sizeof(char));
			sprintf(tempBuffer, "%s%s", tempBuffer2, headEnd+2);
			
			bzero(buffer, sizeof(buffer));
			sprintf(buffer, "%s", tempBuffer);
			free(tempBuffer2);
			free(tempBuffer);
			
			headStart = strstr(buffer, "<!");
		
		} while (headStart != NULL);
		

	} 
	
	rawStartPos = NULL;
	//Seach for opening tag with attributes
	rawStartPos = strstr(buffer, openTag);
	if (rawStartPos == NULL) {
		//No attributes, so search for just the tag itself
		memset(&openTag, 0, sizeof(openTag));
		
		
		//Define opening tag (with attribs)
		sprintf(openTag, "<%s>", searchTag);
		
		rawStartPos = strstr(buffer, openTag);
		
		
	}

	
    //Find the first instance of the search string in the search buffer
	if (rawStartPos != NULL) {
		strcpy(tagMode,"open");

		//Clear current buffer
		// strcpy(tagBuffer, "");
	
		//If we haven't specified a search string, we now update the closing tag to match this
		//opening one
		if (strlen(searchTag) == 0) {
			
			tagStart = strstr(buffer, openTag);
			tagStart++; //Skip the opening <
			tagEnd = strstr(tagStart, ">");
			tagEnd2 = strstr(tagStart, " ");
			
			if (tagEnd2 != NULL) {
				if (tagEnd2 > tagEnd) {
					index = tagEnd - tagStart;
				} else {
					index = tagEnd2 - tagStart;
				}
			} else {
				index = tagEnd - tagStart;
			}
			
			tempStringHolder = calloc(index +2, sizeof(char));
			
			strncpy(tempStringHolder, tagStart, index);
			
			sprintf(closeTag, "</%s>", tempStringHolder);
			if (thisElement->key == NULL) {
				thisElement->key = calloc(strlen(tempStringHolder) + 2, sizeof(char));
				sprintf(thisElement->key, "%s", tempStringHolder);
			}
			
			free(tempStringHolder);
			
			//printf("Auto closing tag is %s\n", thisElement->key);

		} 
		//printf("Opening tag %s", strstr(cThisLine, searchTag));
	}

	if (strcmp(tagMode,"open") == 0) {
		//Strip off tags
		tagBuffer = calloc(strlen(buffer) +2, sizeof(char));
		
		strcat(tagBuffer, buffer);
		//Strip off opening tag if present
	//	if (strstr(tagBuffer, openTag) != NULL) {
			//easy, just subtract after first closing > (end of opening tag)
	//	}
	}

	if ((rawEndPos = strstr(buffer, closeTag)) != NULL) {
		strcpy(tagMode,"close");
	
		char *pos;
		strcpy(newstring, "");

		if (strstr(tagBuffer, openTag) != NULL) {
			
			strcpy(newstring, strstr(tagBuffer, openTag) + strlen(openTag));
			
		} else {
			strcpy(newstring, tagBuffer);
		}
		
		if (tagBuffer != NULL) {
			free(tagBuffer);
			tagBuffer = NULL;
		}
		
		tempStringHolder = calloc(strlen(newstring) +4, sizeof(char));
		if (strstr(openTag, ">") == NULL) {
			strcpy(tempStringHolder, strstr(newstring, ">") + 1);
		} else {
			strcpy(tempStringHolder, newstring);
		}
		
		

		bzero(newstring, sizeof(newstring));
		strcpy(newstring, tempStringHolder);
		free(tempStringHolder);
  
		//index=strstr(newstring, "<") - newstring;

		//pos = strrchr(newstring, 60);
		//try instead
		pos = strstr(newstring, closeTag);

		index = pos - newstring;

		strncpy(newstring2, newstring, index);
		
		//Have to manually terminate cause strncpy doesn't do it
		newstring2[index] = '\0';

		

		thisElement->value = calloc(strlen(newstring2) + 2, sizeof(char));
		sprintf(thisElement->value, "%s", newstring2);
		
		index = (rawEndPos - rawStartPos) + strlen(closeTag);
		thisElement->refKey = calloc(index +4, sizeof(char));
		
		strncpy(thisElement->refKey, rawStartPos, index);
		
		rawStartPos = strstr(thisElement->value, "<");
		if (rawStartPos != NULL) {
			thisElement->type = XML_PARENT;
		}
		
		free(buffer);
		free(newstring);
		if (newstring2 != NULL) {
			free(newstring2);
		}

		return thisElement;
	}
	
	if (tagBuffer != NULL) {
		free(tagBuffer);
		tagBuffer = NULL;
	}
	
	if (newstring2 != NULL) {
		free(newstring2);
	}
	free(newstring);
	
	free(thisElement->key);
	free(thisElement);
	free(buffer);
	return(NULL);
}



/*********************************
The wrapper function to allow offset queries for objects....

Same as findXMLObject, but this one allows for picking out indexed nodes (and thus recursive parsing)

********************************/

struct xmlNode *findXMLObjectTest(char *buffer, char *searchTag, int matchOffset)
{

	char *searchBuffer, *searchBufferTemp;
    char *matchPos;
    char *matchTemp;
		
    int matchCount, lastMatchIndex, i;

	struct xmlNode *myFoundNode;
		

    matchCount = 0;
	
	searchBuffer = calloc(strlen(buffer) + 2, sizeof(char));

	i = 0;
	lastMatchIndex = 0;

	strcpy(searchBuffer, buffer);

	//printf("search buffer %s, offset: %i\n", searchBuffer, matchOffset);

	do {
		matchTemp = NULL;

        //Find the first instance of the search string in the search buffer
     	myFoundNode = findXMLObject(searchBuffer, searchTag);
		
		//printf("got match %s\n", searchBuffer);

		//printf("match :%s at index %i\n", matchTemp, lastMatchIndex);

		
		
		if (myFoundNode != NULL) {
		
			
		
			//Match was found
			if (i != matchOffset) {
				//printf("moving to next\n");

				//Not the one we're looking for, next...

				//printf("%i %s\n", myFoundNode->type, myFoundNode->value);

				//Setup the reference value
				if (myFoundNode->refKey != NULL) {
					//printf("ref not null %s\n", myFoundNode->refKey);
					//Found node is a reference
					matchPos = strstr(searchBuffer, myFoundNode->refKey);
					
					matchPos += strlen(myFoundNode->refKey);
				}

				
				//if (enableDebug) {
					//printf("next match pos: %s\n\n", matchPos);
				//}
				
				
				searchBufferTemp = calloc(strlen(matchPos) +2, sizeof(char));
				strcpy(searchBufferTemp, matchPos);
				
				bzero(searchBuffer, sizeof(searchBuffer));
				
				//free(searchBuffer);
				//searchBuffer = calloc(strlen(searchBufferTemp) +2, sizeof(char));
				
				
				//strcpy(searchBuffer, matchPos);
				sprintf(searchBuffer, "%s", searchBufferTemp);
				//sprintf(searchBuffer, "%s", matchPos);

				free(searchBufferTemp);

				freeXMLObject(myFoundNode);



				//printf("new buffer :%s\n", searchBuffer);

			} else {
				//Found the node we want
				//printf("Found key %s in %s\n", myFoundNode->key, searchBuffer);
			
			}
		} else {
				free(searchBuffer);

			   return 0;
		}
		i++;
	} while (i <= matchOffset);

	//free(tempBuffer);
	free(searchBuffer);

	//thisElement->child = myFoundNode;

	return myFoundNode;
}







/********************************
Function to free/obliterate a xmlNode structure
*****************************/

void freeXMLObject(struct xmlNode *xmlEle) 
{

	if (xmlEle != NULL) {
		if(xmlEle->key != NULL) {
			free(xmlEle->key);
		}
		
		if (xmlEle->value != NULL) {
			free(xmlEle->value);
		}
	
		if (xmlEle->refKey != NULL) {
			free(xmlEle->refKey);
		}
		
		if (xmlEle != NULL) {
			free(xmlEle);
		}
	}

}


/*************************************************
* createXMLElement
*
* create a base XML document/element. This charstring
* can then be used with addXMLElement to build
* documents
*************************************************/
char * createXMLElement(char *arrayName) {
	char *inBuffer;

	if (arrayName != NULL) {
		inBuffer = calloc((strlen(arrayName) *2) + 8, sizeof(char));
		sprintf(inBuffer, "<%s> </%s>", arrayName, arrayName);
	} else {
		inBuffer = calloc(3, sizeof(char));
		//sprintf(inBuffer, "");
	}
	return inBuffer;
}

/*************************************************
* appendToXMLElement
*
* append a value/key to an existing XML char string
* can be used with chars originated by createXMLElement,
* or by values loaded from other XML formatted sources.
*************************************************/
char * appendToXMLElement(char * inBuffer, char *key, char *value)
{

	char *myTag;

	/*char *tempVal;

	char *arrPrefix;
	char *arrSuffix;
	char *arrCounter;
	char *arrSearch;
	int arrIndex, arrTinyNibble, eleCount;*/

	int nibble;
	//int nibbleCounter;
	char *topHalf;				/* Top half of XML/php array */
	char *bottomHalf;			/* bottom half of XML/php array */

	myTag = calloc((strlen(key) * 2) + strlen(value) + 7, sizeof(char));
    sprintf(myTag, "<%s>%s</%s>", key, value, key);

	//Add to array
	nibble = strlen(inBuffer);
	do
	{
		if (inBuffer[nibble] == 60) {
			topHalf = calloc(nibble +2, sizeof(char));
			bottomHalf = calloc((strlen(inBuffer) - nibble) + 2, sizeof(char));

			//Copy the top half
			strncpy(topHalf, inBuffer, nibble);
			//Add the null terminator
			topHalf[nibble +1] = 0;


			//Copy the bottom half
			strcpy(bottomHalf, inBuffer + nibble);

			//Allocate more memory
			inBuffer = realloc(inBuffer, (strlen(topHalf) + strlen(myTag) + strlen(bottomHalf) ) * sizeof(char));

			//Now build the new string
			sprintf(inBuffer, "%s%s%s", topHalf, myTag, bottomHalf);

			break;
		}
		nibble--;
	} while (nibble > 0);
	
	free(bottomHalf);
	free(topHalf);
	free(myTag);
	
	return inBuffer;

}

#endif
