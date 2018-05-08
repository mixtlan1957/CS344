/**********************************************************************
** Program FileName:  francomm.adventure.c
** Author: Mario Franco-Munoz
** Due Date: 5/9/2018
** Description: CS344 Assignment2: this file reads the room files from the most
** reecently created ONID.rooms.PID subidrectory and loads the data so that the user can play a
** text based game that allows him/her to move from start room to room until the END_ROOM is found.
** Player can also enter "time" to see what time it is.
** 
*********************************************************************/

#include <stdio.h>
#include <string.h>
#include <unistd.h>  //for reading files and getting statistics (last directory created)
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <dirent.h> //for navigating directories
#include <time.h>
#include <pthread.h>

//GLOBAL VARIABLES
#define ROOM_COUNT 7           //number of room files to look through
#define MAX_CONNECTIONS 6      //WARNING: max connections must be less than 10, code must be modified otherwise
#define MAX_FILE_NAME_LENGTH 9    //max file length + null terminator = 9
char roomsLoaded[ROOM_COUNT][56];     //for storing the rooms loaded in a run instance of this file
char roomConnections[ROOM_COUNT][MAX_CONNECTIONS][56];	 //for storing the associated connections
int connectionCount[ROOM_COUNT];      //for tracking how many connections each room has
const char *roomTypes[3] = {"START_ROOM", "MID_ROOM", "END_ROOM"};
int roomTypeAssign[ROOM_COUNT];
int _startRoom;
int _endRoom;
char victoryPath[999][20]; 
int stepCount = 0;
pthread_mutex_t mutex;

//function prototypes
void newestDir(char*, int);
void loadRooms();
void gameLoop();
int whichRoom(char*);
void pthreads();
void* displayTime();
void* writeTime();
void test();


/*
*whichRoom
*Description: short helper function that associates the string of a room name to the number stored/identifying
* that room
* 
*Inputs: name of the room to have its index identified for
*Outputs: room index
*/
int whichRoom(char *roomNameIn) {
	int i;

	for(i = 0; i < ROOM_COUNT; i++) {
		if (strcmp(roomNameIn, roomsLoaded[i]) == 0) {
			return i;
		}		
	} 
	return -1;
}

/*
*displayTime
*Description: reads the time stored as a string in the "currentTime.txt" file and displays it in I/O
*
*preconditions: requires that the currentTimme.txt file exist and have the time written to it prior to calling
* displayTime
*Outputs:(I/O: displays stored time string)
*/
void* displayTime() {
	pthread_mutex_lock(&mutex);  //lock the consumer thread

	char *readBuffer = NULL;
	FILE *timeFile;
	size_t bufferSize = 0;
	
	//read the contents of file using getline
	timeFile = fopen("currentTime.txt", "r");
	if (timeFile == NULL) {
		printf("There was an error opening \"currentTime.txt\"\n");
	}
	getline(&readBuffer, &bufferSize, timeFile);
	printf("\n%s\n\n\n", readBuffer);

	free(readBuffer);
	readBuffer = NULL; 
	
	pthread_mutex_unlock(&mutex);
	return NULL;           //type void* is what pthread_create needs, so void* is what it will get
}

/*
*writeTime
*Description: writes the current time to "currentTime.txt" in the hr:mm(am/pm), weekday, month day, year format
*
*Outputs:creates/truncates "currentTime.txt"
*Sources Cited: https://linux.die.net/man/3/strftime 
*/
void* writeTime() {

	FILE *timeFile;
	char buffer[256];
	time_t t;
	struct tm *tmp;
	t = time(NULL);
	tmp = localtime(&t); 

	//obtain the time using the strftime function and write result to c-string
	memset(buffer, '\0', sizeof(buffer));
	strftime(buffer, sizeof(buffer), "%I:%M%P, %A, %B %e, %Y", tmp);
	
	//store the result in the currentTime.txt file
	timeFile = fopen("currentTime.txt", "w");
	fprintf(timeFile, "%s", buffer);
	fclose(timeFile);
	
	//this function is the parent node, unlock here
	pthread_mutex_unlock(&mutex);

	return NULL;	
}

