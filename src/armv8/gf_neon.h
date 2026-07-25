#ifndef _GF_NEON_H_
#define _GF_NEON_H_

#include <arm_neon.h>

/** 
 * x^i modulo x^8+x^4+x^3+x^2+1 duplicate 4 times to fit a 256-bit register
 */
static const uint8x16_t index_table = {0, 2, 4, 6, 8, 10, 12, 14, 0, 0, 0, 0, 0, 0, 0, 0};
static const uint16x8_t mask = {0x00ff, 0x00ff, 0x00ff, 0x00ff, 0x00ff, 0x00ff, 0x00ff, 0x00ff};
static const poly8x8_t multiplier = {0x1D, 0x1D, 0x1d, 0x1D, 0x1D, 0x1D, 0x1D, 0x1D};

//uint16x8_t gf_mul_vect_arm(const uint16x8_t a, const uint16x8_t b);

static const unsigned char __gf256_bit_8_11_reduce[16] __attribute__((aligned(16))) = {
    0x00, 0x1d, 0x3a, 0x27, 0x74, 0x69, 0x4e, 0x53, 0xe8, 0xf5, 0xd2, 0xcf, 0x9c, 0x81, 0xa6, 0xbb
};

static const unsigned char __gf256_bit_12_15_reduce[16] __attribute__((aligned(16))) = {
    0x00, 0xcd, 0x87, 0x4a, 0x13, 0xde, 0x94, 0x59, 0x26, 0xeb, 0xa1, 0x6c, 0x35, 0xf8, 0xb2, 0x7f
};


static inline
uint8x16_t _gf256v_reduce_tbl_neon( uint16x8_t ab0 , uint16x8_t ab1 , uint8x16_t mask_f , uint8x16_t tab_rd0 , uint8x16_t tab_rd1 )
{
    uint8x16_t abl = vreinterpretq_u8_p8( vuzp1q_p8(ab0,ab1) );
    uint8x16_t abh = vreinterpretq_u8_p8( vuzp2q_p8(ab0,ab1) );
// reduce
    return abl ^ vqtbl1q_u8( tab_rd0 , abh&mask_f ) ^ vqtbl1q_u8( tab_rd1 , vshrq_n_u8(abh,4) );
}

static inline
uint8x16_t _gf256v_mul_neon( uint8x16_t a , uint8x16_t b , uint8x16_t mask_f , uint8x16_t tab_rd0 , uint8x16_t tab_rd1 )
{
    poly16x8_t ab0 = vmull_p8( vget_low_p8(a) , vget_low_p8(b) );
    poly16x8_t ab1 = vmull_high_p8( a , b );

    return _gf256v_reduce_tbl_neon( ab0 , ab1 , mask_f , tab_rd0 , tab_rd1 );
}


static inline
uint8x16_t _gf256v_reduce_tbl_neon2( uint16x8_t ab0 , uint16x8_t ab1)
{
    uint8x16_t mask_f  = vdupq_n_u8(0xf);
    uint8x16_t tab_rd0 = vld1q_u8(__gf256_bit_8_11_reduce);
    uint8x16_t tab_rd1 = vld1q_u8(__gf256_bit_12_15_reduce);
    
    uint8x16_t abl = vreinterpretq_u8_p8( vuzp1q_p8(ab0,ab1) );
    uint8x16_t abh = vreinterpretq_u8_p8( vuzp2q_p8(ab0,ab1) );

    return abl ^ vqtbl1q_u8( tab_rd0 , abh&mask_f ) ^ vqtbl1q_u8( tab_rd1 , vshrq_n_u8(abh,4) );
}

static inline
uint8x16_t _gf256v_mul_neon2( uint8x16_t a , uint8x16_t b)
{
    poly16x8_t ab0 = vmull_p8( vget_low_p8(a) , vget_low_p8(b) );
    poly16x8_t ab1 = vmull_high_p8( a , b );

    return _gf256v_reduce_tbl_neon2( ab0 , ab1);
}

static inline
uint16x8_t gf_mul_vect_arm( const uint16x8_t a , const uint16x8_t b )
{
    uint8x16_t mask_f = vdupq_n_u8( 0xf );
    uint8x16_t tab_rd0 = vld1q_u8(__gf256_bit_8_11_reduce);
    uint8x16_t tab_rd1 = vld1q_u8(__gf256_bit_12_15_reduce);
    uint8x16_t a_u8 = vreinterpretq_u8_u16(a);
    uint8x16_t b_u8 = vreinterpretq_u8_u16(b);
    return vreinterpretq_u16_u8(_gf256v_mul_neon( a_u8 , b_u8 , mask_f , tab_rd0 , tab_rd1 ));
}





#endif // ifndef _GF_NEON_H_
