/**
 * @file Implementation of MPI-IO functions.
 */
#include "libiotrace_config.h"

#include "json_include_struct.h"
#include "event.h"
#include "wrapper_defines.h"
#include "mpi.h"

enum access_mode get_access_amode(int mode)
{
	if (mode & MPI_MODE_RDONLY)
	{
		return read_only;
	}
	if (mode & MPI_MODE_RDWR)
	{
		return write_only;
	}
	if (mode & MPI_MODE_WRONLY)
	{
		return read_and_write;
	}
	return unknown_access_mode;
}

void get_creation_amode(const int mode, struct creation_flags *cf)
{
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

void get_status_amode(const int mode, struct status_flags *sf)
{
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

enum seek_where get_seek_where_mpi(int whence)
{
	switch (whence)
	{
	case MPI_SEEK_SET:
		return beginning_of_file;
	case MPI_SEEK_CUR:
		return current_position;
	case MPI_SEEK_END:
		return end_of_file;

	default:
		return unknown_seek_where;
	}
}

int MPI_File_open(MPI_Comm comm, const char *filename, int amode, MPI_Info info,
				  MPI_File *fh)
{
	int ret;
	struct basic data;
	struct file_mpi file_mpi_data;
	struct mpi_open_function open_data;

	WRAP_MPI_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, mpi_open_function, open_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	open_data.file_name = filename;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_mpi, file_mpi_data)
	open_data.mode = get_access_amode(amode);
	get_creation_amode(amode, &open_data.creation);
	get_status_amode(amode, &open_data.status);

	//ToDo: get_mode_flags(0, &open_data.file_mode); from MPI_Info parameter
	open_data.file_mode.execute_by_group = 0;
	open_data.file_mode.execute_by_others = 0;
	open_data.file_mode.execute_by_owner = 0;
	open_data.file_mode.read_by_group = 0;
	open_data.file_mode.read_by_others = 0;
	open_data.file_mode.read_by_owner = 0;
	open_data.file_mode.write_by_group = 0;
	open_data.file_mode.write_by_others = 0;
	open_data.file_mode.write_by_owner = 0;

	CALL_REAL_MPI_FUNCTION_RET(data, ret, MPI_File_open, comm, filename, amode,
							   info, fh)

	JSON_STRUCT_SET_KEY_VALUE_ARRAY_NULL(open_data, file_hints)

	if (MPI_INFO_NULL != info)
	{
		int count_elements;
		int flag;
		char *keys[MAX_MPI_FILE_HINTS];
		char *values[MAX_MPI_FILE_HINTS];
		char key_strings[MAX_MPI_FILE_HINTS][MAX_MPI_FILE_HINT_LENGTH];
		char value_strings[MAX_MPI_FILE_HINTS][MAX_MPI_FILE_HINT_LENGTH];

		MPI_Info_get_nkeys(info, &count_elements);

		if (count_elements >= 1)
		{
			JSON_STRUCT_INIT_KEY_VALUE_ARRAY(open_data, file_hints, keys,
											 values)

			for (int i = 0; i < count_elements && i < MAX_MPI_FILE_HINTS; i++)
			{
				MPI_Info_get_nthkey(info, i, key_strings[i]);
				MPI_Info_get(info, key_strings[i], MPI_MAX_INFO_VAL - 1,
							 value_strings[i], &flag);

				JSON_STRUCT_ADD_KEY_VALUE(open_data, file_hints, key_strings[i],
										  value_strings[i])
			}
		}
	}

	if (ret != MPI_SUCCESS)
	{
		data.return_state = error;
		SET_MPI_ERROR(ret, MPI_STATUS_IGNORE)
		open_data.id.device_id = 0;
		open_data.id.inode_nr = 0;
		file_mpi_data.mpi_file = 0;
	}
	else
	{
		data.return_state = ok;
		get_file_id_by_path(filename, &(open_data.id));
		file_mpi_data.mpi_file = MPI_File_c2f(*fh);
	}

	WRAP_MPI_END(data)

	return ret;
}

int MPI_File_write(MPI_File fh, const void *buf, int count, MPI_Datatype datatype, MPI_Status *status)
{
	int ret;
	struct basic data;
	struct write_function write_data;
	struct file_mpi file_mpi_data;
	int datatype_size;
	char own_status_used = 0;
	MPI_Status own_status;
	int get_count;

	WRAP_MPI_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, write_function, write_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_mpi, file_mpi_data)

	file_mpi_data.mpi_file = MPI_File_c2f(fh);
	MPI_Type_size(datatype, &datatype_size);

	if (MPI_STATUS_IGNORE == status)
	{

		status = &own_status;
		own_status_used = 1;
	}

	CALL_REAL_MPI_FUNCTION_RET(data, ret, MPI_File_write, fh, buf, count, datatype, status)

	if (ret != MPI_SUCCESS)
	{
		data.return_state = error;
		write_data.written_bytes = 0;
		SET_MPI_ERROR(ret, status)
	}
	else
	{
		data.return_state = ok;
		MPI_Get_count(status, datatype, &get_count);
		write_data.written_bytes = datatype_size * get_count;
	}

	if (own_status_used)
	{
		status = MPI_STATUS_IGNORE;
	}
	WRAP_MPI_END(data)
	return ret;
}

