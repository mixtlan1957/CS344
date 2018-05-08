/*********************************************************************
** Program Filename: francomm.buildrooms.c
** Author: Mario Franco-Munoz
** Due Date: 5/9/2018
** Description:CS344 Assignment 2: This file creates seven room files containing
** a randomly assigned room type, and random amount of connections to other rooms.
**  
*********************************************************************/

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>  //for creating directory
#include <sys/stat.h>
#include <stdlib.h> //srand, rand among others
#include <time.h> //for srand seed


//GLOBAL VARIABLES
const char *roomTypes[3] = { "START_ROOM", "MID_ROOM", "END_ROOM" };
const char *roomNames[10] = { "Azcanta", "Kolios", "BandRoom", "Water",
"Wind", "Fire", "Ice", "Alara", "Ravnica", "Monster" };
int connectionCount[7] = {0, 0, 0, 0, 0, 0, 0};
int roomAssignment[7] = {-1, -1, -1, -1, -1, -1, -1};
int roomType[7];  //which room type was assigned to a given room goes here
int connectivityMap[7][7]; 


//function prototypes
void createRooms();
int randomInt(int, int);
void fillMap();
int isGraphFull();
void addRandomConnection();
int canAddConnectionFrom(int);

struct stat st = {0};


/*
*createRooms
*Description: creates new directory and places room files within that directory. Also
* assigns, but does not write the room properties. (Type, name, connection count).
*Inputs: None
*Output: (file output) creates room files under the ONID.rooms.PID directory
*/
void createRooms() {
	int i, j, k;			//loop counter variables
	int temp = 0;       //temporary variable for storing process id
	int startRoomSelected = 0; //boolean values to track if start and end
	int endRoomSelected = 0;  //rooms have been assigned  
	int roomSelected = 0;
	int roomTaken;
	char filePath[256];      //./ONID.rooms.PID/<roomFileName>
	//FILE *roomFile;  

	//create directory
	//get the current process id
	temp = getpid();
	
	//convert the process id to a string
	char convPID[45];
	sprintf(convPID, "%d", temp);	
	
	//declare dir name
	char dirName[45];
	memset(dirName, '\0', 45);
	strcpy(dirName, "francomm.rooms.");

	//concatenate the pid and directory name
	strcat(dirName, convPID);

	//create directory
	int result = mkdir(dirName, 0777);
	if (result == -1) {
		printf("Error creating directory.\n");
	}


	//create the 7 rooms files under the new directory
	
	for (i = 0; i < 7; i++) {
		roomSelected = -1;
		//randomly assign room name
		while (roomSelected == -1) { 
			temp = randomInt(0, 9);
			//check if that room name has already been taken
			j = 0;
			roomTaken = 0;
			while (j < 7 && roomTaken == 0) {
				if (roomAssignment[j] == temp) {
					roomTaken = 1;
				}
				j++;	
			}
			if (roomTaken == 0) {
				roomSelected = temp;
				roomAssignment[i] = roomSelected;			

			}

		}
		

		//set the directory name with slashes and file name(i.e. /dirName/)		
		memset(filePath, '\0', 256);
		filePath[0] = '.';
		filePath[1] = '/';
		strcat(filePath, dirName);
		strcat(filePath, "/");
		strcat(filePath, roomNames[roomAssignment[i]]);


		//create room file, append name to file
		FILE *roomFile;
		roomFile = fopen(filePath, "w");
		fprintf(roomFile, "ROOM Name: %s\n", roomNames[roomAssignment[i]]);
		fclose(roomFile);

		//randomly assign room type
		if (startRoomSelected == 0 || endRoomSelected == 0) {
			temp = randomInt(0, 2);
		}
	
		if (startRoomSelected == 0 && temp == 0) {
			roomType[i] = 0;
			startRoomSelected = 1;	
		}
		else if (endRoomSelected == 0 && temp == 2) {
			roomType[i] = 2;
			endRoomSelected = 1;
		}
		else {
			roomType[i] = 1;	
		}
		//if we are at room 6, and we still don't have a start room, forcefully make it a start room
		if (i == 5 && startRoomSelected != 1) {
			roomType[i] = 0;
			startRoomSelected = 1;
		}
		//if we are at room 7, and we still don't have a end room, forcefully make it an end room
		if (i == 6 && endRoomSelected != 1) {
			roomType[i] = 2;
			endRoomSelected = 1;
		}

	}

	//call function to create room connections
	fillMap();	
	
	//append connections and room type  to the different room files
	for (i = 0; i < 7; i++) {
		//concat the file name
		memset(filePath, '\0', 256);
		filePath[0] = '.';
		filePath[1] = '/';
		strcat(filePath, dirName);
		strcat(filePath, "/");
		strcat(filePath, roomNames[roomAssignment[i]]);

		//open the file in append mode
		FILE *roomFile2;
		roomFile2 = fopen(filePath, "a");


		//append the connections
		k = 0;
		for (j = 0; j < connectionCount[i]; j++) {
			while (connectivityMap[i][k] == 0) {
				k++;
			}
			fprintf(roomFile2, "CONNECTION %d: %s\n", (j + 1), roomNames[roomAssignment[k]]);
			k++;
		}

		//append what type of room it is
		fprintf(roomFile2, "ROOM TYPE: %s\n", roomTypes[roomType[i]]); 

		fclose(roomFile2);			
	}
}

