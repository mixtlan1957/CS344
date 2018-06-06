/*********************************************************************
** Program Filename: otp_dec.c
** Author: Mario Franco-Munoz
** Due Date: 6/12/2018
** Description:CS344 Assignment 4: otp_dec takes an encrypted message, a key
**  asks otp_dec_d to decrypt the message and outputs the result to stdout.
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
	char *message;      //initial read of msg from file
	char *key;			//initial read of key from file
	ssize_t msgLen;    //outbound message length        
	ssize_t keyLength; //outbound key length
	size_t buffersize = 0;
	int ascii;
	//variables for sending client data to server
	int portNumber = atoi(port);
	int socketFD, charsRead;
	struct sockaddr_in serverAddress;
	struct hostent* serverHostInfo;
	char *completeMsg = NULL; 	
	char *recMsg = NULL;
	char *finalOutput = NULL;
	int currentRead = 0;      //variables to keep track of where we are in the sending process
	int currentSend = 0;
	char readBuffer[1000];

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
    		fprintf(stderr, "otp_dec error: input contains bad characters\n");
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


	//combine all the necessary strings into one big message to send to the server
	completeMsg = createMessage(message, key);	
	
	int errorFlag = 0;  //simple error flag to keep track of weather or not there was an error so we can exit with 1	
	socketFD = 0;	

	//send to otp_enc_d (encryption dameon)
	memset((char*)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
 	serverAddress.sin_family = AF_INET;  //create a network-cable socket (IPV4)
	serverAddress.sin_port = htons(portNumber); // Store the port number	
	serverHostInfo = gethostbyname("localhost"); // Convert the machine name into a special form of address
	//validate host
	if (serverHostInfo == NULL) { 
		fprintf(stderr, "CLIENT: ERROR, no such host\n");
		errorFlag = 1; 
		goto cleanup;
	}
	//copy in the address
	memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length);	
	
	//Set up the socket
	socketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (socketFD < 0) { 
		fprintf(stderr, "CLIENT: ERROR opening socket\n");
		errorFlag = 1;
		goto cleanup;
	}

	//connect to server
	if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) { // Connect socket to address
		fprintf(stderr, "CLIENT: ERROR connecting\n");
		errorFlag = 1;
		goto cleanup;
	}
	
	msgLen = strlen(completeMsg);
	//send data to server, starting with the message
	ssize_t byteSent = send(socketFD, completeMsg, 1000, 0);
	if (byteSent < 0) {
		fprintf(stderr, "CLIENT: Send ERROR\n");
		errorFlag = 1;
		goto cleanup;
	}
	//send message in 1000 byte chunks until it is completely delivered
	while(byteSent < msgLen) {     
		currentSend = send(socketFD, &completeMsg[byteSent], 1000, 0);   //send out the next chunk of data
		
		//check for send errors
		if( currentSend < 0) {
			fprintf(stderr, "CLIENT: Send ERROR\n");
			errorFlag = 1;
			goto cleanup;
		}
		//if our send was successful, tally up the bytes to move the pointer
		else {
			byteSent += currentSend;
		}	
	}	

	//get return from server
	//calculate how long the return statement from server will be:
	//header + encrypted message + newline character + null terminator
	char *incomingHeader = "OTP_DEC";
	msgLen = strlen(message) + strlen(incomingHeader) + 2; //strlen does not include null terminator acording to the interwebs

	
	//allocate the recieving buffer
	recMsg = malloc(sizeof(char) * msgLen );
	int temp = sizeof(recMsg);   //the fact that GNU complains about sizeof being declared inside memset is very annoying
	memset(recMsg, '\0', temp);      

	//read the first chunk of data	
	charsRead = recv(socketFD, readBuffer, 1000, 0);

	//error handling
	if (charsRead == -1) {
		fprintf(stderr, "Error recieving message from otp_dec_d (to otp_dec)\n");
		errorFlag = 1;
		goto cleanup;
	}
	//check for aborted server connection
	else if (charsRead == 0) {
		 fprintf(stderr, "Error: could not contact otp_enc_d on port %d\n", portNumber);
		 errorFlag = 1;
    	 goto cleanup;
	}
	else {
		strcat(recMsg, readBuffer);
	}
		
	//if all is good, continue reading the stream
	while (charsRead < msgLen) {
		//reset buffer and read next chunk
		memset(readBuffer, 0, sizeof(readBuffer));
		currentRead = recv(socketFD, readBuffer, 1000, 0);	
		//check for error
		if (currentRead == -1) {
			fprintf(stderr, "Error recieving message from otp_dec_d (to otp_dec)\n");
			errorFlag = 1;
			goto cleanup; 
		}
		//check if server aborted connection
	    else if (currentRead == 0) {
	        fprintf(stderr, "Server aborted connection.\n");
			errorFlag = 1;
		    goto cleanup;
	    }
		else {
			charsRead += currentRead;
			strcat(recMsg, readBuffer);
		}			
	}  	

	//check to make sure we are comunicated with the correct server
	if (strstr(recMsg, "OTP_DEC") == NULL) {
		fprintf(stderr, "Comunicated from incorrect server.\n");
		errorFlag = 1;
		goto cleanup;
	}	
    
	//strip off the header/identifier
	temp = strlen(recMsg) - strlen("OTP_DEC");
	memset(&recMsg[temp], '\0', strlen("OTP_DEC")); 
	
	//add newline character
	strcat(recMsg, "\n");

	//output to stdout
	fprintf(stdout, recMsg);

	//free memory allocated for strings
	cleanup:
	if (completeMsg != NULL) {
		free(completeMsg);
	}
	if (message != NULL) {	
		free(message);
	}
	if (key != NULL) {
		free(key);
	}
	if (recMsg != NULL) {
		free(recMsg);
	}
	if(finalOutput != NULL) {
		free(finalOutput);
	}
	if (socketFD != 0) {
		close(socketFD);
	}
	
	if (errorFlag == 1) {
		exit(1);
	}
	if (finalOutput	!= NULL) {
		free(finalOutput);
	}
	if (socketFD != 0) {
		close(socketFD);
	}
}

//combine all the different strings that need to be sent to the server
char *createMessage(char *message, char *key) {
	int len; 
	//we need to input identifiers of the different parts of the message
	char *header = "HEADER_OTP_DEC "; //indentifies the sender
	char *msgId = "$$MESSAGE$$: ";   //indicates start of message to be decrypted
	char *keyId = "$$KEY$$: ";      //indicates start of key for decryption
	char *EOT = "$$EOT$$\n\0";			//indcates the end of the transmission

	len = strlen(header) + strlen(msgId)  + strlen(message) + strlen(keyId)  + strlen(key) + strlen(EOT);
	char *msg = malloc(sizeof(char) * (len + 1)); //leave space for the null terminator
	len = sizeof(msg); 
	memset(msg, '\0', len);

	//combine strings
	memcpy(msg, header, strlen(header)); 
	strcat(msg, msgId);
	strcat(msg, message);
	strcat(msg, keyId);
	strcat(msg, key);
	strcat(msg, EOT);
	
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
		fprintf(stderr,"Incorrect number of arguments passed to function.\n");	
	}
	else {
		sendMessage(argv[1], argv[2], argv[3]);	

	}	



		        


	return 0;
}
