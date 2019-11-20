#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

uint8_t alu_neg_u8(uint8_t x);
 int8_t alu_neg_i8( int8_t x);
uint8_t alu_add_u8(uint8_t x, uint8_t y);
 int8_t alu_add_i8( int8_t x,  int8_t y);
uint8_t alu_sub_u8(uint8_t x, uint8_t y);
 int8_t alu_sub_i8( int8_t x,  int8_t y);
uint8_t alu_mul_u8(uint8_t x, uint8_t y);
 int8_t alu_mul_i8( int8_t x,  int8_t y);
uint8_t alu_div_u8(uint8_t n, uint8_t d);
 int8_t alu_div_i8( int8_t n,  int8_t d);
uint8_t alu_fma_u8(uint8_t x, uint8_t y, uint8_t z);
 int8_t alu_fma_i8( int8_t x,  int8_t y,  int8_t z);

#ifdef __cplusplus
} // extern "C"
#endif
