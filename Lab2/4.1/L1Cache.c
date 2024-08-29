#include "L1Cache.h"

uint8_t DRAM[DRAM_SIZE];
uint32_t time;
L1Cache l1cache;

/**************** Time Manipulation ****************/
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
  for (int i = 0; i < L1_NUM_LINES; i++) {
    l1cache.line[i].Valid = 0;
    l1cache.line[i].Dirty = 0;
    l1cache.line[i].Tag = 0;
    for (int j = 0; j < BLOCK_SIZE; j+=WORD_SIZE) {
      l1cache.line[i].Data[j] = 0;
    }
  } 
  for (int i = 0; i < DRAM_SIZE; i+=WORD_SIZE) {
    DRAM[i] = 0;
  }
}

void accessL1(uint32_t address, uint8_t *data, uint32_t mode) {
  uint32_t index, tag, offset;

  tag = address / (L1_NUM_LINES * BLOCK_SIZE);
  index = (address / BLOCK_SIZE) % L1_NUM_LINES;
  offset = address % BLOCK_SIZE;

  if (l1cache.line[index].Valid && l1cache.line[index].Tag == tag) {
    // L1 cache hit
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
    // L1 cache miss
    if (l1cache.line[index].Dirty) { 
      // dirty block -> writes back to DRAM
      
      accessDRAM(l1cache.line[index].Tag * L1_SIZE + index * BLOCK_SIZE, l1cache.line[index].Data, MODE_WRITE);
      l1cache.line[index].Dirty = 0;
    }
    // fetches block from DRAM 
    accessDRAM(address - offset, l1cache.line[index].Data, MODE_READ);
    if (mode == MODE_READ) {
      memcpy(data, &(l1cache.line[index].Data[offset]), WORD_SIZE);
      time += L1_READ_TIME;
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
