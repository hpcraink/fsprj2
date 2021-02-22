#define WRAPPER_NAME_TO_VARIABLE 0
#define WRAPPER_NAME_TO_SET_VARIABLE 1

#undef WRAPPER_NAME

#if WRAPPER_NAME_TO_SOURCE == WRAPPER_NAME_TO_VARIABLE
#define WRAPPER_NAME(function_name) char libio_##function_name = WRAPPER_ACTIVE;

#elif WRAPPER_NAME_TO_SOURCE == WRAPPER_NAME_TO_SET_VARIABLE
#define WRAPPER_NAME(function_name) WRAPPER_ACTIVATE(line, function_name, toggle)
#endif
