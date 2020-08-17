#include <xeroskernel.h>

/* Function prototypes for the keyboard device */
int kbdOpen(pcb *p, void *device);
int kbdClose(pcb *p, int fd);
int kbdRead(pcb *p, int fd, void *buff, int bufflen);
int kbdWrite(pcb *p, int fd, void *buff, int bufflen);
int kbdIoctl(int fd, unsigned long command, int arg3);