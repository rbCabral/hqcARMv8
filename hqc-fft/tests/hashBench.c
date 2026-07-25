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

uint8_t hash_ek_kem[SEED_BYTES] = {0};
uint8_t sigma[PARAM_SECURITY_BYTES] = {0};
uint8_t K_bar[SHARED_SECRET_BYTES] = {0};
uint8_t K_dummy[SHARED_SECRET_BYTES] = {0};

ciphertext_kem_t c_kem_t = {0}; 

uint64_t parsed_pk_h[VEC_N_SIZE_64] = {0};
uint64_t parsed_pk_s[VEC_N_SIZE_64] = {0};
uint64_t parsed_sk_y[VEC_N_SIZE_64] = {0};

uint8_t bench_theta[SEED_BYTES] = {0x42}; 
uint64_t bench_sample_out[VEC_N_SIZE_64] = {0};

uint8_t m[PARAM_SECURITY_BYTES] = {0}; 
uint8_t K_theta[SHARED_SECRET_BYTES + SEED_BYTES] = {0};

void setup_crypto_state(void) {
    crypto_kem_keypair(ek_kem, dk_kem);
    
    memcpy(sigma, dk_kem + PUBLIC_KEY_BYTES + SEED_BYTES, PARAM_SECURITY_BYTES);
    
    hash_h(hash_ek_kem, ek_kem);

    crypto_kem_enc((uint8_t *)&c_kem_t, K_dummy, ek_kem);
}


static inline void bench_ek_pke_from_string(size_t iters) {
    for (size_t i = 0; i < iters; i++) {
        hqc_ek_pke_from_string(parsed_pk_h, parsed_pk_s, ek_kem);
    }
}

static inline void bench_dk_pke_from_string(size_t iters) {
    for (size_t i = 0; i < iters; i++) {
        hqc_dk_pke_from_string(parsed_sk_y, dk_kem);
    }
}

static inline void bench_hash_h(size_t iters) {
    for (size_t i = 0; i < iters; i++) {
        hash_h(hash_ek_kem, ek_kem);
    }
}

static inline void bench_hash_j(size_t iters) {
    for (size_t i = 0; i < iters; i++) {
        hash_j(K_bar, hash_ek_kem, sigma, &c_kem_t);
    }
}


static inline void bench_hash_g(size_t iters) {
    for (size_t i = 0; i < iters; i++) {
        hash_g(K_theta, hash_ek_kem, m, c_kem_t.salt);
    }
}

static inline void bench_sample_weight_r(size_t iters) {
    shake256_xof_ctx xof_ctx;
    
    xof_init(&xof_ctx, bench_theta, SEED_BYTES);

    for (size_t i = 0; i < iters; i++) {
        vect_sample_fixed_weight2(&xof_ctx, bench_sample_out, PARAM_OMEGA_R);
    }
}

static inline void bench_sample_weight_e(size_t iters) {
    shake256_xof_ctx xof_ctx;
    xof_init(&xof_ctx, bench_theta, SEED_BYTES);

    for (size_t i = 0; i < iters; i++) {
        vect_sample_fixed_weight2(&xof_ctx, bench_sample_out, PARAM_OMEGA_E);
    }
}

#define WARMUP_ITERS 8
#define BENCH_ITERS 32
#define RUN_ITERS 250 

int bench_main(int argc, char *argv[]) {
    
    uint64_t cycles_min, cycles_median;

int run_all = (argc == 1);
    
    int run_parse_ek = run_all || (argc > 1 && (strcmp(argv[1], "parse_ek") == 0 || strcmp(argv[1], "parse") == 0));
    int run_parse_dk = run_all || (argc > 1 && (strcmp(argv[1], "parse_dk") == 0 || strcmp(argv[1], "parse") == 0));
    int run_hash_h   = run_all || (argc > 1 && (strcmp(argv[1], "hash_h") == 0   || strcmp(argv[1], "hash") == 0));
    int run_hash_j   = run_all || (argc > 1 && (strcmp(argv[1], "hash_j") == 0   || strcmp(argv[1], "hash") == 0));
    int run_hash_g   = run_all || (argc > 1 && (strcmp(argv[1], "hash_g") == 0   || strcmp(argv[1], "hash") == 0));
    int run_sample   = run_all || (argc > 1 && strcmp(argv[1], "sample") == 0);

    if (!run_all) {
        printf("Running specific benchmark target: %s\n", argv[1]);
    }

    printf("Generating keys and preparing the state...\n");
    setup_crypto_state();

    printf("\n----------------------------------------------------------------------\n");
    printf("Results (Outer Samples: %d, Inner Loop: %d)\n", BENCH_ITERS, RUN_ITERS);
    printf("----------------------------------------------------------------------\n");

    if (run_parse_ek) {
        bench_run(bench_ek_pke_from_string(RUN_ITERS), BENCH_ITERS, WARMUP_ITERS, cycles_min, cycles_median);
        printf("[PARSE] ek_pke_from_string : %.0f ciclos\n", (double)cycles_min / RUN_ITERS);
    }

    if (run_parse_dk) {
        bench_run(bench_dk_pke_from_string(RUN_ITERS), BENCH_ITERS, WARMUP_ITERS, cycles_min, cycles_median);
        printf("[PARSE] dk_pke_from_string : %.0f ciclos\n", (double)cycles_min / RUN_ITERS);
    }

    if (run_parse_ek || run_parse_dk) {
        printf("----------------------------------------------------------------------\n");
    }

    if (run_hash_h) {
        bench_run(bench_hash_h(RUN_ITERS), BENCH_ITERS, WARMUP_ITERS, cycles_min, cycles_median);
        printf("[HASH]  Hash_H             : %.0f ciclos\n", (double)cycles_min / RUN_ITERS);
    }

    if (run_hash_j) {
        bench_run(bench_hash_j(RUN_ITERS), BENCH_ITERS, WARMUP_ITERS, cycles_min, cycles_median);
        printf("[HASH]  Hash_J             : %.0f ciclos\n", (double)cycles_min / RUN_ITERS);
    }

    if (run_hash_g) {
        bench_run(bench_hash_g(RUN_ITERS), BENCH_ITERS, WARMUP_ITERS, cycles_min, cycles_median);
        printf("[HASH]  Hash_G             : %.0f ciclos\n", (double)cycles_min / RUN_ITERS);
    }

    if (run_hash_h || run_hash_j || run_hash_g) {
        printf("----------------------------------------------------------------------\n");
    }

    if (run_sample) {
        bench_run(bench_sample_weight_r(RUN_ITERS), BENCH_ITERS, WARMUP_ITERS, cycles_min, cycles_median);
        printf("[SAMPLE] Fixed Weight (R)  : %.0f ciclos\n", (double)cycles_min / RUN_ITERS);

        bench_run(bench_sample_weight_e(RUN_ITERS), BENCH_ITERS, WARMUP_ITERS, cycles_min, cycles_median);
        printf("[SAMPLE] Fixed Weight (E)  : %.0f ciclos\n", (double)cycles_min / RUN_ITERS);
        
        printf("----------------------------------------------------------------------\n");
    }

    (void)cycles_median;

    if (parsed_sk_y[0] == 0xDEADBEEF) printf(" ");
    if (parsed_pk_h[0] == 0xDEADBEEF) printf(" ");
    if (hash_ek_kem[0] == 0xDE) printf(" ");
    if (K_theta[0] == 0xDE) printf(" ");
    if (K_bar[0] == 0xDE) printf(" ");

    return 0;
}