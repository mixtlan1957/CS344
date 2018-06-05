/*********************************************************************
** Program Filename: otp_dec_d.c
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
char *decryptMessage(char *, char *);
void processData(int);
char *getMessage(char *);
char *getKey(char *, int);
int correctConnection(char *);


char *decryptMessage(char *message, char *key) {
	char *encMsg;
	int msgInt;
	int keyInt;
	int pminusk;
	int rem;
	char *header = "OTP_DEC";	
	int encMsgSize = strlen(message) + strlen(header) + 1;

	//malloc for size of encMsg, and size of header and null terminator
	encMsg = malloc(sizeof(char) * encMsgSize);
	memset(encMsg, '\0', encMsgSize);


	//conduct the decryption: Add the key to message then apply modulus 27
	for(int i = 0; i < strlen(message); i++) {

		//first check for space character
		if(message[i] == ' ') {
			msgInt = 26;
		}
		//if it's not a space character, get the ascii value then substract value of A
		else {
			msgInt = (int)message[i];
			msgInt -= 65;
		}
		//do the same for the key
		if (key[i] == ' ') {
			keyInt = 26;
		}
		else {
			keyInt = (int)key[i];
			keyInt -= 65;
		}		
		
		//substract key from message
		pminusk = msgInt - keyInt;

		//if number is less than zero, add 27
		if (pminusk < 0) {
			pminusk += 27;
		}

		//apply mod 27
		rem = pminusk % 27;

		//convert back to an uppercase character
		if (rem == 26) {
			encMsg[i] = ' ';
		}
		//if character doesn't correspond to a space...
		else {
			rem = rem + (65);	    //to get the char value add the ascii of 'A'	
			encMsg[i] = (char)rem;  //cast it into an upper case letter
		}
	}
	
	//add the header to the end of the file
	strcat(encMsg, header);

	return encMsg;
}

//function used to obtain the "key" portion from the full transmission
//this function is intended to be run AFTER the message has been obtained
//msgSize needs to be the strlen of message (not sizeof)
char *getKey(char *fullstr, int msgSize) {
	int startIdx;     //start index of message 
	char *key;    	//return variable  
	char *keyId = "$$KEY$$: "; 
	int foundStart = 0;
	int i = 0;

	//the key length has already been verified, thus we only need to grab as many characters are in the
	//message in order to encrypt. 
	while (i < strlen(fullstr) && foundStart == 0) {

		//search for the start of the message
		if (fullstr[i] == '$' && fullstr[i+1] == '$' && fullstr[i+2] == 'K' && fullstr[i+3] == 'E' && 
			 fullstr[i+4] == 'Y' && fullstr[i+5] == '$' && fullstr[i+6] == '$') {
			//store the start index of key
			foundStart = 1;
			startIdx = i + strlen(keyId);
		}
		i++;
	}

	//malloc for key
	key = malloc(sizeof(char) * (msgSize + 1));
	//copy the key contents over from the full message
	memcpy(key, &fullstr[startIdx], msgSize);
	key[msgSize] = '\0';

	return key;
}



//function used to obtain the "message" portion from the full transmission
char *getMessage(char *fullstr) {

	int startIdx;     //start index of message 
 	int endIdx;       //end index of message
	char *message;    //return variable
	char *msgId = "$$MESSAGE$$: ";   
	int foundStart = 0;
	int foundEnd = 0;
	int i = 0;
	int bitsCpy; 

	//parse the full string: the header we are looking for is 
	while (i < strlen(fullstr) && (foundStart == 0 || foundEnd == 0)) {

		//search for the start of the message
		if (fullstr[i] == '$' && fullstr[i+1] == '$' && fullstr[i+2] == 'M' && fullstr[i+3] == 'E' &&
			 fullstr[i+4] == 'S' && fullstr[i+5] == 'S') {

			//if we found the occurance of the message header, store its start location
			foundStart = 1;
			startIdx = i + strlen(msgId);	
		}

		//now we search for the end of the message index
		if (fullstr[i] == '$' && fullstr[i+1] == '$' && fullstr[i+2] == 'K' && fullstr[i+3] == 'E' 
			&& fullstr[i+4] == 'Y' && fullstr[i+5] == '$' && fullstr[i+6] == '$') {
			foundEnd = 1;
			endIdx = i-1;  //the end index is -1 due to the pointer currently pointing at start of 
						  //"$$KEY$$" and we don't need the \n character 
			 
		} 	
		//increment index
		i++;
	}

	//check for any errors
	if (foundStart == 0 || foundEnd == 0) {
		fprintf(stderr, "Something has gone terribly wrong: server cannot parse message.");
		printf("foundstart = %d, foundEnd = %d\n", foundStart, foundEnd);
	}
	
	//otherwise we are ready to copy the substring and go on about our merry way
	bitsCpy = endIdx - startIdx;
	message = malloc(sizeof(char) * bitsCpy + 1);
	memcpy(message, &fullstr[startIdx], bitsCpy);
	message[bitsCpy] = '\0';  //set the null terminator bit

	
	return message;
}




void processData(int port) {
	
	//store the input variable
	int portNumber = port;
	
	//variables for handling server communication
	int listenSocketFD, establishedConnectionFD, charsRead;
	socklen_t sizeOfClientInfo;
	char readBuffer[1000];
	struct sockaddr_in serverAddress, clientAddress;
	int errorFlag = 0;
	int readIncrement = 1000;  
	int msgSize = 1000;
	char *completeMsg = NULL;
	char *key = NULL;         
	char *msg = NULL;
	char *decrypted = NULL;
	pid_t spawnPid; 

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

	//set up for reuse to avoid problems
	//source: GATECH lecture notes	
	int optval = 1;
	setsockopt(listenSocketFD, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));


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
		// Accept a connection, blocking if one is not available until one connects
		sizeOfClientInfo = sizeof(clientAddress); // Get the size of the address for the client that will connect
		establishedConnectionFD = accept(listenSocketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); // Accept
		if (establishedConnectionFD < 0) {
			fprintf(stderr, "ERROR on accept\n");
			printf("error on accept\n");
			goto cleanup;
		}
		
			
		//FORK HERE!
		spawnPid = fork();
	

		//validate fork/pid

		if (spawnPid == -1) {      //spawn child error case
								
			perror("Hull breach! fork error!\n");
			goto cleanup;
		}

		//child case
		if (spawnPid == 0) {
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
			retval = select(maxFD + 1, &readFDs, NULL, NULL, &timeToWait);
			if (retval == -1) {
				errorFlag = 1;
				fprintf(stderr, "Error on client recieving socket.");
				goto cleanup;
			}
	

			//if the resource is available for reading, read the stream
			else if (retval != 0) {
				if (FD_ISSET(establishedConnectionFD, &readFDs)) {

					//variables to keep track of what we are currently reading and if we found the EOT descriptor
					int currentRead = 0;
					char *tempStr = NULL;
					int found = 0;

					//get message from client
					completeMsg = malloc(sizeof(char) * msgSize);
					memset(readBuffer, 0, 1000);

					// Read the client's message from the socket
					charsRead = recv(establishedConnectionFD, readBuffer, 1000, 0); 
					if(charsRead < 0) {
						fprintf(stderr, "ERROR reading from socket\n");
						printf("error reading from socket\n");
						goto cleanup;
						errorFlag = 1;
					}
										
					//if initial read was a success, cat over the first read to "completeMsg"
					strcat(completeMsg, readBuffer);
					if (strstr(completeMsg, "$$EOT$$") != NULL) { //unless we already found the EOT flag
						found = 1;
					}
		
					//read the complete message (continue until we have reached the EOT flag)			
					while (found == 0) {
						//reset readBuffer
						memset(readBuffer, 0, 1000);
		
						//check if we need to increase the size of the where the message is being stored
						if (charsRead + readIncrement >= sizeof(completeMsg)) {
							//resize completeMsg as necessasry
							msgSize = msgSize * 2;	
							tempStr = malloc(sizeof(char) * msgSize);
						
							//copy over what we already read
							memcpy(tempStr, completeMsg, charsRead);
							free(completeMsg);
							completeMsg = tempStr;
						}
				
						//read the next chunk of bytes	
						currentRead = recv(establishedConnectionFD, readBuffer, 1000, 0);	
				
						//check for any errors
						if (currentRead < 0) {
							fprintf(stderr, "ERROR reading from socket\n");
							goto cleanup;
						}
						else {
							charsRead += currentRead;
							strcat(completeMsg, readBuffer);
							if (strstr(completeMsg, "$$EOT$$") != NULL) {
								found = 1;
							}
						}											
					}				

					//validate that the header is correct (that we are talking to otp_dec)
					if (strstr(completeMsg, "HEADER_OTP_DEC") == NULL) {
						//fprintf(stderr, "Wrong client connection.\n");
						shutdown(establishedConnectionFD, 2);
						errorFlag = 1;
						goto cleanup;
					}

					//parse the message
					msg = getMessage(completeMsg);

					//parse the key
					key = getKey(completeMsg, strlen(msg));

					//generate encrypted message
					decrypted = decryptMessage(msg, key);

					//send the client the encrypted message
					ssize_t byteSent = send(establishedConnectionFD, decrypted, 1000, 0);
					if (byteSent < 0) {
						fprintf(stderr, "SERVER: Send ERROR\n");
						errorFlag = 1;
						goto cleanup;
					}
					int currentSend;
					//if initial send was successful, send in 1000 byte chunks until it is completly delivered
					while(byteSent < strlen(decrypted)) {     
						//send out the next chunk of data
						currentSend = send(establishedConnectionFD, &completeMsg[byteSent], 1000, 0);  
		
						//check for send errors
						if( currentSend < 0) {
								fprintf(stderr, "SERVER: Send ERROR\n");
								errorFlag = 1;
								goto cleanup;
						}
						//if our send was successful, tally up the bytes to move the pointer
						else {
							byteSent += currentSend;
						}
					}										
				}
			}
			//free malloc'd strings
			if (decrypted != NULL) {
				free(decrypted);
			}
			if (msg != NULL) {
				free(msg);
			}
			if (key != NULL) {
				free(key);
			}
			if (completeMsg != NULL) {
				free(completeMsg);
			}
			//close the socket we were using		
			close(establishedConnectionFD);
		}	
	}	


	cleanup:


	//free malloc'd strings
	if (decrypted != NULL) {
		free(decrypted);
	}
	if (msg != NULL) {
		free(msg);
	}
	if (key != NULL) {
		free(key);
	}
	if (completeMsg != NULL) {
		free(completeMsg);
	}


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
		argRead = argv[1];
		listeningPort = atoi(argRead);
		//include int checking statement here later 
	
		processData(listeningPort);
			
	}



	return 0;
}



