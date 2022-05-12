#include "wrapper_name.h"

#ifdef LOG_WRAPPER_TIME
WRAPPER_NAME(init_on_load)
#  ifdef IOTRACE_ENABLE_LOGFILE
WRAPPER_NAME(cleanup_process)
#  endif
#endif
#ifdef WITH_STD_IO
WRAPPER_NAME(open_std_fd)
WRAPPER_NAME(open_std_file)
#endif
