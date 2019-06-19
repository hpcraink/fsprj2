/**
 * @file Implementation of Posix-IO functions.
 */
#include "libiotrace_config.h"
#ifdef HAVE_STDLIB_H
//#  include <stdlib.h>
#endif
#ifdef HAVE_UNISTD_H
#  include <unistd.h>
#endif

#include <assert.h>
#include <wchar.h>
#include <stdio.h>
#include <stdio_ext.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/mman.h>

#include <string.h>
#include <stdarg.h>
#include <time.h>
#include "json_include_struct.h"
#include "event.h"
#include "wrapper_defines.h"
#include "posix_io.h"

// ToDo: wrap open, socket, accept, connectx(mac) (for logging)
//       or use a function like lsof to get type of files

enum access_mode get_access_mode(int flags) {
	int access_mode = flags & O_ACCMODE;

	if (access_mode == O_RDONLY) {
		return read_only;
	} else if (access_mode == O_WRONLY) {
		return write_only;
	} else if (access_mode == O_RDWR) {
		return read_and_write;
	} else {
		return unknown_access_mode;
	}
}

enum lock_mode get_lock_mode(int type) {
	switch (type) {
	case FSETLOCKING_INTERNAL:
		return internal;
	case FSETLOCKING_BYCALLER:
		return bycaller;
	case FSETLOCKING_QUERY:
		return query_lock_mode;
	default:
		return unknown_lock_mode;
	}
}

enum lock_mode get_orientation_mode(int mode, char param) {
	if (mode > 0) {
		return wide;
	} else if (mode < 0) {
		return narrow;
	} else if (param) {
		return query_orientation_mode;
	} else {
		return not_set;
	}
}

enum buffer_mode get_buffer_mode(int mode) {
	switch (mode) {
	case _IOFBF:
		return fully_buffered;
	case _IOLBF:
		return line_buffered;
	case _IONBF:
		return unbuffered;
	default:
		return unknown_buffer_mode;
	}
}

enum read_write_state get_return_state_c(int ret) {
	if (ret == EOF) {
		return eof;
	} else {
		return ok;
	}
}

enum read_write_state get_return_state_wc(wint_t ret) {
	if (ret == WEOF) {
		return eof;
	} else {
		return ok;
	}
}

enum seek_where get_seek_where(int whence) {
	switch (whence) {
	case SEEK_SET:
		return beginning_of_file;
	case SEEK_CUR:
		return current_position;
	case SEEK_END:
		return end_of_file;
	default:
		return unknown_seek_where;
	}
}

enum madvice_advice get_madvice_advice(int advice) {
	switch (advice) {
	case MADV_NORMAL:
		return normal;
	case MADV_RANDOM:
		return madvice_random;
	case MADV_SEQUENTIAL:
		return sequential;
	case MADV_WILLNEED:
		return willneed;
	case MADV_DONTNEED:
		return dontneed;
#if MADV_REMOVE
	case MADV_REMOVE:
		return madvice_remove;
#endif
#if MADV_DONTFORK
	case MADV_DONTFORK:
		return dontfork;
#endif
#if MADV_DOFORK
	case MADV_DOFORK:
		return dofork;
#endif
#if MADV_HWPOISON
	case MADV_HWPOISON:
		return hwpoison;
#endif
#if MADV_MERGEABLE
	case MADV_MERGEABLE:
		return mergeable;
#endif
#if MADV_UNMERGEABLE
	case MADV_UNMERGEABLE:
		return unmergeable;
#endif
#if MADV_SOFT_OFFLINE
		case MADV_SOFT_OFFLINE:
		return soft_offline;
#endif
#if MADV_HUGEPAGE
	case MADV_HUGEPAGE:
		return hugepage;
#endif
#if MADV_NOHUGEPAGE
	case MADV_NOHUGEPAGE:
		return nohugepage;
#endif
#if MADV_DONTDUMP
	case MADV_DONTDUMP:
		return dontdump;
#endif
#if MADV_DODUMP
	case MADV_DODUMP:
		return dodump;
#endif
#if MADV_FREE
	case MADV_FREE:
		return madvice_free;
#endif
#if MADV_WIPEONFORK
	case MADV_WIPEONFORK:
		return wipeonfork;
#endif
#if MADV_KEEPONFORK
	case MADV_KEEPONFORK:
		return keeponfork;
#endif
	default:
		return unknown_madvice_advice;
	}
}

void get_creation_flags(const int flags, struct creation_flags *cf) {
#if HAVE_O_CLOEXEC
	cf->cloexec = flags & O_CLOEXEC ? 1 : 0;
#endif
#if HAVE_O_DIRECTORY
	cf->directory = flags & O_DIRECTORY ? 1 : 0;
#endif
#if HAVE_O_NOFOLLOW
	cf->nofollow = flags & O_NOFOLLOW ? 1 : 0;
#endif
#if HAVE_O_TMPFILE
	cf->tmpfile = flags & O_TMPFILE ? 1 : 0;
#endif
	cf->creat = flags & O_CREAT ? 1 : 0;
	cf->excl = flags & O_EXCL ? 1 : 0;
	cf->noctty = flags & O_NOCTTY ? 1 : 0;
	cf->trunc = flags & O_TRUNC ? 1 : 0;
}

void get_status_flags(const int flags, struct status_flags *sf) {
#if HAVE_O_DIRECT
	sf->direct = flags & O_DIRECT ? 1 : 0;
#endif
#if HAVE_O_NOATIME
	sf->noatime = flags & O_NOATIME ? 1 : 0;
#endif
#if HAVE_O_PATH
	sf->path = flags & O_PATH ? 1 : 0;
#endif
#if HAVE_O_LARGEFILE
	sf->largefile = flags & O_LARGEFILE ? 1 : 0;
#endif
	sf->append = flags & O_APPEND ? 1 : 0;
	sf->async = flags & O_ASYNC ? 1 : 0;
	sf->dsync = flags & O_DSYNC ? 1 : 0;
	sf->nonblock = flags & O_NONBLOCK ? 1 : 0;
	sf->ndelay = flags & O_NDELAY ? 1 : 0;
	sf->sync = flags & O_SYNC ? 1 : 0;
}

void get_rwf_flags(const int flags, struct rwf_flags *rf) {
	rf->hipri = flags & RWF_HIPRI ? 1 : 0;
	rf->dsync = flags & RWF_DSYNC ? 1 : 0;
	rf->sync = flags & RWF_SYNC ? 1 : 0;
	rf->nowait = flags & RWF_NOWAIT ? 1 : 0;
#ifdef HAVE_RWF_APPEND
	rf->append = flags & RWF_APPEND ? 1 : 0;
#endif
}

void get_mode_flags(mode_t mode, struct mode_flags *mf) {
	mf->read_by_owner = mode & S_IRUSR ? 1 : 0;
	mf->write_by_owner = mode & S_IWUSR ? 1 : 0;
	mf->execute_by_owner = mode & S_IXUSR ? 1 : 0;
	mf->read_by_group = mode & S_IRGRP ? 1 : 0;
	mf->write_by_group = mode & S_IWGRP ? 1 : 0;
	mf->execute_by_group = mode & S_IXGRP ? 1 : 0;
	mf->read_by_others = mode & S_IROTH ? 1 : 0;
	mf->write_by_others = mode & S_IWOTH ? 1 : 0;
	mf->execute_by_others = mode & S_IXOTH ? 1 : 0;
}

void get_memory_protection_flags(int protect,
		struct memory_protection_flags *mmf) {
	mmf->executed = protect & PROT_EXEC ? 1 : 0;
	mmf->read = protect & PROT_READ ? 1 : 0;
	mmf->written = protect & PROT_WRITE ? 1 : 0;
}

#if HAVE_MREMAP
void get_memory_remap_flags(int flags, struct memory_remap_flags *mrf) {
	mrf->maymove = flags & MREMAP_MAYMOVE ? 1 : 0;
	mrf->fixed = flags & MREMAP_FIXED ? 1 : 0;
}
#endif

void get_memory_map_flags(int flags, struct memory_map_flags *mpf) {
	mpf->shared = flags & MAP_SHARED ? 1 : 0;
	mpf->private = flags & MAP_PRIVATE ? 1 : 0;
	mpf->fixed = flags & MAP_FIXED ? 1 : 0;
#if HAVE_MAP_32BIT
	mpf->bit32 = flags & MAP_32BIT ? 1 : 0;
#endif
#if HAVE_MAP_ANONYMOUS
	mpf->anonymous = flags & MAP_ANONYMOUS ? 1 : 0;
#endif
#if HAVE_MAP_DENYWRITE
	mpf->denywrite = flags & MAP_DENYWRITE ? 1 : 0;
#endif
#if HAVE_MAP_EXECUTABLE
	mpf->executable = flags & MAP_EXECUTABLE ? 1 : 0;
#endif
#if HAVE_MAP_FILE
	mpf->file = flags & MAP_FILE ? 1 : 0;
#endif
#if HAVE_MAP_GROWSDOWN
	mpf->growsdown = flags & MAP_GROWSDOWN ? 1 : 0;
#endif
#if HAVE_MAP_HUGETLB
	mpf->hugetlb = flags & MAP_HUGETLB ? 1 : 0;
#endif
#if HAVE_MAP_HUGE_2MB
	mpf->huge_2mb = flags & MAP_HUGE_2MB ? 1 : 0;
#endif
#if HAVE_MAP_HUGE_1GB
	mpf->huge_1gb = flags & MAP_HUGE_1GB ? 1 : 0;
#endif
#if HAVE_MAP_LOCKED
	mpf->locked = flags & MAP_LOCKED ? 1 : 0;
#endif
#if HAVE_MAP_NONBLOCK
	mpf->nonblock = flags & MAP_NONBLOCK ? 1 : 0;
#endif
#if HAVE_MAP_NORESERVE
	mpf->noreserve = flags & MAP_NORESERVE ? 1 : 0;
#endif
#if HAVE_MAP_POPULATE
	mpf->populate = flags & MAP_POPULATE ? 1 : 0;
#endif
#if HAVE_MAP_STACK
	mpf->stack = flags & MAP_STACK ? 1 : 0;
#endif
#if HAVE_MAP_UNINITIALIZED
	mpf->uninitialized = flags & MAP_UNINITIALIZED ? 1 : 0;
#endif
}

