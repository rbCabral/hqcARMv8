#ifndef BENCH_H
#define BENCH_H

#include <inttypes.h>
#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <time.h>

#ifdef __APPLE__
#include "bench_macos.h"
#elif __linux__
#include "bench_linux.h"
#else
#error "Unsupported OS"
#endif

#ifndef RT_NS
#define RT_NS UINT64_C(0)
#endif

#ifdef __clang__
#define DoNotOptimize(x) __asm__ __volatile__("" : "+r,m"(x) : : "memory");
#elif !defined(__clang__) && __GNUC__ <= 8
#define DoNotOptimize(x) asm volatile("" : : "m,r"(x) : "memory");
#else
#define DoNotOptimize(x) asm volatile("" : "+m,r"(x) : : "memory");
#endif

#if defined(__aarch64__)
#define DoNotOptimizeVec(x) asm volatile("" : "+w"(x))
#elif defined(__x86_64__)
#define DoNotOptimizeVec(x) asm volatile("" : "+x"(x))
#else
#define DoNotOptimizeVec(x) DoNotOptimize(x)
#endif

int cmp_u64(const void *a, const void *b);
void bench_estimate_overhead(size_t ITERS, uint64_t *OVERHEAD_MIN, uint64_t *OVERHEAD_MEDIAN);

#define BENCH_RUN_MAIN(CODE, ITERS, WARMUP_ITERS, CYCLES_MIN, CYCLES_MEDIAN)                                   \
    do {                                                                                                       \
        uint64_t cycles[ITERS];                                                                                \
        counter_t counter_start, counter_end;                                                                  \
        struct timespec start, end;                                                                            \
        uint64_t nsec;                                                                                         \
        struct rusage usage_start, usage_end;                                                                  \
        long voluntary_csw, involuntary_csw;                                                                   \
                                                                                                               \
        sched_yield();                                                                                         \
                                                                                                               \
        clock_gettime(CLOCK_MONOTONIC, &start);                                                                \
        getrusage(RUSAGE_SELF, &usage_start);                                                                  \
                                                                                                               \
        for (size_t i = 0; i < WARMUP_ITERS; i++) {                                                            \
            CODE;                                                                                              \
        }                                                                                                      \
                                                                                                               \
        for (size_t i = 0; i < ITERS; i++) {                                                                   \
            bench_start(counter_start);                                                                        \
            CODE;                                                                                              \
            bench_end(counter_end);                                                                            \
            cycles[i] = bench_get_cycles(counter_start, counter_end);                                          \
        }                                                                                                      \
                                                                                                               \
        getrusage(RUSAGE_SELF, &usage_end);                                                                    \
        clock_gettime(CLOCK_MONOTONIC, &end);                                                                  \
                                                                                                               \
        nsec = UINT64_C(1000000000) * (end.tv_sec - start.tv_sec) + end.tv_nsec - start.tv_nsec;               \
        voluntary_csw = usage_end.ru_nvcsw - usage_start.ru_nvcsw;                                             \
        involuntary_csw = usage_end.ru_nivcsw - usage_start.ru_nivcsw;                                         \
                                                                                                               \
        if (voluntary_csw > 0 || involuntary_csw > 0) {                                                        \
            printf("***************************************************************************\n");           \
            printf(                                                                                            \
                "WARNING: one or more context switches occurred during benchmarking "                          \
                "(voluntary = %ld, involuntary = %ld), consider disregarding this run\n",                      \
                voluntary_csw, involuntary_csw);                                                               \
                                                                                                               \
            if (RT_NS > 0 && nsec > RT_NS) {                                                                   \
                printf("This may have happened because the benchmark execution time of %" PRIu64 ".%06" PRIu64 \
                       " ms exceeded the real-time quantum limit of %" PRIu64 ".%06" PRIu64 " ms\n",           \
                       nsec / 1000000, nsec % 1000000, RT_NS / 1000000, RT_NS % 1000000);                      \
            }                                                                                                  \
            printf("***************************************************************************\n");           \
        }                                                                                                      \
                                                                                                               \
        sched_yield();                                                                                         \
                                                                                                               \
        qsort(cycles, ITERS, sizeof(uint64_t), cmp_u64);                                                       \
                                                                                                               \
        CYCLES_MIN = cycles[0];                                                                                \
        CYCLES_MEDIAN = cycles[ITERS / 2];                                                                     \
    }                                                                                                          \
    while (0)

#ifdef CORRECT_FOR_OVERHEAD
#define bench_run(CODE, ITERS, WARMUP_ITERS, CYCLES_MIN, CYCLES_MEDIAN)       \
    do {                                                                      \
        BENCH_RUN_MAIN(CODE, ITERS, WARMUP_ITERS, CYCLES_MIN, CYCLES_MEDIAN); \
                                                                              \
        uint64_t overhead_min, overhead_median;                               \
                                                                              \
        bench_estimate_overhead(1024, &overhead_min, &overhead_median);       \
                                                                              \
        CYCLES_MIN -= overhead_min;                                           \
        CYCLES_MEDIAN -= overhead_median;                                     \
    }                                                                         \
    while (0)
#else
#define bench_run(CODE, ITERS, WARMUP_ITERS, CYCLES_MIN, CYCLES_MEDIAN) \
    BENCH_RUN_MAIN(CODE, ITERS, WARMUP_ITERS, CYCLES_MIN, CYCLES_MEDIAN)
#endif

#endif  // BENCH_H
