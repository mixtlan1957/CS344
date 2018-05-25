/**********************************************************************
** Program FileName:  smallish.c
** Author: Mario Franco-Munoz
** Due Date: 5/27/2018
** Description: CS344 Block 3 Assignment: small implementation of a bash like shell
** 
*********************************************************************/

#define _GNU_SOURCE    //no idea why, but i get a bunch of extra errors if i don't include this
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>     //POSIX operating system API
#include <sys/types.h>   //for process id and types
#include <sys/stat.h>   //for umask, and file permissions
#include <sys/wait.h>  //for WIFEXITED, WNOHANG, etc 
#include <signal.h>   //for signal set usage
#include <string.h>  //string manipulation
#include <fcntl.h>  //for file creation flags (O_TRNC etc)

//global variables
int GLOBAL_STATUS; //status variable for checking how a process exited/terminated (particularly, child processes)
struct sigaction S_IGNORE_action = {{0}};  // wierd GNU bug requires double braces
struct sigaction SIGTSTP_action = {{0}};
int FOREGROUND_MODE;   //makeshift bool flag for tracking SIGTSTP 
int BG_CHILDREN[50];   //sloppy global array since we dont't have access to the glorious C++ STL
int BG_C_INDEX;       //track how many background children are active
int PROC_COUNT;

//function prototypes
void catchSIGSTP(int);
void primaryLoop();
void replaceStr(char*);
void bgProcess(int);

