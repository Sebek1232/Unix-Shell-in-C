#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>


// Takes an array of argumnets and executes a program. 
void execute(char** args)
{
	int pid = fork();
	if (pid == 0) // child
		execvp(args[0], args);
	else // parent
	{
		int status;
		wait(&status);
		printf("pid:%d status:%d\n", pid, WEXITSTATUS(status));
	}
	return;
}


// signal handler for SIGNIT and SIGTSTP
static void handler(int signum)
{
	if (signum == SIGINT) 
	{
		write(1, "caught sigint\n", 21);
	}
	if (signum == SIGTSTP) 
	{
		write(1, "caught sigtstp\nCS361 > ", 21);
	}
}

// Takes two arrays of argument and executes two programs
// using the output of command1 as the input for command 2. 
void executepipe(char** args1, char** args2) 
{
	int p[2];
	pipe(p); //opens comunications between procceses 
	int pid = fork();
	if (pid == 0)// child 1
	{
		dup2(p[1], 1); // replaces output with the pipe output 
		close(p[0]); // close unused pipe
		execvp(args1[0], args1);
	}
	else
	{
		int pid2 = fork();
		if (pid2 == 0)// child 2
		{
			dup2(p[0], 0); // replaces input with the pipe input
			close(p[1]); // close unused pipe
			execvp(args2[0], args2);
		}
		else // parent
		{
			close(p[1]); // close unused pipes
			close(p[0]);
			int status;
			wait(&status);
			int status2;
			wait(&status2);
			printf("pid:%d status:%d\n", pid, WEXITSTATUS(status)); // child 1
			printf("pid:%d status:%d\n", pid2, WEXITSTATUS(status2));// child 2
		}
	}

}

// takes a array of charcters and parses it into a 
// array of commands. Using a space as the seprater.
void parseCommand(char line[], char** argsarray) 
{
	char* word = strtok(line, " ");
	int i = 0;
	while (word) {
		strcpy(argsarray[i], word);
		word = strtok(NULL, " ");
		i = i + 1;
	}
	argsarray[i] = '\0'; // addes a null terminating character at the end
}

// parse an array charcters that is seperated by a flag. In this program
// a ; or |. Then it stores the left hand side of the flag into arry1 and the 
// right hand side into arry2.
void parse2Command(char line[], char** argsarray, char** argsarray2, char* flag)
{

	// seperat commands by the flag
	char* command = strtok(line, flag);
	char* command1 = command;
	command = strtok(NULL, flag);
	char* command2 = command;

	// command 1
	int i = 0;
	command1 = strtok(command1, " ");
	while (command1) {
		strcpy(argsarray[i], command1);
		command1 = strtok(NULL, " ");
		i = i + 1;
	}
	argsarray[i] = '\0';

	// command 2
	i = 0;
	command2 = strtok(command2, " ");
	while (command2) {
		strcpy(argsarray2[i], command2);
		command2 = strtok(NULL, " ");
		i = i + 1;
	}
	argsarray2[i] = '\0';
}
int main()
{
	// signal handler setup
	struct sigaction sa;
	sa.sa_handler = handler;
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGTSTP, &sa, NULL);

	// sotorage for user input and arguments.
	char line[500];
	char** argsarray;
	char** argsarray2;

	while (1)
	{
		// user input
		printf("CS361 > ");
		fgets(line, 500, stdin);

		// get rid of new line charcter and 
		// replace with null charcter
		int len = strlen(line) - 1;
		if (line[len] == '\n')
			line[len] = '\0';

		// exit command
		if (strcmp(line, "exit") == 0) {
			exit(0);
		}
		
		// memory allocation
		argsarray = malloc(20 * sizeof(char*));
		for (int i = 0; i < 20; i++) {

			argsarray[i] = malloc(500 * sizeof(char));

		}

		argsarray2 = malloc(20 * sizeof(char*));
		for (int i = 0; i < 20; i++) {

			argsarray2[i] = malloc(500 * sizeof(char));

		}

		// command 1 ; command 2
		if (strstr(line, " ; ") != NULL)
		{
			parse2Command(line, argsarray, argsarray2, ";");
			execute(argsarray);
			execute(argsarray2);
		}
		// command 1 | command 2
		else if (strstr(line, " | ") != NULL)
		{
			parse2Command(line, argsarray, argsarray2, "|");
			executepipe(argsarray, argsarray2);
		}
		// command1
		else
		{
			parseCommand(line, argsarray);
			execute(argsarray);
		}

		// free memory
		free(argsarray[20]);
		free(argsarray2[20]);
	}
	return 0;
}

