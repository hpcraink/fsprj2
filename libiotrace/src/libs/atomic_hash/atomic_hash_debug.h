/**
 * Optional header for testing the map implementation
 */
#ifndef ATOMIC_HASH_DEBUG_H
#define ATOMIC_HASH_DEBUG_H

#include <unistd.h>
#include "atomic_hash.h"


void (*atomic_hash_debug_get_hash_func(hash_t *h))(const void *key, size_t len, void *r);

void *atomic_hash_debug_get_teststr(hash_t *h);
void atomic_hash_debug_set_teststr(hash_t *h, void *teststr);

unsigned long atomic_hash_debug_get_teststr_num(hash_t *h);
void atomic_hash_debug_set_teststr_num(hash_t *h, unsigned long teststr_num);

#endif /* ATOMIC_HASH_DEBUG_H */