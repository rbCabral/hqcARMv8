#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "fips202.h"

extern void keccak_f1600(uint64_t state[25]);

static inline uint64_t load64(const uint8_t *x) {
    return (uint64_t)x[0] | ((uint64_t)x[1] << 8) |
           ((uint64_t)x[2] << 16) | ((uint64_t)x[3] << 24) |
           ((uint64_t)x[4] << 32) | ((uint64_t)x[5] << 40) |
           ((uint64_t)x[6] << 48) | ((uint64_t)x[7] << 56);
}

static inline void store64(uint8_t *x, uint64_t u) {
    x[0] = (uint8_t)(u);
    x[1] = (uint8_t)(u >> 8);
    x[2] = (uint8_t)(u >> 16);
    x[3] = (uint8_t)(u >> 24);
    x[4] = (uint8_t)(u >> 32);
    x[5] = (uint8_t)(u >> 40);
    x[6] = (uint8_t)(u >> 48);
    x[7] = (uint8_t)(u >> 56);
}


static void keccak_absorb(uint64_t *s, uint32_t r, const uint8_t *m, size_t mlen, uint8_t p) {
    size_t i;
    uint8_t t[200];

    memset(s, 0, 25 * sizeof(uint64_t));

    while (mlen >= r) {
        for (i = 0; i < r / 8; ++i) {
            s[i] ^= load64(m + 8 * i);
        }
        keccak_f1600(s);
        mlen -= r;
        m += r;
    }

    memset(t, 0, r);
    for (i = 0; i < mlen; ++i) {
        t[i] = m[i];
    }
    t[mlen] = p;
    t[r - 1] |= 128;
    
    for (i = 0; i < r / 8; ++i) {
        s[i] ^= load64(t + 8 * i);
    }
}

static void keccak_squeezeblocks(uint8_t *h, size_t nblocks, uint64_t *s, uint32_t r) {
    while (nblocks > 0) {
        keccak_f1600(s);
        for (size_t i = 0; i < (r >> 3); i++) {
            store64(h + 8 * i, s[i]);
        }
        h += r;
        nblocks--;
    }
}

static void keccak_inc_init(uint64_t *s_inc) {
    memset(s_inc, 0, 26 * sizeof(uint64_t));
}

static void keccak_inc_absorb(uint64_t *s_inc, uint32_t r, const uint8_t *m, size_t mlen) {
    size_t i;

    while (mlen + s_inc[25] >= r) {
        if (s_inc[25] == 0) {
            for (i = 0; i < r / 8; i++) {
                s_inc[i] ^= load64(m + 8 * i);
            }
            mlen -= r;
            m += r;
        } else {
            for (i = 0; i < r - (uint32_t)s_inc[25]; i++) {
                s_inc[(s_inc[25] + i) >> 3] ^= (uint64_t)m[i] << (8 * ((s_inc[25] + i) & 0x07));
            }
            mlen -= (size_t)(r - s_inc[25]);
            m += r - s_inc[25];
            s_inc[25] = 0;
        }
        keccak_f1600(s_inc);
    }

    for (i = 0; i < mlen; i++) {
        s_inc[(s_inc[25] + i) >> 3] ^= (uint64_t)m[i] << (8 * ((s_inc[25] + i) & 0x07));
    }
    s_inc[25] += mlen;
}

static void keccak_inc_finalize(uint64_t *s_inc, uint32_t r, uint8_t p) {
    s_inc[s_inc[25] >> 3] ^= (uint64_t)p << (8 * (s_inc[25] & 0x07));
    s_inc[(r - 1) >> 3] ^= (uint64_t)128 << (8 * ((r - 1) & 0x07));
    s_inc[25] = 0;
}

static void keccak_inc_squeeze(uint8_t *h, size_t outlen, uint64_t *s_inc, uint32_t r) {
    size_t i;

    for (i = 0; i < outlen && i < s_inc[25]; i++) {
        h[i] = (uint8_t)(s_inc[(r - s_inc[25] + i) >> 3] >> (8 * ((r - s_inc[25] + i) & 0x07)));
    }
    h += i;
    outlen -= i;
    s_inc[25] -= i;

    while (outlen > 0) {
        keccak_f1600(s_inc);

        size_t take = outlen < r ? outlen : r;
        size_t lanes = take / 8;

        for (i = 0; i < lanes; i++) {
            store64(h + 8 * i, s_inc[i]);
        }

        for (i = lanes * 8; i < take; i++) {
            h[i] = (uint8_t)(s_inc[i >> 3] >> (8 * (i & 0x07)));
        }

        h += take;
        outlen -= take;
        s_inc[25] = r - take;
    }
}

/* =====================================================================
 * API IMPLEMENTATIONS (SHAKE E SHA3)
 * ===================================================================== */

void shake128_inc_init(shake128incctx *state) {
    keccak_inc_init(state->ctx);
}

void shake128_inc_absorb(shake128incctx *state, const uint8_t *input, size_t inlen) {
    keccak_inc_absorb(state->ctx, SHAKE128_RATE, input, inlen);
}

void shake128_inc_finalize(shake128incctx *state) {
    keccak_inc_finalize(state->ctx, SHAKE128_RATE, 0x1F);
}

void shake128_inc_squeeze(uint8_t *output, size_t outlen, shake128incctx *state) {
    keccak_inc_squeeze(output, outlen, state->ctx, SHAKE128_RATE);
}

void shake256_inc_init(shake256incctx *state) {
    keccak_inc_init(state->ctx);
}

void shake256_inc_absorb(shake256incctx *state, const uint8_t *input, size_t inlen) {
    keccak_inc_absorb(state->ctx, SHAKE256_RATE, input, inlen);
}

