#include "wrapper_name.h"

#ifdef putc
#if putc == _IO_putc
#define putc_MACRO _IO_putc
#else
#error "Unknown macro for putc function!"
#endif
#else
#define putc_MACRO putc
#endif

#ifdef putwc
#error "Unknown macro for putwc function!"
#else
#define putwc_MACRO putwc
#endif

#ifdef putc_unlocked
#error "Unknown macro for putc_unlocked function!"
#else
#define putc_unlocked_MACRO putc_unlocked
#endif

#ifdef putwc_unlocked
#error "Unknown macro for putwc_unlocked function!"
#else
#define putwc_unlocked_MACRO putwc_unlocked
#endif

#ifdef getc
#if getc == _IO_getc
#define getc_MACRO _IO_getc
#else
#error "Unknown macro for getc function!"
#endif
#else
#define getc_MACRO getc
#endif

#ifdef getwc
#error "Unknown macro for getwc function!"
#else
#define getwc_MACRO getwc
#endif

#ifdef getc_unlocked
#error "Unknown macro for getc_unlocked function!"
#else
#define getc_unlocked_MACRO getc_unlocked
#endif

#ifdef getwc_unlocked
#error "Unknown macro for getwc_unlocked function!"
#else
#define getwc_unlocked_MACRO getwc_unlocked
#endif

#ifdef HAVE___OPEN_2
WRAPPER_NAME(__open_2)
#endif
WRAPPER_NAME(open)
#ifdef HAVE_OPEN64
WRAPPER_NAME(open64)
#endif
#ifdef HAVE_OPENAT
WRAPPER_NAME(openat)
#endif
WRAPPER_NAME(creat)
#ifdef HAVE_CREAT64
WRAPPER_NAME(creat64)
#endif
WRAPPER_NAME(close)
WRAPPER_NAME(read)
#ifdef HAVE_PREAD
WRAPPER_NAME(pread)
#endif
#ifdef HAVE_PREAD64
WRAPPER_NAME(pread64)
#endif
WRAPPER_NAME(write)
#ifdef HAVE_PWRITE
WRAPPER_NAME(pwrite)
#endif
#ifdef HAVE_PWRITE64
WRAPPER_NAME(pwrite64)
#endif
WRAPPER_NAME(lseek)
#ifdef HAVE_LSEEK64
WRAPPER_NAME(lseek64)
#endif
#ifdef HAVE_READV
WRAPPER_NAME(readv)
#endif
#ifdef HAVE_WRITEV
WRAPPER_NAME(writev)
#endif
#ifdef HAVE_PREADV
WRAPPER_NAME(preadv)
#endif
#ifdef HAVE_PREADV64
WRAPPER_NAME(preadv64)
#endif
#ifdef HAVE_PWRITEV
WRAPPER_NAME(pwritev)
#endif
#ifdef HAVE_PWRITEV64
WRAPPER_NAME(pwritev64)
#endif
#ifdef HAVE_PREADV2
WRAPPER_NAME(preadv2)
#endif
#ifdef HAVE_PREADV64V2
WRAPPER_NAME(preadv64v2)
#endif
#ifdef HAVE_PWRITEV2
WRAPPER_NAME(pwritev2)
#endif
#ifdef HAVE_PWRITEV64V2
WRAPPER_NAME(pwritev64v2)
#endif
#ifdef HAVE_COPY_FILE_RANGE
WRAPPER_NAME(copy_file_range)
#endif
#ifdef HAVE_MMAP
WRAPPER_NAME(mmap)
#endif
#ifdef HAVE_MMAP64
WRAPPER_NAME(mmap64)
#endif
#ifdef HAVE_MUNMAP
WRAPPER_NAME(munmap)
#endif
#ifdef HAVE_MSYNC
WRAPPER_NAME(msync)
#endif
#ifdef HAVE_MREMAP
WRAPPER_NAME(mremap)
#endif
#ifdef HAVE_MADVISE
WRAPPER_NAME(madvise)
#endif
#ifdef HAVE_POSIX_MADVISE
WRAPPER_NAME(posix_madvise)
#endif
WRAPPER_NAME(select)
#ifdef HAVE_SYNC
WRAPPER_NAME(sync)
#endif
#ifdef HAVE_SYNCFS
WRAPPER_NAME(syncfs)
#endif
#ifdef HAVE_FSYNC
WRAPPER_NAME(fsync)
#endif
#ifdef HAVE_FDATASYNC
WRAPPER_NAME(fdatasync)
#endif
WRAPPER_NAME(dup)
WRAPPER_NAME(dup2)
#ifdef HAVE_DUP3
WRAPPER_NAME(dup3)
#endif
WRAPPER_NAME(fcntl)
WRAPPER_NAME(socket)
WRAPPER_NAME(accept)
#ifdef HAVE_ACCEPT4
WRAPPER_NAME(accept4)
#endif
WRAPPER_NAME(socketpair)
WRAPPER_NAME(connect)
WRAPPER_NAME(bind)
WRAPPER_NAME(pipe)
#ifdef HAVE_PIPE2
WRAPPER_NAME(pipe2)
#endif
#ifdef HAVE_MEMFD_CREATE
WRAPPER_NAME(memfd_create)
#endif
#ifdef HAVE_EPOLL_CREATE
WRAPPER_NAME(epoll_create)
#endif
#ifdef HAVE_EPOLL_CREATE1
WRAPPER_NAME(epoll_create1)
#endif
#ifdef HAVE_MKSTEMP
WRAPPER_NAME(mkstemp)
#endif
#ifdef HAVE_MKOSTEMP
WRAPPER_NAME(mkostemp)
#endif
#ifdef HAVE_MKSTEMPS
WRAPPER_NAME(mkstemps)
#endif
#ifdef HAVE_MKOSTEMPS
WRAPPER_NAME(mkostemps)
#endif
#ifdef HAVE_EVENTFD
WRAPPER_NAME(eventfd)
#endif
#ifdef HAVE_INOTIFY_INIT
WRAPPER_NAME(inotify_init)
#endif
#ifdef HAVE_INOTIFY_INIT1
WRAPPER_NAME(inotify_init1)
#endif
WRAPPER_NAME(readdir)
#ifdef HAVE_DIRFD
WRAPPER_NAME(dirfd)
#endif
WRAPPER_NAME(sendmsg)
WRAPPER_NAME(recvmsg)
#ifdef HAVE_SENDMMSG
WRAPPER_NAME(sendmmsg)
#endif
#ifdef HAVE_RECVMMSG
WRAPPER_NAME(recvmmsg)
#endif

