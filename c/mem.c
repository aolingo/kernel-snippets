/* mem.c : memory manager
 */

#include <i386.h>
#include <xeroskernel.h>
#include <xeroslib.h>

extern long freemem;  /* start of free memory (set in i386.c) */
extern char *maxaddr; /* max memory address (set in i386.c)	*/

/* Define the struct for the mem header */
struct memHeader {
  unsigned long size;
  struct memHeader *prev;
  struct memHeader *next;
  char *sanityCheck;
  unsigned char dataStart[0];
};

// The initial memSlot header pointer that resides in the kernel, points to the
// first free memory
struct memHeader *memSlot;

// Declarations of helper functions here
unsigned long aligned(unsigned long addr);
void completeHeader(struct memHeader *header, unsigned long size,
                    struct memHeader *prev, struct memHeader *next);

// Helper to check if address is 16 byte aligned and converts if its not
unsigned long aligned(unsigned long addr) {
  unsigned long remainder = addr % 16;
  if (remainder) {
    return addr + 16 - (addr % 16);
  }
  return addr;
}

// Helper to create a new header pointer
void completeHeader(struct memHeader *newHeader, unsigned long size,
                    struct memHeader *prev, struct memHeader *next) {
  newHeader->size = size;
  newHeader->prev = prev;
  newHeader->next = next;
  newHeader->sanityCheck = (char *)(size + 16);
}

/* Initialize the initial linked list data struct for the kernel. The linkedlist
will have 2 nodes representing 2 chunks of free memory (before hole and after
hole)
*/
void kmeminit(void) {
  kprintf("kmeminit start\n\n");

  // Initialize linkedlist nodes for the 2 free chunks of memory
  struct memHeader *beforeHole, *afterHole;

  beforeHole = (void *)aligned((unsigned long)freemem);
  beforeHole->size = (unsigned long)HOLESTART -
                     aligned((unsigned long)freemem) - sizeof(struct memHeader);
  beforeHole->prev = NULL;
  beforeHole->next = (void *)HOLEEND;
  beforeHole->sanityCheck = (char *)(beforeHole->size + 16);

  afterHole = beforeHole->next;
  afterHole->size = (unsigned long)maxaddr - (unsigned long)HOLEEND -
                    sizeof(struct memHeader);
  afterHole->prev = beforeHole;
  afterHole->next = NULL;
  afterHole->sanityCheck = (char *)(afterHole->size + 16);

  // Set HEAD of linkedlist to first free memory chunk beforeHole
  memSlot = beforeHole;

  // For beforeHole
  if (beforeHole != (void *)aligned((unsigned long)freemem)) {
    kprintf("beforeHole is wrong, expected: %X got: %X \n\n",
            aligned((unsigned long)freemem), beforeHole);
  }
  if (beforeHole->size !=
      (unsigned long)HOLESTART - aligned((unsigned long)freemem) - 16) {
    kprintf("beforeHole->size is wrong, expected: %X got: %X \n\n",
            (unsigned long)HOLESTART - aligned((unsigned long)freemem) - 16,
            beforeHole->size);
  }
  if (beforeHole->prev != NULL) {
    kprintf("beforeHole->prev should be null");
  }
  if (beforeHole->next != (void *)0x196000) {
    kprintf("beforeHole->next is wrong, expected: %X got: %X \n\n", 0x196000,
            beforeHole->next);
  }
  if (beforeHole->dataStart !=
      (void *)(aligned((unsigned long)freemem) + sizeof(struct memHeader))) {
    kprintf("beforeHole->dataStart is wrong, expected: %X got: %X \n\n",
            aligned((unsigned long)freemem) + sizeof(struct memHeader),
            beforeHole->dataStart);
  }
  // For afterHole
  if ((int)afterHole != 0x196000) {
    kprintf("after is wrong, expected: %X got: %X \n\n", 0x196000, afterHole);
  }
  if ((int)afterHole->size != 0x269FEF) {
    kprintf("after->size is wrong, expected: %X got: %X \n\n", 0x269FEF,
            afterHole->size);
  }
  if (afterHole->prev != beforeHole) {
    kprintf("after->prev should point to beforeHole");
  }
  if (afterHole->next != NULL) {
    kprintf("after->next should be null/0");
  }
  if ((int)afterHole->dataStart != 0x196010) {
    kprintf("after->dataStart is wrong, expected: %X got: %X \n\n", 0x196010,
            afterHole->dataStart);
  }
  if (memSlot != beforeHole) {
    kprintf("memSlot should point to beforeHole in kmeminit");
  }
}

