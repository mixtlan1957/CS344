/*********************************************************************
** Program Filename: otp_enc.c
** Author: Mario Franco-Munoz
** Due Date: 6/12/2018
** Description:CS344 Assignment 4:
**  
*********************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

//function prototypes
void error(const char *);
void sendMessage(char*, char*, int);







void sendMessage(char* ptextFileName, char* keyFileName, char* port) {
	FILE *messageFile;
	FILE *keyFile;
	char *message;
	char *cypher;
	int msgLen;
	int cypherLen;
	size_t buffersize = 0;
	int ascii;
	int portRead = atoi(port); 	


	//first check to make sure that we are getting actual data and not null arguments			
	if (ptextFileName == NULL || key == NULL || port == NULL) {
		fprintf(stderr, "Error processing data.\n");
		exit(1);	
	}

	//open the text and cypher files
	messageFile	= open(ptextFileName, "r");
    keyFile = open(keyFileName, "r");

    msgLen = getline(&message, &buffersize, messageFile);
    cypherLen = getline(&cypher, &buffersize, keyFile);

    //check to make sure that the files are of the same length
    if (msgLen != cypherLen) {
    	if (cypherLen < msgLen) {
    		fprintf(stderr, "Error: key '%s' is too short\n", keyFileName);
    	}
    	else {
    		fprintf(stderr, "Error: key '%s' is too long\n", keyFileName);
    	}

    	//free memory and close files upon forceful exit	
    	free(message);
    	free(cypher);
    	close(messageFile);
    	close(keyFile);	

    	exit(1);	
    }

    //validate the message: (should only contain capital letters and spaces)
    for (int i = 0; i < msgLen; i++) {
    	ascii = (int)message[i]; 
    	if ( (ascii < 65 && ascii != 32) || (ascii > 90  && ascii != 32) ) {
    		fprintf(stderr, "otp_enc error: input contains bad characters");
    	}
    }	

    //free memory and close files	
	free(message);
    free(cypher);
    close(messageFile);
    close(keyFile);	

}

// Error function used for reporting issues
void error(const char *msg) { 
	fprintf(stderr, "Error Message: %s\n", strerror(errorno)); 
	exit(1); 
} 

int main(int argc, char *argv[]) {
	char* cypherText = NULL;	


	//make sure that three arguments are being passed (plaintext, key, port) - four including the program itself	
	if (argc != 4) {
		error("Incorrect number of arguments passed to function.");	
	}
	else {
		sendMessage(argv[1], argv[2], argv[3])	

	}	








/*
		        
	int socketFD, portNumber, charsWritten, charsRead;
	struct sockaddr_in serverAddress;
	struct hostent* serverHostInfo;
	char buffer[256];
    
	if (argc < 3) { 
		fprintf(stderr,"USAGE: %s hostname port\n", argv[0]); 
		exit(0); 
	} // Check usage & args


	// Set up the server address struct
	memset((char*)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	portNumber = atoi(argv[2]); // Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); // Store the port number
	serverHostInfo = gethostbyname(argv[1]); // Convert the machine name into a special form of address
	if (serverHostInfo == NULL) { fprintf(stderr, "CLIENT: ERROR, no such host\n"); exit(0); }
	//copy in the address
	memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length);

	// Set up the socket
	socketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (socketFD < 0) error("CLIENT: ERROR opening socket");
	
	// Connect to server
	if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to address
		error("CLIENT: ERROR connecting");

	// Get input message from user
	printf("CLIENT: Enter text to send to the server, and then hit enter: ");
	memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer array
	fgets(buffer, sizeof(buffer) - 1, stdin); // Get input from the user, trunc to buffer - 1 chars, leaving \0
	buffer[strcspn(buffer, "\n")] = '\0'; // Remove the trailing \n that fgets adds

	// Send message to server
	charsWritten = send(socketFD, buffer, strlen(buffer), 0); // Write to the server
	if (charsWritten < 0) error("CLIENT: ERROR writing to socket");
	if (charsWritten < strlen(buffer)) printf("CLIENT: WARNING: Not all data written to socket!\n");

	// Get return message from server
	memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer again for reuse
	charsRead = recv(socketFD, buffer, sizeof(buffer) - 1, 0); // Read data from the socket, leaving \0 at end
	if (charsRead < 0) error("CLIENT: ERROR reading from socket");
	printf("CLIENT: I received this from the server: \"%s\"\n", buffer);

	close(socketFD); // Close the socket

	*/
	return 0;
}
