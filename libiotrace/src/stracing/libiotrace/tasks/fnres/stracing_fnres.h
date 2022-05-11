#ifndef LIBIOTRACE_STRACING_FNRES_H
#define LIBIOTRACE_STRACING_FNRES_H


/* -- Function prototypes -- */
void stracing_fnres_init_scerb(void);
void stracing_fnres_check_and_add_scevents(void);
int stracing_fnres_lookup_and_alias_stream(struct basic* ioevent_ptr);

#endif /* LIBIOTRACE_STRACING_FNRES_H */
