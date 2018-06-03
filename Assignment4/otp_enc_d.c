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

//global variables
#define NUM_THREADS 5
int listeningPort;
int portUsage;



//funcntion prototypes
char *encryptMessage(char *, char *);
void *processData(int);
char *fragMessage(char *);











char *encryptMessage(char *message, char *key) {



}



char *fragMessage(char *) {


}


void *processData(void* port) {
	//store the input variable
	int portNumber = *((int *)port);

	//variables for handling server communication
	int listenSocketFD, establishedConnectionFD, charsRead;
	socklen_t sizeOfClientInfo;
	char *readBuffer = NULL;
	struct sockaddr_in serverAddress, clientAdress;
	int errorFlag = 0;


	// Set up the address struct for this process (the server)
	memset((char *)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); // Store the port number
	serverAddress.sin_addr.s_addr = INADDR_ANY; // Any address is allowed for connection to this process


	// Set up the socket
	listenSocketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (listenSocketFD < 0) {
		fprintf(stderr, "ERROR opening socket\n");
		goto cleanup;
	}	

	//Enable the socket to begin listening
	// Enable the socket to begin listening
	if (bind(listenSocketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {// Connect socket to port
		error("ERROR on binding");
	}
	listen(listenSocketFD, 5); // Flip the socket on - it can now receive up to 5 connections

	//primary loop
	while(1) {

		// Accept a connection, blocking if one is not available until one connects
		sizeOfClientInfo = sizeof(clientAddress); // Get the size of the address for the client that will connect
		establishedConnectionFD = accept(listenSocketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); // Accept
		if (establishedConnectionFD < 0) error("ERROR on accept");

		
	
		//setup select variables
		fd_set readFDs;
		fd_set writeFDs;
		struct timeval timeToWait;
		int retval;
		int maxFD;  //max file descriptor

		//watch the sockets for when they have input/output
		FD_ZERO(&readFDs);
		FD_ZERO(&writeFDs);
		FD_SET(listenSocketFD, readFDs);
		FD_SET(establishedConnectionFD, writeFDs);


		//Wait up to 50 seconds per loop cycle
		timeToWait.tv_sec = 50;
		timeToWait.tv_usec = 0;


		//determine that max file descriptor and add 1
		if (listenSocketFD > establishedConnectionFD ) {
			maxFD = listenSocketFD;
		}
		else {
			maxFD = establishedConnectionFD;
		}
		maxFD++;

		//call select to read on listenSocketFD
		retval = select(maxFD, &listenSocketFD, NULL, NULL, &timeToWait);

		if (retval == -1) {
			errorFlag = 1;
			goto cleanup;
		}
		//if the resource is available for reading, read the stream
		else if (retval != 0) {
			//get message from client
			readBuffer = malloc(sizeof(char) * 1000);
			memset(readBuffer, '\0', 1000);
			charsRead = recv(establishedConnectionFD, buffer, 1000, 0); // Read the client's message from the socket
			if(charsRead < 0) {
				fprintf(stderr, "ERROR reading from socket\n");
				goto cleanup;
			}
			printf("SERVER: I recieved this from the client: \"%s\"\n", buffer);

		}
	}	


	cleanup:

	if (errorFlag == 1) {
		exit(1);
	}
}




int main(int argc, char *argv[]) {
	//initialize gobal thread tracking variable
	portUsage = 0;

	//make sure that only one argument is being passed (port number)
	if (argc != 2) {
		fprintf(stderr, "Incorrect number of arguments passed to function.\n");
	}
	else {
		listeningPort = argv[1]; 
	}

	//create 5 threads - implement this part last
	pthread_t threads[NUM_THREADS];
	int thread_args[NUM_THREADS];
	int result_code, index;
	for (int i = 0; i < NUM_THREADS; i++) {
		threads_args[index] = index;
		result_code = pthread_create(&threads[index], NULL, processData, &listeningPort);
		if (result_code != 0) {
			fprintf(stderr, "Error creating threads...\n");
		}
	}





	return 0;
}



