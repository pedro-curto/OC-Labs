#ifndef L2CACHE2W_H
#define L2CACHE2W_H   

#include "../4.2/L2Cache.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdint.h>

#define ASSOCIATIVITY_L2 2 
#define L2_NUM_SETS (L2_NUM_LINES/ASSOCIATIVITY_L2)

void resetTime();

uint32_t getTime();

/****************  RAM memory (byte addressable) ***************/
void accessDRAM(uint32_t, uint8_t *, uint32_t);

/*********************** Cache *************************/

void initCache();
void accessL2(uint32_t, uint8_t *, uint8_t);


typedef struct L2CacheLine2W {
  uint8_t Valid;
  uint8_t Dirty;
  uint32_t Tag;
  uint32_t LastUse;
  uint8_t Data[BLOCK_SIZE];
} L2CacheLine2W;

typedef struct L2CacheSet2W {
  L2CacheLine2W line[ASSOCIATIVITY_L2];
} L2CacheSet2W;

// Definição da estrutura para a cache L2
typedef struct L2Cache2W {
  L2CacheSet2W set[L2_NUM_SETS];
} L2Cache2W;


/*********************** Interfaces *************************/

void read(uint32_t, uint8_t *);

void write(uint32_t, uint8_t *);

#endif
