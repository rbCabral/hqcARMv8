
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>  // uint64_t
#include <stdlib.h>  // rand()

#include "parameters.h"
#include "reed_muller.h"
#include "reed_solomon.h"
#include "code.h"


#define TEST_RUN 1000


#include "parameters.h"
// void vect_mul(__m256i *o, const __m256i *v1, const __m256i *v2);


int main(void) {

    uint64_t v[VEC_N1N2_SIZE_64] = {0};
    uint8_t m[VEC_K_SIZE_BYTES] = {0};
    uint64_t tmp[VEC_N1_SIZE_64] = {0};
    uint8_t m2[VEC_K_SIZE_BYTES] = {0};
    
        

    printf("===========code_decode test()  ================\n\n");
    for (unsigned i = 0; i < TEST_RUN; i++) {
        for (int i = 0; i < VEC_K_SIZE_BYTES; i++) {
            m[i] = rand() % 256; // Random message
        }
        reed_solomon_encode(tmp, (uint64_t*)m);
        reed_muller_encode(v, tmp);
        reed_muller_decode(tmp, v);
        reed_solomon_decode((uint64_t*)m2, tmp);
        for(int i = 0; i < VEC_K_SIZE_BYTES; i++) {
            if(m[i] != m2[i]) {
                printf("Error: m != m2\n");
                return -1;
            }
        }
        
    }
    printf("All tests passed!\n");

    return 0;
}

