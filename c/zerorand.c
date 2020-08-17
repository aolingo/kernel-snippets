
#include <stdarg.h>
#include <xeroskernel.h>
#include <xeroslib.h>
#include <zerorand.h>

// Implement the upper and lower half of the device driver code for the zero
// device here The zero device is an infinite stream of 0s. Any read of the
// device for any number of bytes alwasy returns the requested number of bytes,
// but each byte is 0 (i.e. the bits are all 0s)
int zeroOpen(pcb *p, void *device) {
  for (int i = 0; i < MAX_FDT; i++) {
    // Find the next available fd in the process' fd table
    fdStruct *fdStruct = &p->fdt[i];
    if (fdStruct->status == CLOSE) {
      fdStruct->status = OPEN;
      fdStruct->device = device;
      return i;
    }
  }
  // Open failed since all fds (0 to 3) are already used
  return -1;
}

int zeroClose(pcb *p, int fd) {
  // set status of input fd in p's fdt to be CLOSE
  fdStruct *fdStruct = &p->fdt[fd];
  fdStruct->device = NULL;
  fdStruct->status = CLOSE;
  return 0;
}

int zeroRead(pcb *p, int fd, void *buff, int bufflen) {
  fdStruct *fdStruct = &p->fdt[fd];
  memset(buff, 0, bufflen);
  fdStruct->dataBuffer = 0;  // clear the data buff in device fdStruct
  return bufflen;
}

int zeroWrite(pcb *p, int fd, void *buff, int bufflen) {
  // consume all bytes from buffer and return bufflen
  fdStruct *fdStruct = &p->fdt[fd];
  memCpy(fdStruct->dataBuffer, buff, bufflen);
  return bufflen;
}

int zeroIoctl(int fd, unsigned long command, int seed) {
  // Zero device doesn't support ioctl
  return -1;
}

// Implement the upper and lower half of the device driver code for the random
// device here The random device is an infinite stream of random bytes. Any read
// of the device for any number of bytes alwasy returns the requested number of
// bytes, but each byte is a random value. Use the rand() and srand() functions
// from the xeroslib. The srand() function can be used to set a seed value for
// the rand() function
int randOpen(pcb *p, void *device) {
  for (int i = 0; i < MAX_FDT; i++) {
    // Find the next available fd in the process' fd table
    fdStruct *fdStruct = &p->fdt[i];
    if (fdStruct->status == CLOSE) {
      fdStruct->status = OPEN;
      fdStruct->device = device;
      return i;
    }
  }
  // Open failed since all fds (0 to 3) are already used
  return -1;
}

int randClose(pcb *p, int fd) {
  // set status of input fd in p's fdt to be CLOSE
  fdStruct *fdStruct = &p->fdt[fd];
  fdStruct->device = NULL;
  fdStruct->status = CLOSE;
  return 0;
}

int randRead(pcb *p, int fd, void *buff, int bufflen) {
  int i = 0;
  int bytesRead = 0;
  while (i < bufflen) {
    int r = rand() % 255;
    memset(buff + i, r, 4);
    i += 4;  // since rand() returns 4 bytes
    bytesRead += 4;
  }
  if (i < bufflen) {
    int diff = bufflen - i;
    int r = rand() % 255;
    memset(buff + i, r, diff);
    bytesRead += diff;
  }
  return bytesRead;
}

int randWrite(pcb *p, int fd, void *buff, int bufflen) {
  // Write is not supported for rand device
  return -1;
}

int randIoctl(int fd, unsigned long command, int seed) {
  if (command != 81) {
    kprintf("invalid command for randIoctl encountered\n");
    return -1;
  }
  srand(seed);
  return 0;
}
