#include "L2Cache2W.h"

uint8_t DRAM[DRAM_SIZE];
uint32_t time;
L1Cache l1cache;
L2Cache2W l2cache;

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

/*********************** L1 cache *************************/

void initCache() { 
  for (int i = 0; i < L1_NUM_LINES; i++) {
    l1cache.line[i].Valid = 0;
    l1cache.line[i].Dirty = 0;
    l1cache.line[i].Tag = 0;
    for (int j = 0; j < BLOCK_SIZE; j+=WORD_SIZE) {
      l1cache.line[i].Data[j] = 0;
    }
  } 
  for (int i = 0; i < L2_NUM_LINES/ASSOCIATIVITY_L2; i++) {
    for (int j = 0; j < ASSOCIATIVITY_L2; j++) {
      l2cache.set[i].line[j].Valid = 0;
      l2cache.set[i].line[j].Dirty = 0;
      l2cache.set[i].line[j].Tag = 0;
      l2cache.set[i].line[j].LastUse = 0;
      for (int k = 0; k < BLOCK_SIZE; k+=WORD_SIZE) {
        l2cache.set[i].line[j].Data[k] = 0;
      }
    }
  }

  for (int i = 0; i < DRAM_SIZE; i+=WORD_SIZE) {
    DRAM[i] = 0;
  }
}


void accessL2(uint32_t address, uint8_t *data, uint8_t mode) {
  uint32_t tag, setidx, offset;

  tag = address / (L2_NUM_SETS * BLOCK_SIZE * ASSOCIATIVITY_L2);
  setidx = (address / BLOCK_SIZE) % L2_NUM_SETS;
  offset = address % BLOCK_SIZE;

  // lookup within set to find the correct line
  for (int i = 0; i < ASSOCIATIVITY_L2; i++) {
    if (l2cache.set[setidx].line[i].Valid && l2cache.set[setidx].line[i].Tag == tag) {
      // L2 cache hit
      if (mode == MODE_READ) {
        memcpy(data, &(l2cache.set[setidx].line[i].Data), BLOCK_SIZE);
        time += L2_READ_TIME;
      }
      if (mode == MODE_WRITE) {
        memcpy(&(l2cache.set[setidx].line[i].Data), data, BLOCK_SIZE);
        time += L2_WRITE_TIME;
      }
      l2cache.set[setidx].line[i].LastUse = time;
      return;
    }
  }

  // L2 cache miss; if both lines are valid, we use LRU to determine which to replace


  int lruidx = 0;
  uint32_t lru_time = l2cache.set[setidx].line[0].LastUse;

  for (int i = 0; i < ASSOCIATIVITY_L2; i++) {
    if (l2cache.set[setidx].line[i].LastUse < lru_time) {
      lruidx = i;
      lru_time = l2cache.set[setidx].line[i].LastUse;
    }
  }


  if (l2cache.set[setidx].line[lruidx].Dirty) { // dirty block -> writes back

 

    accessDRAM(l2cache.set[setidx].line[lruidx].Tag * (L2_SIZE / ASSOCIATIVITY_L2) * BLOCK_SIZE + setidx * BLOCK_SIZE,
     l2cache.set[setidx].line[lruidx].Data, MODE_WRITE);
    

    l2cache.set[setidx].line[lruidx].Data[0] = 0;
    l2cache.set[setidx].line[lruidx].Data[WORD_SIZE] = 0;
  }
  // fetches new block from DRAM
  accessDRAM(address - offset, l2cache.set[setidx].line[lruidx].Data, MODE_READ);

  l2cache.set[setidx].line[lruidx].Valid = 1;
  l2cache.set[setidx].line[lruidx].Dirty = 0;
  l2cache.set[setidx].line[lruidx].Tag = tag;

  if (mode == MODE_READ) {
    memcpy(data, &(l2cache.set[setidx].line[lruidx].Data[offset]), BLOCK_SIZE);
    time += L2_READ_TIME;
    l2cache.set[setidx].line[lruidx].LastUse = time;
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
    if (l1cache.line[index].Dirty) {
      accessL2(l1cache.line[index].Tag * L1_SIZE  + index * BLOCK_SIZE, 
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