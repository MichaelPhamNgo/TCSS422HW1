/******************************************************************************/
/**  @file         MyShell.c							**/
/**  @author       Juhua Hu							**/
/**  @student	    Phuc Pham N						**/
/******************************************************************************/

/**
DONOT change the existing function definitions. You can add functions, if necessary.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <fcntl.h>
#define MAX_SIZE 256
#define EXIT_SUCCESS 0


/**
  GetTime and Spin are functions in common.h file.
*/
double GetTime() {
    struct timeval t;
    int rc = gettimeofday(&t, NULL);
    assert(rc == 0);
    return (double) t.tv_sec + (double) t.tv_usec/1e6;
}

void Spin(int howlong) {
    double t = GetTime();
    while ((GetTime() - t) < (double) howlong)
	; // do nothing in loop
}

//Count length of array
int lengthOfArray(char** args){
	int i;
	for(i = 0; args[i+1];i++);
	return i;
}

/**
  @brief Fork a child to execute the command using execvp. The parent should wait for the child to terminate
  @param args Null terminated list of arguments (including program).
  @return returns 1, to continue execution and 0 to terminate the MyShell prompt.
 */
int execute(char** args) {

	//return status in the end of execute function. 
	//if status = 1, continue execution
	//if status = 0, terminate the MyShell prompt
	int status = 1;
	
	//get the first argument of the char** array
	char *cmd = args[0];	
	if(strcmp(cmd, "\n") == 0) { //If user hits enter, then repeat the MyShell promt.
		status = 1;
	} else {
		//clarify folk p1 and p2
		pid_t p1,p2;
		
		//fork p1
		p1 = fork();		
		if (p1 < 0) { //check if fork 1 failed, then terminate the MyShell prompt
			printf("fork failed\n");
			exit(1);						
		} else if(p1 == 0) { // if fork is 1 succeed, run child
			//if user typed exit command, return 0 to terminate the MyShell prompt
			if(strcmp(cmd, "exit") == 0) {				
				status = 0;
				exit(0);
			}else { //if use typed a command
				p2 = fork();
				if(p2 < 0) { //check if fork 2 failed, then terminate the MyShell prompt
					printf("fork failed\n");			
				} else if (p2 == 0) { // if fork 2 is succeed, run child
					//count length of args
					int length = lengthOfArray(args);
					
					//check if length array > 1
					if(length > 1) {
						//check if user typed wc MyShell.c > newfile.txt
						char *check = args[length-1];
						if(strcmp(check, ">") == 0) {
							//get filename to print the content to this file
							char *filename = args[length];
							close(STDOUT_FILENO);
							open(filename, O_CREAT|O_WRONLY|O_TRUNC, S_IRWXU);
							//replace > character by NULL
							args[length-1] = NULL;
							//replace filename.txt by NULL
							args[length] = NULL;
						}
					}		
					//execute the execvp with argument respectively								
					status = execvp(args[0], args); //If execution is success, then jump to parent of process 2 				
					//if execution failed, exiting the fork 2 and jump to parent of fork 1
					if(status == -1) {			    
	    			    		printf("error executing command: No such file or directory\n");	    			  	    			    		
					} 					
				} else {
					//execute the execvp successfully, then waiting for process p2
					if(status != -1) {
						waitpid(p2, &status, 0);
					}
					
				}
				//Terminate the child of process p1
				exit(1);											
			}
		} else {
			//waiting for process p1
			waitpid(p1, &status, 0);
		}
	}
	return status;
}

/**
  @brief gets the input from the prompt and splits it into tokens. Prepares the arguments for execvp
  @return returns char** args to be used by execvp
 */
char** parse(void) {
	char** args = (char**)malloc(sizeof(char*)*MAX_SIZE);	
	char input[MAX_SIZE];
	
	//Gets the input typed by the user in the MyShell> prompt in the form of a line
	fgets(input, MAX_SIZE, stdin);
		
	//Split the input typed by the user into tokens and store to array pointer
	char *token = strtok(input, " ");
	
	//Handle if user hits enter
	if((strlen(input) == 1 && strcmp(input, "\n") == 0)) {		
		args[0] = token;
	} else {			//Handle if user hits command
		int count = 0;
		while(token != NULL) {
			token[strcspn(token, "\n")] = '\0';		
			args[count] = strdup(token);
			token = strtok(NULL, " ");
			count++;
		}		
		//Add NULL to the end of array pointer
		args[count+1] = NULL;	
	}	
	return args;
}


/**
   @brief Main function should run infinitely until terminated manually using CTRL+C or typing in the exit command
   It should call the parse() and execute() functions
   @param argc Argument count.
   @param argv Argument vector.
   @return status code
 */
int main(int argc, char **argv)
{
	int condition = 1;
	while (condition) {
		Spin(1);
		printf("MyShell> ");		
		char** args = parse();
		condition = execute(args);
		if(condition == 0) {
			printf("exiting");
		}		
	}
  	return EXIT_SUCCESS;
}