/*
*randomInt
*Description: generates a pseudo-random integer between a given integer range.
*Inputs: lower bound and higher bound (inclusive) of number range for random number geneartion
*Output: pseudo-random integer between low and high bounds
*/
int randomInt(int low, int high) {
	return (rand() % (high - low + 1)) + low;

}

/*
*fill Map
*Description: driver function for creating room connections until the graph is full
* (all rooms have 3 to 6 outbound connections).
*/
void fillMap() {
	
	//create connections
	while (isGraphFull() == 0) {
		addRandomConnection();	
	}

}

/*
* isGraphFull
* Description: returns true if all rooms have 3 to 6 outbound connections, false otherwise
*/
int isGraphFull() {
	int i = 0;
	int tempBool = 1;
	while (i < 7 && tempBool == 1) {
		if (connectionCount[i] < 3) {
			tempBool = 0;
		}
		i++;
	} 
	return tempBool;
}



//Adds a random, valid outbound connection from a Room to another Room
void addRandomConnection() {
	if (isGraphFull() == 0) {
	
		int connectionMade = 0;
		while (connectionMade == 0) {
			//check to see if a connection from the first room can be made
			int room1 = randomInt(0, 6);
			while (canAddConnectionFrom(room1) == 0) {
				room1 = randomInt(0, 6);
			}	

			int room2 = randomInt(0, 6);
			//validate room2: make sure it is not the same room as room1 and a connection can be added
			while (room1 == room2 || canAddConnectionFrom(room2) == 0) {
				room2 = randomInt(0, 6);
			}

			//final check: ensure that the connection doesn't exist already
			if (connectivityMap[room1][room2] == 0 && connectivityMap[room2][room1] == 0) {
				connectivityMap[room1][room2] = 1;
				connectivityMap[room2][room1] = 1;
				connectionMade = 1;

				//update the number of connections connected rooms have
				connectionCount[room1]++;
				connectionCount[room2]++;
			
			}
		}
	}
}

//returns true if a connection can be added from Room x (<6 outbound connections), false otherwise
int canAddConnectionFrom(int x) {
	if (connectionCount[x] < 6) {
		return 1;
	}	
	else {
		return 0;
	}
}


int main() {
	//initialize seed
	srand(time(0));

	//initialize connectivityMap
	int i = 0;
	int j = 0;
	for (i = 0; i < 7; i++) {
		for (j = 0; j < 7; j++) {
			connectivityMap[i][j] = 0;
		}

	}

	//create the directory and rooms where rooms will reside in
	createRooms();


	return 0;
}
