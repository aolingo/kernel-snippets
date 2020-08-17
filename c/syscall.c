/* syscall.c : syscalls
 */

#include <stdarg.h>
#include <xeroskernel.h>

int syscall(int req, ...) {
  /**********************************/

  va_list ap;
  int rc;

  va_start(ap, req);

  __asm __volatile(
      " \
        movl %1, %%eax \n\
        movl %2, %%edx \n\
        int  %3 \n\
        movl %%eax, %0 \n\
        "
      : "=g"(rc)
      : "g"(req), "g"(ap), "i"(KERNEL_INT)
      : "%eax");

  va_end(ap);

  return (rc);
}

int syscreate(funcptr fp, size_t stack) {
  /*********************************************/

  return (syscall(SYS_CREATE, fp, stack));
}

void sysyield(void) {
  /***************************/
  syscall(SYS_YIELD);
}

void sysstop(void) {
  /**************************/

  syscall(SYS_STOP);
}

PID_t sysgetpid(void) {
  /****************************/

  return (syscall(SYS_GETPID));
}

void sysputs(char *str) {
  /********************************/

  syscall(SYS_PUTS, str);
}

unsigned int syssleep(unsigned int t) {
  /*****************************/

  return syscall(SYS_SLEEP, t);
}

int syskill(PID_t pid, int signum) {
  /*****************************/

  return syscall(SYS_KILL, pid, signum);
}

int sysgetcputimes(processStatuses *ps) {
  /*****************************/

  return syscall(SYS_CPUTIMES, ps);
}

sighandler_t syssignal(int signum, sighandler_t handler) {
  // return 0 if signum is not b/w 0-31 or 31 (since 31 has fixed handler)
  if (signum < 0 || signum >= 31) {
    kprintf("invalid signal number, error\n");
    return 0;
  }
  return (sighandler_t)syscall(SYS_SIGHANDLER, signum, handler);
}

void syssigreturn(void *old_sp, int old_ret, int sigLvl) {
  /*****************************/

  syscall(SYS_SIGRETURN, old_sp, old_ret, sigLvl);
}

int syswait(PID_t pid) {
  /*****************************/

  return syscall(SYS_WAIT, pid);
}

int sysopen(int device_no) {
  /*****************************/

  return syscall(SYS_OPEN, device_no);
}

int sysclose(int fd) {
  /*****************************/

  return syscall(SYS_CLOSE, fd);
}

int syswrite(int fd, void *buff, int bufflen) {
  /*****************************/

  return syscall(SYS_WRITE, fd, buff, bufflen);
}

int sysread(int fd, void *buff, int bufflen) {
  /*****************************/

  return syscall(SYS_READ, fd, buff, bufflen);
}

int sysioctl(int fd, unsigned long command, ...) {
  /*****************************/
  // TODO, double check with TA tmr
  va_list ap;
  va_start(ap, command);

  return syscall(SYS_IOCTL, fd, command, ap);
}