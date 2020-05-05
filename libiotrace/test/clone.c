/**
 * based on release 4.15 of the Linux man-pages project
 */

#include <sys/wait.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>

#define errExit(msg)    do { perror(msg); exit(EXIT_FAILURE); \
                               } while (0)

static int /* Start function for cloned child */
childFunc(void *arg) {

	printf("in child\n");

	return 0; /* Child terminates now */
}

#define STACK_SIZE (1024 * 1024)    /* Stack size for cloned child */

int main(int argc, char *argv[]) {
	char *stack; /* Start of stack buffer */
	char *stackTop; /* End of stack buffer */
	pid_t pid;
	pid_t ptid;

	/* Allocate stack for child */
	stack = malloc(STACK_SIZE);
	if (stack == NULL)
		errExit("malloc");
	stackTop = stack + STACK_SIZE; /* Assume stack grows downward */

	pid = clone(childFunc, stackTop, SIGCHLD, NULL);
	if (pid == -1)
		errExit("clone");

	if (waitpid(pid, NULL, 0) == -1) /* Wait for child */
		errExit("waitpid");
	printf("child has terminated\n");

	pid = clone(childFunc, stackTop, SIGCHLD | CLONE_PARENT_SETTID, NULL, &ptid);
	if (pid == -1)
		errExit("clone");

	if (waitpid(pid, NULL, 0) == -1) /* Wait for child */
		errExit("waitpid");
	printf("child has terminated\n");

	exit(EXIT_SUCCESS);
}
