#include "kernel/types.h"
#include "user/user.h"
#include "kernel/fcntl.h"
#include <stddef.h>

/* Read a line of characters from stdin. */
int getcmd(char *buf, int nbuf) {
    printf(">>> ");
    memset(buf, 0, nbuf);    //clear the buffer
    gets(buf, nbuf);

    if (buf[0] == 0){
        return -1;    //handle and empty command
    }

  return 0;
}

/*
  A recursive function which parses the command
  at *buf and executes it.
*/
__attribute__((noreturn))
void run_command(char *buf, int nbuf, int *pcp) {

  /* Useful data structures and flags. */
  char *arguments[10];
  int numargs = 0;
  /* Word start/end */
  int ws = 0;
  //int we = 0;
  char *token_start = NULL;

  int redirection_left = 0;
  int redirection_right = 0;
  //char *file_name_l = 0;
  //char *file_name_r = 0;

  //int p[2];
  int pipe_cmd = 0;

  int sequence_cmd = 0;

  int i = 0;
  /* Parse the command character by character. */
  for (; i < nbuf; i++) {

    char c = buf[i];

    if (strchr(" \t\r\n\v", c)) {
      if (ws) {
        // End of a token
        buf[i] = '\0';  // Null terminate the current token
        arguments[numargs++] = token_start;  // Store the token in arguments[]
        ws = 0;   // We're not in a token anymore
      }
      continue;
    }

    // Check for special symbols
    if (strchr("|&;<>()", c)) {
      // Handle pipe, redirection, or sequence command.
      if (c == '|') {
        pipe_cmd = 1;
      } else if (c == '<') {
        redirection_left = 1;
        //file_name_l = &buf[i + 1];  // Next token will be the input file
      } else if (c == '>') {
        redirection_right = 1;
        //file_name_r = &buf[i + 1];  // Next token will be the output file
      } else if (c == ';') {
        sequence_cmd = 1;
      }

      buf[i] = '\0';  // Null terminate the symbol to isolate the command
      continue;
    }

    // If we are not in a token and we hit a non-whitespace, non-symbol character, start a new token.
    if (!ws) {
      token_start = &buf[i];  // Mark the start of the token
      ws = 1;  // We are now in a token
    }
  }

  // Null terminate the last argument
  if (ws) {
    arguments[numargs++] = token_start;
  }
  arguments[numargs] = NULL;  // exec() requires a NULL-terminated array
  
    /* Parse the current character and set-up various flags:
       sequence_cmd, redirection, pipe_cmd and similar. */

    /* ##### Place your code here. */

    if (!(redirection_left || redirection_right)) {
      /* No redirection, continue parsing command. */

      // Place your code here.
    } else {
      /* Redirection command. Capture the file names. */

      // ##### Place your code here.
    }

  /*
    Sequence command. Continue this command in a new process.
    Wait for it to complete and execute the command following ';'.
  */
  if (sequence_cmd) {
    sequence_cmd = 0;
    if (fork() != 0) {
      wait(0);
      // ##### Place your code here.
    }
  }

  /*
    If this is a redirection command,
    tie the specified files to std in/out.
  */
  if (redirection_left) {
    // ##### Place your code here.
  }
  if (redirection_right) {
    // ##### Place your code here.
  }

  /* Parsing done. Execute the command. */

  /*
    If this command is a CD command, write the arguments to the pcp pipe
    and exit with '2' to tell the parent process about this.
  */
  if (strcmp(arguments[0], "cd") == 0) {
    write(pcp[1], arguments[1], strlen(arguments[1])); // write the arguments to the pipe
    close(pcp[1]); // close the write end of the pipe
    exit(2); // exit with '2'
    
  } else {
    /*
      Pipe command: fork twice. Execute the left hand side directly.
      Call run_command recursion for the right side of the pipe.
    */
    if (pipe_cmd) {
      // ##### Place your code here.
    } else {
        exec(arguments[0], arguments);
      // ##### Place your code here.
    }
  }
  exit(0);

}

int main(void) {

  static char buf[100];

  int pcp[2];
  pipe(pcp);

  /* Read and run input commands. */
  while(getcmd(buf, sizeof(buf)) >= 0){
    if(fork() == 0)
      run_command(buf, 100, pcp);

    /*
      Check if run_command found this is
      a CD command and run it if required.
    */
    int child_status;
    wait(&child_status);

    //int exit_status = WEXITSTATUS(child_status);

    if(child_status == 2){
        char new_directory[100];

        read(pcp[0], new_directory, sizeof(new_directory)); // read the new directory from the pipe
        close(pcp[0]); // close the read end of the pipe

        if (chdir(new_directory) < 0) {
        fprintf(2, "cannot cd %s\n", new_directory);
        }
    }
  }
  exit(0);
}
