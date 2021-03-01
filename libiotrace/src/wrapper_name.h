#define WRAPPER_NAME_TO_JSON_STRUCT 0
#define WRAPPER_NAME_TO_VARIABLE 1
#define WRAPPER_NAME_TO_SET_VARIABLE 2
#define WRAPPER_NAME_TO_DLSYM 3


#undef WRAPPER_NAME

#if WRAPPER_NAME_TO_SOURCE == WRAPPER_NAME_TO_JSON_STRUCT
#  define WRAPPER_NAME(function_name) JSON_STRUCT_CHAR(function_name)

#elif WRAPPER_NAME_TO_SOURCE == WRAPPER_NAME_TO_VARIABLE
#  define WRAPPER_NAME(function_name) active_wrapper_status.function_name = WRAPPER_ACTIVE;

#elif WRAPPER_NAME_TO_SOURCE == WRAPPER_NAME_TO_SET_VARIABLE
#  define WRAPPER_NAME(function_name) WRAPPER_ACTIVATE(line, function_name, toggle)

#elif WRAPPER_NAME_TO_SOURCE == WRAPPER_NAME_TO_DLSYM
#  define WRAPPER_NAME(function_name) DLSYM(function_name);

#endif
