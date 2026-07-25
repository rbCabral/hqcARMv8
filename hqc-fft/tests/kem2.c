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
uint8_t c_kem_out[CIPHERTEXT_BYTES] = {0};
uint8_t K[SHARED_SECRET_BYTES] = {0};
uint8_t K_prime[SHARED_SECRET_BYTES] = {0};

void setup_crypto_state(void) {
    crypto_kem_keypair(ek_kem, dk_kem);
    
    crypto_kem_enc(c_kem_out, K, ek_kem);
}

static inline void bench_keypair(size_t iters) {
    for (size_t i = 0; i < iters; i++) {
        crypto_kem_keypair(ek_kem, dk_kem);    
    }
}

static inline void bench_kem_enc(size_t iters) {
    for (size_t i = 0; i < iters; i++) {
        crypto_kem_enc(c_kem_out, K, ek_kem);
    }
}

static inline void bench_kem_dec(size_t iters) {
    for (size_t i = 0; i < iters; i++) {
        crypto_kem_dec(K_prime, c_kem_out, dk_kem);
    }
}

#define WARMUP_ITERS 8
#define BENCH_ITERS 32
#define RUN_ITERS 100

int bench_main(int argc, char *argv[]) {
    uint64_t cycles_min, cycles_median;

    int run_all = (argc == 1);
    int run_keygen = run_all || (argc > 1 && strcmp(argv[1], "keygen") == 0);
    int run_enc    = run_all || (argc > 1 && strcmp(argv[1], "enc") == 0);
    int run_dec    = run_all || (argc > 1 && strcmp(argv[1], "dec") == 0);

    if (!run_all) {
        printf("Running specific benchmark: %s\n", argv[1]);
    }

    printf("Generating keys and preparing the state...\n");
    setup_crypto_state();

    printf("\n----------------------------------------------------------------------\n");
    printf("Results (Outer Samples: %d, Inner Loop: %d)\n", BENCH_ITERS, RUN_ITERS);
    printf("----------------------------------------------------------------------\n");

    if (run_keygen) {
        bench_run(bench_keypair(RUN_ITERS), BENCH_ITERS, WARMUP_ITERS, cycles_min, cycles_median);
        printf("[KEM]  Keypair         : %.0f ciclos\n", (double)cycles_min / RUN_ITERS);
    }

    if (run_enc) {
        bench_run(bench_kem_enc(RUN_ITERS), BENCH_ITERS, WARMUP_ITERS, cycles_min, cycles_median);
        printf("[KEM]  Encapsulate     : %.0f ciclos\n", (double)cycles_min / RUN_ITERS);
    }

    if (run_dec) {
        bench_run(bench_kem_dec(RUN_ITERS), BENCH_ITERS, WARMUP_ITERS, cycles_min, cycles_median);
        printf("[KEM]  Decapsulate     : %.0f ciclos\n", (double)cycles_min / RUN_ITERS);
    }

    printf("----------------------------------------------------------------------\n");    
    
    (void)cycles_median;

    if (K_prime[0] == 0xDE) printf(" ");
    if (c_kem_out[0] == 0xDE) printf(" ");
    if (ek_kem[0] == 0xDE) printf(" ");

    return 0;
}