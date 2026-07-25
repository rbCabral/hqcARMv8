#include <stdio.h>
#include "api.h"
#include "parameters.h"

#include "stdint.h"

int vec_equal(unsigned char *a, unsigned char *b, int len) {
    for(int i = 0 ; i < len ; ++i) {
        if(a[i] != b[i]) return 0;
    }
    return 1;
}

#define TEST_RUN (1000)

int main(void) {

	printf("\n");
	printf("*********************\n");
	printf("**** HQC-%d-%d ****\n", PARAM_SECURITY, PARAM_DFR_EXP);
	printf("*********************\n");

	printf("\n");
	printf("N: %d   ", PARAM_N);
	printf("N1: %d   ", PARAM_N1);
	printf("N2: %d   ", PARAM_N2);
	printf("OMEGA: %d   ", PARAM_OMEGA);
	printf("OMEGA_R: %d   ", PARAM_OMEGA_R);
	printf("Failure rate: 2^-%d   ", PARAM_DFR_EXP);
	printf("Sec: %d bits", PARAM_SECURITY);
    printf("\n");

	printf("\n");

	unsigned char pk[PUBLIC_KEY_BYTES];
	unsigned char sk[SECRET_KEY_BYTES];
	unsigned char ct[CIPHERTEXT_BYTES];
	unsigned char key1[SHARED_SECRET_BYTES];
	unsigned char key2[SHARED_SECRET_BYTES];

    int passed = 1;
    for(int i=0;i<TEST_RUN;i++) {
    	crypto_kem_keypair(pk, sk);
        crypto_kem_enc(ct, key1, pk);
	    crypto_kem_dec(key2, ct, sk);

        if(!vec_equal(key1, key2, SHARED_SECRET_BYTES)) {
            printf("[%d] Error: key1 != key2\n", i );
            passed = 0;
            break;
        }
    }

	printf("\n\nsecret1: ");
	for(int i = 0 ; i < SHARED_SECRET_BYTES ; ++i) printf("%x", key1[i]);

	printf("\nsecret2: ");
	for(int i = 0 ; i < SHARED_SECRET_BYTES ; ++i) printf("%x", key2[i]);
	printf("\n\n");

    printf("TEST [%d] %s\n", TEST_RUN ,  passed ? "PASSED" : "FAILED");

	return 0;
}
