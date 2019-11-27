
#include <immintrin.h>
#include <nmmintrin.h>
static inline void description()
{
    printf("Using AVX2\n");
}
#define simd_vector_from_mask(a)  (a)
#define simd_imask_from_mask(a)   _mm256_movemask_epi8(a)
#define simd_set_i8(a)            _mm256_set1_epi8(a)
#define simd_setzero()            _mm256_setzero_si256()
#define simd_cmpeq_i8(a, b)       _mm256_cmpeq_epi8((a), (b))
#define simd_andnot_i8(a, b)      _mm256_andnot_si256((a), (b))
#define simd_add_i64(a, b)        _mm256_add_epi64((a), (b))
#define simd_sub_i8(a, b)         _mm256_sub_epi8((a), (b))
#define simd_sad_u8(a, b)         _mm256_sad_epu8((a), (b))
#define simd_imask_popcnt(a)      _mm_popcnt_u32(a)
#define simd_cmpeq_i8_mask(a, b)  simd_cmpeq_i8((a), (b))
#define simd_cmpws_i8_mask(a)     simd_cmpws_i8(a)
#define simd_store(a, b)          _mm256_store_si256((a), (b))
typedef __m256i simd_vector;
typedef uint32_t simd_imask;
typedef simd_vector simd_mask;
static inline simd_vector simd_cmpws_i8(simd_vector a)
{
	simd_vector shuffle_src = _mm256_set_epi64x(0x0d0c0b0a0900, 0x20,
	                                            0x0d0c0b0a0900, 0x20);
	return simd_cmpeq_i8(_mm256_shuffle_epi8(shuffle_src, a), a);
}
// shift a left by 1 byte, shifting in byte from the end of b
static inline simd_vector simd_shl1_from_i8(simd_vector a, simd_vector b)
{
	return _mm256_alignr_epi8(a, _mm256_permute2x128_si256(b, a, 0x21), 15);
}
// endif AVX2