void get_memory_sync_flags(int flags, struct memory_sync_flags *msf) {
	msf->sync = flags & MS_SYNC ? 1 : 0;
	msf->async = flags & MS_ASYNC ? 1 : 0;
	msf->invalidate = flags & MS_INVALIDATE ? 1 : 0;
}

enum access_mode check_mode(const char *mode, struct creation_flags *cf,
		struct status_flags *sf) {
#if HAVE_O_DIRECTORY
	cf->directory = 0;
#endif
#if HAVE_O_NOFOLLOW
	cf->nofollow = 0;
#endif
#if HAVE_O_TMPFILE
	cf->tmpfile = 0;
#endif
	cf->noctty = 0;
#if HAVE_O_DIRECT
	sf->direct = 0;
#endif
#if HAVE_O_NOATIME
	sf->noatime = 0;
#endif
#if HAVE_O_PATH
	sf->path = 0;
#endif
#if HAVE_O_LARGEFILE
	sf->largefile = 0;
#endif
	sf->async = 0;
	sf->dsync = 0;
	sf->nonblock = 0;
	sf->ndelay = 0;
	sf->sync = 0;

	// ToDo: c
	// ToDo: m
	// ToDo: ,ccs=<string>
	// ToDo: largefile from first write/read?
#if HAVE_O_CLOEXEC
	if (strchr(mode, 'e') != NULL) {
		cf->cloexec = 1;
	} else {
		cf->cloexec = 0;
	}
#endif

	if (strchr(mode, 'x') != NULL) {
		cf->excl = 1;
	} else {
		cf->excl = 0;
	}

	if (strchr(mode, 'r') != NULL) {
		cf->creat = 0;
		cf->trunc = 0;
		sf->append = 0;
		if (strchr(mode, '+') == NULL) {
			return read_only;
		} else {
			return read_and_write;
		}
	} else if (strchr(mode, 'w') != NULL) {
		cf->creat = 1;
		cf->trunc = 1;
		sf->append = 0;
		if (strchr(mode, '+') == NULL) {
			return write_only;
		} else {
			return read_and_write;
		}
	} else if (strchr(mode, 'a') != NULL) {
		cf->creat = 1;
		cf->trunc = 0;
		sf->append = 1;
		if (strchr(mode, '+') == NULL) {
			return write_only;
		} else {
			return read_and_write;
		}
	} else {
		cf->creat = 0;
		cf->trunc = 0;
		sf->append = 0;
		return unknown_access_mode;
	}
}

#ifdef HAVE_OPEN_ELLIPSES
int WRAP(open)(const char *filename, int flags, ...) {
#else
//int WRAP(open)(const char *filename, int flags) {
//	return WRAP(open)(filename, flags, 0); // ToDo: get default mode instead of 0
//}
	int WRAP(open)(const char *filename, int flags, mode_t mode) {
#endif
	int ret;
	struct basic data;
	struct file_descriptor file_descriptor_data;
	struct open_function open_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, open_function, open_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	open_data.file_name = filename;
	open_data.mode = get_access_mode(flags);
	get_creation_flags(flags, &open_data.creation);
	get_status_flags(flags, &open_data.status);

#ifdef HAVE_OPEN_ELLIPSES
	if (__OPEN_NEEDS_MODE(flags)) {
		va_list ap;
		mode_t mode;
		va_start(ap, flags);	//get_mode_flags
		mode = va_arg(ap, mode_t);
		va_end(ap);
		get_mode_flags(mode, &open_data.file_mode);
		CALL_REAL_FUNCTION_RET(data, ret, open, filename, flags, mode)
	} else {
		get_mode_flags(0, &open_data.file_mode);
		CALL_REAL_FUNCTION_RET(data, ret, open, filename, flags)
	}
#else
	// ToDo: mode_t mode = os_getmode();
	get_mode_flags(mode, &open_data.file_mode);
	CALL_REAL_FUNCTION_RET(data, ret, open, filename, flags, mode)
#endif

	if (-1 == ret) {
		data.return_state = error;
	} else {
		data.return_state = ok;
	}

	file_descriptor_data.descriptor = ret;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_descriptor, file_descriptor_data)

	WRAP_END(data)
	return ret;
}

#if HAVE_OPEN64
#ifdef HAVE_OPEN_ELLIPSES
int WRAP(open64)(const char *filename, int flags, ...) {
#else
	int WRAP(open64)(const char *filename, int flags, mode_t mode) {
#endif
	int ret;
	struct basic data;
	struct file_descriptor file_descriptor_data;
	struct open_function open_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, open_function, open_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	open_data.file_name = filename;
	open_data.mode = get_access_mode(flags);
	get_creation_flags(flags, &open_data.creation);
	get_status_flags(flags, &open_data.status);

#ifdef HAVE_OPEN_ELLIPSES
	if (__OPEN_NEEDS_MODE(flags)) {
		va_list ap;
		mode_t mode;
		va_start(ap, flags);	//get_mode_flags
		mode = va_arg(ap, mode_t);
		va_end(ap);
		get_mode_flags(mode, &open_data.file_mode);
		CALL_REAL_FUNCTION_RET(data, ret, open64, filename, flags, mode)
	} else {
		get_mode_flags(0, &open_data.file_mode);
		CALL_REAL_FUNCTION_RET(data, ret, open64, filename, flags)
	}
#else
	// ToDo: mode_t mode = os_getmode();
	get_mode_flags(mode, &open_data.file_mode);
	CALL_REAL_FUNCTION_RET(data, ret, open64, filename, flags, mode)
#endif

	if (-1 == ret) {
		data.return_state = error;
	} else {
		data.return_state = ok;
	}

	file_descriptor_data.descriptor = ret;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_descriptor, file_descriptor_data)

	WRAP_END(data)
	return ret;
}
#endif

#if HAVE_OPENAT
#ifdef HAVE_OPEN_ELLIPSES
int WRAP(openat)(int dirfd, const char *pathname, int flags, ...) {
#else
	int WRAP(openat)(int dirfd, const char *pathname, int flags, mode_t mode) {
#endif
	int ret;
	struct basic data;
	struct openat_function openat_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, openat_function, openat_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	openat_data.file_name = pathname;
	openat_data.mode = get_access_mode(flags);
	get_creation_flags(flags, &openat_data.creation);
	get_status_flags(flags, &openat_data.status);
	openat_data.file_descriptor = dirfd;
	if (AT_FDCWD == dirfd) {
		openat_data.relative_to = current_working_dir;
	} else {
		openat_data.relative_to = file;
	}

#ifdef HAVE_OPEN_ELLIPSES
	if (__OPEN_NEEDS_MODE(flags)) {
		va_list ap;
		mode_t mode;
		va_start(ap, flags);
		mode = va_arg(ap, mode_t);
		va_end(ap);
		get_mode_flags(mode, &openat_data.file_mode);
		CALL_REAL_FUNCTION_RET(data, ret, openat, dirfd, pathname, flags, mode)
	} else {
		get_mode_flags(0, &openat_data.file_mode);
		CALL_REAL_FUNCTION_RET(data, ret, openat, dirfd, pathname, flags)
	}
#else
	// ToDo: mode_t mode = os_getmode();
	get_mode_flags(mode, &open_data.file_mode);
	CALL_REAL_FUNCTION_RET(data, ret, open, filename, flags, mode)
#endif

	if (-1 == ret) {
		data.return_state = error;
	} else {
		data.return_state = ok;
	}

	JSON_STRUCT_SET_VOID_P(data, file_type, file_descriptor, ret)

	WRAP_END(data)
	return ret;
}
#endif

int WRAP(creat)(const char *filename, mode_t mode) {
	int ret;
	struct basic data;
	struct open_function open_data;
	int flags = O_CREAT | O_WRONLY | O_TRUNC;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, open_function, open_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	open_data.file_name = filename;
	open_data.mode = get_access_mode(flags);
	get_creation_flags(flags, &open_data.creation);
	get_status_flags(flags, &open_data.status);

	get_mode_flags(mode, &open_data.file_mode);
	CALL_REAL_FUNCTION_RET(data, ret, creat, filename, mode)

	if (-1 == ret) {
		data.return_state = error;
	} else {
		data.return_state = ok;
	}

	JSON_STRUCT_SET_VOID_P(data, file_type, file_descriptor, ret)

	WRAP_END(data)
	return ret;
}

#ifdef HAVE_CREAT64
int WRAP(creat64)(const char *filename, mode_t mode) {
	int ret;
	struct basic data;
	struct open_function open_data;
	int flags = O_CREAT | O_WRONLY | O_TRUNC;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, open_function, open_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	open_data.file_name = filename;
	open_data.mode = get_access_mode(flags);
	get_creation_flags(flags, &open_data.creation);
	get_status_flags(flags, &open_data.status);

	get_mode_flags(mode, &open_data.file_mode);
	CALL_REAL_FUNCTION_RET(data, ret, creat64, filename, mode)

	if (-1 == ret) {
		data.return_state = error;
	} else {
		data.return_state = ok;
	}

	JSON_STRUCT_SET_VOID_P(data, file_type, file_descriptor, ret)

	WRAP_END(data)
	return ret;
}
#endif

int WRAP(close)(int filedes) {
	int ret;
	struct basic data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P_NULL(data, function_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_descriptor, filedes)

	CALL_REAL_FUNCTION_RET(data, ret, close, filedes)

	if (0 == ret) {
		data.return_state = ok;
	} else {
		data.return_state = error;
	}

	WRAP_END(data)
	return ret;
}

ssize_t WRAP(read)(int filedes, void *buffer, size_t size) {
	ssize_t ret;
	struct basic data;
	struct read_function read_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, read_function, read_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_descriptor, filedes)

	CALL_REAL_FUNCTION_RET(data, ret, read, filedes, buffer, size)

	if (ret == -1) {
		data.return_state = error;
		read_data.read_bytes = 0;
	} else if (ret == 0 && size != 0) {
		data.return_state = eof;
		read_data.read_bytes = 0;
	} else {
		data.return_state = ok;
		read_data.read_bytes = ret;
	}

	WRAP_END(data)
	return ret;
}

