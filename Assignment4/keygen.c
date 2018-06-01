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

//function prototypes
int randomInt(int, int);
int createFile(int);


//creates file and poplulates the key
int createFile(int fileLen) {
	FILE *f;   				// file opbject declaration
	//allocate a string with length of file + \n character
	char *tempStr = malloc((fileLen + 1)*sizeof(char));	
	assert(tempStr != NULL);


	//loop through length of key, and add random characters
	for (int i = 0; i < fileLen; i++) {
			  												 

	}
	//append the newline character
	tempStr[fileLen] = '\n';	

	

	//open file for writing
	f = fopen("mykey", "w");  //truncate any existing file
	fprintf(f, "%s", tempStr);

		

	fclose(f);
	free(tempStr);
}







//Function to create a random integer using rand(). Lower and upper bounds are inclusive.
//output is a pseudo random number in the specified range.
int randomInt(int low, int high) {
	return (rand() % (high - low + 1)) + low;

}



int main (int argc, char *argv[]) {
	int fileLength;	


	//get argument from	command line, check to make sure only one argument was passed
	if (argc != 1) {
		perror("No argument provided. Please try again.")
	}
	else {
		fileLength = atoi(argv[0]);
	}






	return	0;
}