int MPI_File_iwrite(MPI_File fh, const void *buf, int count, MPI_Datatype datatype, MPI_Request *request)
{
	int ret;
	struct basic data;
	struct mpi_immediate mpi_immediate_data;
	struct file_mpi file_mpi_data;
	int datatype_size;

	WRAP_MPI_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, mpi_immediate, mpi_immediate_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_mpi, file_mpi_data)

	file_mpi_data.mpi_file = MPI_File_c2f(fh);

	CALL_REAL_MPI_FUNCTION_RET(data, ret, MPI_File_iwrite, fh, buf, count, datatype, request)

	mpi_immediate_data.request_id = MPI_Request_c2f(*request);

	if (ret != MPI_SUCCESS)
	{
		data.return_state = error;
		SET_MPI_ERROR(ret, MPI_STATUS_IGNORE)
	}
	else
	{
		data.return_state = ok;
	}

	WRAP_MPI_END(data)
	return ret;
}

int MPI_File_iwrite_all(MPI_File fh, const void *buf, int count, MPI_Datatype datatype, MPI_Request *request)
{
	int ret;
	struct basic data;
	struct mpi_immediate mpi_immediate_data;
	struct file_mpi file_mpi_data;
	int datatype_size;

	WRAP_MPI_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, mpi_immediate, mpi_immediate_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_mpi, file_mpi_data)

	file_mpi_data.mpi_file = MPI_File_c2f(fh);

	CALL_REAL_MPI_FUNCTION_RET(data, ret, MPI_File_iwrite_all, fh, buf, count, datatype, request)

	mpi_immediate_data.request_id = MPI_Request_c2f(*request);

	if (ret != MPI_SUCCESS)
	{
		data.return_state = error;
		SET_MPI_ERROR(ret, MPI_STATUS_IGNORE)
	}
	else
	{
		data.return_state = ok;
	}

	WRAP_MPI_END(data)
	return ret;
}

int MPI_File_write_all(MPI_File fh, const void *buf, int count, MPI_Datatype datatype, MPI_Status *status)
{
	int ret;
	struct basic data;
	struct write_function write_data;
	struct file_mpi file_mpi_data;
	int datatype_size;
	char own_status_used = 0;
	MPI_Status own_status;
	int get_count;

	WRAP_MPI_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, write_function, write_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_mpi, file_mpi_data)

	file_mpi_data.mpi_file = MPI_File_c2f(fh);
	MPI_Type_size(datatype, &datatype_size);

	if (MPI_STATUS_IGNORE == status)
	{

		status = &own_status;
		own_status_used = 1;
	}

	CALL_REAL_MPI_FUNCTION_RET(data, ret, MPI_File_write, fh, buf, count, datatype, status)

	if (ret != MPI_SUCCESS)
	{
		data.return_state = error;
		write_data.written_bytes = 0;
		SET_MPI_ERROR(ret, status)
	}
	else
	{
		data.return_state = ok;
		MPI_Get_count(status, datatype, &get_count);
		write_data.written_bytes = datatype_size * get_count;
	}

	if (own_status_used)
	{
		status = MPI_STATUS_IGNORE;
	}
	WRAP_MPI_END(data)
	return ret;
}