/*
*pthreads
*Description: manages the thread that controls the writeTime and displayTime functionn
*Sources Cited: https://www.cc.gatech.edu/classes/AY2010/cs4210_fall/lectures/04-PthreadsIntro.pdf
*/
void pthreads() {

	pthread_t thread1;
	pthread_t thread2;
	pthread_mutex_init(&mutex, NULL);   //initialize gobal mutex variable
	pthread_mutex_lock(&mutex);  //lock the mutex and leave read for the parent to use

	//create the threads and join them
	if (pthread_create(&thread1, NULL, writeTime, NULL) != 0) {
		fprintf(stderr, "Unable to create producer thread\n");
		exit(1);
	}
	if (pthread_create(&thread2, NULL, displayTime, NULL) != 0) {
		fprintf(stderr, "Unable to create consumer thread\n");
		exit(1);
	}

	pthread_join(thread1, NULL);
	pthread_join(thread2, NULL);

	pthread_mutex_destroy(&mutex);


}

/*
*gameLoop
*Description: primary driver for adventure game. Loops through user input until user reaches final room
*or reaches the step limit.
*Sources:used to help get input from user: https://stackoverflow.com/questions/29397719/c-getline-and-strcmp-issue 
*/
void gameLoop() {
	size_t bufferSize = 0;
	char* lineEntered = NULL;
	int i;
	int currentRoom;
	int correctInput;
	int errorCounter;
	int stepCounter;

	//set currentRoom to the room that is the "START_ROOM"
	currentRoom = _startRoom;

	errorCounter = 0;
	stepCounter = 0;
	while(currentRoom != _endRoom || errorCounter > 0) {
		if (errorCounter > 0) {
			printf("HUH? I DON'T UNDERSTAND THAT ROOM. TRY AGAIN.\n\n");
		}
		errorCounter = 0;
		printf("CURRENT LOCATION: %s\n", roomsLoaded[currentRoom]);
		printf("POSSIBLE CONNECTIONS: ");
		
		//loop and print possible connections
		for (i = 0; i < connectionCount[currentRoom] - 1; i++) {
			printf("%s, ", roomConnections[currentRoom][i]);
		}
		printf("%s.\n", roomConnections[currentRoom][i]);

		//get user input
		printf("WHERE TO? >");
		getline(&lineEntered, &bufferSize, stdin);
		printf("\n");
		
		//remove potential end-of-line characters from input
		lineEntered[strcspn(lineEntered, "\r\n")] = 0;

		//check to see if time was called
		if (strcmp(lineEntered, "time") == 0) {
			pthreads();
			correctInput = 1;
		}
		else {
			//iterate over the room connections current room has, and validate user input
			correctInput = 0;
			i = 0;
			while(i < connectionCount[currentRoom] && correctInput == 0) {
				if(strcmp(lineEntered, roomConnections[currentRoom][i]) == 0) {
					correctInput = 1;	
					currentRoom = whichRoom(lineEntered);
					stepCounter++;
					if (stepCounter >998) {
						printf("You reached the step limit of 999!\n");
						exit(1);
					}
					else {
						memset(victoryPath[stepCounter - 1], '\0', sizeof(victoryPath[stepCounter - 1])); 
						strcpy(victoryPath[stepCounter - 1], lineEntered);
					}		
				}
				i++;
			}
			if (correctInput != 1) {
				errorCounter++;
			}
		}
		free(lineEntered);
		lineEntered = NULL;
	}

	//end of game
	printf("YOU HAVE FOUND THE END ROOM. CONGRATULATIONS!\n");
	printf("YOU TOOK %d STEPS. YOUR PATH TO VICTORY WAS:\n", stepCounter);
	for (i = 0; i < stepCounter; i++) {
		printf("%s\n", victoryPath[i]);
	}	

}



