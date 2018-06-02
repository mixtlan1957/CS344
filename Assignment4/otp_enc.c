/*********************************************************************
** Program Filename: otp_enc.c
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

//function prototypes
void error(const char *);
void sendMessage(char*, char*, char*);
char* createMessage(char*, char*);



//this function checks that the input arguments are valid
//if input is valid, cypher and mesage to be encrypted are sent to the
//server on the user defined port number
void sendMessage(char* ptextFileName, char* keyFileName, char* port) {
	//variables associated with reading files and file contents
	FILE *messageFile;
	FILE *keyFile;
	char *message;
	char *key;
	ssize_t msgLen;    //outbound message length        
	ssize_t keyLength; //outbound key length
	size_t buffersize = 0;
	int ascii;
	//variables for sending client data to server
	int portNumber = atoi(port);
	int socketFD, charsWritten, charsRead;
	struct sockaddr_in serverAddress;
	struct hostent* serverHostInfo;
	char *completeMsg; 	


	//first check to make sure that we are getting actual data and not null arguments			
	if (ptextFileName == NULL || keyFileName == NULL || port == NULL) {
		fprintf(stderr, "Error processing data.\n");
		exit(1);	
	}

	//open the text and cypher files
	messageFile	= fopen(ptextFileName, "r");
    keyFile = fopen(keyFileName, "r");
	if (messageFile == NULL) {
		fprintf(stderr, "Error opening message file.\n");
		exit(1);
	}
	if (keyFile == NULL) {
		fprintf(stderr, "Error opening key file.\n");
		exit(1);
	}

    msgLen = getline(&message, &buffersize, messageFile);
    keyLength = getline(&key, &buffersize, keyFile);

    //check to make sure that the key isn't too short
    if (msgLen > keyLength) {
    	fprintf(stderr, "Error: key '%s' is too short\n", keyFileName);

    	//free memory and close files upon forceful exit	
    	free(message);
    	free(key);
    	fclose(messageFile);
    	fclose(keyFile);	

    	exit(1);	
    }
	
	//check if the files are empty
	if (msgLen <=0) {
		fprintf(stderr, "Empty or unreadable message file.\n");
		if (message != NULL) { //avoid a seg fault by only freeing memory if not set to null in this case
			free(message);
		}		
		if (key != NULL) {
			free(key);
		}
		fclose(messageFile);
		fclose(keyFile);
		exit(1);
	}
	if (keyLength <= 0) {
		fprintf(stderr, "Empty or unreadble key file.\n");
		if (message != NULL) {
			free(message);
		}		
		if (key != NULL) {
			free(key);
		}
		fclose(messageFile);
		fclose(keyFile);
		exit(1);
	}


    //validate the message: (should only contain capital letters and spaces)
    for (int i = 0; i < msgLen - 1 ; i++) {
    	ascii = (int)message[i]; //cast into an int so we don't have to use a huge if statement
    	if ( (ascii < 65 || ascii > 90) && ascii != 32 ) {
    		fprintf(stderr, "otp_enc error: input contains bad characters");
			free(message);
			free(key);
			fclose(messageFile);
			fclose(keyFile);
			exit(1);   //free memory and exit with an error code
    	}
    }	

    //close files	
    fclose(messageFile);
    fclose(keyFile);	

	/*
	
	//send to otp_enc_d (encryption dameon)
	memset((char*)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
 	serverAddress.sin_family = AF_INET;  //create a network-cable socket (IPV4)
	serverAddress.sin_port = htons(portNumber); // Store the port number	
	serverHostInfo = gethostbyname("localhost"); // Convert the machine name into a special form of address
	//validate host
	if (serverHostInfo == NULL) { 
		fprintf(stderr, "CLIENT: ERROR, no such host\n"); 
		exit(1);
	}
	//copy in the address
	memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length);	
	
	//Set up the socket
	socketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (socketFD < 0) { 
		error("CLIENT: ERROR opening socket");
	}

	//connect to server
	if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) { // Connect socket to address
		error("CLIENT: ERROR connecting");
	}
	*/

	//combine all the necessary strings into one big message to send to the server
	completeMsg = createMessage(message, key);	
	

	/*
	//send data to server, starting with the message
	ssize_t byteSent = send(socketFD, message, 1000, 0);
	while(byteSent < msgLen) {
		byteSent += send(socketFD, message, 1000, 0);
	}
	
	*/
	
	//get return from server


	//output to stdout


	//free memory allocated for strings
	free(completeMsg);
	free(message);
    free(key);
}

//combine all the different strings that need to be sent to the server
char *createMessage(char *message, char *key) {
	int len; 
	//we need to input identifiers of the different parts of the message
	char *header = "HEADER_OTP_ENC "; //indentifies the sender
	char *msgId = "$$MESSAGE$$: ";   //indicates start of message to be encrypted
	char *keyId = "$$KEY$$: ";      //indicates start of key to be encrypted

	len = strlen(header) + strlen(msgId)  + strlen(message) + strlen(keyId)  + strlen(key);
	char *msg = malloc(sizeof(char) * len + 1);

	//combine strings
	memcpy(msg, header, strlen(header)); 
	strcat(msg, msgId);
	strcat(msg, message);
	strcat(msg, keyId);
	strcat(msg, key);
	msg[len] = '\0';		

	return msg;

}

/*
// Error function used for reporting issues
void error(const char *msg) { 
	fprintf(stderr, "Error Message: %s\n", sterror(errno)); 
	exit(1); 
} 
*/
int main(int argc, char *argv[]) {	

	//make sure that three arguments are being passed (plaintext, key, port) - four including the program itself	
	if (argc != 4) {
		fprintf(stderr,"Incorrect number of arguments passed to function.");	
	}
	else {
		sendMessage(argv[1], argv[2], argv[3]);	

	}	



		        


	return 0;
}