int MPI_File_read(MPI_File fh, void *buf, int count, MPI_Datatype datatype, MPI_Status *status)
{

	int ret;
	struct basic data;
	struct read_function read_data;
	struct file_mpi file_mpi_data;
	int datatype_size;
	MPI_Status own_status;
	char own_status_used = 0;
	int get_count;

	WRAP_MPI_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, read_function, read_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_mpi, file_mpi_data)

	file_mpi_data.mpi_file = MPI_File_c2f(fh);
	MPI_Type_size(datatype, &datatype_size);

	if (MPI_STATUS_IGNORE == status)
	{

		status = &own_status;
		own_status_used = 1;
	}

	CALL_REAL_MPI_FUNCTION_RET(data, ret, MPI_File_read, fh, buf, count, datatype, status)

	if (ret != MPI_SUCCESS)
	{
		data.return_state = error;
		read_data.read_bytes = 0;
		SET_MPI_ERROR(ret, status)
	}
	else
	{
		data.return_state = ok;
		MPI_Get_count(status, datatype, &get_count);
		read_data.read_bytes = datatype_size * get_count;
	}

	if (own_status_used)
	{
		status = MPI_STATUS_IGNORE;
	}

	WRAP_MPI_END(data)
	return ret;
}

int MPI_File_iread(MPI_File fh, void *buf, int count, MPI_Datatype datatype, MPI_Request *request)
{

	int ret;
	struct basic data;
	struct mpi_immediate mpi_immediate_data;
	struct file_mpi file_mpi_data;
	int datatype_size;

	WRAP_MPI_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, mpi_immediate, mpi_immediate_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_mpi, file_mpi_data)

	file_mpi_data.mpi_file = MPI_File_c2f(fh);

	CALL_REAL_MPI_FUNCTION_RET(data, ret, MPI_File_iread, fh, buf, count, datatype, request)

	mpi_immediate_data.request_id = MPI_Request_c2f(*request);

	if (ret != MPI_SUCCESS)
	{
		data.return_state = error;
		SET_MPI_ERROR(ret, MPI_STATUS_IGNORE)
	}
	else
	{
		data.return_state = ok;
	}

	WRAP_MPI_END(data)
	return ret;
}

int MPI_File_iread_all(MPI_File fh, void *buf, int count, MPI_Datatype datatype, MPI_Request *request)
{
	int ret;
	struct basic data;
	struct mpi_immediate mpi_immediate_data;
	struct file_mpi file_mpi_data;
	int datatype_size;

	WRAP_MPI_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, mpi_immediate, mpi_immediate_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_mpi, file_mpi_data)

	file_mpi_data.mpi_file = MPI_File_c2f(fh);

	CALL_REAL_MPI_FUNCTION_RET(data, ret, MPI_File_iread_all, fh, buf, count, datatype, request)

	mpi_immediate_data.request_id = MPI_Request_c2f(*request);

	if (ret != MPI_SUCCESS)
	{
		data.return_state = error;
		SET_MPI_ERROR(ret, MPI_STATUS_IGNORE)
	}
	else
	{
		data.return_state = ok;
	}

	WRAP_MPI_END(data)
	return ret;
}

int MPI_File_read_all(MPI_File fh, void *buf, int count, MPI_Datatype datatype, MPI_Status *status)
{
	int ret;
	struct basic data;
	struct read_function read_data;
	struct file_mpi file_mpi_data;
	int datatype_size;
	MPI_Status own_status;
	char own_status_used = 0;
	int get_count;

	WRAP_MPI_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, read_function, read_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_mpi, file_mpi_data)

	file_mpi_data.mpi_file = MPI_File_c2f(fh);
	MPI_Type_size(datatype, &datatype_size);

	if (MPI_STATUS_IGNORE == status)
	{

		status = &own_status;
		own_status_used = 1;
	}

	CALL_REAL_MPI_FUNCTION_RET(data, ret, MPI_File_read, fh, buf, count, datatype, status)

	if (ret != MPI_SUCCESS)
	{
		data.return_state = error;
		read_data.read_bytes = 0;
		SET_MPI_ERROR(ret, status)
	}
	else
	{
		data.return_state = ok;
		MPI_Get_count(status, datatype, &get_count);
		read_data.read_bytes = datatype_size * get_count;
	}

	if (own_status_used)
	{
		status = MPI_STATUS_IGNORE;
	}

	WRAP_MPI_END(data)
	return ret;
}

