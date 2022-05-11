/*
 * Copyright (c) 2016 Mindaugas Rasiukevicius <rmind at noxt eu>
 * All rights reserved.
 *
 * Use is subject to license terms, as specified in the LICENSE file.
 */

#ifndef _RINGBUF_H_
#define _RINGBUF_H_

__BEGIN_DECLS


typedef struct ringbuf_worker ringbuf_worker_t;




/* $$ TAKEN FROM C FILE   (we want to know impl. details of the ringbuffer so we've size information @ compile time) $$ */
typedef uint64_t	ringbuf_off_t;

struct ringbuf_worker {
    volatile ringbuf_off_t	seen_off;
    int			registered;
};

typedef struct ringbuf {
    /* Ring buffer space. */
    size_t			space;

    /*
     * The NEXT hand is atomically updated by the producer.
     * WRAP_LOCK_BIT is set in case of wrap-around; in such case,
     * the producer can update the 'end' offset.
     */
    volatile ringbuf_off_t	next;
    ringbuf_off_t		end;

    /* The following are updated by the consumer. */
    ringbuf_off_t		written;
    unsigned		nworkers;
    ringbuf_worker_t	workers[1];     // $$$  SINCE ALWAYS 1 PRODUCER   (WAS OG Flexible Array Member)  !!!
} ringbuf_t;
/* $$  $$ */





int		ringbuf_setup(ringbuf_t *, size_t);
//void		ringbuf_get_sizes(unsigned, size_t *, size_t *);

ringbuf_worker_t *ringbuf_register(ringbuf_t *, unsigned);
void		ringbuf_unregister(ringbuf_t *, ringbuf_worker_t *);

ssize_t		ringbuf_acquire(ringbuf_t *, ringbuf_worker_t *, size_t);
void		ringbuf_produce(ringbuf_t *, ringbuf_worker_t *);
size_t		ringbuf_consume(ringbuf_t *, size_t *);
void		ringbuf_release(ringbuf_t *, size_t);

__END_DECLS

#endif
