#include "L2Cache.h"

uint8_t DRAM[DRAM_SIZE];
uint32_t time;
L1Cache l1cache;
L2Cache l2cache;

/**************** Time Manipulation ***************/
void resetTime() { time = 0; }

uint32_t getTime() { return time; }

/****************  RAM memory (byte addressable) ***************/
void accessDRAM(uint32_t address, uint8_t *data, uint32_t mode) {

  if (address >= DRAM_SIZE - WORD_SIZE + 1)
    exit(-1);

  if (mode == MODE_READ) {
    memcpy(data, &(DRAM[address]), BLOCK_SIZE);
    time += DRAM_READ_TIME;
  }

  if (mode == MODE_WRITE) {
    memcpy(&(DRAM[address]), data, BLOCK_SIZE);
    time += DRAM_WRITE_TIME;
  }
}


void initCache() { 
  for (int i = 0; i < (L1_SIZE / BLOCK_SIZE); i++) {
    l1cache.line[i].Valid = 0;
    l1cache.line[i].Dirty = 0;
    l1cache.line[i].Tag = 0;
    for (int j = 0; j < BLOCK_SIZE; j+=WORD_SIZE) {
      l1cache.line[i].Data[j] = 0;
    }
  } 
  for (int i = 0; i < (L2_SIZE / BLOCK_SIZE); i++) {
    l2cache.line[i].Valid = 0;
    l2cache.line[i].Dirty = 0;
    l2cache.line[i].Tag = 0;
    for (int k = 0; k < BLOCK_SIZE; k+=WORD_SIZE) {
      l2cache.line[i].Data[k] = 0;
    }
  }
  for (int i = 0; i < DRAM_SIZE; i+=WORD_SIZE) {
    DRAM[i] = 0;
  }
}


void accessL2(uint32_t address, uint8_t *data, uint8_t mode) {
  uint32_t tag, index, offset;

  tag = address / (L2_NUM_LINES * BLOCK_SIZE);
  index = (address / BLOCK_SIZE) % L2_NUM_LINES;
  offset = address % BLOCK_SIZE;

  if (l2cache.line[index].Valid && l2cache.line[index].Tag == tag) {
    // L2 cache hit
    if (mode == MODE_READ) {
      memcpy(data, &(l2cache.line[index].Data[offset]), WORD_SIZE);
      time += L2_READ_TIME;
    }
    if (mode == MODE_WRITE) {
      memcpy(&(l2cache.line[index].Data[offset]), data, WORD_SIZE);
      time += L2_WRITE_TIME;
      l2cache.line[index].Dirty = 1;
    }
  }
  else {

    if (l2cache.line[index].Dirty) { // writes back if line is dirty
    
      accessDRAM(l1cache.line[index].Tag * L2_SIZE + index * BLOCK_SIZE, l2cache.line[index].Data, MODE_WRITE);
      l2cache.line[index].Dirty = 0;
    }
    // fetches new block from DRAM
    accessDRAM(address - offset, l2cache.line[index].Data, MODE_READ);

    if (mode == MODE_READ) {
      memcpy(data, &(l2cache.line[index].Data[offset]), WORD_SIZE);
      time += L2_READ_TIME;
      l2cache.line[index].Valid = 1;
      l2cache.line[index].Tag = tag;
    }
    if (mode == MODE_WRITE) {
      memcpy(&(l2cache.line[index].Data[offset]), data, WORD_SIZE);
      time += L2_WRITE_TIME;
      l2cache.line[index].Dirty = 1;
      l2cache.line[index].Valid = 1;
      l2cache.line[index].Tag = tag;
    }
  }
}


void accessL1(uint32_t address, uint8_t *data, uint32_t mode) {
  uint32_t index, tag, offset;

  tag = address / (L1_NUM_LINES * BLOCK_SIZE);
  index = (address / BLOCK_SIZE) % L1_NUM_LINES;
  offset = address % BLOCK_SIZE;

  if (l1cache.line[index].Valid && l1cache.line[index].Tag == tag) {

    if (mode == MODE_READ) {
      memcpy(data, &(l1cache.line[index].Data[offset]), WORD_SIZE);
      time += L1_READ_TIME;
    }
    if (mode == MODE_WRITE) {
      memcpy(&(l1cache.line[index].Data[offset]), data, WORD_SIZE);
      time += L1_WRITE_TIME;
      l1cache.line[index].Dirty = 1;
    }
  }
  else {
    // L1 cache miss: accesses L2 cache
    if (l1cache.line[index].Dirty) {
      // if L1 cache line is dirty, we write back to L2
      accessL2(l1cache.line[index].Tag * L1_SIZE + index * BLOCK_SIZE, 
      l1cache.line[index].Data, MODE_WRITE);
      l1cache.line[index].Data[0] = 0;
      l1cache.line[index].Data[WORD_SIZE] = 0;
    }

    accessL2(address - offset, l1cache.line[index].Data, MODE_READ);
    if (mode == MODE_READ) {
      memcpy(data, &(l1cache.line[index].Data[offset]), WORD_SIZE);
      time += L1_READ_TIME;
      l1cache.line[index].Dirty = 0; 
      l1cache.line[index].Valid = 1;
      l1cache.line[index].Tag = tag;
    }
    if (mode == MODE_WRITE) {
      memcpy(&(l1cache.line[index].Data[offset]), data, WORD_SIZE);
      time += L1_WRITE_TIME;
      l1cache.line[index].Dirty = 1;
      l1cache.line[index].Valid = 1;
      l1cache.line[index].Tag = tag;
    }
  }

}

void read(uint32_t address, uint8_t *data) {
  accessL1(address, data, MODE_READ);
}

void write(uint32_t address, uint8_t *data) {
  accessL1(address, data, MODE_WRITE);
}