int MPI_File_close(MPI_File *fh)
{

	int ret;
	struct basic data;
	struct file_mpi file_mpi_data;

	WRAP_MPI_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P_NULL(data, function_data);
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_mpi, file_mpi_data)

	file_mpi_data.mpi_file = MPI_File_c2f(*fh);

	CALL_REAL_MPI_FUNCTION_RET(data, ret, MPI_File_close, fh)

	if (ret != MPI_SUCCESS)
	{
		data.return_state = error;
		SET_MPI_ERROR(ret, MPI_STATUS_IGNORE)
	}
	else
	{
		data.return_state = ok;
	}

	WRAP_MPI_END(data)
	return ret;
}

int MPI_File_seek(MPI_File fh, MPI_Offset offset, int whence)
{

	int ret;
	struct basic data;
	struct file_mpi file_mpi_data;
	struct positioning_function positioning_function_data;
	MPI_Offset absolute_offset;
	MPI_Offset view_offset;
	

	WRAP_MPI_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, positioning_function, positioning_function_data);
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_mpi, file_mpi_data)

	file_mpi_data.mpi_file = MPI_File_c2f(fh);

	CALL_REAL_MPI_FUNCTION_RET(data, ret, MPI_File_seek, fh, offset, whence)

	positioning_function_data.offset = offset;
	positioning_function_data.relative_to = get_seek_where_mpi(whence);

	if (ret != MPI_SUCCESS)
	{
		data.return_state = error;
		SET_MPI_ERROR(ret, MPI_STATUS_IGNORE)
	}
	else
	{
		data.return_state = ok;
		if (end_of_file == positioning_function_data.relative_to) {
			// ToDo: file lock over MPI_File_seek; get_position; get_byte_offset
			MPI_File_get_position(fh, &view_offset);
			MPI_File_get_byte_offset(fh, view_offset, &absolute_offset);
			positioning_function_data.offset = absolute_offset;
		}

	}

	WRAP_MPI_END(data)
	return ret;
}

int MPI_File_write_at(MPI_File fh, MPI_Offset offset, const void *buf, int count, MPI_Datatype datatype, MPI_Status *status)
{

	int ret;
	struct basic data;
	struct file_mpi file_mpi_data;
	struct pwrite_function pwrite_function_data;
	MPI_Status own_status;
	char own_status_used = 0;
	int get_count;
	int datatype_size;

	WRAP_MPI_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, pwrite_function, pwrite_function_data);
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_mpi, file_mpi_data)

	file_mpi_data.mpi_file = MPI_File_c2f(fh);
	MPI_Type_size(datatype, &datatype_size);

	if (MPI_STATUS_IGNORE == status)
	{

		status = &own_status;
		own_status_used = 1;
	}

	CALL_REAL_MPI_FUNCTION_RET(data, ret, MPI_File_write_at, fh, offset, buf, count, datatype, status)

	pwrite_function_data.position = offset;

	if (ret != MPI_SUCCESS)
	{
		data.return_state = error;
		pwrite_function_data.written_bytes = 0;
		SET_MPI_ERROR(ret, MPI_STATUS_IGNORE)
	}
	else
	{
		data.return_state = ok;
		MPI_Get_count(status, datatype, &get_count);
		pwrite_function_data.written_bytes = datatype_size * get_count;
	}

	if (own_status_used)
	{
		status = MPI_STATUS_IGNORE;
	}

	WRAP_MPI_END(data)
	return ret;
}

