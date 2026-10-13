#include "zone.c"
#include <search.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <unistd.h>

void *myKalloc(size_t trueDataSize);
void *vmKalloc(size_t dataSize);
void initializeKallocZoneArray();
void *myKallocMain(size_t dataSize);

int main() { myKalloc(1024); }

void *myKalloc(size_t trueDataSize) {
  static bool firstTimeCall = true;
  if (firstTimeCall) {
    initializeKallocZoneArray();
    firstTimeCall = false;
  }

  if (trueDataSize <= 0) {
    printf("Allocated data must be greater than 0 bytes!\n");
    return NULL;
  }

  // trick to round up int to powers of 2
  // see bit twiddling hacks:
  // https://graphics.stanford.edu/~seander/bithacks.html
  size_t v = trueDataSize;
  v--;
  v |= v >> 1;
  v |= v >> 2;
  v |= v >> 4;
  v |= v >> 8;
  v++;

  size_t dataSize = v;

  if (dataSize > 2048) {
    return (vmKalloc(dataSize));
  }
  return (myKallocMain(dataSize));

} // end *myKalloc

// Main Kalloc sequence
void *myKallocMain(size_t dataSize) {
  // determine the index based on the data size.
  int index = __builtin_ctzl(dataSize) - 4;
  struct zallocSlab *allocated;
  if (zones[index].isEmpty) {
    allocated = allocateFirstSlab(index);
  } else {
    allocated = allocateSlab(index);
  }

  if (allocated == NULL) {
    return NULL;
  }
  return allocated->data;
}

void *vmKalloc(size_t dataSize) {
  // implement direct mmap call for data.
  printf("vmkalloc\n");
  // return void pointer to memory.
}

void initializeKallocZoneArray() {
  // use a simple array to store zones
  for (int i = 0; i < (totalZoneCount); i++) {
    zones[i].zoneSize = (size_t)MIN_ZONE_SIZE
                        << i; // use bit shift to calculate zone size
    zones[i].isEmpty = true;
    zones[i].head = NULL;
  } // end for loop
} // end initializeKallocZoneArray()
