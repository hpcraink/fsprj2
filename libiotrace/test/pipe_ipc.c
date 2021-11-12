/**
 * Demo of pipe for performing IPC b/w 2 child processes
 */

#include <stdio.h>          /* perror, fprintf, puts */
#include <string.h>         /* strcpy */
#include <unistd.h>         /* execvp, close, dup, dup2, pipe, fork, execvp */
#include <sys/wait.h>       /* waitpid */


/* -- Constants -- */
char* const PRODUCER_EXEC_COMMAND_ARG0 = "ls";
char* const PRODUCER_EXEC_COMMAND_ARG1 = "-lah";

char* const COMSUMER_EXEC_COMMAND_ARG0 = "grep";
char* const CONSUMER_EXEC_COMMAND_ARG1 = "total";



int main(void) {
    pid_t pid1, pid2;
    int fd_pipe[2];
    if (pipe(fd_pipe) < 0) {
        perror("Parent: pipe() failed");
        return 1;
    }


    if ((pid1 = fork()) < 0) {
        perror("Parent: fork() for creating child-1 failed!");
        return 1;

    } else if (0 == pid1) {     /* Child-1 [PRODUCER] */
        close(1); dup(fd_pipe[1]);      /* Equivalent to dup2(fd_pipe[1], 1); */
        close(fd_pipe[0]);
        close(fd_pipe[1]);

        fprintf(stderr, ">  Child-1 [PRODUCER]: I'll be executing `%s %s`  <\n", PRODUCER_EXEC_COMMAND_ARG0, PRODUCER_EXEC_COMMAND_ARG1);

        execlp(PRODUCER_EXEC_COMMAND_ARG0,
               PRODUCER_EXEC_COMMAND_ARG0, PRODUCER_EXEC_COMMAND_ARG1, NULL);
        perror("Child-1 [PRODUCER]: execvp() failed, exiting ...");
        return 1;
    }


    if ((pid2 = fork()) < 0) {
        perror("Parent: fork() for creating child-2 failed!");
        return 1;

    } else if (0 == pid2) {     /* Child-2 [CONSUMER] */
        dup2(fd_pipe[0], 0);            /* Equivalent to close(0); dup(fd_pipe[0]); */
        close(fd_pipe[0]);
        close(fd_pipe[1]);

        fprintf(stderr, ">  Child-2 [CONSUMER]: I'll be executing `%s %s`  <\n", COMSUMER_EXEC_COMMAND_ARG0, CONSUMER_EXEC_COMMAND_ARG1);

        execlp(COMSUMER_EXEC_COMMAND_ARG0,
               COMSUMER_EXEC_COMMAND_ARG0, CONSUMER_EXEC_COMMAND_ARG1, NULL);
        perror("Child-2 [CONSUMER]: execvp() failed, exiting ...");
        return 1;
    }

    close(fd_pipe[0]);
    close(fd_pipe[1]);

    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);

    return 0;
}
