// Name(s): Bryant Jee
/* Description: This program is essentially a recreation of a bash shell with most of the commands implemented. 
It contains multiple functions which have several goals such as parsing input and executing commands.
These functions run on a continuous for loop that prompts the user for input through another function.
Some of the commands implemented in this program contain standard input and output through the '<' and '>' characters 
respectively. Some additional ones are the cd, ls, cat, whereis, and ps commands. */

#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdio.h>
#include <libgen.h> // Used to get the basename of a directory (Ex. Desktop$ instead of /home/bryant-jee/Desktop$)

#define MAX_ARGS 100 // used to define max number of arguments to be stored
#define MAX_INPUT 1024 // used to define max amount of input in characters
#define MAX_FILENAME 256 // used to define max amount of characters for a file name

// struct used for the parts of the shell command to be organized and used for later (similar to classes)
struct ShellCommand{
    char *command; // character pointer to the command
    char *args[MAX_ARGS];   /* array of pointer characters with a length of the maximum arguments 
                            it can store according to the constant named such*/
    int argNum; // number used to keep the number of argument

    int inputRedirect;  /* used to check if the input (or '<') was used in the code in the executeCommand function*/
    int outputRedirect; /* used to check if the output (or '>') was used in the code in the executeCommand function*/

    char inputFile[MAX_FILENAME]; // string used to store the input file after the input redirect
    char outputFile[MAX_FILENAME]; // string used to store the output file after the output redirect
};
    

/* 
    A function that causes the prompt to 
    display in the terminal according to the sample run
*/

void displayPrompt(){
    static int isFirstPrompt = 1; /*sets the static isFirstPrompt to true by setting to one
                                    and used to check if it's at the first prompt or not*/
    char cwd[1024]; /* initializing a string array with a length of 1024 characters 
                    for the current working directory (cwd)*/

    // if statement checking if it's the first prompt and if so prints out the correct display and changes after
    if (isFirstPrompt == 1){
        printf("~$ "); // correct display as shown in sample run
        isFirstPrompt = 0; /* sets isFirstPrompt to false by directly assigning it to zero and keeping it at zero
        through every function call because it is static. */
        return;
    }

    // prints out last directory in current working directory for the display
    if(getcwd(cwd, sizeof(cwd)) != NULL){
        char* homeDirectory = getenv("HOME"); /* used to get the "HOME" environment variable which defines the
        the user's home directory. */ 
                                            
        /* if else used to check if the current working directory is the home directory and 
        changing the prompt according to the result according to the sample run. */
        if (homeDirectory != NULL && strcmp(cwd, homeDirectory) == 0) { /* checks if the home directory could be 
            found and if the home directory is the same as the current working directory. */
            printf("$ ");
            return; 
        } else { 
            char *lastDirectory = basename(cwd); // basename used to get the last directory in the path
            printf("%s$ ", lastDirectory);
            return;
        }
    } else {
        perror("Error with prompt function");
    }
}

/*
    A function that takes input from the user and
    stores it at some memory location using a pointer.
*/ 

char* getInput(){ 
    char *input = (char*) malloc(MAX_INPUT); // allocates space for MAX_INPUT only because a characters is 1 byte

    if(fgets(input, MAX_INPUT, stdin) == NULL){ // checks if input passed in is returning some data and giving an error if not
        perror("Error with reading or EOF already reached");
        free(input); // frees the memory
        return NULL; // to not return freed memory
    } 
    
    input[strcspn(input, "\n")] = '\0'; // removes newline character from input
    return input; 
}


/*
    A function that parses through the user and returns a ShellCommand 
    struct containing information to be used later on in the program.
*/

struct ShellCommand parseInput(char* input){

    struct ShellCommand shell_command;
    shell_command.argNum = 0;
    shell_command.inputRedirect = 0; // set to zero for false to say no input redirect is being used
    shell_command.outputRedirect = 0; // set to zero for false to say no output redirect is being used
    
    char* token = strtok(input, " "); // separates the input after the space to create a token