int MPI_File_write_at_all(MPI_File fh, MPI_Offset offset, const void *buf, int count, MPI_Datatype datatype, MPI_Status *status)
{
	int ret;
	struct basic data;
	struct file_mpi file_mpi_data;
	struct pwrite_function pwrite_function_data;
	MPI_Status own_status;
	char own_status_used = 0;
	int get_count;
	int datatype_size;

	WRAP_MPI_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, pwrite_function, pwrite_function_data);
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_mpi, file_mpi_data)

	file_mpi_data.mpi_file = MPI_File_c2f(fh);
	MPI_Type_size(datatype, &datatype_size);

	if (MPI_STATUS_IGNORE == status)
	{

		status = &own_status;
		own_status_used = 1;
	}

	CALL_REAL_MPI_FUNCTION_RET(data, ret, MPI_File_write_at_all, fh, offset, buf, count, datatype, status)

	pwrite_function_data.position = offset;

	if (ret != MPI_SUCCESS)
	{
		data.return_state = error;
		pwrite_function_data.written_bytes = 0;
		SET_MPI_ERROR(ret, MPI_STATUS_IGNORE)
	}
	else
	{
		data.return_state = ok;
		MPI_Get_count(status, datatype, &get_count);
		pwrite_function_data.written_bytes = datatype_size * get_count;
	}

	if (own_status_used)
	{
		status = MPI_STATUS_IGNORE;
	}

	WRAP_MPI_END(data)
	return ret;
}

int MPI_File_read_at(MPI_File fh, MPI_Offset offset, void *buf, int count, MPI_Datatype datatype, MPI_Status *status)
{
	int ret;
	struct basic data;
	struct file_mpi file_mpi_data;
	struct pread_function pread_function_data;
	MPI_Status own_status;
	char own_status_used = 0;
	int get_count;
	int datatype_size;

	WRAP_MPI_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, pread_function, pread_function_data);
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_mpi, file_mpi_data)

	file_mpi_data.mpi_file = MPI_File_c2f(fh);
	MPI_Type_size(datatype, &datatype_size);

	if (MPI_STATUS_IGNORE == status)
	{

		status = &own_status;
		own_status_used = 1;
	}

	CALL_REAL_MPI_FUNCTION_RET(data, ret, MPI_File_read_at, fh, offset, buf, count, datatype, status)

	pread_function_data.position = offset;

	if (ret != MPI_SUCCESS)
	{
		data.return_state = error;
		pread_function_data.read_bytes = 0;
		SET_MPI_ERROR(ret, MPI_STATUS_IGNORE)
	}
	else
	{
		data.return_state = ok;
		MPI_Get_count(status, datatype, &get_count);
		pread_function_data.read_bytes = datatype_size * get_count;
	}

	if (own_status_used)
	{
		status = MPI_STATUS_IGNORE;
	}

	WRAP_MPI_END(data)
	return ret;
}

int MPI_File_read_at_all(MPI_File fh, MPI_Offset offset, void *buf, int count, MPI_Datatype datatype, MPI_Status *status)
{
	int ret;
	struct basic data;
	struct file_mpi file_mpi_data;
	struct pread_function pread_function_data;
	MPI_Status own_status;
	char own_status_used = 0;
	int get_count;
	int datatype_size;

	WRAP_MPI_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, pread_function, pread_function_data);
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_mpi, file_mpi_data)

	file_mpi_data.mpi_file = MPI_File_c2f(fh);
	MPI_Type_size(datatype, &datatype_size);

	if (MPI_STATUS_IGNORE == status)
	{

		status = &own_status;
		own_status_used = 1;
	}

	CALL_REAL_MPI_FUNCTION_RET(data, ret, MPI_File_read_at_all, fh, offset, buf, count, datatype, status)

	pread_function_data.position = offset;

	if (ret != MPI_SUCCESS)
	{
		data.return_state = error;
		pread_function_data.read_bytes = 0;
		SET_MPI_ERROR(ret, MPI_STATUS_IGNORE)
	}
	else
	{
		data.return_state = ok;
		MPI_Get_count(status, datatype, &get_count);
		pread_function_data.read_bytes = datatype_size * get_count;
	}

	if (own_status_used)
	{
		status = MPI_STATUS_IGNORE;
	}

	WRAP_MPI_END(data)
	return ret;
}

