#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "parameters.h"
#include "reed_muller.h"
#include "reed_solomon.h"
#include "code.h"

#define CORRECT_FOR_OVERHEAD
#include "bench.h"

uint64_t em_buffer[VEC_N1N2_SIZE_64] = {0};
uint64_t tmp_buffer[VEC_N1_SIZE_64] = {0};
uint8_t m_buffer[PARAM_SECURITY_BYTES] = {0}; 

void setup_code_state(void) {
    for(size_t i = 0; i < VEC_N1N2_SIZE_64; i++) {
        em_buffer[i] = (uint64_t)rand();
    }
    for(size_t i = 0; i < VEC_N1_SIZE_64; i++) {
        tmp_buffer[i] = (uint64_t)rand();
    }
    for(size_t i = 0; i < PARAM_SECURITY_BYTES; i++) {
        m_buffer[i] = (uint8_t)rand();
    }
}


static inline void bench_rs_encode(size_t iters) {
    for (size_t i = 0; i < iters; i++) {
        reed_solomon_encode(tmp_buffer, m_buffer);
    }
}

static inline void bench_rm_encode(size_t iters) {
    for (size_t i = 0; i < iters; i++) {
        reed_muller_encode(em_buffer, tmp_buffer);
    }
}

static inline void bench_rm_decode(size_t iters) {
    for (size_t i = 0; i < iters; i++) {
        reed_muller_decode(tmp_buffer, em_buffer);
    }
}

static inline void bench_rs_decode(size_t iters) {
    for (size_t i = 0; i < iters; i++) {
        reed_solomon_decode((uint64_t *)m_buffer, tmp_buffer);
    }
}

#define WARMUP_ITERS 8
#define BENCH_ITERS 32
#define RUN_ITERS 250 

int bench_main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;

    uint64_t cycles_min, cycles_median;

    setup_code_state();

    printf("\n----------------------------------------------------------------------\n");
    printf("Encoding\n");
    printf("----------------------------------------------------------------------\n");

    bench_run(bench_rs_encode(RUN_ITERS), BENCH_ITERS, WARMUP_ITERS, cycles_min, cycles_median);
    printf("[CODE] Reed-Solomon Encode : %.0f ciclos\n", (double)cycles_min / RUN_ITERS);

    bench_run(bench_rm_encode(RUN_ITERS), BENCH_ITERS, WARMUP_ITERS, cycles_min, cycles_median);
    printf("[CODE] Reed-Muller Encode  : %.0f ciclos\n", (double)cycles_min / RUN_ITERS);

    printf("\n----------------------------------------------------------------------\n");
    printf("Decoding)\n");
    printf("----------------------------------------------------------------------\n");

    bench_run(bench_rm_decode(RUN_ITERS), BENCH_ITERS, WARMUP_ITERS, cycles_min, cycles_median);
    printf("[CODE] Reed-Muller Decode  : %.0f ciclos\n", (double)cycles_min / RUN_ITERS);

    bench_run(bench_rs_decode(RUN_ITERS), BENCH_ITERS, WARMUP_ITERS, cycles_min, cycles_median);
    printf("[CODE] Reed-Solomon Decode : %.0f ciclos\n", (double)cycles_min / RUN_ITERS);

    printf("----------------------------------------------------------------------\n");

    (void)cycles_median;

    return 0;
}