#if HAVE_PREAD
ssize_t WRAP(pread)(int filedes, void *buffer, size_t size, off_t offset) {
	ssize_t ret;
	struct basic data;
	struct pread_function pread_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, pread_function, pread_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_descriptor, filedes)
	pread_data.position = offset;

	CALL_REAL_FUNCTION_RET(data, ret, pread, filedes, buffer, size, offset)

	if (ret == -1) {
		data.return_state = error;
		pread_data.read_bytes = 0;
	} else if (ret == 0 && size != 0) {
		data.return_state = eof;
		pread_data.read_bytes = 0;
	} else {
		data.return_state = ok;
		pread_data.read_bytes = ret;
	}

	WRAP_END(data)
	return ret;
}
#endif

#if HAVE_PREAD64
ssize_t WRAP(pread64)(int filedes, void *buffer, size_t size, off64_t offset) {
	ssize_t ret;
	struct basic data;
	struct pread_function pread_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, pread_function, pread_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_descriptor, filedes)
	pread_data.position = offset;

	CALL_REAL_FUNCTION_RET(data, ret, pread64, filedes, buffer, size, offset)

	if (ret == -1) {
		data.return_state = error;
		pread_data.read_bytes = 0;
	} else if (ret == 0 && size != 0) {
		data.return_state = eof;
		pread_data.read_bytes = 0;
	} else {
		data.return_state = ok;
		pread_data.read_bytes = ret;
	}

	WRAP_END(data)
	return ret;
}
#endif

ssize_t WRAP(write)(int filedes, const void *buffer, size_t size) {
	ssize_t ret;
	struct basic data;
	struct write_function write_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, write_function, write_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_descriptor, filedes)

	CALL_REAL_FUNCTION_RET(data, ret, write, filedes, buffer, size)

	if (-1 == ret) {
		data.return_state = error;
		write_data.written_bytes = 0;
	} else {
		data.return_state = ok;
		write_data.written_bytes = ret;
	}

	WRAP_END(data)
	return ret;
}

#if HAVE_PWRITE
ssize_t WRAP(pwrite)(int filedes, const void *buffer, size_t size, off_t offset) {
	ssize_t ret;
	struct basic data;
	struct pwrite_function pwrite_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, pwrite_function, pwrite_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_descriptor, filedes)
	pwrite_data.position = offset;

	CALL_REAL_FUNCTION_RET(data, ret, pwrite, filedes, buffer, size, offset)

	if (-1 == ret) {
		data.return_state = error;
		pwrite_data.written_bytes = 0;
	} else {
		data.return_state = ok;
		pwrite_data.written_bytes = ret;
	}

	WRAP_END(data)
	return ret;
}
#endif

#if HAVE_PWRITE64
ssize_t WRAP(pwrite64)(int filedes, const void *buffer, size_t size,
		off64_t offset) {
	ssize_t ret;
	struct basic data;
	struct pwrite_function pwrite_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, pwrite_function, pwrite_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_descriptor, filedes)
	pwrite_data.position = offset;

	CALL_REAL_FUNCTION_RET(data, ret, pwrite, filedes, buffer, size, offset)

	if (-1 == ret) {
		data.return_state = error;
		pwrite_data.written_bytes = 0;
	} else {
		data.return_state = ok;
		pwrite_data.written_bytes = ret;
	}

	WRAP_END(data)
	return ret;
}
#endif

off_t WRAP(lseek)(int filedes, off_t offset, int whence) {
	off_t ret;
	struct basic data;
	struct lpositioning_function lpositioning_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, lpositioning_function,
			lpositioning_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_descriptor, filedes)

	CALL_REAL_FUNCTION_RET(data, ret, lseek, filedes, offset, whence)

	//ToDo: check for SEEK_DATA and SEEK_HOLE (_GNU_SOURCE in <unistd.h>)
	if (-1 == ret) {
		data.return_state = error;
		lpositioning_data.offset = offset;
		lpositioning_data.relative_to = get_seek_where(whence);
		lpositioning_data.new_offset_relative_to_beginning_of_file = ret;
	} else {
		data.return_state = ok;
		lpositioning_data.offset = offset;
		lpositioning_data.relative_to = get_seek_where(whence);
		lpositioning_data.new_offset_relative_to_beginning_of_file = ret;
	}

	WRAP_END(data)
	return ret;
}

#if HAVE_LSEEK64
off64_t WRAP(lseek64)(int filedes, off64_t offset, int whence) {
	off64_t ret;
	struct basic data;
	struct lpositioning_function lpositioning_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, lpositioning_function,
			lpositioning_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_descriptor, filedes)

	CALL_REAL_FUNCTION_RET(data, ret, lseek64, filedes, offset, whence)

	if (-1 == ret) {
		data.return_state = error;
		lpositioning_data.offset = offset;
		lpositioning_data.relative_to = get_seek_where(whence);
		lpositioning_data.new_offset_relative_to_beginning_of_file = ret;
	} else {
		data.return_state = ok;
		lpositioning_data.offset = offset;
		lpositioning_data.relative_to = get_seek_where(whence);
		lpositioning_data.new_offset_relative_to_beginning_of_file = ret;
	}

	WRAP_END(data)
	return ret;
}
#endif

#if HAVE_READV
ssize_t WRAP(readv)(int filedes, const struct iovec *vector, int count) {
	ssize_t ret;
	struct basic data;
	struct read_function read_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, read_function, read_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_descriptor, filedes)

	CALL_REAL_FUNCTION_RET(data, ret, readv, filedes, vector, count)

	if (ret == -1) {
		data.return_state = error;
		read_data.read_bytes = 0;
	} else if (ret == 0 && count != 0) {
		data.return_state = eof;
		read_data.read_bytes = 0;
	} else {
		data.return_state = ok;
		read_data.read_bytes = ret;
	}

	WRAP_END(data)
	return ret;
}
#endif

#if HAVE_WRITEV
ssize_t WRAP(writev)(int filedes, const struct iovec *vector, int count) {
	ssize_t ret;
	struct basic data;
	struct write_function write_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, write_function, write_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_descriptor, filedes)

	CALL_REAL_FUNCTION_RET(data, ret, writev, filedes, vector, count)

	if (-1 == ret) {
		data.return_state = error;
		write_data.written_bytes = 0;
	} else {
		data.return_state = ok;
		write_data.written_bytes = ret;
	}

	WRAP_END(data)
	return ret;
}
#endif

#if HAVE_PREADV
ssize_t WRAP(preadv)(int fd, const struct iovec *iov, int iovcnt, off_t offset) {
	ssize_t ret;
	struct basic data;
	struct pread_function pread_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, pread_function, pread_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_descriptor, fd)
	pread_data.position = offset;

	CALL_REAL_FUNCTION_RET(data, ret, preadv, fd, iov, iovcnt, offset)

	if (ret == -1) {
		data.return_state = error;
		pread_data.read_bytes = 0;
	} else if (ret == 0 && iovcnt != 0) {
		data.return_state = eof;
		pread_data.read_bytes = 0;
	} else {
		data.return_state = ok;
		pread_data.read_bytes = ret;
	}

	WRAP_END(data)
	return ret;
}
#endif

#if HAVE_PREADV64
ssize_t WRAP(preadv64)(int fd, const struct iovec *iov, int iovcnt,
		off64_t offset) {
	ssize_t ret;
	struct basic data;
	struct pread_function pread_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, pread_function, pread_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_descriptor, fd)
	pread_data.position = offset;

	CALL_REAL_FUNCTION_RET(data, ret, preadv64, fd, iov, iovcnt, offset)

	if (ret == -1) {
		data.return_state = error;
		pread_data.read_bytes = 0;
	} else if (ret == 0 && iovcnt != 0) {
		data.return_state = eof;
		pread_data.read_bytes = 0;
	} else {
		data.return_state = ok;
		pread_data.read_bytes = ret;
	}

	WRAP_END(data)
	return ret;
}
#endif

#if HAVE_PWRITEV
ssize_t WRAP(pwritev)(int fd, const struct iovec *iov, int iovcnt, off_t offset) {
	ssize_t ret;
	struct basic data;
	struct pwrite_function pwrite_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, pwrite_function, pwrite_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_descriptor, fd)
	pwrite_data.position = offset;

	CALL_REAL_FUNCTION_RET(data, ret, pwritev, fd, iov, iovcnt, offset)

	if (-1 == ret) {
		data.return_state = error;
		pwrite_data.written_bytes = 0;
	} else {
		data.return_state = ok;
		pwrite_data.written_bytes = ret;
	}

	WRAP_END(data)
	return ret;
}
#endif

#if HAVE_PWRITEV64
ssize_t WRAP(pwritev64)(int fd, const struct iovec *iov, int iovcnt,
		off64_t offset) {
	ssize_t ret;
	struct basic data;
	struct pwrite_function pwrite_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, pwrite_function, pwrite_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_descriptor, fd)
	pwrite_data.position = offset;

	CALL_REAL_FUNCTION_RET(data, ret, pwritev64, fd, iov, iovcnt, offset)

	if (-1 == ret) {
		data.return_state = error;
		pwrite_data.written_bytes = 0;
	} else {
		data.return_state = ok;
		pwrite_data.written_bytes = ret;
	}

	WRAP_END(data)
	return ret;
}
#endif

#if HAVE_PREADV2
ssize_t WRAP(preadv2)(int fd, const struct iovec *iov, int iovcnt, off_t offset,
		int flags) {
	ssize_t ret;
	struct basic data;
	struct pread2_function pread2_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, pread2_function, pread2_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_descriptor, fd)
	pread2_data.position = offset;
	get_rwf_flags(flags, &pread2_data.flags);

	CALL_REAL_FUNCTION_RET(data, ret, preadv2, fd, iov, iovcnt, offset, flags)

	if (ret == -1) {
		data.return_state = error;
		pread2_data.read_bytes = 0;
	} else if (ret == 0 && iovcnt != 0) {
		data.return_state = eof;
		pread2_data.read_bytes = 0;
	} else {
		data.return_state = ok;
		pread2_data.read_bytes = ret;
	}

	WRAP_END(data)
	return ret;
}
#endif

