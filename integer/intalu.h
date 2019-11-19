#pragma once

#include <stdint.h>

uint8_t intalu_neg_u8(uint8_t x);
 int8_t intalu_neg_i8( int8_t x);
uint8_t intalu_add_u8(uint8_t x, uint8_t y);
 int8_t intalu_add_i8( int8_t x,  int8_t y);
uint8_t intalu_sub_u8(uint8_t x, uint8_t y);
 int8_t intalu_sub_i8( int8_t x,  int8_t y);
uint8_t intalu_mul_u8(uint8_t x, uint8_t y);
 int8_t intalu_mul_i8( int8_t x,  int8_t y);
uint8_t intalu_div_u8(uint8_t n, uint8_t d);
 int8_t intalu_div_i8( int8_t n,  int8_t d);
uint8_t intalu_fma_u8(uint8_t a, uint8_t b, uint8_t c);
 int8_t intalu_fma_i8( int8_t a,  int8_t b,  int8_t c);
