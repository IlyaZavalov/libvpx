/*
 *  Copyright (c) 2010 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef VP10_ENCODER_TOKENIZE_H_
#define VP10_ENCODER_TOKENIZE_H_

#include "vp10/common/entropy.h"

#include "vp10/encoder/block.h"
#include "vp10/encoder/treewriter.h"

#ifdef __cplusplus
extern "C" {
#endif

#define EOSB_TOKEN 127     // Not signalled, encoder only

#if CONFIG_VPX_HIGHBITDEPTH
  typedef int32_t EXTRABIT;
#else
  typedef int16_t EXTRABIT;
#endif


typedef struct {
  int16_t token;
  EXTRABIT extra;
} TOKENVALUE;

typedef struct {
  const vpx_prob *context_tree;
#if CONFIG_ANS
  const rans_dec_lut *token_cdf;
#endif  // CONFIG_ANS
  EXTRABIT extra;
  uint8_t token;
  uint8_t skip_eob_node;
} TOKENEXTRA;

extern const vpx_tree_index vp10_coef_tree[];
extern const vpx_tree_index vp10_coef_con_tree[];
#if !CONFIG_ANS
extern const struct vp10_token vp10_coef_encodings[];
#endif  // !CONFIG_ANS

int vp10_is_skippable_in_plane(MACROBLOCK *x, BLOCK_SIZE bsize, int plane);
int vp10_has_high_freq_in_plane(MACROBLOCK *x, BLOCK_SIZE bsize, int plane);

struct VP10_COMP;
struct ThreadData;

#if CONFIG_VAR_TX
void vp10_tokenize_sb_inter(struct VP10_COMP *cpi, struct ThreadData *td,
                            TOKENEXTRA **t, int dry_run, int mi_row, int mi_col,
                            BLOCK_SIZE bsize);
#endif

void vp10_tokenize_palette_sb(struct ThreadData *const td,
                              BLOCK_SIZE bsize, int plane,
                              TOKENEXTRA **t);
void vp10_tokenize_sb(struct VP10_COMP *cpi, struct ThreadData *td,
                     TOKENEXTRA **t, int dry_run, BLOCK_SIZE bsize);
#if CONFIG_SUPERTX
void vp10_tokenize_sb_supertx(struct VP10_COMP *cpi, struct ThreadData *td,
                              TOKENEXTRA **t, int dry_run, BLOCK_SIZE bsize);
#endif

extern const int16_t *vp10_dct_value_cost_ptr;
/* TODO: The Token field should be broken out into a separate char array to
 *  improve cache locality, since it's needed for costing when the rest of the
 *  fields are not.
 */
extern const TOKENVALUE *vp10_dct_value_tokens_ptr;
extern const TOKENVALUE *vp10_dct_cat_lt_10_value_tokens;
extern const int *vp10_dct_cat_lt_10_value_cost;
extern const int16_t vp10_cat6_low_cost[256];
extern const int vp10_cat6_high_cost[64];
extern const int vp10_cat6_high10_high_cost[256];
extern const int vp10_cat6_high12_high_cost[1024];
static INLINE int vp10_get_cost(int16_t token, EXTRABIT extrabits,
                               const int *cat6_high_table) {
  if (token != CATEGORY6_TOKEN)
    return vp10_extra_bits[token].cost[extrabits >> 1];
  return vp10_cat6_low_cost[(extrabits >> 1) & 0xff]
      + cat6_high_table[extrabits >> 9];
}

#if CONFIG_VPX_HIGHBITDEPTH
static INLINE const int* vp10_get_high_cost_table(int bit_depth) {
  return bit_depth == 8 ? vp10_cat6_high_cost
      : (bit_depth == 10 ? vp10_cat6_high10_high_cost :
         vp10_cat6_high12_high_cost);
}
#else
static INLINE const int* vp10_get_high_cost_table(int bit_depth) {
  (void) bit_depth;
  return vp10_cat6_high_cost;
}
#endif  // CONFIG_VPX_HIGHBITDEPTH

static INLINE void vp10_get_token_extra(int v, int16_t *token, EXTRABIT *extra) {
  if (v >= CAT6_MIN_VAL || v <= -CAT6_MIN_VAL) {
    *token = CATEGORY6_TOKEN;
    if (v >= CAT6_MIN_VAL)
      *extra = 2 * v - 2 * CAT6_MIN_VAL;
    else
      *extra = -2 * v - 2 * CAT6_MIN_VAL + 1;
    return;
  }
  *token = vp10_dct_cat_lt_10_value_tokens[v].token;
  *extra = vp10_dct_cat_lt_10_value_tokens[v].extra;
}
static INLINE int16_t vp10_get_token(int v) {
  if (v >= CAT6_MIN_VAL || v <= -CAT6_MIN_VAL)
    return 10;
  return vp10_dct_cat_lt_10_value_tokens[v].token;
}

static INLINE int vp10_get_token_cost(int v, int16_t *token,
                                          const int *cat6_high_table) {
  if (v >= CAT6_MIN_VAL || v <= -CAT6_MIN_VAL) {
    EXTRABIT extrabits;
    *token = CATEGORY6_TOKEN;
    extrabits = abs(v) - CAT6_MIN_VAL;
    return vp10_cat6_low_cost[extrabits & 0xff]
        + cat6_high_table[extrabits >> 8];
  }
  *token = vp10_dct_cat_lt_10_value_tokens[v].token;
  return vp10_dct_cat_lt_10_value_cost[v];
}

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // VP10_ENCODER_TOKENIZE_H_