/* Find the next appropriate free memory chunk and allocates it */
void *kmalloc(size_t size) {
  // 4 cases: occupy entire chunk, only node in list, node is first and node
  // after, node is last with node before and node between
  // case2: occupy entire chunk
  // case3: doesnt occupy entire chunk
  // case4: doesnt occupy entire chunk
  // Aligns input size, does not include header size
  unsigned long req_size = aligned((unsigned long)size);
  struct memHeader *curr = memSlot;
  // Base case when requested size is 0 or null or there is no free memory
  // available
  if (req_size == 0 || req_size == NULL || curr == NULL) {
    return 0;
  }
  // kprintf("wanted size: %X\n\n", size);
  // kprintf("required size: %X\n\n", req_size);
  // kprintf("space available: %X\n\n", curr->size);
  while (curr != NULL) {
    // Case when req size doesn't take up entire memory chunk, so have
    // to create newstruct MemHeader
    if (req_size + sizeof(struct memHeader) < curr->size) {
      // kprintf("Case A: req_size doesn't take up entire memory chunk");
      // kprintf("before size: %X\n\n", curr->size);
      // kprintf("memSlot before: %X\n\n", memSlot);
      struct memHeader *newHeader = (void *)(curr->dataStart + req_size);
      completeHeader(newHeader, curr->size - req_size, curr->prev, curr->next);
      curr->size = req_size;
      curr->sanityCheck = (char *)(req_size + 16);
      if (curr->next != NULL) {
        struct memHeader *tmp = curr->next;
        tmp->prev = newHeader;
      }
      if (curr->prev != NULL) {
        struct memHeader *temp = curr->prev;
        temp->next = newHeader;
      }
      memSlot = newHeader;
      // kprintf("allocated header after size: %X\n\n", curr->size);
      // kprintf("memSlot after: %X\n\n", memSlot);
      // kprintf("newHeader location: %X\n\n", newHeader->dataStart);
      // kprintf("newHeader->prev: %X\n\n", newHeader->prev);
      // kprintf("newHeader->next: %X\n\n", newHeader->next);
      return curr->dataStart;
    } else if (req_size <= curr->size) {
      // Cases when req_size take up entire memory chunk
      // No newHeaders will be created
      // kprintf("Case B: req_size takes up entire memory chunk");
      // Free memory chunk is last in list
      if (curr->prev == NULL && curr->next == NULL) {
        // all memory are now used
        memSlot = NULL;
        // kprintf("got here A memslot should be null %X\n\n", memSlot);
        return curr->dataStart;
      }
      // Free memory chunk is first in list of many of free chunks
      if (curr->prev == NULL && curr->next != NULL) {
        // kprintf("Before memslot: %X\n\n", memSlot);
        memSlot = curr->next;
        struct memHeader *tmp = curr->next;
        tmp->prev = NULL;
        // kprintf("After memslot: %X\n\n", memSlot);
        // kprintf("After curr->prev: %X\n\n", curr->prev);
        return curr->dataStart;
      }
      // Not testable until kfree implemented
      if (curr->prev != NULL && curr->next != NULL) {
        memSlot = curr->prev;
        struct memHeader *tmp = curr->prev;
        tmp->next = curr->next;
        tmp = curr->next;
        tmp->prev = curr->prev;
        return curr->dataStart;
      }
      if (curr->prev != NULL && curr->next == NULL) {
        memSlot = curr->prev;
        struct memHeader *tmp = curr->prev;
        tmp->next = NULL;
        return curr->dataStart;
      }
    }
    curr = curr->next;
  }
  // none of the free memory chunks have enough memory
  return 0;
}

