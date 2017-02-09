/**
 * @file quash.c
 *
 * Quash's main file
 */

/**************************************************************************
 * Included Files
 **************************************************************************/
#include "quash.h" // Putting this above the other includes allows us to ensure
                   // this file's headder's #include statements are self
                   // contained.
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <strings.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


/**************************************************************************
 * Private Variables
 **************************************************************************/
/**
 * Keep track of whether Quash should request another command or not.
 */
// NOTE: "static" causes the "running" variable to only be declared in this
// compilation unit (this file and all files that include it). This is similar
// to private in other languages.
static bool running;

/**************************************************************************
 * Private Functions
 **************************************************************************/
/**
 * Start the main loop by setting the running flag to true
 */
static void start() {
  running = true;
}

#define MAX_PROCESSES 20

// Struct for background processes
struct BackgroundProcesses
{
  pid_t pids[MAX_PROCESSES];   // Track background processes
  bool processFin[MAX_PROCESSES];
  bool shownFin[MAX_PROCESSES];
  char invokeCommands[MAX_PROCESSES][200];
};

/**
* 2D array that stores parsing of previous line
*/
#define MAX_SPLITS 10
char* userCmd[MAX_SPLITS][25];
command_t cmd; //< Command holder argument
int numBackground = -1;  // # of processes running in the background
int status;
int max = 0;
struct BackgroundProcesses BP;
char* signals[] = { "empty", "SIGHUP", "SIGINT", "SIGQUIT", "SIGILL", "SIGTRAP", "SIGABRT", "SIGBUS", "SIGFPE", "SIGKILL", "SIGUSR1", "SIGUSR1", "SIGUSR2", "SIGPIPE", "SIGALRM", "SIGTERM", "SIGSTKFLT", "SIGCHLD", "SIGCONT", "SIGSTOP", "SIGTSTP", "SIGTTIN", "SIGTTOU", "SIGURG", "SIGXCPU", "SIGXFSZ", "SIGVTALRM", "SIGPROF", "SIGWINCH", "SIGIO", "SIGPWR", "SIGSYS"};

/**************************************************************************
 * Public Functions
 **************************************************************************/
bool is_running() {
  return running;
}

void terminate() {
  running = false;
}

// Just found a trim function; removes leading and trailing spaces
// http://stackoverflow.com/questions/122616/how-do-i-trim-leading-trailing-whitespace-in-a-standard-way
char *trim(char *str)
{
    size_t len = 0;
    char *frontp = str;
    char *endp = NULL;

    if( str == NULL ) { return NULL; }
    if( str[0] == '\0' ) { return str; }

    len = strlen(str);
    endp = str + len;

    /* Move the front and back pointers to address the first non-whitespace
     * characters from each end.
     */
    while( isspace(*frontp) ) { ++frontp; }
    if( endp != frontp )
    {
        while( isspace(*(--endp)) && endp != frontp ) {}
    }

    if( str + len - 1 != endp )
            *(endp + 1) = '\0';
    else if( frontp != str &&  endp == frontp )
            *str = '\0';

    /* Shift the string so that it starts at str so that if it's dynamically
     * allocated, we can still free it on the returned pointer.  Note the reuse
     * of endp to mean the front of the string buffer now.
     */
    endp = str;
    if( frontp != str )
    {
            while( *frontp ) { *endp++ = *frontp++; }
            *endp = '\0';
    }

    return str;
}


// Split the input if needed and then split by spaces
void stringSplit(char* inputVals, char* splitChar)
{
  char* word;
  int count = 0;
  max = 0;

  // If splitChar is not " " meaning it must be split
  if (strcmp(splitChar, " "))
  {
    // Find the first command set
    word = strtok(inputVals, splitChar);
    userCmd[count][0] = trim(word);

    // Find all other command sets
    while( word != NULL )
    {
       count++;
       word = strtok(NULL, splitChar);
       userCmd[count][0] = trim(word);
    }
    max = count;

    // Divide each command set into its individual words
    for (int i = 0; i < max; i++)
    {
      count = 0;

      /* get the first token */
      word = strtok(userCmd[i][0], " ");
      userCmd[i][count] = word;

      /* walk through other tokens */
      while( word != NULL )
      {
	       count++;
         word = strtok(NULL, " ");
         userCmd[i][count] = word;

      }
    }
  }
  // Simply split on spaces
  else
  {
    /* get the first token */
    word = strtok(inputVals, " ");
    userCmd[0][count] = word;

    /* walk through other tokens */
    while( word != NULL )
    {
	     count++;
       word = strtok(NULL, " ");
       userCmd[0][count] = word;
    }
  }
}

// Check for input file, output file, and piping; still needs more check;
void stringParse(char* inputVals)
{
  if (strchr(inputVals, '>'))
  {
    stringSplit(inputVals, ">");
  }
  else if (strchr(inputVals, '<'))
  {
    stringSplit(inputVals, "<");
  }
  else if (strchr(inputVals, '&'))
  {
    stringSplit(inputVals, "&");
  }
  else if (strchr(inputVals, '|'))
  {
    stringSplit(inputVals, "|");
  }
  else
  {
    stringSplit(inputVals, " ");
  }
}