void primaryLoop() {
	size_t buffersize = 0; // Holds how large the allocated buffer is
	char* lineEntered = NULL; // Points to a buffer allocated by getline() that holds our entered string + \n + \0
	int argIndex; 		//for keeping track of each argument entered in command (space delimited - limit is 512 arguments)
	char* arguments[512];    //array of characters for separating the input into separate arguments
	char *tempChar;			//temp variable for holding arguments
	int tempInt;
	char *inputFileName = NULL;		//variables for storing file names directed to by < (inputFile) and > (outputFile)
	char *outputFileName = NULL; 
	int backgroundFlag;      //default false for bg flag (&)
	//forking variables
	pid_t spawnPid = -5;
	int f1, f2, fNull_1, fNull_2;
	int childPID;

	//arguments[0] = "ls";
	//main run loop
	backgroundFlag = 0;
	childPID = 0;
	while(1) {
	
		//check for finished background process if applicable
		childPID = waitpid(-1, &GLOBAL_STATUS, WNOHANG);
		bgProcess(childPID);	

		lineEntered = NULL;
		//display shell input line and immediately clear stdout	
		printf(": ");
		fflush(stdout);
		//get input from user	
		//read a maximum of 512 commands

		while(1) {
			//reset variables
			inputFileName = NULL;
			outputFileName = NULL;	
			argIndex = 0;
			int numCharsEntered;
			numCharsEntered = getline(&lineEntered, &buffersize, stdin);
			//printf("\n");
			//if getline returns error value or characters exceed limit just restart
			
			if (numCharsEntered == -1 || numCharsEntered > 2048) { 
				clearerr(stdin);
				printf(": ");
				fflush(stdout);
				break;
			}
			else {
				break;
			}
		}

		//call replaceStr to handle any occurences of "$$"	
		while (strstr(lineEntered, "$$") != NULL) {
			replaceStr(lineEntered);
		}


		//tokenize the string - commands are space delimited and final character is a newline character	
		char* token = strtok(lineEntered, " \n");
		//if the very first token is a null, then jump back to start of loop
		if (token == NULL) {
			free(lineEntered);
			continue;
		}

		//check for comment
		else if (strncmp(token, "#", 1) == 0) {
			free(lineEntered);
			continue;  //jump back to start of while loop
		}
		
		//check for exit
		else if (strcmp(token, "exit") == 0) {
			int i = 0;	
			//kill off any child processes
			while(BG_C_INDEX > 0) {   //use BG_C_INDEX which tracks how many bg children there are
				if(BG_CHILDREN[i] != -1) {
					kill(BG_CHILDREN[i], SIGKILL);
					BG_C_INDEX--;
				}				
				i++;
			}
	
			if (lineEntered != NULL) {
				free(lineEntered);
			}

			printf("\n");
			fflush(stdout);
			GLOBAL_STATUS = 0;
			//shell exit
			exit(0);
		}
	

		//check the very first token for cd or status
		else if (strcmp(token, "cd") == 0) {
			//get the argument
			token = strtok(NULL, " \n");

			//check if argument is empty (so as to set to HOME path)
			if (token == NULL) {
				tempChar = getenv("HOME");
				chdir(tempChar);
			}
			//otherwise cd to destination
			else {
				tempInt = chdir(token);
				if (tempInt == -1) {//check to see if error value was returned
					free(lineEntered);
					perror("Error has occured");
				}
			}
			//read next token/advance token pointer
			token = strtok(NULL, " \n");

		}
		else if (strcmp(token, "status") == 0) {
			//WIFEXITED()
			//check to see if exited via exit command
			if (PROC_COUNT > 0) { //if only the shell has been running at this point, no need to bother
				if (WIFEXITED(GLOBAL_STATUS)) {
					int exitStatus = WEXITSTATUS(GLOBAL_STATUS);
					printf("exit value %d\n", exitStatus);
					fflush(stdout);
				}

				//WIFSIGNALED()
				//check to see if terminated by signal
				else if (WIFSIGNALED(GLOBAL_STATUS) != 0) {
					printf("terminated by signal");
					int termSignal = WTERMSIG(GLOBAL_STATUS);
					printf(" %d\n", termSignal);
					fflush(stdout);
				}
			}
			//read next token/advance token pointer
			token = strtok(NULL, " \n");
		}
	
		argIndex = 0;  //ARG INDEX IS RESET HERE! :<
		backgroundFlag = 0;
		while (token != NULL && argIndex < 512) {
			//reset the background process flag at the start of every loop iteration. & needs to be at the end of line
			backgroundFlag = 0;

			//check if input was ">" writing to output file
			if (strcmp(token, ">") == 0) {
				//get the output file name
				token = strtok(NULL, " \n");
				outputFileName = strdup(token);  
				
			}	

			//check if input was "<" reading from input file
			else if (strcmp(token, "<") == 0) {
				token = strtok(NULL, " \n");
				inputFileName = strdup(token);

			}

			//check if input was "&" and activate the backgroundFlag only if FOREGROUND_MODE is off
			else if (strcmp(token, "&" ) == 0) {
				if (FOREGROUND_MODE == 0) { 
					backgroundFlag = 1;
					arguments[argIndex] = strdup(token);
					argIndex++;
				}
				token = strtok(NULL, " \n");
			}

			//safe to store argument since token pointer has advanced past potential input/output file name
			else {
				//increment the index and store argument
				arguments[argIndex] = strdup(token);
				argIndex++;
			}

			//read the next piece of the line	
			token = strtok(NULL, " \n");
		}
		arguments[argIndex] = NULL;  //add the trailing NULL here so that execvp works correctly
	
		//if the backgroundFlag was set that memans we need to ignore the trailing '&' character
		if (backgroundFlag == 1) {
			arguments[argIndex - 1] = NULL;
			argIndex--;
		}

		if (arguments[0] != NULL) {
		PROC_COUNT++; 
		//fork and process stuff here....	
		spawnPid = fork();  //citation: class lecture notes "3.1 Processes.pdf" & "3.4 More UNIX IO"
		switch (spawnPid) {
			//error case
			case -1:  
				perror("Hull Breach!\n");
				GLOBAL_STATUS = 1; 
				exit(1); 
			break;
		
			//child case
			case 0:
				//children should ignore the SIGTSTP signal
				sigaction(SIGTSTP, &S_IGNORE_action, NULL);

				//if background flag is not set, we need to enable CTRL-C (SIGINT) termination of foreground process
				if (backgroundFlag != 1) {
					//S_IGNORE_action.sa_handler = SIG_DFL;
					sigaction(SIGINT, &S_IGNORE_action, NULL);
				}

				//if the background flag was enabled and a file wasn't specified to be written to:
				//first, make it so that standard input redirected from /dev/null
				if (inputFileName == NULL && backgroundFlag == 1) {
						
					//specify the devnull filepath
					//"0 is stdin, 1 is stdout, 2 is stderr"
					fNull_1 = open("/dev/null", O_RDONLY);
					if (fNull_1 < 0) {
						perror("output file (/dev/null) error");
						if (lineEntered != NULL) {
							free(lineEntered);
						}
						GLOBAL_STATUS = 1;
						exit(1);
					}
					//child sent to foreground and input is redirected
					int result = dup2(fNull_1, 0);
					if (result == -1) { 
						perror("target fNull open()");
						if (lineEntered != NULL) {
							free(lineEntered);
						}
						GLOBAL_STATUS = 1;  
						exit(1); 
					}
				}

				//redirect foregrounded process' output to /dev/null if no file was specified
				if (outputFileName == NULL && backgroundFlag == 1) {

					//specify the devnull filepath
					fNull_2 = open("/dev/null", O_WRONLY, S_IRUSR | S_IWUSR);
					if (fNull_2 < 0) {
						perror("input file (/dev/null) error");
						if (lineEntered != NULL) {
							free(lineEntered);
						}
						GLOBAL_STATUS = 1;
						exit(1);
					}	

					//foregrounded child output redirected
					int result = dup2(fNull_2, 1);
					if (result == -1) {
						perror("target fNull open()");
						if (lineEntered != NULL) {
							free(lineEntered);
						}
						GLOBAL_STATUS = 1;
						exit(1);
					}
				}



				//if "inputFile" is not null process it
				if (inputFileName != NULL) {

					//in this case we are reading the contents from a file (<), directing that to stdin
					//instead of our keyboard providing stdin, the file is
					f1 = open(inputFileName, O_RDONLY);
					//check for file open error
					if (f1 < 0) {
						printf("cannot open %s for input\n", inputFileName);
						fflush(stdout);
						if (lineEntered != NULL) {
							free(lineEntered);
						}
						GLOBAL_STATUS = 1;
						exit(1);
					}
				
					else {
						//0 is stdin, 1 is stdout, 2 is stderr
						int result = dup2(f1, 0);
						if (result == -1) { 
							perror("target open()");
							GLOBAL_STATUS = 1; 
							exit(1); 
						}
					}
				}
				//if "outputFileName" is not null process it
				if (outputFileName != NULL) {

					//create and truncate file that will be written to with ">"
					f2 = open(outputFileName, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
					if (f2 < 0) {
						perror("output file (write) error");
						if (lineEntered != NULL) {
							free(lineEntered);
						}
						GLOBAL_STATUS = 1; 
						exit(1);
					}
					else {
						//use dup2 to switch the stream (stdout) to write to specified file 
						int result = dup2(f2, 1);
						//if dup2 failed, free memory and exit
						if (result == -1) { 
							perror("target open()");
						
							if (lineEntered != NULL) {
								free(lineEntered);
							}
							GLOBAL_STATUS = 1;  
							exit(1); 
						}
					}
				}			


				//call execvp...
				if (arguments[0] != NULL) {
					if (execvp(arguments[0], arguments) < 0) {
						perror(arguments[0]);
						if (lineEntered != NULL) {
							free(lineEntered);
						}
						for (int i = 0; i <argIndex; i++) {
							arguments[i] = NULL;
						}
						GLOBAL_STATUS = 1; 
						exit(1);
					}
					//close output and input file if appropriate
					if (outputFileName != NULL) {
						close(f2);
					}
					if (inputFileName != NULL) {
						close(f1);	
					}
					//close the fNull_x file paths 
 					if (backgroundFlag == 1) { 
						if (inputFileName == NULL) {
							close(fNull_1);
						}
						if (outputFileName == NULL) {
							close(fNull_2);
						}   
					}
					GLOBAL_STATUS = 0;
					exit(GLOBAL_STATUS);
				}
			break;

			//parent case
			default:
				
				//check if the background flag is set to false
				//if that is the case, then we wait for child process to terminate
				if (backgroundFlag != 1) {
					waitpid(spawnPid, &GLOBAL_STATUS, 0);

					//check if child exited normally, if it did not display signal
					
					if (WIFSIGNALED(GLOBAL_STATUS)){
						printf("background pid %d is done: terminated by signal", getpid());
						int termSignal = WTERMSIG(GLOBAL_STATUS);
						printf(" %d\n", termSignal);
						fflush(stdout);
					}	

				}
				else {
					//add child process to global array of child processes in case we need to abpruptly exit
					BG_CHILDREN[BG_C_INDEX] = spawnPid;		
					BG_C_INDEX++;
					//display backgrounded process id
					printf("background pid is %d\n", spawnPid);	
					fflush(stdout);
					//childPID = waitpid(-1, &GLOBAL_STATUS, WNOHANG);

				}

			break;
			//reinstate SIGTSTP handling for parent
			sigaction(SIGTSTP, &SIGTSTP_action, NULL);
		}
	
		}

		//reset arguments array
		for (int i = 0; i < argIndex; i++) {
			arguments[i] = NULL;
		}			

		//free any other malloc'd strings
		
		if (outputFileName != NULL) {
			free(outputFileName);
		}
		if (inputFileName != NULL) {
			free(inputFileName);
		}
		if (lineEntered != NULL) {
			free(lineEntered);
		}	
	}
}


void bgProcess(int childID){
	//for background processes: "check if any process has completed, if there have been 
	//no terminated process continue runnning
	int childPID = childID;	
	int found = 0;
	int i = 0;
	if (childPID > 0) {
		if (WIFEXITED(GLOBAL_STATUS)) {
			printf("background pid %d is done: ", childPID);
			printf("exit value %d\n", WIFEXITED(WIFEXITED(GLOBAL_STATUS)));
			fflush(stdout);
  		}
		else if (WIFSIGNALED(GLOBAL_STATUS) != 0) {
			printf("background pid %d is done: terminated by signal", getpid());
			int termSignal = WTERMSIG(GLOBAL_STATUS);
			printf(" %d\n", termSignal);
			fflush(stdout);
		}
	}
	
	while (childPID > 0) {	
		//remove the child pid from list of background children
		while (i < BG_C_INDEX && found == 0) {
			if (BG_CHILDREN[i] == childPID) {
				found = 1;
				
				//re-adjust our array
				for (int j = i; j < BG_C_INDEX; j++) {
					BG_CHILDREN[i] = BG_CHILDREN[i+1];
		
				}	
				BG_C_INDEX--;
			}
			i++;			}
		childPID = waitpid(-1, &GLOBAL_STATUS, WNOHANG);
	}
	
}



void catchSIGTSTP(int signo) {

	//if foreground mode is not set, activate it
	if (FOREGROUND_MODE == 0) {
		FOREGROUND_MODE = 1;
		//display message
		char *message1 = "\nEntering foreground-only mode (& is now ignored)\n";
		write(STDOUT_FILENO, message1, 50);
	}
	//otherwise turn off foreground mode
	else {
		FOREGROUND_MODE = 0;
		char *message2 = "\nExiting foreground-only mode\n";
		write(STDOUT_FILENO, message2, 30);
	}

}

//shoutout to Lord Pomeroy for being the inspiration for the solution for this function
void replaceStr(char *str) {
	int startIdx;
	int endIdx;
	char before[2048];
	char after[2048];
	int found = -1;

	//initialize before and after temp strings
	memset(before, '\0', sizeof(before));
	memset(after, '\0', sizeof(after));

	//get the PID and save it as a string.
	int temp = getpid();
	char tempBuffer[8]; //allocate for buffer, largest PID can be on a 64 bit system is 4194304
	memset(tempBuffer, '\0', sizeof(tempBuffer));
	//convert the integer to c-string
	sprintf(tempBuffer, "%d", temp);	

	while (found == -1) {
		//search for the last occurence in the inputLine string for "$$"
		for (int i = 0; i < strlen(str) - 1; i++) {
			if( str[i] == '$' && str[i+1] == '$') {
				startIdx = i;
				endIdx = i + 1;
				found = 1;
			}
		}
		//no occurences of "$$"
		if (found == -1) {
			found = 0;
		}
	}	

	//if we found the occurence, modify the input string
	if (found == 1) {
		//copy the first half (prior to $$) over
		if (startIdx > 0) {
			strncpy(before, str, startIdx);
		}
		//copy the second half (after $$) over
		if (endIdx < strlen(str) - 1) {
			memcpy(after, &str[endIdx+1], strlen(str));
			after[strlen(str)] = '\0';
  
		}
		//copy the string pieces
		memset(str, '\0', strlen(str));  //first clear the line (target)
		if (startIdx > 0) { 
			strcpy(str, before);
		}
		strcat(str, tempBuffer); //the pid 
		if (endIdx < strlen(str) - 1) { //second half of line
			strcat(str, after);
		}	
	}
}



int main() {
	//initialize the global status variable
	GLOBAL_STATUS = 0;
	FOREGROUND_MODE = 0;
	BG_C_INDEX = 0;
	memset(BG_CHILDREN, -1, sizeof(BG_CHILDREN));
	PROC_COUNT = 0;

  	//set up program so that SIGINT is ignored from the get-go (struct is global variable)
  	S_IGNORE_action.sa_handler = SIG_IGN;   //set SIGINT to be ignored (CRTL-C) from the get-go
  	sigfillset(&S_IGNORE_action.sa_mask);  //not going to block any signals
  	S_IGNORE_action.sa_flags = 0;			  //no additinal instructions needed
	//by calling this function, this is where we actually invoke SIGINT to be ignored
	sigaction(SIGINT, &S_IGNORE_action, NULL); 

  	//CTRL-Z signal handler
  	SIGTSTP_action.sa_handler = catchSIGTSTP;  //call signal handler function
  	sigfillset(&SIGTSTP_action.sa_mask);      //block all signals while handler is running
	sigaction(SIGTSTP, &SIGTSTP_action, NULL);


  	primaryLoop();


	return 1;    //the only way this program should exit is via the (exit call)
}
