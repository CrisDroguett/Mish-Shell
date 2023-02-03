// Cris Droguett
// CS311
// MISH Assignment

#define _GNU_SOURCE
#define WHITESPACE "\t\v\f\n\r "
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>

int main() {
	//Variable Declarations
	FILE* fp;
	char* whole_input = NULL;
	char* test_variable;
	char* command;
	char* arg[256];
	char* current_dir;
	size_t length = 256;
	ssize_t characters;
	int pid;
	int inside_shell = 0;
	int cnt = 0;
	int exitShell = 0;
	int log_status = 0;
	int numForks = 0;

	//Shell Startup
	printf("Starting Up The Shell.....\n"); //Print startup message before entering shell
	while (!exitShell) {
		inside_shell = 0; //Reset inside_shell back to zero after every loop
		//Getting Command Line Input From User
		whole_input = (char*)malloc(length); //Allocate Memory for the Command Line Input
		printf("MISH>"); //Print the shell prompt for every command 
		if (whole_input == NULL) {  //If whole input is NULL that means our allocation failed and we exit 
			printf("unable to allocate memory");
			exit(1);
		}
		else { //Otherwise we use whole_input to hold the value in the command line 
			if ((characters = getline(&whole_input, &length, stdin)) == -1) //If characters returns -1 there is an error and we quit
			{
				if (feof(stdin)) //This is ran if we get an end of file condition
				{
					printf("End-of-file\n");
					break;
				}
				else { //Otherwise we print an error
					printf("There was an error getting the line\n");
					exit(1);
				}

			}
			else //Otherwise e output that we succeeded in getting the line
				printf("Getting the line was successfull\n");

		}
		//printf("Command Input: %s", whole_input); //We then display the whole command line input to the user 
		char* copy = whole_input; //We create a copy of whole_input that we will use to tokenize 

		// Writing the command line to the file
		if (log_status == 1) { //Log status is 1 when weve run a log command, so we only want to write the command line inputs after weve logged
			const char* print_command = copy;
			fprintf(fp, "%s\n", print_command);
		}

		// Tokenizing the Command Line Input
		test_variable = strsep(&copy, WHITESPACE); //We use strsep to tokenize the command line into arguements
		cnt = 0; //We set variables cnt and inside_shell to 0  before the while loop
		while (test_variable != NULL) //This loop runs as long as the current token isnt NULL
		{
			if (cnt == 0) //The first token in test_varible gets saved as the command 
				command = test_variable;
			int len;
			len = strlen(test_variable); // finds out how many characters are in the token
			if (len > 0) { //The token is only saved if it is not blank
				arg[cnt] = test_variable; //every token in test_variable gets saved to an arg array
				cnt++;
			}
			test_variable = strsep(&copy, WHITESPACE); //We then get the next token
		}
		if (arg[0] == NULL) { //This tells the shell to ignore blank commands 
			printf("Error: Please insert a command to the shell\n");
			inside_shell = 1;
		}
		printf("arg[0]: %s\n", arg[0]);
		//arg[cnt - 1] = NULL; // For some reason the strsep function does not stop properly when its supposed to so we need to hard code the last element left in the array to NULL
		//printf("The Command Is: %s\n", command);// We display what the command input was
		for (int i = 0; i < cnt; i++) // We cycle thru all the elements in the arg array and display them
		{
			printf("The Arguements Are: %s\n", arg[i]);
		}

		//Cases to be handled by the shell

		// Exit case
		if (strcmp(command, "exit") == 0) { // If the command is exit we run this if block
			inside_shell = 1; // We set inside shell to 1 so that we do not create a seperate child process
			printf("User decided to exit\n"); // We display that we are going to exit
			exitShell = 1; // We set the exit shell variable to 1, which ends the while loop
			if (log_status == 1) { //if we are currently logging, we close the file before exiting
				fclose(fp);
			}
		}

		//CD case
		if (strcmp(command, "cd") == 0) {// If the command is cd we run this if block
			inside_shell = 1; // We set inside shell to 1 so that we do not create a seperate child process
			printf("User decided to change directories\n"); //We display that we are going to change directories
			if (chdir(arg[1]) != 0) // if chdir returns a 0 then it changed the directory correctly 
				printf("There was an error changing directory\n");
			else //Otherwise it failed
				printf("Directory was changed\n");
		}

		//pwd case
		if (strcmp(command, "pwd") == 0) { //If the command is pwd we run this if block
			inside_shell = 1; // We set inside shell to 1 so that we do not create a seperate child process
			printf("The User Wants To See The Current Directory\n"); //We display that we are going to print the current directory
			current_dir = get_current_dir_name(); //We use the get_current_dir_name function 
			printf("%s\n", current_dir);//We print the return value of the function
			free(current_dir); //We then have to be sure to then free the memory from get_current_dir_name()
		}

		//log case
		if (strcmp(command, "log") == 0) {//If the command is log we run this block
			inside_shell = 1;// We set inside shell to 1 so that we do not create a seperate child process
			//if (arg[1] = WHITESPACE)
			if (arg[1] == NULL)//If no file name is specified then we close the current log file 
			{
				printf("The User Wants to Stop Logging\n");
				fclose(fp);
				log_status = 0;
			}
			else
			{
				printf("The User Wants to Log all Commands\n"); //We display that we want to log all comands 
				if (log_status) {//if we have already started logging then we close the previous log 
					fclose(fp);
					fp = fopen(arg[1], "w");//we then open a new log file from the command input 
				}
				else {
					log_status = 1; //Otherwise if this is the first log we set the variable to 1 and open a new log file
					fp = fopen(arg[1], "w");
				}
			}
		}

		//Forking
		if (inside_shell == 0) { // The child process only happens in the shell if the shell hasnt already done a process
			if ((pid = fork()) < 0) { //If the pid of the fork is ever less than 0 then there was an error forking
				fprintf(stderr, "fork failed\n");
				exit(1); //And we exit
			}
			if (pid == 0) { //If pid == 0 then we are in child process
				if (execvp(command, arg)) //In the child process we run execvp with the command and the arguements
					fprintf(stderr, "exce failed\n"); //If we ever get to this if then we know the exec failed
				exit(2); // and so we exit 
			}
			else
			{
				numForks++; //Code the professor asked us to put in to stop breaking bingweb
				if (numForks > 100) {
					printf("Number of forks exceeded\n");
					exit(1);
				}
				wait(NULL); // waits for the child process to finish
				printf("The parent waited for the child\n"); //We display that the parent successfully waited for the child
			}
		}
		for (int i = cnt; i > 0; i--) { //This clears any left over tokens in the arg array
			arg[i] = 0;
		}

		free(whole_input); //Once we are done with whole_input we free the memory allocated
	}
	printf("Thanks For Using The Shell, Goodbye\n"); //Goodbye message to the user after they're done using the shell
	return(0);//We return 0
}