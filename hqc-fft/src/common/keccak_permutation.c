#include <arm_neon.h>
#include <stddef.h>
#include <stdint.h>

#include "keccak_permutation.h"

// #define KECCAK_ROUNDS 24

static const uint64_t RC[KECCAK_ROUNDS] = {
    0x0000000000000001ULL, 0x0000000000008082ULL, 0x800000000000808aULL, 0x8000000080008000ULL, 0x000000000000808bULL,
    0x0000000080000001ULL, 0x8000000080008081ULL, 0x8000000000008009ULL, 0x000000000000008aULL, 0x0000000000000088ULL,
    0x0000000080008009ULL, 0x000000008000000aULL, 0x000000008000808bULL, 0x800000000000008bULL, 0x8000000000008089ULL,
    0x8000000000008003ULL, 0x8000000000008002ULL, 0x8000000000000080ULL, 0x000000000000800aULL, 0x800000008000000aULL,
    0x8000000080008081ULL, 0x8000000000008080ULL, 0x0000000080000001ULL, 0x8000000080008008ULL};

#define O00 0
#define O01 36
#define O02 3
#define O03 41
#define O04 18
#define O10 1
#define O11 44
#define O12 10
#define O13 45
#define O14 2
#define O20 62
#define O21 6
#define O22 43
#define O23 15
#define O24 61
#define O30 28
#define O31 55
#define O32 25
#define O33 21
#define O34 56
#define O40 27
#define O41 20
#define O42 39
#define O43 8
#define O44 14

#define XOR5(a, b, c, d, e) veor3q_u64(veor3q_u64(a, b, c), d, e)
#define XAR(a, b, off) vxarq_u64(a, b, 64 - (off))
#define BCAX(a, b, c) vbcaxq_u64(a, b, c)