int MPI_File_iread_at(MPI_File fh, MPI_Offset offset, void *buf, int count, MPI_Datatype datatype, MPI_Request *request)
{
	int ret;
	struct basic data;
	struct mpi_immediate_at mpi_immediate_at_data;
	struct file_mpi file_mpi_data;
	int datatype_size;

	WRAP_MPI_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, mpi_immediate_at, mpi_immediate_at_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_mpi, file_mpi_data)

	file_mpi_data.mpi_file = MPI_File_c2f(fh);

	CALL_REAL_MPI_FUNCTION_RET(data, ret, MPI_File_iread_at, fh, offset, buf, count, datatype, request)

	mpi_immediate_at_data.request_id = MPI_Request_c2f(*request);

	mpi_immediate_at_data.position = offset;

	if (ret != MPI_SUCCESS)
	{
		data.return_state = error;
		SET_MPI_ERROR(ret, MPI_STATUS_IGNORE)
	}
	else
	{
		data.return_state = ok;
	}

	WRAP_MPI_END(data)
	return ret;
}

int MPI_File_iread_at_all(MPI_File fh, MPI_Offset offset, void *buf, int count, MPI_Datatype datatype, MPI_Request *request)
{
	int ret;
	struct basic data;
	struct mpi_immediate_at mpi_immediate_at_data;
	struct file_mpi file_mpi_data;
	int datatype_size;

	WRAP_MPI_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, mpi_immediate_at, mpi_immediate_at_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_mpi, file_mpi_data)

	file_mpi_data.mpi_file = MPI_File_c2f(fh);

	CALL_REAL_MPI_FUNCTION_RET(data, ret, MPI_File_iread_at_all, fh, offset, buf, count, datatype, request)

	mpi_immediate_at_data.request_id = MPI_Request_c2f(*request);

	mpi_immediate_at_data.position = offset;

	if (ret != MPI_SUCCESS)
	{
		data.return_state = error;
		SET_MPI_ERROR(ret, MPI_STATUS_IGNORE)
	}
	else
	{
		data.return_state = ok;
	}

	WRAP_MPI_END(data)
	return ret;
}

int MPI_File_iwrite_at(MPI_File fh, MPI_Offset offset, const void *buf, int count, MPI_Datatype datatype, MPI_Request *request)
{
	int ret;
	struct basic data;
	struct mpi_immediate_at mpi_immediate_at_data;
	struct file_mpi file_mpi_data;
	int datatype_size;

	WRAP_MPI_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, mpi_immediate_at, mpi_immediate_at_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_mpi, file_mpi_data)

	file_mpi_data.mpi_file = MPI_File_c2f(fh);

	CALL_REAL_MPI_FUNCTION_RET(data, ret, MPI_File_iwrite_at, fh, offset, buf, count, datatype, request)

	mpi_immediate_at_data.request_id = MPI_Request_c2f(*request);

	mpi_immediate_at_data.position = offset;

	if (ret != MPI_SUCCESS)
	{
		data.return_state = error;
		SET_MPI_ERROR(ret, MPI_STATUS_IGNORE)
	}
	else
	{
		data.return_state = ok;
	}

	WRAP_MPI_END(data)
	return ret;
}

int MPI_File_iwrite_at_all(MPI_File fh, MPI_Offset offset, const void *buf, int count, MPI_Datatype datatype, MPI_Request *request)
{
	int ret;
	struct basic data;
	struct mpi_immediate_at mpi_immediate_at_data;
	struct file_mpi file_mpi_data;
	int datatype_size;

	WRAP_MPI_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, mpi_immediate_at, mpi_immediate_at_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_mpi, file_mpi_data)

	file_mpi_data.mpi_file = MPI_File_c2f(fh);

	CALL_REAL_MPI_FUNCTION_RET(data, ret, MPI_File_iwrite_at_all, fh, offset, buf, count, datatype, request)

	mpi_immediate_at_data.request_id = MPI_Request_c2f(*request);

	mpi_immediate_at_data.position = offset;

	if (ret != MPI_SUCCESS)
	{
		data.return_state = error;
		SET_MPI_ERROR(ret, MPI_STATUS_IGNORE)
	}
	else
	{
		data.return_state = ok;
	}

	WRAP_MPI_END(data)
	return ret;
}

