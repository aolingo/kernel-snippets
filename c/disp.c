/* disp.c : dispatcher
 */

#include <i386.h>
#include <stdarg.h>
#include <xeroskernel.h>
#include <xeroslib.h>

extern char *maxaddr;

int getCPUtimes(pcb *p, processStatuses *ps);
pcb *findPCB(int pid);

static int kill(pcb *currP, int pid, int signum);

static pcb *head = NULL;
static pcb *tail = NULL;

void dispatch(void) {
  /********************************/

  pcb *p;
  int r;
  funcptr fp;
  int stack;
  va_list ap;
  char *str;
  int len;
  int signum;
  int pid;
  void *old_sp;
  int old_ret;
  int sigLvl;
  int fd;
  int device_no;
  void *buff;
  int bufflen;
  unsigned long command;
  int readRet;
  int arg3;

  for (p = next(); p;) {
    //      kprintf("Process %x selected stck %x\n", p, p->esp);

    r = contextswitch(p);
    switch (r) {
      case (SYS_CREATE):
        ap = (va_list)p->args;
        fp = (funcptr)(va_arg(ap, int));
        stack = va_arg(ap, int);
        p->ret = create(fp, stack);
        break;
      case (SYS_YIELD):
        ready(p);
        p = next();
        break;
      case (SYS_STOP):
        p->state = STATE_STOPPED;
        p = next();
        break;
      case (SYS_KILL):
        ap = (va_list)p->args;
        pid = va_arg(ap, int);
        signum = va_arg(ap, int);
        p->ret = kill(p, pid, signum);
        break;
      case (SYS_CPUTIMES):
        ap = (va_list)p->args;
        p->ret = getCPUtimes(p, va_arg(ap, processStatuses *));
        break;
      case (SYS_PUTS):
        ap = (va_list)p->args;
        str = va_arg(ap, char *);
        kprintf("%s", str);
        p->ret = 0;
        break;
      case (SYS_GETPID):
        p->ret = p->pid;
        break;
      case (SYS_SLEEP):
        ap = (va_list)p->args;
        len = va_arg(ap, int);
        sleep(p, len);
        p = next();
        break;
      case (SYS_TIMER):
        tick();
        // kprintf("T");
        p->cpuTime++;
        ready(p);
        p = next();
        end_of_intr();
        break;
      case (SYS_SIGHANDLER):
        // kprintf("setting new handler\n");
        ap = (va_list)p->args;
        signum = va_arg(ap, int);
        sighandler_t handler = va_arg(ap, sighandler_t);
        // Check if handler is in the hole or beyond the end of main memory
        if ((((unsigned long)handler) >= HOLESTART &&
             ((unsigned long)handler <= HOLEEND)) ||
            (char *)handler > maxaddr) {
          p->ret = 0;
        } else {
          p->ret =
              (int)(p->sigtab[signum]);  // set ret to be addr of old handler
          p->sigtab[signum] = handler;
        }
        break;
      case (SYS_SIGRETURN):
        ap = (va_list)p->args;
        old_sp = va_arg(ap, void *);
        old_ret = va_arg(ap, int);
        sigLvl = va_arg(ap, int);
        int max = 1 << 31;  // 0x100000000...
        for (int i = max; i > 1; i = i / 2) {
          // find the highest priority signal (eg. most significant bit)
          if (i & p->sigmask) {
            // flip signal bit back to 0
            p->sigmask -= i;
            break;
          }
        }
        // restore process' old return value and stack pointer, update siglvl
        p->esp = old_sp;
        p->currSigLvl = sigLvl;
        p->ret = old_ret;
        break;
      case (SYS_WAIT):
        ap = (va_list)p->args;
        pid = va_arg(ap, int);
        // kprintf("dispatcher handling syswait\n");
        pcb *targetPCB = findPCB(pid);
        if (pid == 0 || !targetPCB) {
          p->ret = -1;
          break;
        }
        p->state = STATE_WAIT;
        p->waitpid = pid;
        p->ret = 0;
        // kprintf("proc %d's state is now %d\n", p->pid, p->state);
        // kprintf("proc %d is waiting for process %d\n", p->pid, p->waitpid);
        p = next();
        break;
      case (SYS_OPEN):
        ap = (va_list)p->args;
        device_no = va_arg(ap, int);
        p->ret = di_open(p, device_no);
        break;
      case (SYS_CLOSE):
        ap = (va_list)p->args;
        fd = va_arg(ap, int);
        p->ret = di_close(p, fd);
        break;
      case (SYS_READ):
        ap = (va_list)p->args;
        fd = va_arg(ap, int);
        buff = va_arg(ap, void *);
        bufflen = va_arg(ap, int);
        readRet = di_read(p, fd, buff, bufflen);
        if(readRet == -3){
          p = next();
        } else{
          p->ret = readRet;
        }
        break;
      case (SYS_WRITE):
        ap = (va_list)p->args;
        fd = va_arg(ap, int);
        buff = va_arg(ap, void *);
        bufflen = va_arg(ap, int);
        p->ret = di_write(p, fd, buff, bufflen);
        break;
      case (SYS_IOCTL):
        ap = (va_list)p->args;
        fd = va_arg(ap, int);
        command = va_arg(ap, unsigned long);
        arg3 = va_arg(ap, int);
        p->ret = di_ioctl(p, fd, command, arg3);
        break;
      case (SYS_KEYBOARD):
        kbdHandler(p);
        // ready(p);
        // p = next();
        end_of_intr();
        break;
      default:
        kprintf("Bad Sys request %d, pid = %d\n", r, p->pid);
    }
  }

  kprintf("Out of processes: dying\n");

  for (;;)
    ;
}

