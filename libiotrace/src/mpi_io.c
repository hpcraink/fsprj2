/**
 * @file Implementation of MPI-IO functions.
 */
#include "libiotrace_config.h"

#include "json_include_struct.h"
#include "event.h"
#include "wrapper_defines.h"
#include "mpi.h"

enum access_mode get_access_amode(int mode) {
	if (mode & MPI_MODE_RDONLY) {
		return read_only;
	}
	if (mode & MPI_MODE_RDWR) {
		return write_only;
	}
	if (mode & MPI_MODE_WRONLY) {
		return read_and_write;
	}
	return unknown_access_mode;
}

void get_creation_amode(const int mode, struct creation_flags *cf) {
	cf->creat = mode & MPI_MODE_CREATE ? 1 : 0;
	cf->excl = mode & MPI_MODE_EXCL ? 1 : 0;
#ifdef HAVE_O_CLOEXEC
	cf->cloexec = 0;
#endif
#ifdef HAVE_O_DIRECTORY
	cf->directory = 0;
#endif
#ifdef HAVE_O_NOFOLLOW
	cf->nofollow = 0;
#endif
#ifdef HAVE_O_TMPFILE
	cf->tmpfile = 0;
#endif
	cf->noctty = 0;
	cf->trunc = 0;
}

void get_status_amode(const int mode, struct status_flags *sf) {
	sf->initial_append = mode & MPI_MODE_APPEND ? 1 : 0;
	sf->delete_on_close = mode & MPI_MODE_DELETE_ON_CLOSE ? 1 : 0;
	sf->unique_open = mode & MPI_MODE_UNIQUE_OPEN ? 1 : 0;
	sf->sequential = mode & MPI_MODE_SEQUENTIAL ? 1 : 0;
#ifdef HAVE_O_DIRECT
	sf->direct = 0;
#endif
#ifdef HAVE_O_NOATIME
	sf->noatime = 0;
#endif
#ifdef HAVE_O_PATH
	sf->path = 0;
#endif
#ifdef HAVE_O_LARGEFILE
	sf->largefile = 0;
#endif
	sf->append = 0;
	sf->async = 0;
	sf->dsync = 0;
	sf->nonblock = 0;
	sf->ndelay = 0;
	sf->sync = 0;
}

int MPI_File_open(MPI_Comm comm, const char *filename, int amode,
		MPI_Info info, MPI_File *fh) {
	int ret;
	struct basic data;
	struct file_mpi file_mpi_data;
	struct open_function open_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, open_function, open_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	open_data.file_name = filename;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_mpi,
			file_mpi_data)
	open_data.mode = get_access_amode(amode);
	get_creation_amode(amode, &open_data.creation);
	get_status_amode(amode, &open_data.status);
	
	open_data.file_mode.execute_by_group = 0;
	open_data.file_mode.execute_by_others = 0;
	open_data.file_mode.execute_by_owner = 0;
	open_data.file_mode.read_by_group = 0;
	open_data.file_mode.read_by_others = 0;
	open_data.file_mode.read_by_owner = 0;
	open_data.file_mode.write_by_group = 0;
	open_data.file_mode.write_by_others = 0;
	open_data.file_mode.write_by_owner = 0;

	CALL_REAL_MPI_FUNCTION_RET(data, ret, MPI_File_open, comm, filename, amode, info, fh)

	//ToDo: get_mode_flags(0, &open_data.file_mode); from amode parameter
	if (MPI_INFO_NULL != info) {
		int count_elements;
		int flag;
		char key[MPI_MAX_INFO_KEY];
		char value[MPI_MAX_INFO_VAL];
		MPI_Info_get_nkeys(info, &count_elements);
		for (int i = 0; i < count_elements; i++) {
			MPI_Info_get_nthkey(info, i, &key[0]);
			MPI_Info_get(info, &key[0], MPI_MAX_INFO_VAL-1, &value[0], &flag);
			printf("key: %s, value: %s\n",key,value);
		}
	}

	if (-1 == ret) {
		data.return_state = error;
		open_data.id.device_id = 0;
		open_data.id.inode_nr = 0;
	} else {
		data.return_state = ok;
		get_file_id_by_path(filename, &(open_data.id));
	}

	file_mpi_data.mpi_file = fh;

	WRAP_END(data)
	return ret;
}
