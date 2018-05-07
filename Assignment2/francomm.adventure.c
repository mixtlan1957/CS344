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
const int ROOM_COUNT = 7;           //number of room files to look through
const int MAX_CONNECTIONS = 6;      //WARNING: max connections must be less than 10, code must be modified otherwise
const int MAX_FILE_NAME_LENGTH = 9 //max file length + null terminator = 9
char roomsLoaded[ROOM_COUNT][56];     //for storing the rooms loaded in a run instance of this file
char roomConnections[ROOM_COUNT][MAX_CONNECTIONS][56];	 //for storing the associated connections
const char *roomTypes[3] = {"START_ROOM", "MID_ROOM", "END_ROOM"};
int roomTypeAssign[ROOM_COUNT];

int stepCount = 0;
char *pathTaken[256];



//function prototypes
void newestDir(char*, int);
void loadRooms();


/*
*
*Description: 
*
*Inputs:
*Output: 
*/
void gameLoop() {




}



/*
*
*Description: 
*
*Inputs:
*Output: 
*/
void loadRooms() {
	size_t bufferSize = 0;      //for parsing each line (from file)
	char* lineEntered = NULL;  //for parsing each line (from file)
	FILE *readFile;			//file read object
	char workingDir[256];   //variable for determinine most recent directory
	char filePath[256];		//full file path of different room files
	int i, j, k, temp;
	DIR *dirLoadFrom;
	struct dirent *fileInDir;
	int fileIndex = 0;      //tracking variable for determining which file has been read
	char subS[56];          //substring variable

	//determine which directory to load rooms from
	newestDir(workingDir, sizeof(workingDir));

	
	//load data from each room
	//set the directoy name with slashes and file name(i.e. ./dirName/)
	memset(filePath, '\0', sizeof(filePath));
	filePath[0] = '.';
	filePath[1] = '/';
	strcat(filePath, workingDir);
	
	//open directory to search in
	dirLoadFrom = opendir(filePath);

	//start reading each file within the most recent directory
	if (dirLoadFrom > 0) {
		while ((fileInDir = readdir(dirLoadFrom)) != NULL) {
			
			//skip over sub directories and parent directory
			if (!strcmp(fileInDir->d_name, "..")) {
				continue;
			}
			if (!strcmp(fileInDir->d_name, ".")) { //this statement shouldn't be necessary
				continue;
			}

			//open file in read mode only
			readFile = fopen(fileInDir->d_name, "r");			
			if (readFile == NULL) {
				printf("Room file could not be oppend for some reason...\n");
				exit 1;
			}

			//store file name
			memset(roomsLoaded[fileIndex], '\0', sizeof(roomsLoaded[fileIndex]));
			strcpy(roomsLoaded[fileIndex], fileInDir->d_name);  //files should not have suffix so no need for strncpy

			//call getline to advance to next line
			getLine(&lineEntered, &bufferSize, readFile);
 			
			k = 0;
			//store connections in our 3D global variable array [room index][connection index][name of connection]
			while(strncmp(lineEntered, "ROOM TYPE:", 10) != 0) {
				//reset line entered
				free(lineEntered);
				lineEntered = NULL;
				
				//get the line containing the connection info
				getline(&lineEntered, &bufferSize, readFile);
				
				//get the substring
				memset(subS, '\0', sizeof(subS));
				sscanf(lineEntered, "%*s, %*s, %s", subS);
				memset(roomConnections[fileIndex][k], '\0', sizeof(roomConnections[fileIndex][k]));
				strcpy(roomConnections[fileIndex][k], subS);	

				/*
				//get the substring
				temp = 14;  //start index if connection count is less than 2 digits "CONNECTION X: "
				memset(roomsLoaded[fileIndex][k], '\0', sizeof(roomsLoaded[fileIndex][k]));
			    
				//copy substring one character at time
				j = 0; 
				while(roomsLoaded[fileIndex][k][j] != '\n') {
					roomsLoaded[fileIndex][k][j] = lineEntered[temp + j];
					j++;
				}
				roomsLoaded[fileIndex][j] = '\0';	
				k++;
				*/
				k++;				
			}
			//store room type
			memset(subS, '\0', sizeof(subS));
			sscanf(lineEntered, "%*s %*s %s", subS);		
		
			/*
			temp = 11; //start of index after "ROOM TYPE: "
			j = 0;
		    memset(subS, '\0', sizeof(subS));
			for (j = temp; j < strlen(lineEntered) - 1; j++) {
				subS[j - temp] = lineEntered[j];
			}
			*/ 	

			//determine if room is a start room
			if (strcmp(subS, roomTypes[0]) == 0) {
				
				roomTypeAssign[fileIndex] = 0;
			}
			//or if room is a mid room
			else if (strcmp(subS, roomTypes[1] == 0)) {
				
				roomTypeAssign[fileIndex] = 1;
			}
			//or if room is end room
			else if (strcmp(subS, roomTypes[2] == 0)) {

				roomTypeAssign[fileIndex] = 2;
			} 
			else {
				//error has occured
				printf("Error has occured in room type assignment.\n");
				roomTypeAssign[fileIndex] = -1;
			}

			//increment the file index
			fileIndex++;

			//if extra files were somehow added to the file path, exit
			if (fileIndex > ROOM_COUNT + 1) {

				exit(1);
			}

			//reset lineEntered
			free(lineEntered);
			lineEntered = NULL;

			//close file
			fclose(readFile);

		}
	}
			
	//close directory
	closedir(dirLoadFrom);	
}



/*
*
*Description: 
*
*Inputs: takes an empty string with a buffer size of 256
*Output:
*Sources Citation: The code for this function was heavily based on Professor Brewster's class notes 
* returning strings based on: https://stackoverflow.com/questions/1496313/returning-c-string-from-a-function/1496328
*/
void newestDir(char *inputString, int bufferSize) {
	int newestDirTime = -1; //"Modified timestamp of newest subir examined"
	char targetDirPrefix[32] = "francomm.rooms."; //"Prefix we're looking for
	char newestDirName[256]; //"Holds the name of the newest dir that contains prefix"           
	memset(newestDirName, '\0', sizeof(newestDirName)); 

	DIR *dirToCheck; //"Holds the directory we're starting in"
	struct dirent *fileInDir; //"Holds the current subidr of the starting dir"
	struct stat dirAttributes; //"Holds information we've gained about subidr"

	dirToCheck = opendir("."); //"Open up the directory this program was run in"

	if (dirToCheck > 0) { // "Make sure the current directory could be oppened"
		
		while ((fileInDir = readdir(dirToCheck)) != NULL) { // "Check each entry in dir"
			
			if (strstr(fileInDir->d_name, targetDirPrefix) != NULL) {
				stat(fileInDir->d_name, &dirAttributes);   // "Get attributes of the entry"
					
				if ((int)dirAttributes.st_mtime > newestDirTime) {
					newestDirTime = (int)dirAttributes.st_mtime;
					memset(newestDirName, '\0', sizeof(newestDirName));
					strcpy(newestDirName, fileInDir->d_name);
				}
			}
		}
	}
	closedir(dirToCheck); // "Close the directory we oppened"
	memset(inputString, '\0', bufferSize);
	strncpy(inputString, newestDirName, bufferSize -1);
	inputString[bufferSize -1] = '\0';    //just in case
}

/*
*
*Description: 
*
*Inputs:
*Output: 
*/




int main() {
	loadRooms();

	return 0;
}