WRAPPER_NAME(fopen)
#ifdef HAVE_FOPEN64
WRAPPER_NAME(fopen64)
#endif
WRAPPER_NAME(freopen)
#ifdef HAVE_FREOPEN64
WRAPPER_NAME(freopen64)
#endif
#ifdef HAVE_FDOPEN
WRAPPER_NAME(fdopen)
#endif
WRAPPER_NAME(fclose)
#ifdef HAVE_FCLOSEALL
WRAPPER_NAME(fcloseall)
#endif
#ifdef HAVE_FLOCKFILE
WRAPPER_NAME(flockfile)
#endif
#ifdef HAVE_FTRYLOCKFILE
WRAPPER_NAME(ftrylockfile)
#endif
#ifdef HAVE_FUNLOCKFILE
WRAPPER_NAME(funlockfile)
#endif
#ifdef HAVE_FWIDE
WRAPPER_NAME(fwide)
#endif
WRAPPER_NAME(fputc)
WRAPPER_NAME(fputwc)
#ifdef HAVE_FPUTC_UNLOCKED
WRAPPER_NAME(fputc_unlocked)
#endif
#ifdef HAVE_FPUTWC_UNLOCKED
WRAPPER_NAME(fputwc_unlocked)
#endif
WRAPPER_NAME(putc_MACRO)
WRAPPER_NAME(putwc_MACRO)
#ifdef HAVE_PUTC_UNLOCKED
WRAPPER_NAME(putc_unlocked_MACRO)
#endif
#ifdef HAVE_PUTWC_UNLOCKED
WRAPPER_NAME(putwc_unlocked_MACRO)
#endif
WRAPPER_NAME(fputs)
WRAPPER_NAME(fputws)
#ifdef HAVE_FPUTS_UNLOCKED
WRAPPER_NAME(fputs_unlocked)
#endif
#ifdef HAVE_FPUTWS_UNLOCKED
WRAPPER_NAME(fputws_unlocked)
#endif
#ifdef HAVE_PUTW
WRAPPER_NAME(putw)
#endif
WRAPPER_NAME(fgetc)
WRAPPER_NAME(fgetwc)
#ifdef HAVE_FGETC_UNLOCKED
WRAPPER_NAME(fgetc_unlocked)
#endif
#ifdef HAVE_FGETWC_UNLOCKED
WRAPPER_NAME(fgetwc_unlocked)
#endif
WRAPPER_NAME(getc_MACRO)
WRAPPER_NAME(getwc_MACRO)
#ifdef HAVE_GETC_UNLOCKED
WRAPPER_NAME(getc_unlocked_MACRO)
#endif
#ifdef HAVE_GETWC_UNLOCKED
WRAPPER_NAME(getwc_unlocked_MACRO)
#endif
#ifdef HAVE_GETW
WRAPPER_NAME(getw)
#endif
#ifdef HAVE_GETLINE
WRAPPER_NAME(getline)
#endif
#ifdef HAVE_GETDELIM
WRAPPER_NAME(getdelim)
#endif
WRAPPER_NAME(fgets)
WRAPPER_NAME(fgetws)
#ifdef HAVE_FGETS_UNLOCKED
WRAPPER_NAME(fgets_unlocked)
#endif
#ifdef HAVE_FGETWS_UNLOCKED
WRAPPER_NAME(fgetws_unlocked)
#endif
WRAPPER_NAME(ungetc)
WRAPPER_NAME(ungetwc)
WRAPPER_NAME(fread)
#ifdef HAVE_FREAD_UNLOCKED
WRAPPER_NAME(fread_unlocked)
#endif
WRAPPER_NAME(fwrite)
#ifdef HAVE_FWRITE_UNLOCKED
WRAPPER_NAME(fwrite_unlocked)
#endif
WRAPPER_NAME(fprintf)
#ifdef HAVE_FWPRINTF
WRAPPER_NAME(fwprintf)
#endif
WRAPPER_NAME(vfprintf)
#ifdef HAVE_VFWPRINTF
WRAPPER_NAME(vfwprintf)
#endif
WRAPPER_NAME(fscanf)
#ifdef HAVE_FWSCANF
WRAPPER_NAME(fwscanf)
#endif
#ifdef HAVE_VFSCANF
WRAPPER_NAME(vfscanf)
#endif
#ifdef HAVE_VFWSCANF
WRAPPER_NAME(vfwscanf)
#endif
WRAPPER_NAME(feof)
#ifdef HAVE_FEOF_UNLOCKED
WRAPPER_NAME(feof_unlocked)
#endif
WRAPPER_NAME(ferror)
#ifdef HAVE_FERROR_UNLOCKED
WRAPPER_NAME(ferror_unlocked)
#endif
WRAPPER_NAME(clearerr)
#ifdef HAVE_CLEARERR_UNLOCKED
WRAPPER_NAME(clearerr_unlocked)
#endif
WRAPPER_NAME(ftell)
#ifdef HAVE_FTELLO
WRAPPER_NAME(ftello)
#endif
#ifdef HAVE_FTELLO64
WRAPPER_NAME(ftello64)
#endif
WRAPPER_NAME(fseek)
#ifdef HAVE_FSEEKO
WRAPPER_NAME(fseeko)
#endif
#ifdef HAVE_FSEEKO64
WRAPPER_NAME(fseeko64)
#endif
WRAPPER_NAME(rewind)
WRAPPER_NAME(fgetpos)
#ifdef HAVE_FGETPOS64
WRAPPER_NAME(fgetpos64)
#endif
WRAPPER_NAME(fsetpos)
#ifdef HAVE_FSETPOS64
WRAPPER_NAME(fsetpos64)
#endif
WRAPPER_NAME(fflush)
#ifdef HAVE_FFLUSH_UNLOCKED
WRAPPER_NAME(fflush_unlocked)
#endif
WRAPPER_NAME(setvbuf)
WRAPPER_NAME(setbuf)
#ifdef HAVE_SETBUFFER
WRAPPER_NAME(setbuffer)
#endif
#ifdef HAVE_SETLINEBUF
WRAPPER_NAME(setlinebuf)
#endif
#ifdef HAVE_FILENO
WRAPPER_NAME(fileno)
#endif
WRAPPER_NAME(tmpfile)
#ifdef HAVE_TMPFILE64
WRAPPER_NAME(tmpfile64)
#endif
WRAPPER_NAME(popen)

WRAPPER_NAME(__freadable)
WRAPPER_NAME(__fwritable)
WRAPPER_NAME(__freading)
WRAPPER_NAME(__fwriting)
WRAPPER_NAME(__fsetlocking)
WRAPPER_NAME(_flushlbf)
WRAPPER_NAME(__fpurge)
WRAPPER_NAME(__flbf)
WRAPPER_NAME(__fbufsize)
WRAPPER_NAME(__fpending)

WRAPPER_NAME(fork)
#ifdef HAVE_VFORK
WRAPPER_NAME(vfork)
#endif
#ifdef HAVE_CLONE
WRAPPER_NAME(clone)
#endif
