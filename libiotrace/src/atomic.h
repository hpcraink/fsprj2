/* 
 * File:   atomic.h
 * Author: hpcraink
 *
 * Created on December 30, 2018, 4:59 PM
 */

#ifndef LIBIOTRACE_ATOMIC_H
#define LIBIOTRACE_ATOMIC_H

#ifdef HAVE_STDBOOL_H
#  include <stdbool.h>
#endif
#ifdef HAVE_STDINT_H
#  include <stdint.h>
#endif

#include "libiotrace_config.h"

BEGIN_C_DECLS

#define ATOMIC_COMPILE_TIME_ASSERT(expr_) \
    do { switch(0) { case 0: case (expr_): default: break; } } while (0)

/**
 * Atomically Compare-Swap-16 Bytes (128 Bit compare-exchange)
 * 
 * This function returns true, if it can successfully exchange the memory
 * atomically at addr: the memory is checked to contain value "compare" and is
 * replaced with the value "with".
 * 
 * @param[in] address    Memory location to atomically check the value
 * @param[in] compare    Value to compare with
 * @param[in] with       Value to exchange with
 * @return true          Iff expected value could be replaced
 *         false         Otherwise
 */
static inline ATTRIBUTE_NONNULL(1)
bool atomic_cas128(volatile __uint128_t * address, const __uint128_t compare, const __uint128_t with) {
    bool ret;
    uint64_t cmp_hi = compare >> 64;
    uint64_t cmp_lo = compare & (((__uint128_t)0x1 << 64) -1);
    const uint64_t with_hi = with >> 64;
    const uint64_t with_lo = with & (((__uint128_t)0x1 << 64) -1);

    asm volatile (
        "lock cmpxchg16b %[address]\n\t"
        "setz %[ret]"
        : /* INOUT */
        [ret] "=q" (ret),
        [address] "+m" (*address),
        "+d" (cmp_hi),          // Store & expect high 64-bit in RDX
        "+a" (cmp_lo)           // Store & expect low 64-bit in RAX
        : /* INPUT-ONLY */
        "c" (with_hi),          // Store high 64-bit of with into RCX
        "b" (with_lo)           // Store low 64-bit of with into RBX
        : /* CLOBBERS */
        "cc", "memory"
    );
    return ret;
}


/**
 * Atomically add 32 Bit value to variable.
 * 
 * @param[in] address     Pointer to a 32 bit integer.
 * @param[in] value       A 32-bit value to add.
 * 
 * @return Returns the OLD value before the operation.
 */
static inline ATTRIBUTE_NONNULL(1)
uint32_t atomic_add32(int32_t * address, int32_t value)
{
    __asm__ volatile("lock xaddl %[value], %[address]\n\t"
        : /* INOUT*/
        [value] "+r" (value),
        [address] "+m" (*address)
        : /* NO INPUT-ONLY */
        : /* CLOBBERS */
        "memory"
    );
    return value;
}

/**
 * Atomically add 64 Bit value to variable.
 * 
 * @param[in] address     Pointer to a 64 bit integer.
 * @param[in] value       A 64-bit value to add.
 * 
 * @return Returns the OLD value before the operation
 */
static inline ATTRIBUTE_NONNULL(1)
int64_t atomic_add64(int64_t * address, int64_t value)
{
    __asm__ volatile("lock xaddq %[value], %[address]\n\t"
        : /* INOUT*/
        [value] "+r" (value),
        [address] "+m" (*address)
        : /* NO INPUT-ONLY */
        : /* CLOBBERS */
        "memory"
    );
    return value;
}

#define atomic_add(x, y) (sizeof(*x) == 4) ? atomic_add32((int32_t*)x, y) : atomic_add64((int64_t*)x, y)

END_C_DECLS

#endif /* LIBIOTRACE_ATOMIC_H */

