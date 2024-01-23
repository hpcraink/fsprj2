function (CheckCompilerAttributes)
    include (CheckCSourceCompiles)

    check_c_source_compiles ("
        extern char * strcpy(char * dst, const char * src);
        void testFunc(char * var) __attribute__((nonnull(1)));
        void testFunc(char * var) { strcpy (var, \"Hello World\"); }
        int main(void) {
            // testFunc(NULL);
            return 0;
        }
    "  HAVE_ATTRIBUTE_NONNULL)
    # message ("HAVE_ATTRIBUTE_NONNULL: ${HAVE_ATTRIBUTE_NONNULL}")

    # TODO: HAVE_BUILTIN_EXPECT

    check_c_source_compiles ("
        typedef int more_aligned_int __attribute__ ((aligned (64)));
        more_aligned_int testFunc();
        more_aligned_int testFunc() { more_aligned_int i = 0; return i; }
        int main(void) {
            return (int)testFunc();
        }
    "  HAVE_ATTRIBUTE_ALIGNED)
    # message ("HAVE_ATTRIBUTE_ALIGNED: ${HAVE_ATTRIBUTE_ALIGNED}")

    check_c_source_compiles ("
        typedef int more_aligned_int __declspec(align(64));
        more_aligned_int testFunc();
        more_aligned_int testFunc() { more_aligned_int i = 0; return i; }
        int main(void) {
            return (int)testFunc();
        }
    "  HAVE_DECLSPEC_ALIGNED)
    # message ("HAVE_DECLSPEC_ALIGNED: ${HAVE_DECLSPEC_ALIGNED}")

    check_c_source_compiles ("
        int testFunc(char * var __attribute__((unused)));
        int testFunc(char * var) { return 0; }
        int main(void) {
            testFunc(\"Hello World\");
            return 0;
        }
    "  HAVE_ATTRIBUTE_UNUSED)
    # message ("HAVE_ATTRIBUTE_UNUSED: ${HAVE_ATTRIBUTE_UNUSED}")

    check_c_source_compiles ("
        static void init(void) __attribute__((constructor));
        static void init(void) {}
        int main(void) {
            return 0;
        }
    "  HAVE_ATTRIBUTE_CONSTRUCTOR)
    # message ("HAVE_ATTRIBUTE_CONSTRUCTOR: ${HAVE_ATTRIBUTE_CONSTRUCTOR}")

    check_c_source_compiles ("
        static void exit(void) __attribute__((destructor));
        static void exit(void) {}
        int main(void) {
            return 0;
        }
    "  HAVE_ATTRIBUTE_DESTRUCTOR)
    # message ("HAVE_ATTRIBUTE_DESTRUCTOR: ${HAVE_ATTRIBUTE_DESTRUCTOR}")

    check_c_source_compiles ("
        static __thread int test = 0;
        int main(void) {
            return test;
        }
    "  HAVE_ATTRIBUTE_THREAD)
    # message ("HAVE_ATTRIBUTE_THREAD: ${HAVE_ATTRIBUTE_THREAD}")

    check_c_source_compiles ("
        static __declspec(thread) int test = 0;
        int main(void) {
            return test;
        }
    "  HAVE_DECLSPEC_THREAD)
    # message ("HAVE_DECLSPEC_THREAD: ${HAVE_DECLSPEC_THREAD}")

    check_c_source_compiles ("
        #define _GNU_SOURCE
        #include <sys/socket.h>

        extern int recvmmsg (int __fd, struct mmsghdr *__vmessages,
                     unsigned int __vlen, int __flags,
                     const struct timespec *__tmo);

        int main(void) {
            return 0;
        }
    "  HAVE_RECVMMSG_CONST_TIMESPEC)
    # message ("HAVE_RECVMMSG_CONST_TIMESPEC: ${HAVE_RECVMMSG_CONST_TIMESPEC}")
    
    check_c_source_compiles ("
        #include <sys/stat.h>
        #include <assert.h>

        extern int __xstat(int ver, const char *pathname, struct stat *statbuf);

        int main(void) {
            return 0;
        }
    "  HAVE___XSTAT)
    # message ("HAVE___XSTAT: ${HAVE___XSTAT}")

    check_c_source_compiles ("
        #include <sys/stat.h>

        extern int __fxstat(int ver, int fd, struct stat *statbuf);

        int main(void) {
            return 0;
        }
    "  HAVE___FXSTAT)
    # message ("HAVE___FXSTAT: ${HAVE___FXSTAT}")
endfunction ()
