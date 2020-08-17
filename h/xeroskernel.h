/* xeroskernel.h - disable, enable, halt, restore, isodd, min, max */

#ifndef XEROSKERNEL_H
#define XEROSKERNEL_H

/* Symbolic constants used throughout Xinu */

typedef char Bool;           /* Boolean type                  */
typedef unsigned int size_t; /* Something that can hold the value of
                              * theoretical maximum number of bytes
                              * addressable in this architecture.
                              */

typedef unsigned int PID_t;  // What a process ID is defined to be

#define FALSE 0 /* Boolean constants             */
#define TRUE 1
#define EMPTY (-1)  /* an illegal gpq                */
#define NULL 0      /* Null pointer for linked lists */
#define NULLCH '\0' /* The null character            */

#define CREATE_FAILURE -1 /* Process creation failed     */

/* Universal return constants */

#define OK 1        /* system call ok               */
#define SYSERR -1   /* system call failed           */
#define EOF -2      /* End-of-file (usu. from read)	*/
#define TIMEOUT -3  /* time out  (usu. recvtim)     */
#define INTRMSG -4  /* keyboard "intr" key pressed	*/
                    /*  (usu. defined as ^B)        */
#define BLOCKERR -5 /* non-blocking op would block  */

/* Functions defined by startup code */

void bzero(void *base, int cnt);
void bcopy(const void *src, void *dest, unsigned int n);
void disable(void);
unsigned short getCS(void);
unsigned char inb(unsigned int);
void init8259(void);
int kprintf(char *fmt, ...);
void lidt(void);
void outb(unsigned int, unsigned char);

/* Some constants involved with process creation and managment */

/* Maximum number of processes */
#define MAX_PROC 64
/* Maximum number of devices */
#define MAX_DEV 3
/* Maximum number of signals */
#define MAX_SIG 32
/* Maximum number of fdt entries in process */
#define MAX_FDT 4
/* Kernel trap number          */
#define KERNEL_INT 80
/* Interrupt number for the timer */
#define TIMER_INT (TIMER_IRQ + 32)
/* Interrupt number for the keyboard */
#define KEYBOARD_INT 33
/* Minimum size of a stack when a process is created */
#define PROC_STACK (4096 * 4)

/* Number of milliseconds in a tick */
#define MILLISECONDS_TICK 10

/* Constants to track states that a process is in */
#define STATE_STOPPED 0
#define STATE_READY 1
#define STATE_WAIT 21
#define STATE_SLEEP 22
#define STATE_RUNNING 23
#define STATE_BLOCKED 24

/* System call identifiers */
#define SYS_STOP 10
#define SYS_YIELD 11
#define SYS_CREATE 22
#define SYS_TIMER 33
#define SYS_GETPID 144
#define SYS_PUTS 155
#define SYS_SLEEP 166
#define SYS_KILL 177
#define SYS_CPUTIMES 178
#define SYS_SIGHANDLER 179
#define SYS_SIGRETURN 180
#define SYS_WAIT 181
#define SYS_OPEN 182
#define SYS_CLOSE 183
#define SYS_WRITE 184
#define SYS_READ 185
#define SYS_IOCTL 186
#define SYS_KEYBOARD 187

/* Device table identifiers */
#define DEV_ZERO 0
#define DEV_RANDOM 1
#define DEV_KBD 2

/* Device fd identifiers */
#define OPEN 1
#define CLOSE 0
#define ECHO_ON 1
#define ECHO_OFF 0

/* Structure to track the information associated with a single process */

typedef struct struct_pcb pcb;
typedef void (*sighandler_t)(void *);

/* Device structure */
typedef struct devsw {
  int dvnum;
  int (*dvopen)(pcb *p, void *device);
  int (*dvclose)(pcb *p, int fd);
  int (*dvread)(pcb *p, int fd, void *buff, int bufflen);
  int (*dvwrite)(pcb *p, int fd, void *buff, int bufflen);
  int (*dvioctl)(int fd, unsigned long command, int arg3);
} devsw;

extern devsw devtable[MAX_DEV];

/* Structure to track info associated with a FDT entry */
typedef struct fdStruct {
  devsw *device;
  int status;
  void *dataBuffer;  // data buffer for fd in device
} fdStruct;

