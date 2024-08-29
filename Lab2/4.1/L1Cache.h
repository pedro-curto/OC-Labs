#ifndef L1CACHE_H
#define L1CACHE_H   

#include "../Cache.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdint.h>

#define L1_NUM_LINES (L1_SIZE / BLOCK_SIZE) 
#define BITS_OFFSET 6 // log2(BLOCK_SIZE)
#define BITS_INDEX 8 // L1_SIZE / (BLOCK_SIZE * )

void resetTime();

uint32_t getTime();

/****************  RAM memory (byte addressable) ***************/
void accessDRAM(uint32_t, uint8_t *, uint32_t);

/*********************** Cache *************************/

void initCache();
void accessL1(uint32_t, uint8_t *, uint32_t);

typedef struct L1CacheLine {
  uint8_t Valid;
  uint8_t Dirty;
  uint32_t Tag;
  uint8_t Data[BLOCK_SIZE];
} L1CacheLine;

typedef struct L1Cache {
  L1CacheLine line[L1_NUM_LINES];
} L1Cache;

/*********************** Interfaces *************************/

void read(uint32_t, uint8_t *);

void write(uint32_t, uint8_t *);

#endif