/**********************************************************************
** Program FileName:  francomm.adventure.c
** Author: Mario Franco-Munoz
** Due Date: 5/9/2018
** Description:
** 
** 
** Input:  
** Output: 
*********************************************************************/

#include <stdio.h>
#include <string.h>
#include <unistd.h>  //for reading files and getting statistics (last directory created)
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <dirent.h> //for navigating directories

//GLOBAL VARIABLES



//function prototypes
const char *newestDir();




/*
*
*Description: 
*
*Inputs:
*Output: 
*/




/*
*
*Description: 
*
*Inputs:
*Output:
*Sources Citation: The code for this function was heavily based on Professor Brewster's class notes 
*/
const char* newestDir() {
	int newestDirTime = -1; //"Modified timestamp of newest subir examined"
	char targetDirPrefix[32] = "francomm.rooms."; //"Prefix we're looking for
	char newestDirName[256]; //"Holds the name of the newest dir that contains prefix"           
	memset(newestDirName, '\0', sizeof(newestDirName)); 

	DIR *dirToCheck; //"Holds the directory we're starting in"
	struct dirent *fileInDir; //"Holds the current subidr of the starting dir"
	struct stat dirAttributes; //"Holds information we've gained about subidr"

	dirToCheck = opendir("."); //"Open up the directory this program was run in"

	if (dirToCheck > 0) { // "Make sure the current directory could be oppened"
		while ((fileInDir = readdir(dirToCheck)) != NULL) // "Check each entry in dir"
		if ((int)dirAttributes.st_mtime > newestDirTime) {
			newestDirTime = (int)dirAttributes.st_mtime;
			memset(newestDirName, '\0', sizeof(newestDirName));
			strcpy(newestDirName, fileInDir->d_name);
		}
	}
	closedir(dirToCheck); // "Close the directory we oppened"
	return newestDirName;
}




/*
*
*Description: 
*
*Inputs:
*Output: 
*/







int main() {




	return 0;
}

