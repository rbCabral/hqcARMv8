#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "api.h"
#include "hqc.h"
#include "parameters.h"
#include "symmetric.h" 
#include "parsing.h"
#include "vector.h"
#include "code.h" 
#include "gf2x.h"

#define CORRECT_FOR_OVERHEAD
#include "bench.h"

uint8_t ek_kem[PUBLIC_KEY_BYTES] = {0};
uint8_t dk_kem[SECRET_KEY_BYTES] = {0};

uint8_t m[PARAM_SECURITY_BYTES] = {0};
uint8_t m_prime[PARAM_SECURITY_BYTES] = {0};
uint8_t theta[SEED_BYTES] = {0};
uint8_t dk_pke[SEED_BYTES] = {0};

ciphertext_kem_t c_pke_out = {0};

void setup_crypto_state(void) {
    crypto_kem_keypair(ek_kem, dk_kem);
    
    memcpy(dk_pke, dk_kem + PUBLIC_KEY_BYTES, SEED_BYTES);
    
    hqc_pke_encrypt(&c_pke_out.c_pke, ek_kem, (uint64_t *)m, theta);
}

static inline void bench_pke_enc(size_t iters) {
    for (size_t i = 0; i < iters; i++) {
        hqc_pke_encrypt(&c_pke_out.c_pke, ek_kem, (uint64_t *)m, theta);
    }
}

static inline void bench_pke_dec(size_t iters) {
    for (size_t i = 0; i < iters; i++) {
        hqc_pke_decrypt((uint64_t *)m_prime, dk_pke, &c_pke_out.c_pke);
    }
}

#define WARMUP_ITERS 8
#define BENCH_ITERS 32
#define RUN_ITERS 100 

int bench_main(int argc, char *argv[]) {
    uint64_t cycles_min, cycles_median;

    int run_all = (argc == 1);
    int run_pkeEnc    = run_all || (argc > 1 && strcmp(argv[1], "pkeEnc") == 0);
    int run_pkeDec    = run_all || (argc > 1 && strcmp(argv[1], "pkeDec") == 0);

    if (!run_all) {
        printf("Running specific benchmark: %s\n", argv[1]);
    }

    printf("Generating keys and preparing the state...\n");
    setup_crypto_state();

    printf("\n----------------------------------------------------------------------\n");
    printf("Results\n");
    printf("----------------------------------------------------------------------\n");

    if (run_pkeEnc) {
        bench_run(bench_pke_enc(RUN_ITERS), BENCH_ITERS, WARMUP_ITERS, cycles_min, cycles_median);
        printf("[PKE]  Encrypt         : %.0f ciclos\n", (double)cycles_min / RUN_ITERS);

    }

    if(run_pkeDec) {
        bench_run(bench_pke_dec(RUN_ITERS), BENCH_ITERS, WARMUP_ITERS, cycles_min, cycles_median);
        printf("[PKE]  Decrypt         : %.0f ciclos\n", (double)cycles_min / RUN_ITERS);
    }


    printf("----------------------------------------------------------------------\n");

    (void)cycles_median;

    return 0;
}