void shake256_inc_finalize(shake256incctx *state) {
    keccak_inc_finalize(state->ctx, SHAKE256_RATE, 0x1F);
}

void shake256_inc_squeeze(uint8_t *output, size_t outlen, shake256incctx *state) {
    keccak_inc_squeeze(output, outlen, state->ctx, SHAKE256_RATE);
}

void shake128_absorb(shake128ctx *state, const uint8_t *input, size_t inlen) {
    keccak_absorb(state->ctx, SHAKE128_RATE, input, inlen, 0x1F);
}

void shake128_squeezeblocks(uint8_t *output, size_t nblocks, shake128ctx *state) {
    keccak_squeezeblocks(output, nblocks, state->ctx, SHAKE128_RATE);
}

void shake256_absorb(shake256ctx *state, const uint8_t *input, size_t inlen) {
    keccak_absorb(state->ctx, SHAKE256_RATE, input, inlen, 0x1F);
}

void shake256_squeezeblocks(uint8_t *output, size_t nblocks, shake256ctx *state) {
    keccak_squeezeblocks(output, nblocks, state->ctx, SHAKE256_RATE);
}

void shake128(uint8_t *output, size_t outlen, const uint8_t *input, size_t inlen) {
    size_t nblocks = outlen / SHAKE128_RATE;
    uint8_t t[SHAKE128_RATE];
    shake128ctx s;

    shake128_absorb(&s, input, inlen);
    shake128_squeezeblocks(output, nblocks, &s);

    output += nblocks * SHAKE128_RATE;
    outlen -= nblocks * SHAKE128_RATE;

    if (outlen) {
        shake128_squeezeblocks(t, 1, &s);
        for (size_t i = 0; i < outlen; ++i) {
            output[i] = t[i];
        }
    }
}

void shake256(uint8_t *output, size_t outlen, const uint8_t *input, size_t inlen) {
    size_t nblocks = outlen / SHAKE256_RATE;
    uint8_t t[SHAKE256_RATE];
    shake256ctx s;

    shake256_absorb(&s, input, inlen);
    shake256_squeezeblocks(output, nblocks, &s);

    output += nblocks * SHAKE256_RATE;
    outlen -= nblocks * SHAKE256_RATE;

    if (outlen) {
        shake256_squeezeblocks(t, 1, &s);
        for (size_t i = 0; i < outlen; ++i) {
            output[i] = t[i];
        }
    }
}

void sha3_256_inc_init(sha3_256incctx *state) {
    keccak_inc_init(state->ctx);
}

void sha3_256_inc_absorb(sha3_256incctx *state, const uint8_t *input, size_t inlen) {
    keccak_inc_absorb(state->ctx, SHA3_256_RATE, input, inlen);
}

void sha3_256_inc_finalize(uint8_t *output, sha3_256incctx *state) {
    uint8_t t[SHA3_256_RATE];
    keccak_inc_finalize(state->ctx, SHA3_256_RATE, 0x06);

    keccak_squeezeblocks(t, 1, state->ctx, SHA3_256_RATE);

    for (size_t i = 0; i < 32; i++) {
        output[i] = t[i];
    }
}

void sha3_256(uint8_t *output, const uint8_t *input, size_t inlen) {
    uint64_t s[25];
    uint8_t t[SHA3_256_RATE];

    keccak_absorb(s, SHA3_256_RATE, input, inlen, 0x06);
    keccak_squeezeblocks(t, 1, s, SHA3_256_RATE);

    for (size_t i = 0; i < 32; i++) {
        output[i] = t[i];
    }
}

void sha3_384_inc_init(sha3_384incctx *state) {
    keccak_inc_init(state->ctx);
}

void sha3_384_inc_absorb(sha3_384incctx *state, const uint8_t *input, size_t inlen) {
    keccak_inc_absorb(state->ctx, SHA3_384_RATE, input, inlen);
}

void sha3_384_inc_finalize(uint8_t *output, sha3_384incctx *state) {
    uint8_t t[SHA3_384_RATE];
    keccak_inc_finalize(state->ctx, SHA3_384_RATE, 0x06);

    keccak_squeezeblocks(t, 1, state->ctx, SHA3_384_RATE);

    for (size_t i = 0; i < 48; i++) {
        output[i] = t[i];
    }
}

void sha3_384(uint8_t *output, const uint8_t *input, size_t inlen) {
    uint64_t s[25];
    uint8_t t[SHA3_384_RATE];

    keccak_absorb(s, SHA3_384_RATE, input, inlen, 0x06);
    keccak_squeezeblocks(t, 1, s, SHA3_384_RATE);

    for (size_t i = 0; i < 48; i++) {
        output[i] = t[i];
    }
}

void sha3_512_inc_init(sha3_512incctx *state) {
    keccak_inc_init(state->ctx);
}

void sha3_512_inc_absorb(sha3_512incctx *state, const uint8_t *input, size_t inlen) {
    keccak_inc_absorb(state->ctx, SHA3_512_RATE, input, inlen);
}

void sha3_512_inc_finalize(uint8_t *output, sha3_512incctx *state) {
    uint8_t t[SHA3_512_RATE];
    keccak_inc_finalize(state->ctx, SHA3_512_RATE, 0x06);

    keccak_squeezeblocks(t, 1, state->ctx, SHA3_512_RATE);

    for (size_t i = 0; i < 64; i++) {
        output[i] = t[i];
    }
}

void sha3_512(uint8_t *output, const uint8_t *input, size_t inlen) {
    uint64_t s[25];
    uint8_t t[SHA3_512_RATE];

    keccak_absorb(s, SHA3_512_RATE, input, inlen, 0x06);
    keccak_squeezeblocks(t, 1, s, SHA3_512_RATE);

    for (size_t i = 0; i < 64; i++) {
        output[i] = t[i];
    }
}