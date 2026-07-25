#include "bench.h"

#include <inttypes.h>
#include <stdint.h>
#include <stdlib.h>

#ifdef __APPLE__
#include "bench_macos.h"
#endif

#ifndef BENCH_SP_ALIGNMENT
#define BENCH_SP_ALIGNMENT 4096
#endif

// #define BENCH_DEBUG_ALIGNMENT
void set_dit_bit(void);
void set_dit_bit(void) {
    uint64_t dit = 1 << 24;
    __asm__ volatile("msr s3_3_c4_c2_5, %0" : : "r"(dit));

    dit = 0;

    __asm__ volatile("mrs %0, s3_3_c4_c2_5" : "=r"(dit));

    if (dit != 1 << 24) {
        exit(1);
    }
}


int bench_main(int argc, char *argv[]);

int cmp_u64(const void *a, const void *b) {
    uint64_t aa = *(const uint64_t *)a;
    uint64_t bb = *(const uint64_t *)b;
    if (aa < bb) return -1;
    if (aa > bb) return 1;
    return 0;
}

void bench_estimate_overhead(size_t ITERS, uint64_t *OVERHEAD_MIN, uint64_t *OVERHEAD_MEDIAN) {
    counter_t counter_start, counter_end;
    uint64_t overhead[ITERS];

    for (size_t i = 0; i < ITERS; i++) {
        bench_start(counter_start);
        bench_end(counter_end);
        overhead[i] = bench_get_cycles(counter_start, counter_end);
    }

    qsort(overhead, ITERS, sizeof(uint64_t), cmp_u64);

    *OVERHEAD_MIN = overhead[0];
    *OVERHEAD_MEDIAN = overhead[ITERS / 2];

    printf("Estimated overhead: min %" PRIu64 ", median %" PRIu64 "\n", *OVERHEAD_MIN, *OVERHEAD_MEDIAN);
}

int bench_setup_and_run(int argc, char *argv[]) {
#ifdef BENCH_DEBUG_ALIGNMENT
    printf("[BENCH_DEBUG_ALIGNMENT] sp = %p, bench_main = %p\n", (void *)bench_get_sp(), (void *)bench_main);
#endif

    int ret;
    if ((ret = bench_init())) {
        exit(ret);
    }

    ret = bench_main(argc, argv);

    bench_deinit();

    return ret;
}

int main(int argc, char *argv[]) {
    // Pad the stack to the required alignment
    // A first attempt was made by declaring the pointer as volatile, but for some reason it didn't work; the
    // DoNotOptimize() macro appears to do the trick
    set_dit_bit();
    void *pad = alloca(bench_get_sp() % BENCH_SP_ALIGNMENT);
    DoNotOptimize(pad);

    return bench_disable_aslr(bench_setup_and_run, argc, argv);
}
