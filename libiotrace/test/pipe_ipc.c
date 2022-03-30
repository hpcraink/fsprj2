/**
 * Demo of pipe for performing IPC b/w 2 child processes
 */

#include <stdio.h>          /* perror, fprintf, puts */
#include <string.h>         /* strcpy */
#include <unistd.h>         /* execvp, close, dup, dup2, pipe, fork, execvp */
#include <sys/wait.h>       /* waitpid */

#include "common/error.h"


/* -- Constants -- */
char* const PRODUCER_EXEC_COMMAND_ARG0 = "ls";
char* const PRODUCER_EXEC_COMMAND_ARG1 = "-lah";

char* const COMSUMER_EXEC_COMMAND_ARG0 = "grep";
char* const CONSUMER_EXEC_COMMAND_ARG1 = "total";



int main(void) {
    pid_t pid1, pid2;

    int fd_pipe[2];
    DIE_WHEN_ERRNO( pipe(fd_pipe) );


    if (0 == (pid1 = DIE_WHEN_ERRNO( fork() ))) {     /* Child-1 [PRODUCER] */
        DIE_WHEN_ERRNO( dup2(fd_pipe[1], STDOUT_FILENO) );
        DIE_WHEN_ERRNO( close(fd_pipe[0] ));
        DIE_WHEN_ERRNO( close(fd_pipe[1] ));

        fprintf(stderr, ">  Child-1 [PRODUCER]: I'll be executing `%s %s`  <\n", PRODUCER_EXEC_COMMAND_ARG0, PRODUCER_EXEC_COMMAND_ARG1);

        execlp(PRODUCER_EXEC_COMMAND_ARG0,
               PRODUCER_EXEC_COMMAND_ARG0, PRODUCER_EXEC_COMMAND_ARG1, NULL);

        perror("Child-1 [PRODUCER]: execvp() failed, exiting ...");
        return 1;
    }


    if (0 == (pid2 = DIE_WHEN_ERRNO( fork() ))) {      /* Child-2 [CONSUMER] */
        DIE_WHEN_ERRNO( dup2(fd_pipe[0], STDIN_FILENO) );
        DIE_WHEN_ERRNO( close(fd_pipe[0]) );
        DIE_WHEN_ERRNO( close(fd_pipe[1]) );

        fprintf(stderr, ">  Child-2 [CONSUMER]: I'll be executing `%s %s`  <\n", COMSUMER_EXEC_COMMAND_ARG0, CONSUMER_EXEC_COMMAND_ARG1);

        execlp(COMSUMER_EXEC_COMMAND_ARG0,
               COMSUMER_EXEC_COMMAND_ARG0, CONSUMER_EXEC_COMMAND_ARG1, NULL);

        perror("Child-2 [CONSUMER]: execvp() failed, exiting ...");
        return 1;
    }


    DIE_WHEN_ERRNO( close(fd_pipe[0]) );
    DIE_WHEN_ERRNO( close(fd_pipe[1]) );


    DIE_WHEN_ERRNO( waitpid(pid1, NULL, 0) );
    DIE_WHEN_ERRNO( waitpid(pid2, NULL, 0) );

    return 0;
}