#if HAVE_PREADV64V2
ssize_t WRAP(preadv64v2)(int fd, const struct iovec *iov, int iovcnt,
		off64_t offset, int flags) {
	ssize_t ret;
	struct basic data;
	struct pread2_function pread2_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, pread2_function, pread2_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_descriptor, fd)
	pread2_data.position = offset;
	get_rwf_flags(flags, &pread2_data.flags);

	CALL_REAL_FUNCTION_RET(data, ret, preadv64v2, fd, iov, iovcnt, offset,
			flags)

	if (ret == -1) {
		data.return_state = error;
		pread2_data.read_bytes = 0;
	} else if (ret == 0 && iovcnt != 0) {
		data.return_state = eof;
		pread2_data.read_bytes = 0;
	} else {
		data.return_state = ok;
		pread2_data.read_bytes = ret;
	}

	WRAP_END(data)
	return ret;
}
#endif

#if HAVE_PWRITEV2
ssize_t WRAP(pwritev2)(int fd, const struct iovec *iov, int iovcnt,
		off_t offset, int flags) {
	ssize_t ret;
	struct basic data;
	struct pwrite2_function pwrite2_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, pwrite2_function, pwrite2_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_descriptor, fd)
	pwrite2_data.position = offset;
	get_rwf_flags(flags, &pwrite2_data.flags);

	CALL_REAL_FUNCTION_RET(data, ret, pwritev2, fd, iov, iovcnt, offset, flags)

	if (-1 == ret) {
		data.return_state = error;
		pwrite2_data.written_bytes = 0;
	} else {
		data.return_state = ok;
		pwrite2_data.written_bytes = ret;
	}

	WRAP_END(data)
	return ret;
}
#endif

#if HAVE_PWRITEV64V2
ssize_t WRAP(pwritev64v2)(int fd, const struct iovec *iov, int iovcnt,
		off64_t offset, int flags) {
	ssize_t ret;
	struct basic data;
	struct pwrite2_function pwrite2_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, pwrite2_function, pwrite2_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_descriptor, fd)
	pwrite2_data.position = offset;
	get_rwf_flags(flags, &pwrite2_data.flags);

	CALL_REAL_FUNCTION_RET(data, ret, pwritev64v2, fd, iov, iovcnt, offset,
			flags)

	if (-1 == ret) {
		data.return_state = error;
		pwrite2_data.written_bytes = 0;
	} else {
		data.return_state = ok;
		pwrite2_data.written_bytes = ret;
	}

	WRAP_END(data)
	return ret;
}
#endif

#if HAVE_COPY_FILE_RANGE
ssize_t WRAP(copy_file_range)(int inputfd, off64_t *inputpos, int outputfd,
		off64_t *outputpos, size_t length, unsigned int flags) {
	ssize_t ret;
	struct basic data;
	struct copy_read_function copy_read_data;
	struct copy_write_function copy_write_data;
	WRAP_START(data)

	get_basic(&data);
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	if (NULL != inputpos) {
		copy_read_data.relative_to = beginning_of_file;
		copy_read_data.position = *inputpos;
	} else {
		copy_read_data.relative_to = current_position;
		copy_read_data.position = 0;
	}
	if (NULL != outputpos) {
		copy_write_data.relative_to = beginning_of_file;
		copy_write_data.position = *outputpos;
	} else {
		copy_write_data.relative_to = current_position;
		copy_write_data.position = 0;
	}
	copy_read_data.to_file_descriptor = outputfd;
	copy_write_data.from_file_descriptor = inputfd;

	CALL_REAL_FUNCTION_RET(data, ret, copy_file_range, inputfd, inputpos,
			outputfd, outputpos, length, flags)

	if (ret == -1) {
		data.return_state = error;
		copy_read_data.read_bytes = 0;
		copy_write_data.written_bytes = 0;
	} else if (ret == 0 && length != 0) {
		data.return_state = eof;
		copy_read_data.read_bytes = 0;
		copy_write_data.written_bytes = 0;
	} else {
		data.return_state = ok;
		copy_read_data.read_bytes = ret;
		copy_write_data.written_bytes = ret;
	}

	JSON_STRUCT_SET_VOID_P(data, file_type, file_descriptor, inputfd)
	JSON_STRUCT_SET_VOID_P(data, function_data, copy_read_function,
			copy_read_data)
	WRAP_END(data)
	JSON_STRUCT_SET_VOID_P(data, file_type, file_descriptor, outputfd)
	JSON_STRUCT_SET_VOID_P(data, function_data, copy_write_function,
			copy_write_data)
	WRAP_END(data)
	return ret;
}
#endif

#if HAVE_MMAP
void * WRAP(mmap)(void *address, size_t length, int protect, int flags,
		int filedes, off_t offset) {
	void *ret;
	struct basic data;
	struct memory_map_function memory_map_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, memory_map_function,
			memory_map_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_descriptor, filedes)
	get_memory_protection_flags(protect, &memory_map_data.protection_flags);
	get_memory_map_flags(flags, &memory_map_data.map_flags);
	memory_map_data.offset = offset;
	memory_map_data.length = length;

	CALL_REAL_FUNCTION_RET(data, ret, mmap, address, length, protect, flags,
			filedes, offset)

	if (MAP_FAILED == ret) {
		data.return_state = error;
	} else {
		data.return_state = ok;
	}
	memory_map_data.address = ret;

	WRAP_END(data)
	return ret;
}
#endif

#if HAVE_MMAP64
void * WRAP(mmap64)(void *address, size_t length, int protect, int flags,
		int filedes, off64_t offset) {
	void *ret;
	struct basic data;
	struct memory_map_function memory_map_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, memory_map_function,
			memory_map_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_descriptor, filedes)
	get_memory_protection_flags(protect, &memory_map_data.protection_flags);
	get_memory_map_flags(flags, &memory_map_data.map_flags);
	memory_map_data.offset = offset;
	memory_map_data.length = length;

	CALL_REAL_FUNCTION_RET(data, ret, mmap64, address, length, protect, flags,
			filedes, offset)

	if (MAP_FAILED == ret) {
		data.return_state = error;
	} else {
		data.return_state = ok;
	}
	memory_map_data.address = ret;

	WRAP_END(data)
	return ret;
}
#endif

#if HAVE_MUNMAP
int WRAP(munmap)(void *addr, size_t length) {
	int ret;
	struct basic data;
	struct file_memory file_memory_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P_NULL(data, function_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_memory, file_memory_data)
	file_memory_data.length = length;
	file_memory_data.address = addr;

	CALL_REAL_FUNCTION_RET(data, ret, munmap, addr, length)

	if (-1 == ret) {
		data.return_state = error;
	} else {
		data.return_state = ok;
	}

	WRAP_END(data)
	return ret;
}
#endif

#if HAVE_MSYNC
int WRAP(msync)(void *address, size_t length, int flags) {
	int ret;
	struct basic data;
	struct file_memory file_memory_data;
	struct memory_sync_function memory_sync_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, memory_sync_function,
			memory_sync_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_memory, file_memory_data)
	get_memory_sync_flags(flags, &memory_sync_data.sync_flags);
	file_memory_data.length = length;
	file_memory_data.address = address;

	CALL_REAL_FUNCTION_RET(data, ret, msync, address, length, flags)

	if (-1 == ret) {
		data.return_state = error;
	} else {
		data.return_state = ok;
	}

	WRAP_END(data)
	return ret;
}
#endif

#if HAVE_MREMAP
void * WRAP(mremap)(void *old_address, size_t old_length, size_t new_length,
		int flags, ...) {
	void *ret;
	struct basic data;
	struct file_memory file_memory_data;
	struct memory_remap_function memory_remap_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, memory_remap_function,
			memory_remap_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_memory, file_memory_data)
	get_memory_remap_flags(flags, &memory_remap_data.remap_flags);
	file_memory_data.length = old_length;
	file_memory_data.address = old_address;
	memory_remap_data.new_length = new_length;

	if (memory_remap_data.remap_flags.maymove
			&& memory_remap_data.remap_flags.fixed) {
		va_list ap;
		void *new_address;
		va_start(ap, flags);
		new_address = va_arg(ap, void *);
		va_end(ap);
		CALL_REAL_FUNCTION_RET(data, ret, mremap, old_address, old_length,
				new_length, flags, new_address)
	} else {
		CALL_REAL_FUNCTION_RET(data, ret, mremap, old_address, old_length,
				new_length, flags)
	}

	if (MAP_FAILED == ret) {
		data.return_state = error;
	} else {
		memory_remap_data.new_address = ret;
		data.return_state = ok;
	}

	WRAP_END(data)
	return ret;
}
#endif

#if HAVE_MADVISE
int WRAP(madvise)(void *addr, size_t length, int advice) {
	int ret;
	struct basic data;
	struct file_memory file_memory_data;
	struct memory_madvise_function memory_madvise_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, memory_madvise_function,
			memory_madvise_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_memory, file_memory_data)
	memory_madvise_data.advice = get_madvice_advice(advice);
	file_memory_data.length = length;
	file_memory_data.address = addr;

	CALL_REAL_FUNCTION_RET(data, ret, madvise, addr, length, advice)

	if (-1 == ret) {
		data.return_state = error;
	} else {
		data.return_state = ok;
	}

	WRAP_END(data)
	return ret;
}
#endif

FILE * WRAP(fopen)(const char *filename, const char *opentype) {
	FILE * file;
	struct basic data;
	struct open_function open_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, open_function, open_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	open_data.file_name = filename;
	open_data.mode = check_mode(opentype, &open_data.creation,
			&open_data.status);
	get_mode_flags(S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH,
			&open_data.file_mode);

	CALL_REAL_FUNCTION_RET(data, file, fopen, filename, opentype)

	if (NULL == file) {
		data.return_state = error;
	} else {
		data.return_state = ok;
	}

	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, file)

	WRAP_END(data)
	return file;
}

#if HAVE_FOPEN64
FILE * WRAP(fopen64)(const char *filename, const char *opentype) {
	FILE * file;
	struct basic data;
	struct open_function open_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, open_function, open_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	open_data.file_name = filename;
	open_data.mode = check_mode(opentype, &open_data.creation,
			&open_data.status);
	get_mode_flags(S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH,
			&open_data.file_mode);

	CALL_REAL_FUNCTION_RET(data, file, fopen64, filename, opentype)

	if (NULL == file) {
		data.return_state = error;
	} else {
		data.return_state = ok;
	}

	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, file)

	WRAP_END(data)
	return file;
}
#endif

