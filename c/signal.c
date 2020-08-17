#include <xeroskernel.h>
#include <xeroslib.h>

/* Your code goes here */
void sigtramp(void (*handler)(void *), void *cntx, void *old_sp, int old_ret,
              int sigLvl) {
  // cntx being handled
  handler(cntx);
  syssigreturn(old_sp, old_ret, sigLvl);
}

int signal(pcb *p, int signum) {
  if (p->sigtab[signum]) {
    int mask = 1 << signum;
    p->sigmask = p->sigmask | mask;
    // if signal is higher priority than current, push onto stack (deliver)
    if (signum > p->currSigLvl) {
      // Need to set up stack to handle new signal
      struct signal_frame *sigframe =
          (struct signal_frame *)(p->esp - sizeof(struct signal_frame));
      struct context_frame *cf =
          (struct context_frame *)(p->esp - sizeof(struct signal_frame) -
                                   sizeof(struct context_frame));
      cf->esp = (unsigned long)cf;
      cf->iret_eip = (unsigned long)sigtramp;
      cf->iret_cs = getCS();
      cf->eflags = 0x00003200;
      // Push arguments of sigtramp onto stack
      sigframe->handler = p->sigtab[signum];
      sigframe->cf = cf;
      sigframe->old_sp = p->esp;
      sigframe->old_ret = p->ret;
      sigframe->sigLvl = p->currSigLvl;

      p->currSigLvl = signum;
      p->esp = cf;
    }
    return 0;
  } else {
    // no handler function, signal is to be ignored
    return 0;
  }
}