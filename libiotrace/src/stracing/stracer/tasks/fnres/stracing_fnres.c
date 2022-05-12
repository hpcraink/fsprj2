#include "stracing_fnres.h"

#include <stdio.h>
#include "../../../common/stracer_consts.h"
#include "scerb/scerb_producer.h"
#include "scerbmap/scerbmap.h"

#include <assert.h>
#include "../../common/error.h"
#define DEV_DEBUG_ENABLE_LOGS
#include "../../common/debug.h"


/* -- Functions -- */
bool stracing_fnres_is_inited(void) {
    return scerbmap_is_inited();
}

void stracing_fnres_init(long scerbmap_max_size) {
    assert( !scerbmap_is_inited() && "Got already init" );

    scerbmap_create(scerbmap_max_size);
}

void stracing_fnres_fin(void) {
    assert( scerbmap_is_inited() && "Got no init yet" );

    scerbmap_destroy();
}


void stracing_fnres_attach_sm(pid_t tid) {
    assert( scerbmap_is_inited() && "Got no init yet" );

    char *smo_name;
    DIE_WHEN_ERRNO( asprintf(&smo_name, STRACING_FNRES_SMO_NAME_FORMAT, tid) );
    sm_scerb_t *sm_scerb;
    if (-1 == scerb_attach(&sm_scerb, smo_name)) {
        LOG_ERROR_AND_EXIT("Couldn't attach to scerb w/ smo-identifier \"%s\"", smo_name);
    }
    free(smo_name);

    scerbmap_add(&tid, sm_scerb);
}


void stracing_fnres_destroy_sm(pid_t tid) {
    assert( scerbmap_is_inited() && "Got no init yet" );

    sm_scerb_t *sm_scerb;
    if ((-1 == scerbmap_get(&tid, &sm_scerb)) || (-1 == scerbmap_remove(&tid))) {
        LOG_ERROR_AND_EXIT("Couldn't delete scerb-pointer for tid=%ld", tid);
    }

    char *smo_name;
    DIE_WHEN_ERRNO( asprintf(&smo_name, STRACING_FNRES_SMO_NAME_FORMAT, tid) );
    if (-1 == scerb_destory_detach(&sm_scerb, smo_name)) {
        LOG_ERROR_AND_EXIT("Couldn't destroy scerb for tid=%ld", tid);
    }
    free(smo_name);
}

void stracing_fnres_write_scevent(pid_t tid, scevent_t* event_buf_ptr) {
    assert( scerbmap_is_inited() && "Got no init yet" );

    sm_scerb_t *sm_scerb;
    if (-1 == scerbmap_get(&tid, &sm_scerb)) {
        LOG_ERROR_AND_EXIT("Couldn't find scerb-pointer for tid=%ld", tid);
    }

    if (0 != scerb_offer(sm_scerb, event_buf_ptr) ) {
        LOG_WARN("Couldn't write syscall event in buffer for tid=%ld", tid);
    }
}