FILE * WRAP(freopen)(const char *filename, const char *opentype, FILE *stream) {
	FILE * file;
	struct basic data;
	struct open_function open_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, open_function, open_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	open_data.file_name = filename;
	open_data.mode = check_mode(opentype, &open_data.creation,
			&open_data.status);
	get_mode_flags(S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH,
			&open_data.file_mode);

	CALL_REAL_FUNCTION_RET(data, file, freopen, filename, opentype, stream)

	if (NULL == file) {
		data.return_state = error;
	} else {
		data.return_state = ok;
	}

	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, file)

	WRAP_END(data)
	return file;
}

#if HAVE_FREOPEN64
FILE * WRAP(freopen64)(const char *filename, const char *opentype, FILE *stream) {
	FILE * file;
	struct basic data;
	struct open_function open_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, open_function, open_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	open_data.file_name = filename;
	open_data.mode = check_mode(opentype, &open_data.creation,
			&open_data.status);
	get_mode_flags(S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH,
			&open_data.file_mode);

	CALL_REAL_FUNCTION_RET(data, file, freopen64, filename, opentype, stream)

	if (NULL == file) {
		data.return_state = error;
	} else {
		data.return_state = ok;
	}

	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, file)

	WRAP_END(data)
	return file;
}
#endif

#if HAVE_FDOPEN
FILE * WRAP(fdopen)(int fd, const char *opentype) {
	FILE * file;
	struct basic data;
	struct fdopen_function fdopen_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, fdopen_function, fdopen_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	fdopen_data.descriptor = fd;
	fdopen_data.mode = check_mode(opentype, &fdopen_data.creation,
			&fdopen_data.status);

	CALL_REAL_FUNCTION_RET(data, file, fdopen, fd, opentype)

	if (NULL == file) {
		data.return_state = error;
	} else {
		data.return_state = ok;
	}

	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, file)

	WRAP_END(data)
	return file;
}
#endif

