/*********************************************************************
** Program Filename: otp_enc_d.c
** Author: Mario Franco-Munoz
** Due Date: 6/12/2018
** Description:CS344 Assignment 4:
**  
*********************************************************************/
#define _GNU_SOURCE   //to be able to use getline without problems
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <time.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/time.h>

//global variables
#define NUM_THREADS 1
char *argRead;
int listeningPort;
int portUsage;



//funcntion prototypes
char *encryptMessage(char *, char *);
void *processData(void *);
char *getMessage(char *);
char *getKey(char *);


/*
char *encryptMessage(char *message, char *key) {





}


char *getMessage(char *fullStr) {

	int startIdx;
	int endIdx;
	char *message;
	
	
	

}

*/


void *processData(void* port) {
	printf("hey we're in!\n");
	
	//store the input variable
	int portNumber = *((int *)port);
	
	//variables for handling server communication
	int listenSocketFD, establishedConnectionFD, charsRead;
	socklen_t sizeOfClientInfo;
	char *readBuffer = NULL;
	struct sockaddr_in serverAddress, clientAddress;
	int errorFlag = 0;
	int readIncrement = 1000;   //variable for keeping track of how many bytes are read per recv
	int readBufferSize = 1000;
	char *completeMsg = NULL;

	// Set up the address struct for this process (the server)
	memset((char *)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); // Store the port number
	serverAddress.sin_addr.s_addr = INADDR_ANY; // Any address is allowed for connection to this process

	// Set up the socket
	listenSocketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (listenSocketFD < 0) {
		fprintf(stderr, "ERROR opening socket\n");
		printf("Error opening socket\n");
		goto cleanup;
	}	

	//Enable the socket to begin listening
	// Enable the socket to begin listening
	if (bind(listenSocketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {// Connect socket to port
		fprintf(stderr, "ERROR on binding\n");
		printf("Error on binding\n");
		goto cleanup;
	}
	listen(listenSocketFD, 5); // Flip the socket on - it can now receive up to 5 connections

	//primary loop
	while(1) {
		printf("we made it inside the loop.\n");
		// Accept a connection, blocking if one is not available until one connects
		sizeOfClientInfo = sizeof(clientAddress); // Get the size of the address for the client that will connect
		establishedConnectionFD = accept(listenSocketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); // Accept
		if (establishedConnectionFD < 0) {
			fprintf(stderr, "ERROR on accept\n");
			printf("error on accept\n");
		}
		
	
		//setup select variables
		fd_set readFDs;
		fd_set writeFDs;
		struct timeval timeToWait;
		int retval;
		int maxFD;  //max file descriptor

		//watch the sockets for when they have input/output
		FD_ZERO(&readFDs);
		FD_ZERO(&writeFDs);
		FD_SET(establishedConnectionFD, &readFDs);
		FD_SET(establishedConnectionFD, &writeFDs);


		//Wait up to 50 seconds per loop cycle
		timeToWait.tv_sec = 50;
		timeToWait.tv_usec = 0;


		//determine that max file descriptor and add 1
		maxFD = establishedConnectionFD;
		
		//call select to read on listenSocketFD
	//	retval = select(maxFD + 1, &readFDs, NULL, NULL, &timeToWait);
//		printf("this is our retval: %d\n", retval);
//		if (retval == -1) {
//			errorFlag = 1;
//			goto cleanup;
//		}
	

		//if the resource is available for reading, read the stream
		//else if (retval != 0) {
		//	if (FD_ISSET(establishedConnectionFD, &readFDs)) {

				//get message from client
				readBuffer = malloc(sizeof(char) * 1000);
				memset(readBuffer, 0, 1000);
				charsRead = recv(establishedConnectionFD, readBuffer, 1000, 0); // Read the client's message from the socket
				if(charsRead < 0) {
					fprintf(stderr, "ERROR reading from socket\n");
					printf("error reading from socket\n");
					goto cleanup;
				}

				//if read was successful copy over first chunk of bytes to our mesage variable
				completeMsg = malloc(sizeof(char) * 1000); 
				memcpy(completeMsg, readBuffer, 1000);
				
				//variables to keep track of what we are currently reading and if we found the EOT descriptor
				int currentRead = 0;
				char *tempStr = NULL;
				int found = 0;
				//read the complete message (continue reading until we have obtained the end of transmission flag)
				//reset the buffer
			
				while (found == 0) {

					memset(readBuffer, 0, 1000); 	//reset the read buffer
					//check if we need to increase the size of the where the message is being stored
					/*
					if (charsRead + readIncrement >= sizeof(completeMsg)) {
						readBufferSize = readBufferSize * 2;	
						tempStr = malloc(sizeof(char) * readBufferSize);
						
						int tempInt = sizeof(readBuffer);
						memcpy(tempStr, readBuffer, tempInt);
						free(readBuffer);
						readBuffer = tempStr;
					}
					*/
					//read the next chunk of bytes	
					currentRead = recv(establishedConnectionFD, readBuffer, 1000, 0);	
					printf("Bytes read inside the loop %d\n", currentRead);
					//check for any errors
					if (currentRead < 0) {
						fprintf(stderr, "ERROR reading from socket\n");
						goto cleanup;
					}
					else {
						charsRead += currentRead;
						if (strstr(completeMsg, "$$KEY$$")) {
							found = 1;
						}
					}
					strcat(completeMsg, readBuffer);											
				}				

				printf("SERVER: I recieved this from the client: \"%s\"\n", completeMsg);
		//	}
	//	}
	}	


	cleanup:

	if (errorFlag == 1) {
		exit(1);
	}
	return NULL;
}



int main(int argc, char *argv[]) {
	//initialize gobal thread tracking variable
	portUsage = 0;

	//make sure that only one argument is being passed (port number)
	if (argc != 2) {
		fprintf(stderr, "Incorrect number of arguments passed to function.\n");
	}
	else {
		argRead = argv[1];
		listeningPort = atoi(argRead);
		//include int checking statement here later 
	
		processData((void *) &listeningPort);
			
		/*
		//create 5 threads - implement this part last
		pthread_t threads[NUM_THREADS];
		int result_code;
		for (int i = 0; i < NUM_THREADS; i++) {
			result_code = pthread_create(&threads[i], NULL, processData, (void *) &listeningPort);
			if (result_code != 0) {
				fprintf(stderr, "Error creating threads...\n");
			}
			printf("what what in the butt%d\n", result_code);
		}
		*/	

	}



	return 0;
}



