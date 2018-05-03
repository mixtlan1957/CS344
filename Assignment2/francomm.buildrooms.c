/*********************************************************************
** Program Filename: francomm.buildrooms.c
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
#include <unistd.h>
#include <sys/types.h>  //for creating directory
#include <sys/stat.h>

//GLOBAL VARIABLES
const char *roomTypes[3] = { "START_ROOM", "MID_ROOM", "END_ROOM" };
const char *roomNames[10] = { "Azcanta", "Kolios", "BandRoom", "Water",
"Wind", "Fire", "Ice", "Alara", "Ravnica", "Monster" };


//#include <stdlib.h>




//struct Room {
//	char *roomName;
//	Room *connection1;
//	Room *connection2;
//	Room *connection3;
//	Room *connection4;
//	Room *connection5;
//	Room *connection6;
//};

//Array of different room types
//void defineRoomTypes() {
//	char *roomTypes[3];
//	roomTypes[0] = "START_ROOM";
//	roomTypes[1] = "MID_ROOM";
//	roomTypes[2] = "END_ROOM";
//}

//Create all seven rooms
void createRooms() {
	int i, n;
	int temp;
	int connections;
	int roomSelector;
	time_t t;
	struct stat st = {0};
	FILE *roomFile = NULL;
	//char *roomName;

	//create directory
	//get the current process id
	temp = getpid();
			
	//convert the process id to a string
	char snum[20];
	memset(snum, '\0', 20);
	sprintf(snum, "%d", temp);		
		
	//testing
	printf("%d", snum);	

	
	//declare dir name
	char dirName[256];
	memset(dirName, '\0', 256);
	strcpy(dirName, "/francomm.room.");
		
	//append the process id
	strcat(dirName, snum);
	strcat(dirName, "/");
	
	//if directory does not exist, create it
	if (stat(dirName, &st) == -1) {
		mkdir(dirName, 0700);
	}



	//create the 7 rooms files with the different process id 
	for (i = 0; i < 7; i++) {
		//use a temp string for room file creation
		char tempDirName[256];
		memset(tempDirName, '\0', 256);
		strcpy(tempDirName, dirName);
		//append the name of the room to the file
		strcat(tempDirName, roomNames[i]);
		
		//create the file
		roomFile = fopen(tempDirName, "w");
		fprintf(roomFile, "ROOM NAME: %s", roomNames[i]);

		//assign the room a random number of connections
		unsigned modify = (unsigned)i;
		srand((unsigned)time(&t) + modify);
		connections = rand()%4 + 3;


		//close file and set roomFile pointer to null
		fclose(roomFile);

	}


}

//Returns true if all rooms have 3 to 6 outbound connections, false otherwise




//Adds a random, valid outbound connection from a Room to another Room






//Returns a random Room, does NOT validate if connection can be added




//returns true if a connection can be added from Room x (<6 outbound connections), false otherwise





//connects rooms x and y together, does not check if this connection is valid





//returns true if rooms x and y are the same room, false otherwise

int main() {
	createRooms();


	return 0;
}