struct struct_pcb {
  void *esp; /* Pointer to top of saved stack           */
  pcb *next; /* Next process in the list, if applicable */
  pcb *prev; /* Previous proccess in list, if applicable*/
  int state; /* State the process is in, see above      */
  PID_t pid; /* The process's ID                        */
  int ret;   /* Return value of system call             */
             /* if process interrupted because of system*/
             /* call                                    */
  long args;
  unsigned int otherpid;
  void *buffer;
  int bufferlen;
  int sleepdiff;
  long cpuTime; /* CPU time consumed                     */
  sighandler_t sigtab[MAX_SIG];
  int sigmask;
  int currSigLvl;
  PID_t waitpid;
  fdStruct fdt[MAX_FDT];
};

typedef struct struct_ps processStatuses;
struct struct_ps {
  int entries;             // Last entry used in the table
  PID_t pid[MAX_PROC];     // The process ID
  int status[MAX_PROC];    // The process status
  long cpuTime[MAX_PROC];  // CPU time used in milliseconds
};

/* The actual space is set aside in create.c */
extern pcb proctab[MAX_PROC];

#pragma pack(1)

/* What the set of pushed registers looks like on the stack */
typedef struct context_frame {
  unsigned long edi;
  unsigned long esi;
  unsigned long ebp;
  unsigned long esp;
  unsigned long ebx;
  unsigned long edx;
  unsigned long ecx;
  unsigned long eax;
  unsigned long iret_eip;
  unsigned long iret_cs;
  unsigned long eflags;
  unsigned long stackSlots[];
} context_frame;

/* Struct for signal processing */
typedef struct signal_frame {
  void *return_useless;
  void (*handler)(void *);
  context_frame *cf;
  void *old_sp;
  int old_ret;
  int sigLvl;
} signal_frame;

/* Memory mangement system functions, it is OK for user level   */
/* processes to call these.                                     */

int kfree(void *ptr);
void kmeminit(void);
void *kmalloc(size_t);

/* A typedef for the signature of the function passed to syscreate */
typedef void (*funcptr)(void);

/* Internal functions for the kernel, applications must never  */
/* call these.                                                 */
void dispatch(void);
void dispatchinit(void);
void ready(pcb *p);
pcb *next(void);
void contextinit(void);
int contextswitch(pcb *p);
int create(funcptr fp, size_t stack);
void set_evec(unsigned int xnum, unsigned long handler);
void printCF(void *stack);  /* print the call frame */
int syscall(int call, ...); /* Used in the system call stub */
void sleep(pcb *, unsigned int);
void tick(void);
void removeFromSleep(pcb *p);
int killInSleep(pcb *p);
void deviceinit(void);
int di_open(pcb *p, int device_no);
int di_close(pcb *p, int fd);
int di_read(pcb *p, int fd, void *buff, int bufflen);
int di_write(pcb *p, int fd, void *buff, int bufflen);
int di_ioctl(pcb *p, int fd, unsigned long command, int arg3);
void kbdHandler(pcb *p); /* Interrupt handler for keyboard */

/* Function prototypes for system calls as called by the application */
int syscreate(funcptr fp, size_t stack);
void sysyield(void);
void sysstop(void);
PID_t sysgetpid(void);
PID_t syssleep(unsigned int);
void sysputs(char *str);
int syskill(PID_t pid, int signum);
int sysgetcputimes(processStatuses *ps);
// Signals
sighandler_t syssignal(int signum, sighandler_t handler);
void syssigreturn(void *old_sp, int old_re, int sigLvl);
int syswait(PID_t pid);
// Devices
int sysopen(int device_no);
int sysclose(int fd);
int syswrite(int fd, void *buff, int bufflen);
int sysread(int fd, void *buff, int bufflen);
int sysioctl(int fd, unsigned long command, ...);

/* Function protoypes for signal.c */
void sigtramp(void (*handler)(void *), void *cntx, void *old_sp, int old_ret,
              int sigLvl);
int signal(pcb *p, int signum);

/* The initial process that the system creates and schedules */
void root(void);

/* Init program for A3 */
void initProgram(void);
void shell(void);

/* Processes for tests */
void test1(void);
void test2(void);
void test3(void);
void test4(void);
void hello(void);
void bye(void);
void invalidDeviceTest(void);
void zeroDeviceTest(void);
void randomDeviceTest(void);
void memCpy(void *dest, void *src, size_t n);
void keyboardTest(void);

void set_evec(unsigned int xnum, unsigned long handler);

/* Anything you add must be between the #define and this comment */
#endif
