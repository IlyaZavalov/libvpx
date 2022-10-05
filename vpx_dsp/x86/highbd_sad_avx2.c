/*
 *  Copyright (c) 2022 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */
#include <immintrin.h>
#include "./vpx_dsp_rtcd.h"
#include "vpx/vpx_integer.h"

static VPX_FORCE_INLINE unsigned int calc_final(const __m256i sums_32) {
  const __m256i t0 = _mm256_add_epi32(sums_32, _mm256_srli_si256(sums_32, 8));
  const __m256i t1 = _mm256_add_epi32(t0, _mm256_srli_si256(t0, 4));
  const __m128i sum = _mm_add_epi32(_mm256_castsi256_si128(t1),
                                    _mm256_extractf128_si256(t1, 1));
  return (unsigned int)_mm_cvtsi128_si32(sum);
}

static VPX_FORCE_INLINE void highbd_sad32xH(__m256i *sums_16,
                                            const uint16_t *src, int src_stride,
                                            uint16_t *ref, int ref_stride,
                                            int height) {
  int i;
  for (i = 0; i < height; ++i) {
    // load src and all ref[]
    const __m256i s0 = _mm256_load_si256((const __m256i *)src);
    const __m256i s1 = _mm256_load_si256((const __m256i *)(src + 16));
    const __m256i r0 = _mm256_loadu_si256((const __m256i *)ref);
    const __m256i r1 = _mm256_loadu_si256((const __m256i *)(ref + 16));
    // absolute differences between every ref[] to src
    const __m256i abs_diff0 = _mm256_abs_epi16(_mm256_sub_epi16(r0, s0));
    const __m256i abs_diff1 = _mm256_abs_epi16(_mm256_sub_epi16(r1, s1));
    // sum every abs diff
    *sums_16 = _mm256_add_epi16(*sums_16, abs_diff0);
    *sums_16 = _mm256_add_epi16(*sums_16, abs_diff1);

    src += src_stride;
    ref += ref_stride;
  }
}

#define HIGHBD_SAD32XN(n)                                                    \
  unsigned int vpx_highbd_sad32x##n##_avx2(                                  \
      const uint8_t *src8_ptr, int src_stride, const uint8_t *ref8_ptr,      \
      int ref_stride) {                                                      \
    const uint16_t *src = CONVERT_TO_SHORTPTR(src8_ptr);                     \
    uint16_t *ref = CONVERT_TO_SHORTPTR(ref8_ptr);                           \
    __m256i sums_32 = _mm256_setzero_si256();                                \
    int i;                                                                   \
                                                                             \
    for (i = 0; i < (n / 8); ++i) {                                          \
      __m256i sums_16 = _mm256_setzero_si256();                              \
                                                                             \
      highbd_sad32xH(&sums_16, src, src_stride, ref, ref_stride, 8);         \
                                                                             \
      /* sums_16 will outrange after 8 rows, so add current sums_16 to       \
       * sums_32*/                                                           \
      sums_32 = _mm256_add_epi32(                                            \
          sums_32,                                                           \
          _mm256_add_epi32(                                                  \
              _mm256_cvtepu16_epi32(_mm256_castsi256_si128(sums_16)),        \
              _mm256_cvtepu16_epi32(_mm256_extractf128_si256(sums_16, 1)))); \
                                                                             \
      src += src_stride << 3;                                                \
      ref += ref_stride << 3;                                                \
    }                                                                        \
    return calc_final(sums_32);                                              \
  }

// 32x64
HIGHBD_SAD32XN(64)

// 32x32
HIGHBD_SAD32XN(32)

// 32x16
HIGHBD_SAD32XN(16)

static VPX_FORCE_INLINE void highbd_sad16xH(__m256i *sums_16,
                                            const uint16_t *src, int src_stride,
                                            uint16_t *ref, int ref_stride,
                                            int height) {
  int i;
  for (i = 0; i < height; i += 2) {
    // load src and all ref[]
    const __m256i s0 = _mm256_load_si256((const __m256i *)src);
    const __m256i s1 = _mm256_load_si256((const __m256i *)(src + src_stride));
    const __m256i r0 = _mm256_loadu_si256((const __m256i *)ref);
    const __m256i r1 = _mm256_loadu_si256((const __m256i *)(ref + ref_stride));
    // absolute differences between every ref[] to src
    const __m256i abs_diff0 = _mm256_abs_epi16(_mm256_sub_epi16(r0, s0));
    const __m256i abs_diff1 = _mm256_abs_epi16(_mm256_sub_epi16(r1, s1));
    // sum every abs diff
    *sums_16 = _mm256_add_epi16(*sums_16, abs_diff0);
    *sums_16 = _mm256_add_epi16(*sums_16, abs_diff1);

    src += src_stride << 1;
    ref += ref_stride << 1;
  }
}

unsigned int vpx_highbd_sad16x32_avx2(const uint8_t *src8_ptr, int src_stride,
                                      const uint8_t *ref8_ptr, int ref_stride) {
  const uint16_t *src = CONVERT_TO_SHORTPTR(src8_ptr);
  uint16_t *ref = CONVERT_TO_SHORTPTR(ref8_ptr);
  __m256i sums_32 = _mm256_setzero_si256();
  int i;

  for (i = 0; i < 2; ++i) {
    __m256i sums_16 = _mm256_setzero_si256();

    highbd_sad16xH(&sums_16, src, src_stride, ref, ref_stride, 16);

    // sums_16 will outrange after 16 rows, so add current sums_16 to sums_32
    sums_32 = _mm256_add_epi32(
        sums_32,
        _mm256_add_epi32(
            _mm256_cvtepu16_epi32(_mm256_castsi256_si128(sums_16)),
            _mm256_cvtepu16_epi32(_mm256_extractf128_si256(sums_16, 1))));

    src += src_stride << 4;
    ref += ref_stride << 4;
  }
  return calc_final(sums_32);
}

unsigned int vpx_highbd_sad16x16_avx2(const uint8_t *src8_ptr, int src_stride,
                                      const uint8_t *ref8_ptr, int ref_stride) {
  const uint16_t *src = CONVERT_TO_SHORTPTR(src8_ptr);
  uint16_t *ref = CONVERT_TO_SHORTPTR(ref8_ptr);
  __m256i sums_16 = _mm256_setzero_si256();

  highbd_sad16xH(&sums_16, src, src_stride, ref, ref_stride, 16);

  {
    const __m256i sums_32 = _mm256_add_epi32(
        _mm256_cvtepu16_epi32(_mm256_castsi256_si128(sums_16)),
        _mm256_cvtepu16_epi32(_mm256_extractf128_si256(sums_16, 1)));
    return calc_final(sums_32);
  }
}

unsigned int vpx_highbd_sad16x8_avx2(const uint8_t *src8_ptr, int src_stride,
                                     const uint8_t *ref8_ptr, int ref_stride) {
  const uint16_t *src = CONVERT_TO_SHORTPTR(src8_ptr);
  uint16_t *ref = CONVERT_TO_SHORTPTR(ref8_ptr);
  __m256i sums_16 = _mm256_setzero_si256();

  highbd_sad16xH(&sums_16, src, src_stride, ref, ref_stride, 8);

  {
    const __m256i sums_32 = _mm256_add_epi32(
        _mm256_cvtepu16_epi32(_mm256_castsi256_si128(sums_16)),
        _mm256_cvtepu16_epi32(_mm256_extractf128_si256(sums_16, 1)));
    return calc_final(sums_32);
  }
}