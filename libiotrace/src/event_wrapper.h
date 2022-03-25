#include "wrapper_name.h"

WRAPPER_NAME(execve)
WRAPPER_NAME(execv)
WRAPPER_NAME(execl)
WRAPPER_NAME(execvp)
WRAPPER_NAME(execlp)
#ifdef HAVE_EXECVPE
WRAPPER_NAME(execvpe)
#endif
WRAPPER_NAME(execle)
WRAPPER_NAME(_exit)
#ifdef HAVE_EXIT
WRAPPER_NAME(_Exit)
#endif
#ifdef HAVE_EXIT_GROUP
WRAPPER_NAME(exit_group)
#endif
WRAPPER_NAME(pthread_create)
