/*********************************************************************
** Program Filename: keygen.c
** Author: Mario Franco-Munoz
** Due Date: 6/12/2018
** Description:CS344 Assignment 4:
**  
*********************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <assert.h>
#include <time.h>

//function prototypes
int randomInt(int, int);
void createFile(int);


//creates and poplulates the key
void createFile(int fileLen) {
	int randTemp;

	//allocate a string with length of file + \n character and '\0' character
	char *tempStr = malloc((fileLen + 2)*sizeof(char));	
	assert(tempStr != NULL);


	//loop through length of key, and add random characters
	for (int i = 0; i < fileLen; i++) {
		//generate a random integer within the ascii range of capital letters
		//91 signifies the '[' character, but we will treat it as a space character
		randTemp = randomInt(65, 91);
		if (randTemp == 91) {
			tempStr[i] = ' ';
		}
		else {
			tempStr[i] = (char)randTemp;
		}		  												 
	}
	//append the newline character
	tempStr[fileLen] = '\0';
	tempStr[fileLen - 1] = '\n';	

	//output to stdout
	fprintf(stdout, "%s", tempStr);


	free(tempStr);
}







//Function to create a random integer using rand(). Lower and upper bounds are inclusive.
//output is a pseudo random number in the specified range.
int randomInt(int low, int high) {
	return (rand() % (high - low + 1)) + low;

}



int main (int argc, char *argv[]) {
	int fileLength;	
	srand(time(0));  //initialize seed

	//get argument from	command line, check to make sure only one argument was passed
	if (argc != 2) {
		printf("No argument provided. Please try again.\n");
	}
	else {
		fileLength = atoi(argv[1]);
		createFile(fileLength);
	}
	

	return	0;
}
