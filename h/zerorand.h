/* Function prototypes for zero device */
int zeroOpen(pcb *p, void *device);
int zeroClose(pcb *p, int fd);
int zeroRead(pcb *p, int fd, void *buff, int bufflen);
int zeroWrite(pcb *p, int fd, void *buff, int bufflen);
int zeroIoctl(int fd, unsigned long command, int seed);

/* Function prototypes for random device */
int randOpen(pcb *p, void *device);
int randClose(pcb *p, int fd);
int randRead(pcb *p, int fd, void *buff, int bufflen);
int randWrite(pcb *p, int fd, void *buff, int bufflen);
int randIoctl(int fd, unsigned long command, int seed);