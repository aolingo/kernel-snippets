
#include <di_calls.h>
#include <i386.h>
#include <kbd.h>
#include <stdarg.h>
#include <xeroskernel.h>

devsw devtable[MAX_DEV];
extern char *maxaddr;

// Implement the Device Indepedent code in this file.

/* The corresponding DII call for sysopen()
returns a fd in [0,3] on success and -1 on failure */
int di_open(pcb *p, int device_no) {
  for (int i = 0; i < MAX_DEV; i++) {
    if (devtable[i].dvnum == device_no) {
      // call device specific open
      devsw *device = &devtable[i];
      return device->dvopen(p, device);
    }
  }
  // Open failed, couldn't find device with device_no
  kprintf("device not found in open\n");
  return -1;
}

/* DII call for sysclose()
Returns 0 on success and -1 on failure */
int di_close(pcb *p, int fd) {
  fdStruct *fdStruct = &p->fdt[fd];
  if (fd < 0 || fd > 3 || fdStruct->status == CLOSE) {
    kprintf("invalid fd for close\n");
    return -1;
  }
  return fdStruct->device->dvclose(p, fd);
}

int di_read(pcb *p, int fd, void *buff, int bufflen) {
  fdStruct *fdStruct = &p->fdt[fd];
  if (fd < 0 || fd > 3 || fdStruct->status == CLOSE) {
    kprintf("invalid fd for read\n");
    return -1;
  }
  // buffer addr is invalid
  if ((((unsigned long)buff) >= HOLESTART &&
       ((unsigned long)buff <= HOLEEND)) ||
      (char *)buff > maxaddr) {
    return -1;
  }
  return fdStruct->device->dvread(p, fd, buff, bufflen);
}

int di_write(pcb *p, int fd, void *buff, int bufflen) {
  fdStruct *fdStruct = &p->fdt[fd];
  if (fd < 0 || fd > 3 || fdStruct->status == CLOSE) {
    kprintf("invalid fd for write\n");
    return -1;
  }
  // buffer addr is invalid
  if ((((unsigned long)buff) >= HOLESTART &&
       ((unsigned long)buff <= HOLEEND)) ||
      (char *)buff > maxaddr) {
    return -1;
  }
  return fdStruct->device->dvwrite(p, fd, buff, bufflen);
}

int di_ioctl(pcb *p, int fd, unsigned long command, int arg3) {
  fdStruct *fdStruct = &p->fdt[fd];
  if (fd < 0 || fd > 3 || fdStruct->status == CLOSE) {
    kprintf("invalid fd for ioctl\n");
    return -1;
  }
  return fdStruct->device->dvioctl(fd, command, arg3);
}