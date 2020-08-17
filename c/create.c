/* create.c : create a process
 */

#include <xeroskernel.h>
#include <xeroslib.h>

pcb proctab[MAX_PROC];

/* make sure interrupts are armed later on in the kernel development  */
#define STARTING_EFLAGS 0x00003200

// PIDs can't start at 0 nor can they be negative
static PID_t nextpid = 0;

int create(funcptr fp, size_t stackSize) {
  /***********************************************/

  context_frame *cf;
  pcb *p = NULL;
  int i;

  /* If the PID becomes 0 it  has wrapped.
   * This means that the next PID we handout could be
   * in use. To find such a free number we have to propose a
   * new PID and then scan to see if it is in the table. If it
   * is then we have to try again.
   */

  // if (nextpid == 0) {
  //   return CREATE_FAILURE;
  // }

  // If the stack is too small make it larger
  if (stackSize < PROC_STACK) {
    stackSize = PROC_STACK;
  }

  for (i = 0; i < MAX_PROC; i++) {
    nextpid++;
    PID_t pIndex = nextpid % MAX_PROC;
    if (proctab[pIndex].state == STATE_STOPPED) {
      p = &proctab[pIndex];
      break;
    }
  }

  //    Some stuff to help wih debugging
  //    char buf[100];
  //    sprintf(buf, "Slot %d empty\n", i);
  //    kprintf(buf);
  //    kprintf("Slot %d empty\n", i);

  if (!p) {
    return CREATE_FAILURE;
  }

  // TODO: Make pid unique for constant time lookup and set return address
  // of process to be &sysstop.

  cf = kmalloc(stackSize);
  if (!cf) {
    return CREATE_FAILURE;
  }

  cf = (context_frame *)((unsigned char *)cf + stackSize - 4);
  cf--;

  memset(cf, 0xA5, sizeof(context_frame));

  cf->iret_cs = getCS();
  cf->iret_eip = (unsigned int)fp;
  cf->eflags = STARTING_EFLAGS;
  cf->returnAddr = (unsigned long)&sysstop;

  cf->esp = (int)(cf + 1);
  cf->ebp = cf->esp;
  p->esp = (unsigned long *)cf;
  p->state = STATE_READY;
  p->pid = nextpid;

  /* Modified process creation to make pid unique */
  // if (p->pid == 0) {
  //   p->pid = nextpid;
  // }
  // p->pid = p->pid + MAX_PROC;
  // if (nextpid > MAX_PROC) {
  // }

  ready(p);

  return p->pid;
}