int WRAP(fclose)(FILE *stream) {
	int ret;
	struct basic data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P_NULL(data, function_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	CALL_REAL_FUNCTION_RET(data, ret, fclose, stream)

	if (0 == ret) {
		data.return_state = ok;
	} else {
		data.return_state = error;
	}

	WRAP_END(data)
	return ret;
}

#if HAVE_FCLOSEALL
int WRAP(fcloseall)(void) {
	int ret;
	struct basic data;
	FILE *file = NULL;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P_NULL(data, function_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, file)

	CALL_REAL_FUNCTION_RET(data, ret, fcloseall)

	if (0 == ret) {
		data.return_state = ok;
	} else {
		data.return_state = error;
	}

	WRAP_END(data)
	return ret;
}
#endif

#if HAVE_FLOCKFILE
void WRAP(flockfile)(FILE *stream) {
	struct basic data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P_NULL(data, function_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	CALL_REAL_FUNCTION(data, flockfile, stream)

	data.return_state = ok;

	WRAP_END(data)
	return;
}
#endif

#if HAVE_FTRYLOCKFILE
int WRAP(ftrylockfile)(FILE *stream) {
	int ret;
	struct basic data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P_NULL(data, function_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	CALL_REAL_FUNCTION_RET(data, ret, ftrylockfile, stream)

	if (0 == ret) {
		data.return_state = ok;
	} else {
		data.return_state = error;
	}

	WRAP_END(data)
	return ret;
}
#endif

#if HAVE_FUNLOCKFILE
void WRAP(funlockfile)(FILE *stream) {
	struct basic data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P_NULL(data, function_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	CALL_REAL_FUNCTION(data, funlockfile, stream)

	data.return_state = ok;

	WRAP_END(data)
	return;
}
#endif

#if HAVE_FWIDE
int WRAP(fwide)(FILE *stream, int mode) {
	int ret;
	struct basic data;
	struct orientation_mode_function orientation_mode_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, orientation_mode_function,
			orientation_mode_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	orientation_mode_data.set_mode = get_orientation_mode(mode, 1);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	CALL_REAL_FUNCTION_RET(data, ret, fwide, stream, mode)

	data.return_state = ok;
	orientation_mode_data.return_mode = get_orientation_mode(ret, 0);

	WRAP_END(data)
	return ret;
}
#endif

int WRAP(fputc)(int c, FILE *stream) {
	int ret;
	struct basic data;
	struct write_function write_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, write_function, write_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	CALL_REAL_FUNCTION_RET(data, ret, fputc, c, stream)

	data.return_state = get_return_state_c(ret);
	if (data.return_state == ok) {
		write_data.written_bytes = 1;
	} else {
		write_data.written_bytes = 0;
	}

	WRAP_END(data)
	return ret;
}

wint_t WRAP(fputwc)(wchar_t wc, FILE *stream) {
	wint_t ret;
	struct basic data;
	struct write_function write_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, write_function, write_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	CALL_REAL_FUNCTION_RET(data, ret, fputwc, wc, stream)

	data.return_state = get_return_state_wc(ret);
	if (data.return_state == ok) {
		write_data.written_bytes = sizeof(wchar_t);
	} else {
		write_data.written_bytes = 0;
	}

	WRAP_END(data)
	return ret;
}

#if HAVE_FPUTC_UNLOCKED
int WRAP(fputc_unlocked)(int c, FILE *stream) {
	int ret;
	struct basic data;
	struct write_function write_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, write_function, write_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	CALL_REAL_FUNCTION_RET(data, ret, fputc_unlocked, c, stream)

	data.return_state = get_return_state_c(ret);
	if (data.return_state == ok) {
		write_data.written_bytes = 1;
	} else {
		write_data.written_bytes = 0;
	}

	WRAP_END(data)
	return ret;
}
#endif

#if HAVE_FPUTWC_UNLOCKED
wint_t WRAP(fputwc_unlocked)(wchar_t wc, FILE *stream) {
	wint_t ret;
	struct basic data;
	struct write_function write_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, write_function, write_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	CALL_REAL_FUNCTION_RET(data, ret, fputwc_unlocked, wc, stream)

	data.return_state = get_return_state_wc(ret);
	if (data.return_state == ok) {
		write_data.written_bytes = sizeof(wchar_t);
	} else {
		write_data.written_bytes = 0;
	}

	WRAP_END(data)
	return ret;
}
#endif

int WRAP(putc_MACRO)(int c, FILE *stream) {
	int ret;
	struct basic data;
	struct write_function write_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, write_function, write_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	CALL_REAL_FUNCTION_RET(data, ret, putc_MACRO, c, stream)

	data.return_state = get_return_state_c(ret);
	if (data.return_state == ok) {
		write_data.written_bytes = 1;
	} else {
		write_data.written_bytes = 0;
	}

	WRAP_END(data)
	return ret;
}

wint_t WRAP(putwc_MACRO)(wchar_t wc, FILE *stream) {
	wint_t ret;
	struct basic data;
	struct write_function write_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, write_function, write_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	CALL_REAL_FUNCTION_RET(data, ret, putwc_MACRO, wc, stream)

	data.return_state = get_return_state_wc(ret);
	if (data.return_state == ok) {
		write_data.written_bytes = sizeof(wchar_t);
	} else {
		write_data.written_bytes = 0;
	}

	WRAP_END(data)
	return ret;
}

#if HAVE_PUTC_UNLOCKED
int WRAP(putc_unlocked_MACRO)(int c, FILE *stream) {
	int ret;
	struct basic data;
	struct write_function write_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, write_function, write_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	CALL_REAL_FUNCTION_RET(data, ret, putc_unlocked_MACRO, c, stream)

	data.return_state = get_return_state_c(ret);
	if (data.return_state == ok) {
		write_data.written_bytes = 1;
	} else {
		write_data.written_bytes = 0;
	}

	WRAP_END(data)
	return ret;
}
#endif

#if HAVE_PUTWC_UNLOCKED
wint_t WRAP(putwc_unlocked_MACRO)(wchar_t wc, FILE *stream) {
	wint_t ret;
	struct basic data;
	struct write_function write_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, write_function, write_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	CALL_REAL_FUNCTION_RET(data, ret, putwc_unlocked_MACRO, wc, stream)

	data.return_state = get_return_state_wc(ret);
	if (data.return_state == ok) {
		write_data.written_bytes = sizeof(wchar_t);
	} else {
		write_data.written_bytes = 0;
	}

	WRAP_END(data)
	return ret;
}
#endif

int WRAP(fputs)(const char *s, FILE *stream) {
	int ret;
	struct basic data;
	struct write_function write_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, write_function, write_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	CALL_REAL_FUNCTION_RET(data, ret, fputs, s, stream)

	data.return_state = get_return_state_c(ret);
	if (data.return_state == ok) {
		write_data.written_bytes = strlen(s);
	} else {
		write_data.written_bytes = 0;
	}

	WRAP_END(data)
	return ret;
}

int WRAP(fputws)(const wchar_t *ws, FILE *stream) {
	int ret;
	struct basic data;
	struct write_function write_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, write_function, write_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	CALL_REAL_FUNCTION_RET(data, ret, fputws, ws, stream)

	// ToDo: wchar.h says WEOF is error, man pages says -1 is error, what if WEOF isn't -1 ??? ???
	data.return_state = get_return_state_wc(ret);
	if (data.return_state == ok) {
		write_data.written_bytes = wcslen(ws) * sizeof(wchar_t);
	} else {
		write_data.written_bytes = 0;
	}

	WRAP_END(data)
	return ret;
}

#if HAVE_FPUTS_UNLOCKED
int WRAP(fputs_unlocked)(const char *s, FILE *stream) {
	int ret;
	struct basic data;
	struct write_function write_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, write_function, write_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	CALL_REAL_FUNCTION_RET(data, ret, fputs_unlocked, s, stream)

	data.return_state = get_return_state_c(ret);
	if (data.return_state == ok) {
		write_data.written_bytes = strlen(s);
	} else {
		write_data.written_bytes = 0;
	}

	WRAP_END(data)
	return ret;
}
#endif

#if HAVE_FPUTWS_UNLOCKED
int WRAP(fputws_unlocked)(const wchar_t *ws, FILE *stream) {
	int ret;
	struct basic data;
	struct write_function write_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, write_function, write_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	CALL_REAL_FUNCTION_RET(data, ret, fputws_unlocked, ws, stream)

	// ToDo: wchar.h says WEOF is error, man pages says -1 is error, what if WEOF isn't -1 ???
	data.return_state = get_return_state_wc(ret);
	if (data.return_state == ok) {
		write_data.written_bytes = wcslen(ws) * sizeof(wchar_t);
	} else {
		write_data.written_bytes = 0;
	}

	WRAP_END(data)
	return ret;
}
#endif

#if HAVE_PUTW
int WRAP(putw)(int w, FILE *stream) {
	int ret;
	struct basic data;
	struct write_function write_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, write_function, write_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	CALL_REAL_FUNCTION_RET(data, ret, putw, w, stream)

	// ToDo: behavior as described in man pages because header file says nothing about errors
	data.return_state = get_return_state_c(ret);
	if (data.return_state == ok) {
		write_data.written_bytes = sizeof(int);
	} else {
		write_data.written_bytes = 0;
	}

	WRAP_END(data)
	return ret;
}
#endif

int WRAP(fgetc)(FILE *stream) {
	int ret;
	struct basic data;
	struct read_function read_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, read_function, read_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	CALL_REAL_FUNCTION_RET(data, ret, fgetc, stream)

	data.return_state = get_return_state_c(ret);
	if (data.return_state == ok) {
		read_data.read_bytes = 1;
	} else {
		read_data.read_bytes = 0;
	}

	WRAP_END(data)
	return ret;
}

wint_t WRAP(fgetwc)(FILE *stream) {
	wint_t ret;
	struct basic data;
	struct read_function read_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, read_function, read_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	CALL_REAL_FUNCTION_RET(data, ret, fgetwc, stream)

	data.return_state = get_return_state_wc(ret);
	if (data.return_state == ok) {
		read_data.read_bytes = sizeof(wchar_t);
	} else {
		read_data.read_bytes = 0;
	}

	WRAP_END(data)
	return ret;
}

#if HAVE_FGETC_UNLOCKED
int WRAP(fgetc_unlocked)(FILE *stream) {
	int ret;
	struct basic data;
	struct read_function read_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, read_function, read_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	CALL_REAL_FUNCTION_RET(data, ret, fgetc_unlocked, stream)

	data.return_state = get_return_state_c(ret);
	if (data.return_state == ok) {
		read_data.read_bytes = 1;
	} else {
		read_data.read_bytes = 0;
	}

	WRAP_END(data)
	return ret;
}
#endif

#if HAVE_FGETWC_UNLOCKED
wint_t WRAP(fgetwc_unlocked)(FILE *stream) {
	wint_t ret;
	struct basic data;
	struct read_function read_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, read_function, read_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	CALL_REAL_FUNCTION_RET(data, ret, fgetwc_unlocked, stream)

	data.return_state = get_return_state_wc(ret);
	if (data.return_state == ok) {
		read_data.read_bytes = sizeof(wchar_t);
	} else {
		read_data.read_bytes = 0;
	}

	WRAP_END(data)
	return ret;
}
#endif

int WRAP(getc_MACRO)(FILE *stream) {
	int ret;
	struct basic data;
	struct read_function read_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, read_function, read_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	CALL_REAL_FUNCTION_RET(data, ret, getc_MACRO, stream)

	data.return_state = get_return_state_c(ret);
	if (data.return_state == ok) {
		read_data.read_bytes = 1;
	} else {
		read_data.read_bytes = 0;
	}

	WRAP_END(data)
	return ret;
}

wint_t WRAP(getwc_MACRO)(FILE *stream) {
	wint_t ret;
	struct basic data;
	struct read_function read_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, read_function, read_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	CALL_REAL_FUNCTION_RET(data, ret, getwc_MACRO, stream)

	data.return_state = get_return_state_wc(ret);
	if (data.return_state == ok) {
		read_data.read_bytes = sizeof(wchar_t);
	} else {
		read_data.read_bytes = 0;
	}

	WRAP_END(data)
	return ret;
}

#if HAVE_GETC_UNLOCKED
int WRAP(getc_unlocked_MACRO)(FILE *stream) {
	int ret;
	struct basic data;
	struct read_function read_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, read_function, read_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	CALL_REAL_FUNCTION_RET(data, ret, getc_unlocked_MACRO, stream)

	data.return_state = get_return_state_c(ret);
	if (data.return_state == ok) {
		read_data.read_bytes = 1;
	} else {
		read_data.read_bytes = 0;
	}

	WRAP_END(data)
	return ret;
}
#endif

#if HAVE_GETWC_UNLOCKED
wint_t WRAP(getwc_unlocked_MACRO)(FILE *stream) {
	wint_t ret;
	struct basic data;
	struct read_function read_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, read_function, read_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	CALL_REAL_FUNCTION_RET(data, ret, getwc_unlocked_MACRO, stream)

	data.return_state = get_return_state_wc(ret);
	if (data.return_state == ok) {
		read_data.read_bytes = sizeof(wchar_t);
	} else {
		read_data.read_bytes = 0;
	}

	WRAP_END(data)
	return ret;
}
#endif

#if HAVE_GETW
int WRAP(getw)(FILE *stream) {
	int ret;
	struct basic data;
	struct read_function read_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, read_function, read_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	CALL_REAL_FUNCTION_RET(data, ret, getw, stream)

	data.return_state = get_return_state_c(ret);
	if (data.return_state == ok) {
		read_data.read_bytes = sizeof(int);
	} else {
		read_data.read_bytes = 0;
	}

	WRAP_END(data)
	return ret;
}
#endif

#if HAVE_GETLINE
ssize_t WRAP(getline)(char **lineptr, size_t *n, FILE *stream) {
	ssize_t ret;
	struct basic data;
	struct read_function read_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, read_function, read_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	CALL_REAL_FUNCTION_RET(data, ret, getline, lineptr, n, stream)

	if (ret == -1) {
		data.return_state = eof;
		read_data.read_bytes = 0;
	} else {
		data.return_state = ok;
		read_data.read_bytes = ret;
	}

	WRAP_END(data)
	return ret;
}
#endif

#if HAVE_GETDELIM
ssize_t WRAP(getdelim)(char **lineptr, size_t *n, int delimiter, FILE *stream) {
	ssize_t ret;
	struct basic data;
	struct read_function read_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, read_function, read_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	CALL_REAL_FUNCTION_RET(data, ret, getdelim, lineptr, n, delimiter, stream)

	if (ret == -1) {
		data.return_state = eof;
		read_data.read_bytes = 0;
	} else {
		data.return_state = ok;
		read_data.read_bytes = ret;
	}

	WRAP_END(data)
	return ret;
}
#endif

char * WRAP(fgets)(char *s, int count, FILE *stream) {
	char * ret;
	struct basic data;
	struct read_function read_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, read_function, read_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	CALL_REAL_FUNCTION_RET(data, ret, fgets, s, count, stream)

	if (NULL == ret) {
		data.return_state = eof;
		read_data.read_bytes = 0;
	} else {
		data.return_state = ok;
		read_data.read_bytes = strlen(s);
	}

	WRAP_END(data)
	return ret;
}

wchar_t * WRAP(fgetws)(wchar_t *ws, int count, FILE *stream) {
	wchar_t * ret;
	struct basic data;
	struct read_function read_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, read_function, read_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	CALL_REAL_FUNCTION_RET(data, ret, fgetws, ws, count, stream)

	if (NULL == ret) {
		data.return_state = eof;
		read_data.read_bytes = 0;
	} else {
		data.return_state = ok;
		read_data.read_bytes = wcslen(ws) * sizeof(wchar_t);
	}

	WRAP_END(data)
	return ret;
}

#if HAVE_FGETS_UNLOCKED
char * WRAP(fgets_unlocked)(char *s, int count, FILE *stream) {
	char * ret;
	struct basic data;
	struct read_function read_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, read_function, read_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	CALL_REAL_FUNCTION_RET(data, ret, fgets_unlocked, s, count, stream)

	if (NULL == ret) {
		data.return_state = eof;
		read_data.read_bytes = 0;
	} else {
		data.return_state = ok;
		read_data.read_bytes = strlen(s);
	}

	WRAP_END(data)
	return ret;
}
#endif

#if HAVE_FGETWS_UNLOCKED
wchar_t * WRAP(fgetws_unlocked)(wchar_t *ws, int count, FILE *stream) {
	wchar_t * ret;
	struct basic data;
	struct read_function read_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, read_function, read_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	CALL_REAL_FUNCTION_RET(data, ret, fgetws_unlocked, ws, count, stream)

	if (NULL == ret) {
		data.return_state = eof;
		read_data.read_bytes = 0;
	} else {
		data.return_state = ok;
		read_data.read_bytes = wcslen(ws) * sizeof(wchar_t);
	}

	WRAP_END(data)
	return ret;
}
#endif

int WRAP(ungetc)(int c, FILE *stream) {
	int ret;
	struct basic data;
	struct unget_function unget_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, unget_function, unget_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	CALL_REAL_FUNCTION_RET(data, ret, ungetc, c, stream)

	data.return_state = get_return_state_c(ret);
	if (data.return_state == ok) {
		unget_data.buffer_bytes = -1;
	} else {
		unget_data.buffer_bytes = 0;
	}

	WRAP_END(data)
	return ret;
}

wint_t WRAP(ungetwc)(wint_t wc, FILE *stream) {
	wint_t ret;
	struct basic data;
	struct unget_function unget_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, unget_function, unget_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	CALL_REAL_FUNCTION_RET(data, ret, ungetwc, wc, stream)

	data.return_state = get_return_state_wc(ret);
	if (data.return_state == ok) {
		unget_data.buffer_bytes = -1;
	} else {
		unget_data.buffer_bytes = 0;
	}

	WRAP_END(data)
	return ret;
}

size_t WRAP(fread)(void *data, size_t size, size_t count, FILE *stream) {
	size_t ret;
	struct basic _data;
	struct read_function read_data;
	WRAP_START(_data)

	get_basic(&_data);
	JSON_STRUCT_SET_VOID_P(_data, function_data, read_function, read_data)
	POSIX_IO_SET_FUNCTION_NAME(_data.function_name);
	JSON_STRUCT_SET_VOID_P(_data, file_type, file_stream, stream)

	CALL_REAL_FUNCTION_RET(_data, ret, fread, data, size, count, stream)

	if (0 == ret) {
		_data.return_state = eof;
		read_data.read_bytes = 0;
	} else {
		_data.return_state = ok;
		read_data.read_bytes = ret * size;
	}

	WRAP_END(_data)
	return ret;
}

#if HAVE_FREAD_UNLOCKED
size_t WRAP(fread_unlocked)(void *data, size_t size, size_t count, FILE *stream) {
	size_t ret;
	struct basic _data;
	struct read_function read_data;
	WRAP_START(_data)

	get_basic(&_data);
	JSON_STRUCT_SET_VOID_P(_data, function_data, read_function, read_data)
	POSIX_IO_SET_FUNCTION_NAME(_data.function_name);
	JSON_STRUCT_SET_VOID_P(_data, file_type, file_stream, stream)

	CALL_REAL_FUNCTION_RET(_data, ret, fread_unlocked, data, size, count,
			stream)

	if (0 == ret) {
		_data.return_state = eof;
		read_data.read_bytes = 0;
	} else {
		_data.return_state = ok;
		read_data.read_bytes = ret * size;
	}

	WRAP_END(_data)
	return ret;
}
#endif

size_t WRAP(fwrite)(const void *data, size_t size, size_t count, FILE *stream) {
	size_t ret;
	struct basic _data;
	struct write_function write_data;
	WRAP_START(_data)

	get_basic(&_data);
	JSON_STRUCT_SET_VOID_P(_data, function_data, write_function, write_data)
	POSIX_IO_SET_FUNCTION_NAME(_data.function_name);
	JSON_STRUCT_SET_VOID_P(_data, file_type, file_stream, stream)

	CALL_REAL_FUNCTION_RET(_data, ret, fwrite, data, size, count, stream)

	if (ret != count) {
		_data.return_state = error;
		write_data.written_bytes = 0;
	} else {
		_data.return_state = ok;
		write_data.written_bytes = ret * size;
	}

	WRAP_END(_data)
	return ret;
}

#if HAVE_FWRITE_UNLOCKED
size_t WRAP(fwrite_unlocked)(const void *data, size_t size, size_t count,
		FILE *stream) {
	size_t ret;
	struct basic _data;
	struct write_function write_data;
	WRAP_START(_data)

	get_basic(&_data);
	JSON_STRUCT_SET_VOID_P(_data, function_data, write_function, write_data)
	POSIX_IO_SET_FUNCTION_NAME(_data.function_name);
	JSON_STRUCT_SET_VOID_P(_data, file_type, file_stream, stream)

	CALL_REAL_FUNCTION_RET(_data, ret, fwrite_unlocked, data, size, count,
			stream)

	if (ret != count) {
		_data.return_state = error;
		write_data.written_bytes = 0;
	} else {
		_data.return_state = ok;
		write_data.written_bytes = ret * size;
	}

	WRAP_END(_data)
	return ret;
}
#endif

int WRAP(fprintf)(FILE *stream, const char *template, ...) {
	int ret;
	va_list ap;
	struct basic data;
	struct write_function write_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, write_function, write_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	va_start(ap, template);
	CALL_REAL_FUNCTION_RET(data, ret, vfprintf, stream, template, ap)
	va_end(ap);

	if (ret < 0) {
		data.return_state = error;
		write_data.written_bytes = 0;
	} else {
		data.return_state = ok;
		write_data.written_bytes = ret;
	}

	WRAP_END(data)
	return ret;
}

#if HAVE_FWPRINTF
int WRAP(fwprintf)(FILE *stream, const wchar_t *template, ...) {
	int ret;
	va_list ap;
	struct basic data;
	struct write_function write_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, write_function, write_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	va_start(ap, template);
	CALL_REAL_FUNCTION_RET(data, ret, vfwprintf, stream, template, ap)
	va_end(ap);

	if (ret < 0) {
		data.return_state = error;
		write_data.written_bytes = 0;
	} else {
		data.return_state = ok;
		write_data.written_bytes = ret * sizeof(wchar_t);
	}

	WRAP_END(data)
	return ret;
}
#endif

int WRAP(vfprintf)(FILE *stream, const char *template, va_list ap) {
	int ret;
	struct basic data;
	struct write_function write_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, write_function, write_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	CALL_REAL_FUNCTION_RET(data, ret, vfprintf, stream, template, ap)

	if (ret < 0) {
		data.return_state = error;
		write_data.written_bytes = 0;
	} else {
		data.return_state = ok;
		write_data.written_bytes = ret;
	}

	WRAP_END(data)
	return ret;
}

#if HAVE_VFWPRINTF
int WRAP(vfwprintf)(FILE *stream, const wchar_t *template, va_list ap) {
	int ret;
	struct basic data;
	struct write_function write_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, write_function, write_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	CALL_REAL_FUNCTION_RET(data, ret, vfwprintf, stream, template, ap)

	if (ret < 0) {
		data.return_state = error;
		write_data.written_bytes = 0;
	} else {
		data.return_state = ok;
		write_data.written_bytes = ret * sizeof(wchar_t);
	}

	WRAP_END(data)
	return ret;
}
#endif

int WRAP(fscanf)(FILE *stream, const char *template, ...) {
	int ret;
	va_list ap;
	struct basic data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P_NULL(data, function_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	va_start(ap, template);
	CALL_REAL_FUNCTION_RET(data, ret, vfscanf, stream, template, ap)
	va_end(ap);

	data.return_state = get_return_state_c(ret);

	WRAP_END(data)
	return ret;
}

#if HAVE_FWSCANF
int WRAP(fwscanf)(FILE *stream, const wchar_t *template, ...) {
	int ret;
	va_list ap;
	struct basic data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P_NULL(data, function_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	va_start(ap, template);
	CALL_REAL_FUNCTION_RET(data, ret, vfwscanf, stream, template, ap)
	va_end(ap);

	data.return_state = get_return_state_wc(ret);

	WRAP_END(data)
	return ret;
}
#endif

#if HAVE_VFSCANF
int WRAP(vfscanf)(FILE *stream, const char *template, va_list ap) {
	int ret;
	struct basic data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P_NULL(data, function_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	CALL_REAL_FUNCTION_RET(data, ret, vfscanf, stream, template, ap)

	data.return_state = get_return_state_c(ret);

	WRAP_END(data)
	return ret;
}
#endif

#if HAVE_VFWSCANF
int WRAP(vfwscanf)(FILE *stream, const wchar_t *template, va_list ap) {
	int ret;
	struct basic data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P_NULL(data, function_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	CALL_REAL_FUNCTION_RET(data, ret, vfwscanf, stream, template, ap)

	data.return_state = get_return_state_wc(ret);

	WRAP_END(data)
	return ret;
}
#endif

int WRAP(feof)(FILE *stream) {
	int ret;
	struct basic data;
	struct information_function information_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, information_function,
			information_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	CALL_REAL_FUNCTION_RET(data, ret, feof, stream)

	data.return_state = ok;
	if (ret == 0) {
		information_data.return_bool = false;
	} else {
		information_data.return_bool = true;
	}

	WRAP_END(data)
	return ret;
}

#if HAVE_FEOF_UNLOCKED
int WRAP(feof_unlocked)(FILE *stream) {
	int ret;
	struct basic data;
	struct information_function information_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, information_function,
			information_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	CALL_REAL_FUNCTION_RET(data, ret, feof_unlocked, stream)

	data.return_state = ok;
	if (0 == ret) {
		information_data.return_bool = false;
	} else {
		information_data.return_bool = true;
	}

	WRAP_END(data)
	return ret;
}
#endif

int WRAP(ferror)(FILE *stream) {
	int ret;
	struct basic data;
	struct information_function information_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, information_function,
			information_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	CALL_REAL_FUNCTION_RET(data, ret, ferror, stream)

	data.return_state = ok;
	if (0 == ret) {
		information_data.return_bool = false;
	} else {
		information_data.return_bool = true;
	}

	WRAP_END(data)
	return ret;
}

#if HAVE_FERROR_UNLOCKED
int WRAP(ferror_unlocked)(FILE *stream) {
	int ret;
	struct basic data;
	struct information_function information_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, information_function,
			information_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	CALL_REAL_FUNCTION_RET(data, ret, ferror_unlocked, stream)

	data.return_state = ok;
	if (0 == ret) {
		information_data.return_bool = false;
	} else {
		information_data.return_bool = true;
	}

	WRAP_END(data)
	return ret;
}
#endif

void WRAP(clearerr)(FILE *stream) {
	struct basic data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P_NULL(data, function_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	CALL_REAL_FUNCTION(data, clearerr, stream)

	data.return_state = ok;

	WRAP_END(data)
	return;
}

#if HAVE_CLEARERR_UNLOCKED
void WRAP(clearerr_unlocked)(FILE *stream) {
	struct basic data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P_NULL(data, function_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	CALL_REAL_FUNCTION(data, clearerr_unlocked, stream)

	data.return_state = ok;

	WRAP_END(data)
	return;
}
#endif

long int WRAP(ftell)(FILE *stream) {
	long int ret;
	struct basic data;
	struct position_function position_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, position_function,
			position_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	CALL_REAL_FUNCTION_RET(data, ret, ftell, stream)

	if (-1 == ret) {
		data.return_state = error;
		position_data.position = 0;
	} else {
		data.return_state = ok;
		position_data.position = ret;
	}

	WRAP_END(data)
	return ret;
}

#if HAVE_FTELLO
off_t WRAP(ftello)(FILE *stream) {
	off_t ret;
	struct basic data;
	struct position_function position_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, position_function,
			position_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	CALL_REAL_FUNCTION_RET(data, ret, ftello, stream)

	if (-1 == ret) {
		data.return_state = error;
		position_data.position = 0;
	} else {
		data.return_state = ok;
		position_data.position = ret;
	}

	WRAP_END(data)
	return ret;
}
#endif

#if HAVE_FTELLO64
off64_t WRAP(ftello64)(FILE *stream) {
	off64_t ret;
	struct basic data;
	struct position_function position_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, position_function,
			position_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	CALL_REAL_FUNCTION_RET(data, ret, ftello64, stream)

	if (-1 == ret) {
		data.return_state = error;
		position_data.position = 0;
	} else {
		data.return_state = ok;
		position_data.position = ret;
	}

	WRAP_END(data)
	return ret;
}
#endif

int WRAP(fseek)(FILE *stream, long int offset, int whence) {
	int ret;
	struct basic data;
	struct positioning_function positioning_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, positioning_function,
			positioning_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	CALL_REAL_FUNCTION_RET(data, ret, fseek, stream, offset, whence)

	if (0 == ret) {
		data.return_state = ok;
		positioning_data.offset = offset;
		positioning_data.relative_to = get_seek_where(whence);
	} else {
		data.return_state = error;
		positioning_data.offset = offset;
		positioning_data.relative_to = get_seek_where(whence);
	}

	WRAP_END(data)
	return ret;
}

#if HAVE_FSEEKO
int WRAP(fseeko)(FILE *stream, off_t offset, int whence) {
	int ret;
	struct basic data;
	struct positioning_function positioning_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, positioning_function,
			positioning_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	CALL_REAL_FUNCTION_RET(data, ret, fseeko, stream, offset, whence)

	if (0 == ret) {
		data.return_state = ok;
		positioning_data.offset = offset;
		positioning_data.relative_to = get_seek_where(whence);
	} else {
		data.return_state = error;
		positioning_data.offset = offset;
		positioning_data.relative_to = get_seek_where(whence);
	}

	WRAP_END(data)
	return ret;
}
#endif

#if HAVE_FSEEKO64
int WRAP(fseeko64)(FILE *stream, off64_t offset, int whence) {
	int ret;
	struct basic data;
	struct positioning_function positioning_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, positioning_function,
			positioning_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	CALL_REAL_FUNCTION_RET(data, ret, fseeko64, stream, offset, whence)

	if (0 == ret) {
		data.return_state = ok;
		positioning_data.offset = offset;
		positioning_data.relative_to = get_seek_where(whence);
	} else {
		data.return_state = error;
		positioning_data.offset = offset;
		positioning_data.relative_to = get_seek_where(whence);
	}

	WRAP_END(data)
	return ret;
}
#endif

void WRAP(rewind)(FILE *stream) {
	struct basic data;
	struct positioning_function positioning_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, positioning_function,
			positioning_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	CALL_REAL_FUNCTION(data, rewind, stream)

	data.return_state = ok;
	positioning_data.offset = 0;
	positioning_data.relative_to = get_seek_where(SEEK_SET);

	WRAP_END(data)
	return;
}

int WRAP(fgetpos)(FILE *stream, fpos_t *position) {
	int ret;
	struct basic data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P_NULL(data, function_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	CALL_REAL_FUNCTION_RET(data, ret, fgetpos, stream, position)

	if (0 != ret) {
		data.return_state = error;
	} else {
		data.return_state = ok;
	}

	WRAP_END(data)
	return ret;
}

#if HAVE_FGETPOS64
int WRAP(fgetpos64)(FILE *stream, fpos64_t *position) {
	int ret;
	struct basic data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P_NULL(data, function_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	CALL_REAL_FUNCTION_RET(data, ret, fgetpos64, stream, position)

	if (0 != ret) {
		data.return_state = error;
	} else {
		data.return_state = ok;
	}

	WRAP_END(data)
	return ret;
}
#endif

int WRAP(fsetpos)(FILE *stream, const fpos_t *position) {
	int ret;
	struct basic data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P_NULL(data, function_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	CALL_REAL_FUNCTION_RET(data, ret, fsetpos, stream, position)

	if (0 == ret) {
		data.return_state = ok;
	} else {
		data.return_state = error;
	}

	WRAP_END(data)
	return ret;
}

#if HAVE_FSETPOS64
int WRAP(fsetpos64)(FILE *stream, const fpos64_t *position) {
	int ret;
	struct basic data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P_NULL(data, function_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	CALL_REAL_FUNCTION_RET(data, ret, fsetpos64, stream, position)

	if (0 == ret) {
		data.return_state = ok;
	} else {
		data.return_state = error;
	}

	WRAP_END(data)
	return ret;
}
#endif

int WRAP(fflush)(FILE *stream) {
	int ret;
	struct basic data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P_NULL(data, function_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	CALL_REAL_FUNCTION_RET(data, ret, fflush, stream)

	data.return_state = get_return_state_c(ret);

	WRAP_END(data)
	return ret;
}

#if HAVE_FFLUSH_UNLOCKED
int WRAP(fflush_unlocked)(FILE *stream) {
	int ret;
	struct basic data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P_NULL(data, function_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	CALL_REAL_FUNCTION_RET(data, ret, fflush_unlocked, stream)

	data.return_state = get_return_state_c(ret);

	WRAP_END(data)
	return ret;
}
#endif

int WRAP(setvbuf)(FILE *stream, char *buf, int mode, size_t size) {
	int ret;
	struct basic data;
	struct buffer_function buffer_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, buffer_function, buffer_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	CALL_REAL_FUNCTION_RET(data, ret, setvbuf, stream, buf, mode, size)

	if (0 == ret) {
		data.return_state = ok;
		buffer_data.buffer_mode = get_buffer_mode(mode);
		buffer_data.buffer_size = size;
	} else {
		data.return_state = error;
		buffer_data.buffer_mode = get_buffer_mode(mode);
		buffer_data.buffer_size = size;
	}

	WRAP_END(data)
	return ret;
}

void WRAP(setbuf)(FILE *stream, char *buf) {
	struct basic data;
	struct buffer_function buffer_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, buffer_function, buffer_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	CALL_REAL_FUNCTION(data, setbuf, stream, buf)

	if (NULL == buf) {
		data.return_state = ok;
		buffer_data.buffer_mode = unbuffered;
		buffer_data.buffer_size = 0;
	} else {
		data.return_state = ok;
		buffer_data.buffer_mode = fully_buffered;
		buffer_data.buffer_size = BUFSIZ;
	}

	WRAP_END(data)
	return;
}

#if HAVE_SETBUFFER
void WRAP(setbuffer)(FILE *stream, char *buf, size_t size) {
	struct basic data;
	struct buffer_function buffer_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, buffer_function, buffer_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	CALL_REAL_FUNCTION(data, setbuffer, stream, buf, size)

	if (NULL == buf) {
		data.return_state = ok;
		buffer_data.buffer_mode = unbuffered;
		buffer_data.buffer_size = 0;
	} else {
		data.return_state = ok;
		buffer_data.buffer_mode = fully_buffered;
		buffer_data.buffer_size = size;
	}

	WRAP_END(data)
	return;
}
#endif

#if HAVE_SETLINEBUF
void WRAP(setlinebuf)(FILE *stream) {
	struct basic data;
	struct buffer_function buffer_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, buffer_function, buffer_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	CALL_REAL_FUNCTION(data, setlinebuf, stream)

	data.return_state = ok;
	buffer_data.buffer_mode = line_buffered;
	buffer_data.buffer_size = 0;

	WRAP_END(data)
	return;
}
#endif

int WRAP(__freadable)(FILE *stream) {
	int ret;
	struct basic data;
	struct information_function information_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, information_function,
			information_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	CALL_REAL_FUNCTION_RET(data, ret, __freadable, stream)

	data.return_state = ok;
	if (0 == ret) {
		information_data.return_bool = false;
	} else {
		information_data.return_bool = true;
	}

	WRAP_END(data)
	return ret;
}

// ToDo: check dependencies
int WRAP(__fwritable)(FILE *stream) {
	int ret;
	struct basic data;
	struct information_function information_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, information_function,
			information_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	CALL_REAL_FUNCTION_RET(data, ret, __fwritable, stream)

	data.return_state = ok;
	if (0 == ret) {
		information_data.return_bool = false;
	} else {
		information_data.return_bool = true;
	}

	WRAP_END(data)
	return ret;
}

int WRAP(__freading)(FILE *stream) {
	int ret;
	struct basic data;
	struct information_function information_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, information_function,
			information_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	CALL_REAL_FUNCTION_RET(data, ret, __freading, stream)

	data.return_state = ok;
	if (0 == ret) {
		information_data.return_bool = false;
	} else {
		information_data.return_bool = true;
	}

	WRAP_END(data)
	return ret;
}

int WRAP(__fwriting)(FILE *stream) {
	int ret;
	struct basic data;
	struct information_function information_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, information_function,
			information_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	CALL_REAL_FUNCTION_RET(data, ret, __fwriting, stream)

	data.return_state = ok;
	if (0 == ret) {
		information_data.return_bool = false;
	} else {
		information_data.return_bool = true;
	}

	WRAP_END(data)
	return ret;
}

int WRAP(__fsetlocking)(FILE *stream, int type) {
	int ret;
	struct basic data;
	struct lock_mode_function lock_mode_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, lock_mode_function,
			lock_mode_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	lock_mode_data.set_mode = get_lock_mode(type);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	CALL_REAL_FUNCTION_RET(data, ret, __fsetlocking, stream, type)

	data.return_state = ok;
	lock_mode_data.return_mode = get_lock_mode(ret);

	WRAP_END(data)
	return ret;
}

void WRAP(_flushlbf)(void) {
	struct basic data;
	FILE *file = NULL;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P_NULL(data, function_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, file)

	CALL_REAL_FUNCTION(data, _flushlbf)

	data.return_state = ok;

	WRAP_END(data)
	return;
}

void WRAP(__fpurge)(FILE *stream) {
	struct basic data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P_NULL(data, function_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	CALL_REAL_FUNCTION(data, __fpurge, stream)

	data.return_state = ok;

	WRAP_END(data)
	return;
}

int WRAP(__flbf)(FILE *stream) {
	int ret;
	struct basic data;
	struct information_function information_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, information_function,
			information_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	CALL_REAL_FUNCTION_RET(data, ret, __flbf, stream)

	data.return_state = ok;
	if (0 == ret) {
		information_data.return_bool = false;
	} else {
		information_data.return_bool = true;
	}

	WRAP_END(data)
	return ret;
}

size_t WRAP(__fbufsize)(FILE *stream) {
	size_t ret;
	struct basic data;
	struct bufsize_function bufsize_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, bufsize_function, bufsize_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	CALL_REAL_FUNCTION_RET(data, ret, __fbufsize, stream)

	data.return_state = ok;
	bufsize_data.buffer_size = ret;

	WRAP_END(data)
	return ret;
}

size_t WRAP(__fpending)(FILE *stream) {
	size_t ret;
	struct basic data;
	struct bufsize_function bufsize_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, bufsize_function, bufsize_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	CALL_REAL_FUNCTION_RET(data, ret, __fpending, stream)

	data.return_state = ok;
	bufsize_data.buffer_size = ret;

	WRAP_END(data)
	return ret;
}