extern void dispatchinit(void) {
  /********************************/

  // bzero( proctab, sizeof( pcb ) * MAX_PROC );
  memset(proctab, 0, sizeof(pcb) * MAX_PROC);
}

extern void ready(pcb *p) {
  /*******************************/

  p->next = NULL;
  p->state = STATE_READY;

  if (tail) {
    tail->next = p;
  } else {
    head = p;
  }

  tail = p;
}

extern pcb *next(void) {
  /*****************************/

  pcb *p;

  p = head;

  if (p) {
    head = p->next;
    if (!head) {
      tail = NULL;
    }
  } else {  // Nothing on the ready Q and there should at least be the idle
            // proc.
    kprintf("BAD\n");
    for (;;)
      ;
  }

  p->next = NULL;
  p->prev = NULL;
  return (p);
}

extern pcb *findPCB(int pid) {
  /******************************/

  int i;

  for (i = 0; i < MAX_PROC; i++) {
    if (proctab[i].pid == pid) {
      return (&proctab[i]);
    }
  }

  return (NULL);
}

// This function takes a pointer to the pcbtab entry of the currently active
// process. The functions purpose is to remove the process being pointed to from
// the ready Q A similar function exists for the management of the sleep Q.
// Things should be re-factored to eliminate the duplication of code if
// possible. There are some challenges to that because the sleepQ is a delta
// list and something more than just removing an element in a list is being
// preformed.

void removeFromReady(pcb *p) {
  if (!head) {
    kprintf("Ready queue corrupt, empty when it shouldn't be\n");
    return;
  }

  if (head == p) {  // At front of list
    // kprintf("Pid %d is at front of list\n", p->pid);
    head = p->next;

    // If the implementation has idle on the ready list this next statement
    // isn't needed. However, it is left just in case someone decides to
    // change things so that the idle process is kept separate.

    if (head == NULL) {  // If the implementation has idle process  on the
      tail = head;       // ready list this should never happen
      kprintf("Kernel bug: Where is the idle process\n");
    }
  } else {  // Not at front, find the process.
    pcb *prev = head;
    pcb *curr;

    for (curr = head->next; curr != NULL; curr = curr->next) {
      if (curr == p) {  // Found process so remove it
        // kprintf("Found %d in list, removing\n", curr->pid);
        prev->next = p->next;
        if (tail == p) {  // last element in list
          tail = prev;
          // kprintf("Last element\n");
        }
        p->next = NULL;  // just to clean things up
        break;
      }
      prev = curr;
    }
    if (curr == NULL) {
      kprintf(
          "Kernel bug: Ready queue corrupt, process %d claimed on queue and "
          "not found\n",
          p->pid);
    }
  }
}