// General handling of user input; provided with the lab
bool get_command(command_t* cmd, FILE* in) {
  if (fgets(cmd->cmdstr, MAX_COMMAND_LENGTH, in) != NULL) {
    size_t len = strlen(cmd->cmdstr);
    char last_char = cmd->cmdstr[len - 1];

    if (last_char == '\n' || last_char == '\r') {

      // Remove trailing new line character.
      cmd->cmdstr[len - 1] = '\0';
      cmd->cmdlen = len - 1;

      // ADDED sends a copy of the user input to be parsed
      char* temp;
      temp = malloc(sizeof(char) * strlen(cmd->cmdstr));
      strcpy(temp, cmd->cmdstr);
      stringParse(temp);
    }
    else
      cmd->cmdlen = len;

    return true;
  }
  else
    return false;
}

void pipeCmd()
{
	int pipes[MAX_SPLITS][2] = {{0}};
	pid_t processes[MAX_SPLITS] = {0};

	if(max > 1)
	{
		pipe(pipes[0]);

		processes[0] = fork();
		if(processes[0] == 0)
		{
			close(pipes[0][0]);
			dup2(pipes[0][1],STDOUT_FILENO);
			close(pipes[0][1]);

			execvp(userCmd[0][0],userCmd[0]);
			exit(0);
		}
		close(pipes[0][1]);

		int i = 0;
		for(i = 1; i <= max - 2; i++)
		{
			pipe(pipes[i]);

			processes[i] = fork();
			if(processes[i] == 0)
			{
				dup2(pipes[i-1][0],STDIN_FILENO); //previous pipes read
				close(pipes[i-1][0]);//close that pipe once it has been rerouted
				close(pipes[i][0]);//close the current pipes read side
				dup2(pipes[i][1], STDOUT_FILENO); //redirect output to current pipes read
				close(pipes[i][1]);//close it after redirection

				execvp(userCmd[i][0],userCmd[i]);
				exit(0);
			}
			close(pipes[i-1][0]);//close previous pipe premanently (already closed the write)
			close(pipes[i][1]);//close the write for newest pipe permanently
		}

		processes[max - 1] = fork();
		if(processes[max - 1] == 0)
		{
			dup2(pipes[max-2][0], STDIN_FILENO);
			close(pipes[max-2][0]);

			execvp(userCmd[max-1][0],userCmd[max-1]);
			exit(0);
		}
		close(pipes[max-2][0]);//close read of the last pipe

    // Force parent to wait
    if (waitpid(processes[max - 1], &status, 0) == -1)
    {
      printf("Parent waitpid error: %d\n", errno);
    }
	}
	else
	{
		printf("Not enough pipes\n");
	}
}

// Run background process
void runBackground()
{
  numBackground++;
  // error
  strcpy(BP.invokeCommands[numBackground], cmd.cmdstr);

  BP.pids[numBackground] = fork();
  if (BP.pids[numBackground] == 0)
  {
    execvp(userCmd[0][0], userCmd[0]);
    puts("The function you tried to call does not exist!  Please try again!");
    exit(0);
  }
  else
  {
    printf("[%d] %d\n", numBackground + 1, BP.pids[numBackground]);
  }
}

// Run foreground process
void runForeground()
{
  pid_t pid1;
  int inputfile;
  int outputfile;


  // Run given command within a process
  pid1 = fork();
  if (pid1 == 0)
  {
    // if there is an input file then change the STDIN_FILENO
    if (strchr(cmd.cmdstr, '<'))
    {
      inputfile = open(userCmd[1][0], O_RDWR, 0644);
      if (inputfile == -1)
      {
        exit(0);
      }
      dup2 (inputfile, STDIN_FILENO);
    }

    // ouput to file
    if (strchr(cmd.cmdstr, '>'))
    {
      outputfile = open(userCmd[1][0], O_RDWR | O_CREAT | O_TRUNC, 0644);
      dup2 (outputfile, STDOUT_FILENO);
    }

    // Execute command
    execvp(userCmd[0][0], userCmd[0]);

    // The command was not an acknowledged commands
    puts("The function you tried to call does not exist!  Please try again!");
    exit(0);
  }

  // The function was not defined
  if (waitpid(pid1, &status, 0) == -1)
  {
    printf("Process 1 encountered an error. \nERROR: %d\n", errno);
    // return EXIT_FAILURE;
  }
}

void printWorkingPath()
{
	const int sizeOfPath = 2048;
	char directory[sizeOfPath]; 
	getcwd(directory,sizeOfPath);
	printf("%s",directory);
}