void keccak_f1600(uint64_t state[25]) {
    uint64x2_t a00, a01, a02, a03, a04;
    uint64x2_t a10, a11, a12, a13, a14;
    uint64x2_t a20, a21, a22, a23, a24;
    uint64x2_t a30, a31, a32, a33, a34;
    uint64x2_t a40, a41, a42, a43, a44;
    uint64x2_t b00, b01, b02, b03, b04;
    uint64x2_t b10, b11, b12, b13, b14;
    uint64x2_t b20, b21, b22, b23, b24;
    uint64x2_t b30, b31, b32, b33, b34;
    uint64x2_t b40, b41, b42, b43, b44;
    uint64x2_t c0, c1, c2, c3, c4;
    uint64x2_t d0, d1, d2, d3, d4;

    // load state into registers
    a00 = vld1q_u64(state + 0);
    a01 = vld1q_u64(state + 1);
    a02 = vld1q_u64(state + 2);
    a03 = vld1q_u64(state + 3);
    a04 = vld1q_u64(state + 4);
    a10 = vld1q_u64(state + 5);
    a11 = vld1q_u64(state + 6);
    a12 = vld1q_u64(state + 7);
    a13 = vld1q_u64(state + 8);
    a14 = vld1q_u64(state + 9);
    a20 = vld1q_u64(state + 10);
    a21 = vld1q_u64(state + 11);
    a22 = vld1q_u64(state + 12);
    a23 = vld1q_u64(state + 13);
    a24 = vld1q_u64(state + 14);
    a30 = vld1q_u64(state + 15);
    a31 = vld1q_u64(state + 16);
    a32 = vld1q_u64(state + 17);
    a33 = vld1q_u64(state + 18);
    a34 = vld1q_u64(state + 19);
    a40 = vld1q_u64(state + 20);
    a41 = vld1q_u64(state + 21);
    a42 = vld1q_u64(state + 22);
    a43 = vld1q_u64(state + 23);
    a44 = vcombine_u64((uint64x1_t){state[24]}, (uint64x1_t){0});  // a44 = vld1q_u64(state+24);

    for (size_t i = 0; i < KECCAK_ROUNDS; i++) {
        c0 = XOR5(a00, a10, a20, a30, a40);
        c1 = XOR5(a01, a11, a21, a31, a41);
        c2 = XOR5(a02, a12, a22, a32, a42);
        c3 = XOR5(a03, a13, a23, a33, a43);
        c4 = XOR5(a04, a14, a24, a34, a44);
        d1 = vrax1q_u64(c0, c2);
        d2 = vrax1q_u64(c1, c3);
        d3 = vrax1q_u64(c2, c4);
        d4 = vrax1q_u64(c3, c0);
        d0 = vrax1q_u64(c4, c1);

        b00 = veorq_u64(a00, d0); // b00 = vxarq_u64(a00,d0,64-O00);
        b20 = XAR(a01, d1, O10);
        b40 = XAR(a02, d2, O20);
        b10 = XAR(a03, d3, O30);
        b30 = XAR(a04, d4, O40);
        
        b31 = XAR(a10, d0, O01);
        b01 = XAR(a11, d1, O11);
        b21 = XAR(a12, d2, O21);
        b41 = XAR(a13, d3, O31);
        b11 = XAR(a14, d4, O41);
        
        b12 = XAR(a20, d0, O02);
        b32 = XAR(a21, d1, O12);
        b02 = XAR(a22, d2, O22);
        b22 = XAR(a23, d3, O32);
        b42 = XAR(a24, d4, O42);
        
        b43 = XAR(a30, d0, O03);
        b13 = XAR(a31, d1, O13);
        b33 = XAR(a32, d2, O23);
        b03 = XAR(a33, d3, O33);
        b23 = XAR(a34, d4, O43);
        
        b24 = XAR(a40, d0, O04);
        b44 = XAR(a41, d1, O14);
        b14 = XAR(a42, d2, O24);
        b34 = XAR(a43, d3, O34);
        b04 = XAR(a44, d4, O44);

        a00 = BCAX(b00, b02, b01);
        a10 = BCAX(b10, b12, b11);
        a20 = BCAX(b20, b22, b21);
        a30 = BCAX(b30, b32, b31);
        a40 = BCAX(b40, b42, b41);
        
        a01 = BCAX(b01, b03, b02);
        a11 = BCAX(b11, b13, b12);
        a21 = BCAX(b21, b23, b22);
        a31 = BCAX(b31, b33, b32);
        a41 = BCAX(b41, b43, b42);

        a02 = BCAX(b02, b04, b03);
        a12 = BCAX(b12, b14, b13);
        a22 = BCAX(b22, b24, b23);
        a32 = BCAX(b32, b34, b33);
        a42 = BCAX(b42, b44, b43);

        a03 = BCAX(b03, b00, b04);
        a13 = BCAX(b13, b10, b14);
        a23 = BCAX(b23, b20, b24);
        a33 = BCAX(b33, b30, b34);
        a43 = BCAX(b43, b40, b44);

        a04 = BCAX(b04, b01, b00);
        a14 = BCAX(b14, b11, b10);
        a24 = BCAX(b24, b21, b20);
        a34 = BCAX(b34, b31, b30);
        a44 = BCAX(b44, b41, b40);

        a00 = veorq_u64(a00, vld1q_dup_u64(RC + i));
    }
    vst1_u64(state + 0, vget_low_u64(a00));
    vst1_u64(state + 1, vget_low_u64(a01));
    vst1_u64(state + 2, vget_low_u64(a02));
    vst1_u64(state + 3, vget_low_u64(a03));
    vst1_u64(state + 4, vget_low_u64(a04));
    vst1_u64(state + 5, vget_low_u64(a10));
    vst1_u64(state + 6, vget_low_u64(a11));
    vst1_u64(state + 7, vget_low_u64(a12));
    vst1_u64(state + 8, vget_low_u64(a13));
    vst1_u64(state + 9, vget_low_u64(a14));
    vst1_u64(state + 10, vget_low_u64(a20));
    vst1_u64(state + 11, vget_low_u64(a21));
    vst1_u64(state + 12, vget_low_u64(a22));
    vst1_u64(state + 13, vget_low_u64(a23));
    vst1_u64(state + 14, vget_low_u64(a24));
    vst1_u64(state + 15, vget_low_u64(a30));
    vst1_u64(state + 16, vget_low_u64(a31));
    vst1_u64(state + 17, vget_low_u64(a32));
    vst1_u64(state + 18, vget_low_u64(a33));
    vst1_u64(state + 19, vget_low_u64(a34));
    vst1_u64(state + 20, vget_low_u64(a40));
    vst1_u64(state + 21, vget_low_u64(a41));
    vst1_u64(state + 22, vget_low_u64(a42));
    vst1_u64(state + 23, vget_low_u64(a43));
    vst1_u64(state + 24, vget_low_u64(a44));
}