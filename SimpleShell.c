#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>

/**************************************************************************
 * Authors: Gary Fleming, Jacob McCloughan
 * Class: CIS452 - Operating Systems Concepts
 * Professor: Dr. Greg Wolffe
 * Date: 1/24/2019
 * 
 * A simple shell that reads in commands from the user. The user can
 * run programs from the shell with a max of 10 arguments. Usage
 * statistics are given after a program has finished running in the
 * shell.
 * 
 * Type 'quit' to exit the shell.
***************************************************************************/

// Number of characters that can be typed when shell is reading input.
#define CHAR_LIMIT 1024

// Max number of arguments that can be passed.
#define ARG_LIMIT 32

/**
 * Clears out arguments from argument buffer for
 * after they have been used.
 * @param argv, the argument buffer to be cleared.
 **/
void clearArgs(char **argv) {
  int i;
  for (i = 0; i < ARG_LIMIT; i++) {
    argv[i] = NULL;
  }
}

/**
 * Main function that drives shell.
 **/
int main() {
  // The input buffer for reading commands.
  char *input = malloc(sizeof(char) * CHAR_LIMIT);

  // The argument buffer for executing programs.
  char **argv = malloc(sizeof(char *) * ARG_LIMIT);

  // Counter for parsing arguments, child process status, child process PID.
  int count, status, child;

  // Temporary variable for time calculations.
  long temp_micro;

  // Struct for gathering usage statistics about child processes.
  struct rusage usageStats;

  printf("Welcome! Type \"quit\" to close the shell.\n");

  /* 
   * Driving loop of program.
   * Inputting 'quit' breaks out of the loop.
   */
  while (1) {
    printf("$ ");

    // Reading user input.
    input = fgets(input, CHAR_LIMIT, stdin);

    // Handling newline input.
    if (!strcmp(input, "\n")) {
      continue;
    }

    // Handling loop exit.
    if (!strcmp(input, "quit\n")) {
      printf("Goodbye!\n");
      break;
    }

    // Removing newlines from end of input.
    input = strtok(input, "\n");

    // Reading first token.
    char *token = strtok(input, " ");

    // Setting counter from parsing tokens.
    count = 0;

    /*
     * Loop for parsing tokens. The following source
     * was used to figure out how to properly use
     * the strtok() subroutine:
     * https://www.geeksforgeeks.org/strtok-strtok_r-functions-c-examples/
     */
    while (token != NULL) {
      // If the counter is at the argument limit, stop.
      if (count == ARG_LIMIT) {
        break;
      }

      // If a space is entered, replace it with a blank character.
      if (!strcmp(token, " ")) {
        argv[count] = "";
      }
      // Otherwise, read token into argument buffer.
      else {
        argv[count] = token;
      }

      // Increment counter.
      count++;

      // Onto the next token!
      token = strtok(NULL, " ");
    }

    // Forking the child process that will exec().
    int pid = fork();

    // Error handling.
    if (pid < 0) {
      printf("Uh-oh. There was an error!\n");
      exit(1);
    }
    // Child process handling.
    else if (pid == 0) {
      // Executing user arguments.
      execvp(argv[0], &argv[0]);

      // Handling errors from bad commands.
      printf("That didn't work.\n");
      exit(1);
    }
    // Parent process handling.
    else {
      // Waiting on child. Grabs usage stats when waiting finishes.
      child = wait3(&status, 0, &usageStats);

      // Clearing argument buffer for next run.
      clearArgs(argv);

      // Resetting counter.
      count = 0;

      // Don't print usage statistics if child errors.
      if (status != 0) {
        continue;
      }
      
      // For user time calculation.
      temp_micro = (usageStats.ru_utime.tv_sec * 1000000) + (usageStats.ru_utime.tv_usec);
      
      // Displaying information to user.
      printf("\nChild process with PID %d finished.\n", child);
      printf("Child process took %ld microseconds user time.\n", temp_micro);
      printf("It also had %ld involuntary context switches.\n", usageStats.ru_nivcsw);
    }

    printf("\n");
  }

  // Freeing allocated memory.
  free(input);
  free(argv);

  // Goodbye.
  return 0;
}
