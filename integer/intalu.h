#pragma once

#include <stdint.h>

uint8_t  intalu_negu8(uint8_t x);
 int8_t  intalu_negs8( int8_t x);

uint8_t intalu_addu8(uint8_t x, uint8_t y);
 int8_t intalu_adds8( int8_t x,  int8_t y);
uint8_t intalu_subu8(uint8_t x, uint8_t y);
 int8_t intalu_subs8( int8_t x,  int8_t y);

uint8_t intalu_mulu8(uint8_t x, uint8_t y);
 int8_t intalu_muls8( int8_t x,  int8_t y);

uint8_t intalu_divu8(uint8_t n, uint8_t d);
 int8_t intalu_divs8( int8_t n,  int8_t d);
