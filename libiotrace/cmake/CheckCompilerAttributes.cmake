function (CheckCompilerAttributes)
    include (CheckCSourceCompiles)

    check_c_source_compiles ("
        char * strcpy(char * dst, const char * src);
        void testFunc(char * var) __attribute__((nonnull(1)));
        void testFunc(char * var) { strcpy (var, \"Hello World\"); }
        int main(void) {
            // testFunc(NULL);
            return 0;
        }
    "  HAVE_ATTRIBUTE_NONNULL)
    # message ("HAVE_ATTRIBUTE_NONNULL: ${HAVE_ATTRIBUTE_NONNULL}")

    check_c_source_compiles ("
        int testFunc(char * var __attribute__((unused)));
        int testFunc(char * var) { return 0; }
        int main(void) {
            testFunc(\"Hello World\");
            return 0;
        }
    "  HAVE_ATTRIBUTE_UNUSED)
    # message ("HAVE_ATTRIBUTE_UNUSED: ${HAVE_ATTRIBUTE_UNUSED}")
endfunction ()