// This function takes 2 paramenters:
//  currP  - a pointer into the pcbtab that identifies the currently running
//  process pid    - the proces ID of the process to be killed.
//
// Note: this function needs to be augmented so that it delivers a kill signal
// to a
//       a particular process. The main functionality of the this routine will
//       remain the same except that when the process is located it needs to be
//       put onto the readyq and a signal needs to be marked for delivery.
//

static int kill(pcb *currP, int pid, int signum) {
  pcb *targetPCB;

  if (signum < 0 || signum > 31) {
    return -1;
  }
  if (!(targetPCB = findPCB(pid))) {
    // kprintf("Target pid not found\n");
    return -999;
  }

  // check if signal is already marked for delivery
  int mask = 1 << signum;
  if (targetPCB->sigmask & mask) {
    return 0;
  } else {
    if (targetPCB->state == STATE_WAIT) {
      // if target process is waiting, unblock it
      targetPCB->ret = -666;
      ready(targetPCB);
    }
    if (targetPCB->state == STATE_SLEEP) {
      targetPCB->ret = killInSleep(targetPCB);
      ready(targetPCB);
    }
    // register signal for delivery to target process
    // case for killing a process, unblock all procs waiting on p (parameter)
    if (signum == 31) {
      if (targetPCB->state == STATE_READY) {
        removeFromReady(targetPCB);
      } else if (targetPCB->state == STATE_SLEEP) {
        removeFromSleep(targetPCB);
      }
      for (int i = 0; i < MAX_PROC; i++) {
        pcb *proc = &proctab[i];
        if (proc->state == STATE_WAIT && proc->waitpid == targetPCB->pid) {
          ready(proc);
          proc->waitpid = 0;
        }
      }
    }
    return signal(targetPCB, signum);
  }
}

/* Memcopy implementation, referenced from:
 * https://www.geeksforgeeks.org/write-memcpy */
void memCpy(void *dest, void *src, size_t n) {
  // Typecast src and dest addresses to (char *)
  char *csrc = (char *)src;
  char *cdest = (char *)dest;

  // Copy contents of src[] to dest[]
  for (int i = 0; i < n; i++) cdest[i] = csrc[i];
}

// This function is the system side of the sysgetcputimes call.
// It places into a the structure being pointed to information about
// each currently active process.
//  p - a pointer into the pcbtab of the currently active process
//  ps  - a pointer to a processStatuses structure that is
//        filled with information about all the processes currently in the
//        system
//

int getCPUtimes(pcb *p, processStatuses *ps) {
  int i, currentSlot;
  currentSlot = -1;

  // Check if address is in the hole
  if (((unsigned long)ps) >= HOLESTART && ((unsigned long)ps <= HOLEEND))
    return -1;

  // Check if address of the data structure is beyone the end of main memory
  if ((((char *)ps) + sizeof(processStatuses)) > maxaddr) return -2;

  // There are probably other address checks that can be done, but this is OK
  // for now

  for (i = 0; i < MAX_PROC; i++) {
    if (proctab[i].state != STATE_STOPPED) {
      // fill in the table entry
      currentSlot++;
      ps->pid[currentSlot] = proctab[i].pid;
      ps->status[currentSlot] =
          p->pid == proctab[i].pid ? STATE_RUNNING : proctab[i].state;
      ps->cpuTime[currentSlot] = proctab[i].cpuTime * MILLISECONDS_TICK;
    }
  }

  return currentSlot;
}
