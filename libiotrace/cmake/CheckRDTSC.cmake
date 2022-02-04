function (CheckRDTSC)
    include (CheckCSourceCompiles)

    check_c_source_compiles ("
        volatile unsigned long long int getrdtsc() {
            unsigned long long x;
#if defined (__i386__)
            __asm__ volatile (\"rdtsc\" : \"=A\" (x));
#elif defined (__x86_64__)
            unsigned long hi, lo;
            __asm__ volatile (\"rdtsc\" : \"=a\" (lo), \"=d\" (hi));
            x = ((unsigned long long)hi << 32 | lo);
#else
#  error \"Instruction only supported on Intel __i386__ and __x86_64__\"
#endif
            return x;
        }
        int main(void) {
            unsigned long long time_now = getrdtsc();
            return 0;
        }
    "  HAVE_TIME_RDTSC)
    # message ("HAVE_TIME_RDTSC: ${HAVE_TIME_RDTSC}" )
endfunction ()
