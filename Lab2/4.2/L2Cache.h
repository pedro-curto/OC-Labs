#ifndef L2CACHE_H
#define L2CACHE_H   

#include "../4.1/L1Cache.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdint.h>

#define L2_NUM_LINES (L2_SIZE / BLOCK_SIZE) 

void resetTime();

uint32_t getTime();

/****************  RAM memory (byte addressable) ***************/
void accessDRAM(uint32_t, uint8_t *, uint32_t);

/*********************** Cache *************************/

void initCache();
void accessL2(uint32_t, uint8_t *, uint8_t);

typedef struct L2CacheLine {
  uint8_t Valid;
  uint8_t Dirty;
  uint32_t Tag;
  uint8_t Data[BLOCK_SIZE];
} L2CacheLine;

typedef struct L2Cache {
  L2CacheLine line[L2_NUM_LINES];
} L2Cache;


/*********************** Interfaces *************************/

void read(uint32_t, uint8_t *);

void write(uint32_t, uint8_t *);

#endif