#include "entrypoint.h"
#include "../error.h"


/* -- Functions -- */
void stracing_launch_tracer(void) {
    LIBIOTRACE_WARN("(A) Got run!");
}

void stracing_register_with_tracer(void) {
    LIBIOTRACE_WARN("(B) Got run!");
}
