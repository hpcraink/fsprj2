#include <stdint.h>
#include <stddef.h>


/* MurmurHash3 was written by Austin Appleby, and is placed in the public
 * domain. The author hereby disclaims copyright to this source code.
 */
void MurmurHash3_x64_128 (const void *s, const size_t len, void *r) {
#define BIG_CONSTANT(x) (x##LLU)
#define SEED 42
    const uint64_t c1 = BIG_CONSTANT(0x87c37b91114253d5);
    const uint64_t c2 = BIG_CONSTANT(0x4cf5ad432745937f);
    const uint64_t *blocks = (const uint64_t*)s;
    uint64_t k1, k2;
    uint64_t h1 = SEED, h2 = SEED;
    const size_t nblocks = (len >> 4);
    for (size_t i = 0; i < nblocks; i++) {
        k1 = blocks[i*2+0]; k2 = blocks[i*2+1];
        k1 *= c1; k1  = ((k1 << 31) | (k1 >> 33)); k1 *= c2; h1 ^= k1;
        h1 = ((h1 << 27) | (h1 >> 37)); h1 += h2; h1 = h1*5+0x52dce729;
        k2 *= c2; k2  = ((k2 << 33) | (k2 >> 31)); k2 *= c1; h2 ^= k2;
        h2 = ((h2 << 31) | (h2 >> 33)); h2 += h1; h2 = h2*5+0x38495ab5;
    }
    k1 = k2 = 0;
    const uint8_t *tail = (const uint8_t*)((unsigned long)s + (nblocks << 4));
    switch(len & 15) {
        case 15: k2 ^= (uint64_t)(tail[14]) << 48; /* fall through */
        case 14: k2 ^= (uint64_t)(tail[13]) << 40; /* fall through */
        case 13: k2 ^= (uint64_t)(tail[12]) << 32; /* fall through */
        case 12: k2 ^= (uint64_t)(tail[11]) << 24; /* fall through */
        case 11: k2 ^= (uint64_t)(tail[10]) << 16; /* fall through */
        case 10: k2 ^= (uint64_t)(tail[ 9]) << 8; /* fall through */
        case  9: k2 ^= (uint64_t)(tail[ 8]) << 0;
                 k2 *= c2; k2 = ((k2 << 33) | (k2 >> 31)); k2 *= c1; h2 ^= k2; /* fall through */
        case  8: k1 ^= (uint64_t)(tail[ 7]) << 56; /* fall through */
        case  7: k1 ^= (uint64_t)(tail[ 6]) << 48; /* fall through */
        case  6: k1 ^= (uint64_t)(tail[ 5]) << 40; /* fall through */
        case  5: k1 ^= (uint64_t)(tail[ 4]) << 32; /* fall through */
        case  4: k1 ^= (uint64_t)(tail[ 3]) << 24; /* fall through */
        case  3: k1 ^= (uint64_t)(tail[ 2]) << 16; /* fall through */
        case  2: k1 ^= (uint64_t)(tail[ 1]) << 8; /* fall through */
        case  1: k1 ^= (uint64_t)(tail[ 0]) << 0;
                 k1 *= c1; k1 = ((k1 << 31) | (k1 >> 33)); k1 *= c2; h1 ^= k1; /* fall through */
        default: break;
    };
    h1 ^= len; h2 ^= len;
    h1 += h2; h2 += h1;
    h1 ^= h1 >> 33; h1 *= BIG_CONSTANT(0xff51afd7ed558ccd);
    h1 ^= h1 >> 33; h1 *= BIG_CONSTANT(0xc4ceb9fe1a85ec53);
    h1 ^= h1 >> 33;
    h2 ^= h2 >> 33; h2 *= BIG_CONSTANT(0xff51afd7ed558ccd);
    h2 ^= h2 >> 33; h2 *= BIG_CONSTANT(0xc4ceb9fe1a85ec53);
    h2 ^= h2 >> 33;
    h1 += h2; h2 += h1;
    ((uint64_t*)r)[0] = h1;
    ((uint64_t*)r)[1] = h2;
}