/* Free and merge memory blocks */
int kfree(void *ptr) {
  // Base case: invalid input address: inside kernel or inside hole or input not
  // aligned
  if (ptr < (void *)(aligned((unsigned long)freemem)) ||
      (((void *)HOLESTART < ptr) && (ptr < (void *)HOLEEND)) ||
      ptr != (void *)(aligned((unsigned long)ptr)) || ptr > (void *)maxaddr) {
    kprintf("free base case\n\n");
    return 0;
  }
  // Get the header of the address to be freed
  struct memHeader *ptrHeader = ptr - sizeof(struct memHeader);

  // sanity check has been corrupted
  if (ptrHeader->sanityCheck != (char *)(ptrHeader->size + 16)) {
    kprintf("sanity check failed\n\n");
    return 0;
  }
  // kprintf("size to be free is: %X\n\n", ptrHeader->size);
  // kprintf("prev is: %X\n\n", ptrHeader->prev);
  // kprintf("next is: %X\n\n", ptrHeader->next);
  // kprintf("sCheck is: %X\n\n", ptrHeader->sanityCheck);
  // Case1: ptr address is smaller than memSlot
  struct memHeader *curr = memSlot;

  // To be freed block will go at start of free list
  if (ptrHeader < curr) {
    // A. merging case, to be freed block is right beside memslot
    if (ptr + ptrHeader->size == curr) {
      unsigned long newSize = (unsigned long)(ptrHeader->size + curr->size +
                                              sizeof(struct memHeader));
      completeHeader(ptrHeader, newSize, NULL, curr->next);
      struct memHeader *tmp = curr->next;
      tmp->prev = ptrHeader;
      memSlot = ptrHeader;
      kprintf("Case A\n\n");
      return 1;
    }
    // B.
    completeHeader(ptrHeader, ptrHeader->size, NULL, curr);
    curr->prev = ptrHeader;
    memSlot = ptrHeader;
    kprintf("Case B\n\n");
    return 1;
  }

  if (ptrHeader > curr) {
    while (curr) {
      // Case where ptr goes to end of the free list
      if (curr->next == NULL) {
        // C. Case where ptr can be merged with the previous free block
        if (ptrHeader == curr + curr->size + sizeof(struct memHeader)) {
          unsigned long newSize = (unsigned long)(ptrHeader->size + curr->size +
                                                  sizeof(struct memHeader));
          curr->size = newSize;
          curr->sanityCheck = (char *)(curr->size + 16);
          kprintf("Case C\n\n");
          return 1;
        }
        // D. Non merge case, create new free chunk at end of list
        completeHeader(ptrHeader, ptrHeader->size, curr, NULL);
        curr->next = ptrHeader;
        kprintf("Case D\n\n");
        return 1;
      }
      // Case where ptr is between curr and curr->next in the free list
      if (curr->next > ptrHeader) {
        unsigned long currEndAddr =
            (unsigned long)(curr + curr->size + sizeof(struct memHeader));
        unsigned long ptrEndAddr = (unsigned long)(ptr + ptrHeader->size);
        struct memHeader *temp = curr->next;
        struct memHeader *tempNext = temp->next;

        // E. Case 1: Merge ptr with both curr and curr->next
        if ((void *)currEndAddr == ptrHeader &&
            (void *)ptrEndAddr == curr->next) {
          unsigned long newSize =
              (unsigned long)(curr->size + ptrHeader->size + temp->size +
                              2 * sizeof(struct memHeader));
          if (tempNext != NULL) {
            completeHeader(curr, newSize, curr->prev, tempNext);
            tempNext->prev = curr;
            kprintf("Case E\n\n");
            return 1;
          }
          completeHeader(curr, newSize, curr->prev, NULL);
          kprintf("Case E1\n\n");
          return 1;
        }
        // F. Case 2: Merge ptr with only curr (left)
        if ((void *)currEndAddr == ptrHeader &&
            (void *)ptrEndAddr != curr->next) {
          curr->size = curr->size + ptrHeader->size + sizeof(struct memHeader);
          curr->sanityCheck = (char *)(curr->size + 16);
          kprintf("Case F\n\n");
          return 1;
        }
        // G. Case 3: Merge ptr with only curr->next (right)
        if ((void *)currEndAddr != ptrHeader &&
            (void *)ptrEndAddr == curr->next) {
          // Need to update ptrHeader
          unsigned long newSize = (unsigned long)(ptrHeader->size + temp->size +
                                                  sizeof(struct memHeader));
          curr->next = ptrHeader;
          if (tempNext != NULL) {
            tempNext->prev = ptrHeader;
            completeHeader(ptrHeader, newSize, curr, tempNext);
            kprintf("Case G\n\n");
            return 1;
          }
          completeHeader(ptrHeader, newSize, curr, NULL);
          kprintf("Case G1\n\n");
          return 1;
        }
        // H. Case 4: Don't merge anything case
        if ((void *)currEndAddr != ptrHeader &&
            (void *)ptrEndAddr != curr->next) {
          completeHeader(ptrHeader, ptrHeader->size, curr, temp);
          curr->next = ptrHeader;
          temp->prev = ptrHeader;
          kprintf("Case H\n\n");
          return 1;
        }
      }
      curr = curr->next;
    }
  }
  kprintf("if you see this line, kfree failed");
  return 0;
}