    while(token != NULL){

        if (strcmp(token, "<") == 0) { // checks if the input redirect character is being used
            shell_command.inputRedirect = 1; /* set to one for true to say input redirect is being used 
            and used in the executeCommand function.*/

            token = strtok(NULL, " "); // creates another token after a space for the input file
            if (token != NULL){ // checks if there is another token after the input

                if (strlen(token) >= MAX_FILENAME){ /* checks if the file name given is longer
                    than the array that was created to store the input filename*/
                    fprintf(stderr, "Error: input file name given is too long\n");
                } else {
                    strcpy(shell_command.inputFile, token); // copies the token into the input file string
                }
            } else { 
				/* returns an error if the "<" or input redirect token was used incorrectly. 
				I don't exactly how to output the exact error message when making this mistake. */
				fprintf(stderr, "Error: Incorrect usage of input redirect\n");
				shell_command.command = NULL;
				return shell_command;
			}
        } else if (strcmp(token, ">") == 0){
            shell_command.outputRedirect = 1; /* set to one for true to say output redirect is being used 
            and used in the executeCommand function.*/

            token = strtok(NULL, " "); // creates another token after a space for the output file
            if (token != NULL){ // checks if there is another token after the output

                if (strlen(token) >= MAX_FILENAME){ /* checks if the file name given is longer
                    than the array that was created to store the output filename*/
                    fprintf(stderr, "Error: output file name given is too long\n");
                } else {
                    strcpy(shell_command.outputFile, token); // copies the token into the output file string
                }
            } else {
				/* returns an error if the ">" or output redirect token was used incorrectly. 
				I don't exactly how to output the exact error message when making this mistake. */
				fprintf(stderr, "Error: Incorrect usage of output redirect\n");
				shell_command.command = NULL;
				return shell_command;
			}
        } else {
            shell_command.args[shell_command.argNum++] = token; /* fills the next empty space in the argument array
            with the next token by entering the argument array at the next available stop using argNum++*/
        }

        token = strtok(NULL, " "); // creates another token after a space to be checked again for the loop
    }

    shell_command.args[shell_command.argNum] = NULL; /* sets the last spot in the array to NULL for 
    execvp to work properly. */

    // checks if an argument was passed in and if not then it returns NULL
    if (shell_command.argNum > 0){ 
        shell_command.command = shell_command.args[0]; // sets the command to the first token in the argument array
    } else {
        shell_command.command = NULL;
    }

    return shell_command; 
}

/*
    A function that executes the command that takes in a struct for the shell command
    Pipe isn't required but could be a nice addition.
*/

void executeCommand(struct ShellCommand shell_command){

    if (shell_command.command == NULL){ 
        return;
    }

    if (strcmp(shell_command.command, "exit") == 0){ 
        exit(0);
    }

    // checks if the command is "cd" and if so runs cd correctly
    if (strcmp(shell_command.command, "cd") == 0){ 
        /* if else statement that checks if no other token is passed in after "cd"; and if so, the user goes to 
        the home directory. If not, it checks if the chdir function worked properly and returns and error if not*/
        if (shell_command.args[1] == NULL) { 
            char* homeDirectory = getenv("HOME"); /* used to get the "HOME" environment variable which defines the
            the user's home directory. */ 

            if (homeDirectory != NULL) { 
                chdir(homeDirectory); // changes directory to home directory
            } else {
                perror("Error with getting to home directory.");
            }
        } else { 
            int chdirResult = chdir(shell_command.args[1]); // runs chdir and stores the result in a variable
            // checks if chdir worked properly and if not prints out an error message by passing in the variable
            if (chdirResult != 0){ 
                fprintf(stderr, "Error %d (%s)\n", errno, strerror(errno)); /* fprintf used to direct to standard 
                error stream and not perror for correct display according to the sample run without a colon. */
            }     
        }
        return; 
    }

    pid_t pid = fork(); // creates a child process for execvp to run in

    if (pid == 0) {
        // child process

        /* checks if input redirect character ("<") was used according to the code in parseInput and runs if so. */
        if (shell_command.inputRedirect == 1) { 
            int fd = open(shell_command.inputFile, O_RDONLY); /* creates a file descriptor to read the input file*/
            if (fd < 0) { 
                fprintf(stderr, "Error %d (%s)\n", errno, strerror(errno));
                exit(1);
            }
            dup2(fd, STDIN_FILENO); /* redirects standard input to the file the user passed in by duplicating
            the first file descriptor to the second one. */
            close(fd);
        }

        // output redirect
        if (shell_command.outputRedirect == 1) {
            int fd = open(shell_command.outputFile, O_WRONLY | O_CREAT | O_TRUNC, 0644); // creates a file decriptor to write, create, or truncate the output file

            if (fd < 0) {
                fprintf(stderr, "Error %d (%s)\n", errno, strerror(errno));
                exit(1);
            }

            dup2(fd, STDOUT_FILENO); /* redirects standard output to the file the user passed in by duplicating
            the first file descriptor to the second one. */
            close(fd); 
        }

        execvp(shell_command.command, shell_command.args); // executes the command

        // if execvp fails prints out error message
        fprintf(stderr, "Error %d (%s)\n", errno, strerror(errno)); 
        exit(1);
    }
    else if (pid > 0) { 
        // parent process
        wait(NULL);
    }
    else {
        perror("fork failed");
    }

}

int main() // MAIN
{
	char* input;
	struct ShellCommand command;
		
	// repeatedly prompt the user for input
	for (;;)
	{
        // display the prompt
        displayPrompt();

	    // get the user's input
	    input = getInput();
	    
	    // parse the command line
	    command = parseInput(input);
	    
	    // execute the command
	    executeCommand(command);

    	free(input); // frees the memory
		
	}

	exit(0);
}





