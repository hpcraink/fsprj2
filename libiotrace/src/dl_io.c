/**
 * @file Implementation of dl functions.
 */
#include "libiotrace_config.h"

#include <dlfcn.h>

#include "json_include_struct.h"
#include "event.h"
#include "wrapper_defines.h"
#include "dl_io.h"

#include "wrapper_name.h"

#ifndef IO_LIB_STATIC
REAL_DEFINITION_TYPE void * REAL_DEFINITION(dlopen)(const char *filename, int flags) REAL_DEFINITION_INIT;
#ifdef HAVE_DLMOPEN
REAL_DEFINITION_TYPE void * REAL_DEFINITION(dlmopen)(Lmid_t lmid, const char *filename, int flags) REAL_DEFINITION_INIT;
#endif
#endif

char toggle_dl_wrapper(char *line, char toggle)
{
	char ret = 1;

	if (!strcmp(line, "")) {
		ret = 0;
	}
#undef WRAPPER_NAME_TO_SOURCE
#define WRAPPER_NAME_TO_SOURCE WRAPPER_NAME_TO_SET_VARIABLE
#include "dl_io_wrapper.h"
	else
	{
		ret = 0;
	}
	return ret;
}

#ifndef IO_LIB_STATIC
char dl_io_init_done = 0;
/* Initialize pointers for dl functions. */
void dl_io_init() {
	if (!dl_io_init_done) {

#undef WRAPPER_NAME_TO_SOURCE
#define WRAPPER_NAME_TO_SOURCE WRAPPER_NAME_TO_DLSYM
#include "dl_io_wrapper.h"

		dl_io_init_done = 1;
	}
}
#endif

void get_dlopen_flags(const int flags, struct dlopen_flags *dlf) {
	dlf->lazy_binding = flags & RTLD_LAZY ? 1 : 0;
	dlf->bind_now = flags & RTLD_NOW ? 1 : 0;
	dlf->global = flags & RTLD_GLOBAL ? 1 : 0;
	dlf->local = flags & RTLD_LOCAL ? 1 : 0;
	dlf->no_delete = flags & RTLD_NODELETE ? 1 : 0;
	dlf->no_load = flags & RTLD_NOLOAD ? 1 : 0;
	dlf->deep_bind = flags & RTLD_DEEPBIND ? 1 : 0;
}

#ifdef HAVE_DLMOPEN
enum so_namespace_mode_enum get_so_namespace_mode_enum(Lmid_t mode) {
	switch (mode) {
	case LM_ID_BASE:
		return initial_namespace;
	case LM_ID_NEWLM:
		return new_namespace;
	default:
		return unknown_so_namespace_mode;
	}
}
#endif

void * WRAP(dlopen)(const char *filename, int flags) {
	void * ret;
	struct basic data;
	struct dlopen_function dlopen_function_data;
	struct shared_library shared_library_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, dlopen_function,
			dlopen_function_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, shared_library, shared_library_data)
	dlopen_function_data.file_name = filename;
	get_dlopen_flags(flags, &dlopen_function_data.dl_flags);

	CALL_REAL_FUNCTION_RET(data, ret, dlopen, filename, flags)

	if (NULL == ret) {
		data.return_state = error;
		shared_library_data.dl_handle = NULL;
		dlopen_function_data.id.device_id = 0;
		dlopen_function_data.id.inode_nr = 0;
	} else {
		data.return_state = ok;
		shared_library_data.dl_handle = ret;
		get_file_id_by_path(filename, &(dlopen_function_data.id));
	}

	WRAP_END(data)
	return ret;
}

#ifdef HAVE_DLMOPEN
void * WRAP(dlmopen)(Lmid_t lmid, const char *filename, int flags) {
	void * ret;
	struct basic data;
	struct dlmopen_function dlmopen_function_data;
	struct shared_library shared_library_data;
	struct so_namespace_id so_namespace_id_data;
	struct so_namespace_mode so_namespace_mode_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, dlmopen_function,
			dlmopen_function_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, shared_library, shared_library_data)
	dlmopen_function_data.file_name = filename;
	get_dlopen_flags(flags, &dlmopen_function_data.dl_flags);
	enum so_namespace_mode_enum mode = get_so_namespace_mode_enum(lmid);
	if (unknown_so_namespace_mode == mode) {
		so_namespace_id_data.id = lmid;
		JSON_STRUCT_SET_VOID_P(dlmopen_function_data, so_namespace,
				so_namespace_id, so_namespace_id_data)
	} else {
		so_namespace_mode_data.mode = mode;
		JSON_STRUCT_SET_VOID_P(dlmopen_function_data, so_namespace,
				so_namespace_mode, so_namespace_mode_data)
	}

	CALL_REAL_FUNCTION_RET(data, ret, dlmopen, lmid, filename, flags)

	if (NULL == ret) {
		data.return_state = error;
		shared_library_data.dl_handle = NULL;
		dlmopen_function_data.id.device_id = 0;
		dlmopen_function_data.id.inode_nr = 0;
	} else {
		data.return_state = ok;
		shared_library_data.dl_handle = ret;
		get_file_id_by_path(filename, &(dlmopen_function_data.id));
	}

	WRAP_END(data)
	return ret;
}
#endif