int shellCommands()
{
	int returnValue = 0;

  //<---Exit and quit command--->
	if (!strcmp(userCmd[0][0], "exit") || !strcmp(userCmd[0][0], "quit"))
  {
		returnValue = 1;
    puts("Thanks for visiting!");
    // terminate(); // Exit Quash
    exit(0);  // Exit immediately
  }
	//<--- CD command --->
	else if(!strcmp(userCmd[0][0], "cd"))
	{
		returnValue = 1;
		if(chdir(userCmd[0][1]) == -1)
		{
			printf("ERROR: %d\n", errno);
		}
	}
  //<--- Set command --->
	else if(!strcmp(userCmd[0][0],"set") && userCmd[0][1] != NULL)
	{
		returnValue = 1;
		if(!strncmp(userCmd[0][1],"HOME=",5))
		{
			char* address = userCmd[0][1] + 5;
			setenv("HOME",address,1);
		}
		else if(!strncmp(userCmd[0][1], "PATH=", 5))
		{
			char* path = getenv("PATH");
			char* address;

			//if path already exsists
			if(path != NULL)
			{
				//adds colon to front of address
				address = userCmd[0][1] + 4;
				address[0] = ':';

				//concatenates new address to end of current path
				char* newPath = malloc(sizeof(char) * ( strlen(path) + strlen(address) ));
				strcpy(newPath, path);
				strcat(newPath, address);

				//sets the path variable to concatenation (1 = overwrite)
				setenv("PATH",newPath,1);

				free(newPath);
			}
			//path was not found create a new one
			else
			{
				address = userCmd[0][1] + 5;
				setenv("PATH",address,1);
			}
		}
	}
  //<--- Jobs command --->
  else if (!strcmp(userCmd[0][0], "jobs"))
  {
    returnValue = 1;
    for (int i = 0; i < numBackground + 1; i++)
    {
      // If the process is finished but never displayed to the user
      // Display "done" and mark as displayed
      if (BP.processFin[i] && !BP.shownFin[i])
      {
        printf("[%d] %d Finished   %s\n", i + 1, BP.pids[i], BP.invokeCommands[i]);
        BP.shownFin[i] = true;
      }
      // The process is still running
      else if (!BP.processFin[i])
      {
        printf("[%d] %d Running   %s\n", i + 1, BP.pids[i], BP.invokeCommands[i]);
      }
    }
  }
  //<--- Kill command --->
  else if (!strcmp(userCmd[0][0], "kill"))
  {
    bool valFound = false;
    returnValue = 1;
    if (userCmd[0][1][0] == '-') userCmd[0][1]++;

    // Search for the process and kill it if found
    for (int i = 0; i < numBackground + 1; i++)
    {
      if (BP.pids[i] == atoi(userCmd[0][2]))
      {
        // Determine the value of the signal
        for (int j = 1; j < 32; j++)
        {
          // Kill process with signal here
          if (!strcmp(signals[j], userCmd[0][1]) || atoi(userCmd[0][1]) == j)
          {
            valFound = true;
            kill(BP.pids[i], j);
          }
        }
      }
    }
    // If no value found
    if (!valFound) puts("Process or signal not found...");
  }
  //<--- Pipe command --->
  else if (strchr(cmd.cmdstr, '|'))
  {
    returnValue = 1;
    pipeCmd();
  }
	//<--- PWD command --->
	else if(!strcmp(userCmd[0][0],"pwd"))
	{
		returnValue = 1;
		printWorkingPath();
		printf("\n");
	}
	//<--- ECHO command --->
	else if(!strcmp(userCmd[0][0],"echo") && userCmd[0][1] != NULL)
	{
		if(!strcmp(userCmd[0][1],"$PATH"))
		{
			returnValue = 1;
			char* path;
			path = getenv("PATH");
			if(path != NULL)
			{
				userCmd[0][1] = path;
			}
			else
			{
				userCmd[0][1] = "failed to find variable";
			}
			printf("%s\n", path);
		}
		else if(!strcmp(userCmd[0][1],"$HOME"))
		{
			returnValue = 1;
			char* home;
			home = getenv("HOME");
			if(home != NULL)
			{
				userCmd[0][1] = home;
			}
			else
			{
				userCmd[0][1] = "failed to find variable";
			}
			printf("%s\n",home);
		}
	}
	return(returnValue);
}



/**
 * Quash entry point
 *
 * @param argc argument count from the command line
 * @param argv argument vector from the command line
 * @return program exit status
 */
int main(int argc, char** argv) {
  // int outputfile = open("outputtext.txt", O_RDWR | O_CREAT | O_TRUNC);
  pid_t pid;
  start();

  puts("Welcome to Quash!");
  puts("Type \"exit\" to quit");

	printWorkingPath();
  printf(" $ ");

  // Main execution loop
  while (is_running() && get_command(&cmd, stdin))
  {
    // Check if user inputs a command
    if (userCmd[0][0] != NULL)
    {
      // Identify when background job is complete and mark its processesFin
      while((pid = waitpid(-1, &status, WNOHANG)) > 0) {
          for (int i = 0; i < numBackground + 1; i++)
          {
            if (pid == BP.pids[i])
            {
              BP.processFin[i] = true;
            }
          }
      }

      // If command is shell command then execute it and return 1; else run it normally
      if (shellCommands() != 1)
      {
        // Run in background
        if (strchr(cmd.cmdstr, '&'))
        {
          runBackground();
        }
        // Run process in foreground
        else
        {
          runForeground();
        }
      }
    }
		printWorkingPath();
    printf(" $ ");
  }

  return EXIT_SUCCESS;
}
