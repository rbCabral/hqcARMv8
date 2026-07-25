#ifndef BENCH_MACOS_H
#define BENCH_MACOS_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// Private stuff

// The maximum number of counters we could read from every class in one go.
// ARMV7: FIXED: 1, CONFIGURABLE: 4
// ARM32: FIXED: 2, CONFIGURABLE: 6
// ARM64: FIXED: 2, CONFIGURABLE: CORE_NCTRS - FIXED (6 or 8)
// x86: 32
#define KPC_MAX_COUNTERS 32

extern int (*kpc_get_thread_counters)(uint32_t tid, uint32_t buf_count, uint64_t *buf);

#define bench_macos_read_cycle_counter(counters)                              \
    do {                                                                      \
        int ret;                                                              \
        if ((ret = kpc_get_thread_counters(0, KPC_MAX_COUNTERS, counters))) { \
            printf("Failed get thread counters before: %d.\n", ret);          \
            exit(1);                                                          \
        }                                                                     \
    }                                                                         \
    while (0)

// Public API

typedef uint64_t counter_t[KPC_MAX_COUNTERS];

#define CLOCK_M1_GHZ 3.204
#define CLOCK_M3_MAX_GHZ 4.056
#define CLOCK_M4_GHZ 4.464
#define CLOCK_GHZ CLOCK_M3_MAX_GHZ

// Maximum RT quantum on macOS is 50ms -- see:
// https://github.com/apple-oss-distributions/xnu/blob/e3723e1f17661b24996789d8afc084c0c3303b26/osfmk/kern/sched_prim.c#L818
#define RT_NS (UINT64_C(50) * 1000 * 1000)  // 50ms

#define bench_get_sp()                             \
    ({                                             \
        register size_t sp;                        \
        __asm__ volatile("mov %0, sp" : "=r"(sp)); \
        sp;                                        \
    })

int bench_disable_aslr(int (*main_no_aslr)(int argc, char *argv[]), int argc, char *argv[]);

int bench_init(void);
void bench_deinit(void);

#define bench_start(counters) bench_macos_read_cycle_counter(counters)
#define bench_end(counters) bench_macos_read_cycle_counter(counters)

uint64_t bench_get_cycles(counter_t counter_start, counter_t counter_end);

#endif  // BENCH_MACOS_H
