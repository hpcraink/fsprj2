#include "stracing_lsep.h"

#include <stdio.h>
#include "../../../common/stracer_consts.h"
#include "scerb/scerb_producer.h"
#include "scerbmap/scerbmap.h"

#include <assert.h>
//#define DEV_DEBUG_ENABLE_LOGS
#include "../../common/error.h"


/* -- Functions -- */
static char* derive_smo_name(pid_t tid) {
    char *smo_name;
    DIE_WHEN_ERRNO( asprintf(&smo_name, STRACING_LSEP_SMO_NAME_FORMAT, tid) );        // MUST BE `free`ED !!!
    return smo_name;
}


/* - Public functions - */
void stracing_lsep_init(long scerbmap_max_size) {
    assert( !scerbmap_is_inited() && "Got already init" );

    scerbmap_create(scerbmap_max_size);
    DEV_DEBUG_PRINT_MSG("Init'ed stracing-lsep module");
}

void stracing_lsep_cleanup(void) {
    assert( scerbmap_is_inited() && "Got no init yet" );

    scerbmap_destroy();
    DEV_DEBUG_PRINT_MSG("Fin'ed stracing-lsep module");
}



void stracing_lsep_tracee_attach(pid_t tid) {
    assert( scerbmap_is_inited() && "Got no init yet" );

    char *smo_name = derive_smo_name(tid);
    sm_scerb_t *sm_scerb;
    if (-1 == scerb_attach(&sm_scerb, smo_name)) {
        LOG_ERROR_AND_DIE("Couldn't attach to scerb w/ smo-identifier \"%s\"", smo_name);
    }
    DEV_DEBUG_PRINT_MSG("Attached sm-scerb w/ id=\"%s\"", smo_name);
    free(smo_name);

    scerbmap_add(&tid, sm_scerb);
}


void stracing_lsep_tracee_detach(pid_t tid) {
    assert( scerbmap_is_inited() && "Got no init yet" );

    sm_scerb_t *sm_scerb;
    if ((-1 == scerbmap_get(&tid, &sm_scerb)) || (-1 == scerbmap_remove(&tid))) {
        LOG_ERROR_AND_DIE("Couldn't delete scerb-pointer");
    }

    char *smo_name = derive_smo_name(tid);
    if (-1 == scerb_destory_detach(&sm_scerb, smo_name)) {
        LOG_ERROR_AND_DIE("Couldn't destroy scerb");
    }
    DEV_DEBUG_PRINT_MSG("Detached sm-scerb w/ id=\"%s\"", smo_name);
    free(smo_name);
}

void stracing_lsep_tracee_add_scevent(pid_t tid, scevent_t* scevent_buf_ptr) {
    assert( scerbmap_is_inited() && "Got no init yet" );

    sm_scerb_t *sm_scerb;
    if (-1 == scerbmap_get(&tid, &sm_scerb)) {
        LOG_ERROR_AND_DIE("Couldn't find scerb-pointer");
    }

    if (0 != scerb_offer(sm_scerb, scevent_buf_ptr) ) {
        LOG_WARN("Couldn't write syscall event in buffer");
    }

    DEV_DEBUG_PRINT_MSG("Wrote new scevent for tid=%d", tid);
}
