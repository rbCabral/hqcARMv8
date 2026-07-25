
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "parameters.h"
#include "gf2x.h"

#define CORRECT_FOR_OVERHEAD
#include "bench.h"

uint64_t poly_a[VEC_N_SIZE_64] = {0};
uint64_t poly_b[VEC_N_SIZE_64] = {0};
uint64_t poly_c[VEC_N_SIZE_64] = {0};

void setup_poly_state(void) {
    for(size_t j = 0; j < VEC_N_SIZE_64; j++) {
        poly_a[j] = rand();
        poly_b[j] = rand();
    }
}

static inline void bench_vect_mul(size_t iters) {
    for (size_t i = 0; i < iters; i++) {
        vect_mul((void*)poly_c, (void*)poly_a, (void*)poly_b);
    }
}

#define WARMUP_ITERS 8
#define BENCH_ITERS 32
#define RUN_ITERS 500 

int bench_main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;

    uint64_t cycles_min, cycles_median;

    printf("Benchmark for gf2x [%d] bits -> [%d] u64 \n\n", PARAM_N, VEC_N_SIZE_64);
    printf("Generating polynomials and preparing the state...\n");
    
    setup_poly_state();

    printf("\n----------------------------------------------------------------------\n");
    printf("Results\n");
    printf("----------------------------------------------------------------------\n");

    bench_run(bench_vect_mul(RUN_ITERS), BENCH_ITERS, WARMUP_ITERS, cycles_min, cycles_median);
    
    printf("[GF2X] vect_mul        : %.0f ciclos\n", (double)cycles_min / RUN_ITERS);

    printf("----------------------------------------------------------------------\n");

    (void)cycles_median;

    printf("XX: %x\n", (unsigned)(poly_c[0] & 0xffff));

    return 0;
}