int MPI_File_read_all_begin(MPI_File fh, void *buf, int count, MPI_Datatype datatype)
{
	int ret;
	struct basic data;
	struct file_mpi file_mpi_data;
	int datatype_size;

	WRAP_MPI_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P_NULL(data, function_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_mpi, file_mpi_data)

	file_mpi_data.mpi_file = MPI_File_c2f(fh);

	CALL_REAL_MPI_FUNCTION_RET(data, ret, MPI_File_read_all_begin, fh, buf, count, datatype)

	if (ret != MPI_SUCCESS)
	{
		data.return_state = error;
		SET_MPI_ERROR(ret, MPI_STATUS_IGNORE)
	}
	else
	{
		data.return_state = ok;
	}

	WRAP_MPI_END(data)
	return ret;
}

int MPI_Wait(MPI_Request *request, MPI_Status *status)
{

	int ret;
	struct basic data;
	struct request_mpi request_mpi_data;
	struct mpi_wait mpi_wait_data;

	MPI_Status own_status;
	char own_status_used = 0;
	int get_count;

	WRAP_MPI_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, mpi_wait, mpi_wait_data)

	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, request_mpi, request_mpi_data)

	request_mpi_data.request_id = MPI_Request_c2f(*request);

	if (MPI_STATUS_IGNORE == status)
	{

		status = &own_status;
		own_status_used = 1;
	}

	CALL_REAL_MPI_FUNCTION_RET(data, ret, MPI_Wait, request, status)

	if (ret != MPI_SUCCESS)
	{
		data.return_state = error;
		mpi_wait_data.count_bytes = 0;
		SET_MPI_ERROR(ret, status)
	}
	else
	{
		data.return_state = ok;
		MPI_Get_count(status, MPI_BYTE, &get_count);
		mpi_wait_data.count_bytes = get_count;
	}

	if (own_status_used)
	{
		status = MPI_STATUS_IGNORE;
	}

	WRAP_MPI_END(data)
	return ret;
}

int MPI_Waitall(int count, MPI_Request array_of_requests[], MPI_Status array_of_statuses[])
{

	int ret;
	struct basic data;
	struct mpi_waitall mpi_waitall_data;

	struct mpi_waitall_element mpi_waitall_element_data[MAX_MPI_IMESSAGES];
	struct mpi_waitall_element *mpi_waitall_element_data_pointer[MAX_MPI_IMESSAGES];

	struct errno_detail request_errno_data[MAX_MPI_IMESSAGES];

	MPI_Status own_status[MAX_MPI_IMESSAGES];
	char own_status_used = 0;
	int get_count;
	char tmp_errno_text[MAX_MPI_IMESSAGES][MPI_MAX_ERROR_STRING];
	int tmp_len_errno_string;

	WRAP_MPI_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, mpi_waitall, mpi_waitall_data)

	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P_NULL(data, file_type)

	if (MPI_STATUSES_IGNORE == array_of_statuses)
	{
		array_of_statuses = own_status;
		own_status_used = 1;
	}

	for (int i = 0; i < count; i++)
	{
		mpi_waitall_element_data[i].request_id = MPI_Request_c2f(array_of_requests[i]);
	}

	CALL_REAL_MPI_FUNCTION_RET(data, ret, MPI_Waitall, count, array_of_requests, array_of_statuses)

	if (ret != MPI_SUCCESS)
	{
		data.return_state = error;
		JSON_STRUCT_SET_STRUCT_ARRAY_NULL(mpi_waitall_data, requests)
		SET_MPI_ERROR(ret, MPI_STATUS_IGNORE)
	}
	else
	{
		data.return_state = ok;

		for (int i = 0; i < count; i++)
		{

			if (array_of_statuses[i].MPI_ERROR != MPI_SUCCESS)
			{
				mpi_waitall_element_data[i].return_state = error;
				mpi_waitall_element_data[i].count_bytes = 0;
				request_errno_data[i].errno_value = array_of_statuses[i].MPI_ERROR;
				MPI_Error_string(request_errno_data[i].errno_value, tmp_errno_text[i], &tmp_len_errno_string);
				request_errno_data[i].errno_text = tmp_errno_text[i];
				mpi_waitall_element_data[i].return_state_detail = &(request_errno_data[i]);
			}

			else
			{
				MPI_Get_count(&array_of_statuses[i], MPI_BYTE, &get_count);
				mpi_waitall_element_data[i].count_bytes = get_count;
				mpi_waitall_element_data[i].return_state_detail = NULL;
				mpi_waitall_element_data[i].return_state = ok;
			}

			mpi_waitall_element_data_pointer[i] = &(mpi_waitall_element_data[i]);
		}

		JSON_STRUCT_SET_STRUCT_ARRAY(mpi_waitall_data, requests, mpi_waitall_element_data_pointer,
									 count)
	}

	if (own_status_used)
	{
		array_of_statuses = MPI_STATUSES_IGNORE;
	}

	WRAP_MPI_END(data)
	return ret;
}

