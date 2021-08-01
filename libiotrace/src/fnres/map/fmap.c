#include <assert.h>
#include <pthread.h>
#include <string.h>

#include "fmap.h"
#include "impl/hashmap.h"


#define FMAP_INITIAL_CAPACITY 15


struct fmap_value {
    fmap_key key;
    char* filename;
};

static struct hashmap* global_map = NULL;
static pthread_mutex_t lock;



// - Internal / Helper-functions -
static uint64_t _hash(const void *item, uint64_t seed0, uint64_t seed1) {
    const struct fmap_value *value = item;
    return hashmap_sip(&(value->key), sizeof(value->key), seed0, seed1);
}

static int _compare(const void *a, const void *b, void *udata) {
    const struct fmap_value *ua = a;
    const struct fmap_value *ub = b;
    return memcmp(&(ua->key), &(ub->key), sizeof(fmap_key));
}


static void _create_map_value(struct fmap_value* value, fmap_key* key, char* filename) {
    memcpy(&(value->key), key, sizeof(fmap_key));
    value->filename = filename;
}

// - Public functions -
void fmap_create(void) {
    assert(global_map == NULL);

    pthread_mutex_lock(&lock);

    global_map = hashmap_new(
            sizeof(struct fmap_value),
            FMAP_INITIAL_CAPACITY,
            0, 0,
            _hash, _compare, NULL);

    pthread_mutex_unlock(&lock);
}
void fmap_destroy(void) {
    assert(global_map != NULL);

    pthread_mutex_lock(&lock);

    hashmap_free(global_map);

    pthread_mutex_unlock(&lock);
}

char* fmap_get(fmap_key* key) {
    assert(global_map != NULL && key != NULL);

    struct fmap_value search_key;
    _create_map_value(&search_key, key, NULL);

    struct fmap_value *found_val = hashmap_get(global_map, &search_key);
    return (found_val == NULL) ? (NULL) : (found_val->filename);
}

void fmap_set(fmap_key* key, char* filename) {
    assert(global_map != NULL && key != NULL && filename != NULL);

    pthread_mutex_lock(&lock);

    struct fmap_value value;
    _create_map_value(&value, key, filename);

    hashmap_set(global_map, &value);

    pthread_mutex_unlock(&lock);
}

void fmap_remove(fmap_key* key) {
    assert(global_map != NULL && key != NULL);

    pthread_mutex_lock(&lock);

    hashmap_delete(global_map, key);

    pthread_mutex_unlock(&lock);
}