/*
*loadRooms
*Description: this function loads the different room files located in the most recently created ONID.rooms.PID 
* directory and stores their data in the global array variables.
*Inputs: (file input: room files)
*/
void loadRooms() {
	size_t bufferSize = 0;      //for parsing each line (from file)
	char* lineEntered = NULL;  //for parsing each line (from file)
	FILE *readFile;			//file read object
	char workingDir[256];   //variable for determinine most recent directory
	char filePath[256];		//full file path of different room files
	int k;
	DIR *dirLoadFrom;
	struct dirent *fileInDir;
	int fileIndex = 0;      //tracking variable for determining which file has been read
	char subS[56];          //substring variable

	//determine which directory to load rooms from
	newestDir(workingDir, sizeof(workingDir));
	
	//open directory to search in
	memset(filePath, '\0', sizeof(filePath));
	filePath[0] = '.';
	filePath[1] = '/';
	strcat(filePath, workingDir);
	dirLoadFrom = opendir(filePath);

	//test
	printf("%s\n", filePath);	

	//start reading each file within the most recent directory
	if (dirLoadFrom > 0) {
		while ((fileInDir = readdir(dirLoadFrom)) != NULL) {
			
			//skip over sub directories and parent directory
			if (strcmp(fileInDir->d_name, "..") == 0) {
				continue;
			}
			if (strcmp(fileInDir->d_name, ".") == 0) {
				continue;
			}
			if (strcmp(fileInDir->d_name, "currentTime.txt") == 0) {
				continue;
			}
		
			//establish the full file path that is to be loaded (now including the name of the file as well)
			memset(filePath, '\0', sizeof(filePath));
			filePath[0] = '.';
			filePath[1] = '/';
			strcat(filePath, workingDir);
			strcat(filePath, "/");
			strcat(filePath, fileInDir->d_name);

			//open file in read mode only
			readFile = fopen(filePath, "r");			
			if (readFile == NULL) {
				printf("Room file could not be oppend for some reason...\n");
				exit(1);
			}

			//store file name
			memset(roomsLoaded[fileIndex], '\0', sizeof(roomsLoaded[fileIndex]));
			strcpy(roomsLoaded[fileIndex], fileInDir->d_name);  //files should not have suffix so no need for strncpy


			//call getline to advance to next line
			getline(&lineEntered, &bufferSize, readFile); 		
	
			k = 0;
			//store connections in our 3D global variable array [room index][connection index][name of connection]
			while(strncmp(lineEntered, "ROOM TYPE:", 10) != 0) {
				//reset line entered
				free(lineEntered);
				lineEntered = NULL;
				//get the line containing the connection info
				getline(&lineEntered, &bufferSize, readFile);
							
				if (strncmp(lineEntered, "ROOM TYPE:", 10) != 0) {
	
					//get the substring
					memset(subS, '\0', sizeof(subS));
					sscanf(lineEntered, "%*s %*s %s", subS);
					memset(roomConnections[fileIndex][k], '\0', sizeof(roomConnections[fileIndex][k]));
					strcpy(roomConnections[fileIndex][k], subS);	
					connectionCount[fileIndex]++; 			//number of connections is tracked separately

					k++;
				}				
			}
			//store room type
			memset(subS, '\0', sizeof(subS));
			sscanf(lineEntered, "%*s %*s %s", subS);			

			//determine if room is a start room
			if (strcmp(subS, roomTypes[0]) == 0) {
				_startRoom = fileIndex;			
				roomTypeAssign[fileIndex] = 0;
			}
			//or if room is a mid room
			else if (strcmp(subS, roomTypes[1]) == 0) {
				
				roomTypeAssign[fileIndex] = 1;
			}
			//or if room is end room
			else if (strcmp(subS, roomTypes[2]) == 0) {
				_endRoom = fileIndex;
				roomTypeAssign[fileIndex] = 2;
			} 
			else {
				//error has occured
				printf("Error has occured in room type assignment.\n");
				roomTypeAssign[fileIndex] = -1;
			}

			//increment the file index
			fileIndex++;

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
*newestDir
*Inputs: takes an empty string with a buffer size of 256
*Output: stores the most recently created (myonid).rooms.(pid) directory in the input string
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
*test
*Description: not required part of program, this function only serves to test the values stored in global array
*/
void test() {
	int i, j;
	printf("Testing rooms loaded...\n");
	for (i = 0; i < ROOM_COUNT; i++) {
		printf("%s\n", roomsLoaded[i]);
	}
	printf("\nTesting room types...\n");
	for (i = 0; i < ROOM_COUNT; i++) {
		printf("%s\n", roomTypes[roomTypeAssign[i]]);
	}
	printf("\nTesting connections...\n");
	for (i = 0; i < ROOM_COUNT; i++) {
		printf("Number of connections: %d\n", connectionCount[i]);
		printf("Printing the connections for room %s\n", roomsLoaded[i]);
		for (j = 0; j < MAX_CONNECTIONS; j++) {
			printf("Connection %d: %s\n", j+1, roomConnections[i][j]);
		}
	}
}



int main() {
	//initialize 3D array (roomConnections)
	int i , j;
	for(i = 0; i < ROOM_COUNT; i++) {
		for (j = 0; j < MAX_CONNECTIONS; j++) {
			memset(roomConnections[i][j], '\0', sizeof(roomConnections[i][j]));			
		}
		connectionCount[i] = 0;   //initialize number of connections at 0
	}
	

	loadRooms();
	gameLoop();
	return 0;
}

