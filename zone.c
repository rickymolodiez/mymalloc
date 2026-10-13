#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
// For small allocations <2 KB, we will use a partition called
// A zone is like a basket that will hold x (from 0 to n) amount  of chocolate
// bars.
// Each chocolate bar will be (16, 32, 64 ... 2048) bytes. up to 2KB.
struct zallocSlab *getPrevSlab(int zoneArrayIndex);
struct zallocSlab *allocateFirstSlab(int zoneArrayIndex);
struct zallocSlab *allocateSlab(int zoneArrayIndex);
enum { totalZoneCount = 8 };
static int MIN_ZONE_SIZE = 16;

struct kallocZone {
  size_t zoneSize; // in bytes;
  bool isEmpty;
  struct zallocSlab *head;
};

struct zallocSlab {
  size_t size;
  struct zallocSlab *next;
  struct zallocSlab *prev;
  double data[];
};

struct kallocZone zones[totalZoneCount];

struct zallocSlab *allocateSlab(int zoneArrayIndex) {
  size_t SLAB_BYTES =
      sizeof(struct zallocSlab) + zones[zoneArrayIndex].zoneSize;

  struct zallocSlab *slab = mmap(NULL, SLAB_BYTES, PROT_READ | PROT_WRITE,
                                 MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

  if (slab == MAP_FAILED)
    return NULL;

  slab->size = zones[zoneArrayIndex].zoneSize;
  slab->prev = getPrevSlab(zoneArrayIndex);
  if (slab->prev != NULL)
    slab->prev->next = slab;
  return slab;
}

struct zallocSlab *allocateFirstSlab(int zoneArrayIndex) {

  struct zallocSlab *slab = allocateSlab(zoneArrayIndex);
  if (slab == NULL) {
    return NULL;
  }
  zones[zoneArrayIndex].isEmpty = false;
  zones[zoneArrayIndex].head = slab;
  return slab;
}

struct zallocSlab *getPrevSlab(int zoneArrayIndex) {

  struct zallocSlab *current = zones[zoneArrayIndex].head;
  if (current == NULL) {
    return NULL;
  }

  while (current->next != NULL) {
    current = current->next;
  }
  return current;
}