int MPI_File_delete(const char *filename, MPI_Info info)
{
	int ret;
	struct basic data;
	struct mpi_delete_function mpi_delete_data;

	WRAP_MPI_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, mpi_delete_function, mpi_delete_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	mpi_delete_data.file_name = filename;
	JSON_STRUCT_SET_VOID_P_NULL(data, file_type)

	CALL_REAL_MPI_FUNCTION_RET(data, ret, MPI_File_delete, filename, info)

	JSON_STRUCT_SET_KEY_VALUE_ARRAY_NULL(mpi_delete_data, file_hints)

	if (MPI_INFO_NULL != info)
	{
		int count_elements;
		int flag;
		char *keys[MAX_MPI_FILE_HINTS];
		char *values[MAX_MPI_FILE_HINTS];
		char key_strings[MAX_MPI_FILE_HINTS][MAX_MPI_FILE_HINT_LENGTH];
		char value_strings[MAX_MPI_FILE_HINTS][MAX_MPI_FILE_HINT_LENGTH];

		MPI_Info_get_nkeys(info, &count_elements);

		if (count_elements >= 1)
		{
			JSON_STRUCT_INIT_KEY_VALUE_ARRAY(mpi_delete_data, file_hints, keys,
											 values)

			for (int i = 0; i < count_elements && i < MAX_MPI_FILE_HINTS; i++)
			{
				MPI_Info_get_nthkey(info, i, key_strings[i]);
				MPI_Info_get(info, key_strings[i], MPI_MAX_INFO_VAL - 1,
							 value_strings[i], &flag);

				JSON_STRUCT_ADD_KEY_VALUE(mpi_delete_data, file_hints, key_strings[i],
										  value_strings[i])
			}
		}
	}

	if (ret != MPI_SUCCESS)
	{
		data.return_state = error;
		SET_MPI_ERROR(ret, MPI_STATUS_IGNORE)
		mpi_delete_data.id.device_id = 0;
		mpi_delete_data.id.inode_nr = 0;
	}
	else
	{
		data.return_state = ok;
		get_file_id_by_path(filename, &(mpi_delete_data.id));
	}

	WRAP_MPI_END(data)

	return ret;
}


int MPI_File_set_view(MPI_File fh, MPI_Offset disp, MPI_Datatype etype, MPI_Datatype filetype, const char *datarep, MPI_Info info){

	int ret;
	struct basic data;
	struct file_mpi file_mpi_data;
	struct positioning_function positioning_function_data;

	WRAP_MPI_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, positioning_function, positioning_function_data);
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_mpi, file_mpi_data)

	file_mpi_data.mpi_file = MPI_File_c2f(fh);

	CALL_REAL_MPI_FUNCTION_RET(data, ret, MPI_File_set_view, fh, disp, etype, filetype, datarep, info)

	positioning_function_data.offset = disp;
	positioning_function_data.relative_to = beginning_of_file;

	if (ret != MPI_SUCCESS)
	{
		data.return_state = error;
		SET_MPI_ERROR(ret, MPI_STATUS_IGNORE)
	}
	else
	{
		data.return_state = ok;
	}

	WRAP_MPI_END(data)
	return ret;



}
