/* user.c : User processes
 */

#include <xeroskernel.h>
#include <xeroslib.h>

/* Your code goes here */
void producer(void) {
  /****************************/

  int i;

  for (i = 0; i < 5; i++) {
    kprintf("Produce %d\n", i);
    // Syskill test
    // int c = syskill(3);
    // kprintf("kill result is %d\n", c);
    // Syssend error tests
    // if (i == 4) {
    //   /* send and recv tests */
    //   char *testBuff = kmalloc(800);
    //   strncpy(testBuff, "hello", 5);
    //   syssend(3, testBuff, 6);
    // }
    sysyield();
  }
  /* Syscalls tests */
  //   int pid = sysgetpid();
  //   kprintf("%d\n", pid);
  //   sysputs("hello");
  // for (int i = 0; i < 20000000; i++) {
  // }
  sysstop();
}

void consumer(void) {
  /****************************/

  int i;

  for (i = 0; i < 5; i++) {
    kprintf("Consume %d \n", i);
    // Syssend and recv error tests
    // syssend(5, 2, 2);
    // sysrecv(3, 2, 2);
    // if (i == 4) {
    //   char *testBuff = kmalloc(800);
    //   strncpy(testBuff, "donald", 3);
    //   sysrecv(2, testBuff, 3);
    //   kprintf("receiving buffer is  %s\n", testBuff);
    // }
    sysyield();
  }
  //   int pid = sysgetpid();
  //   kprintf("%d\n", pid);
  //   sysputs("good bye");
  // for (int i = 0; i < 20000000; i++) {
  // }
  sysstop();
}

void proc1(void) {
  kprintf("Process 3 is now alive\n");
  syssleep(5000);
  int *recvBuff = kmalloc(800);
  sysrecv((PID_t *)2, recvBuff, sizeof(int));
  kprintf("Process 3 message received and time to sleep is %d\n", recvBuff);
  syssleep((unsigned int)recvBuff);
  kprintf("Process 3 sleep has stopped, exiting\n");
  /* kernel reboots if we remove this loop, we think it probably has to do
  with running to end of code too soon */
  for (int i = 0; i < 10000000; i++) {
  }
}

void proc2(void) {
  kprintf("Process 4 is now alive\n");
  syssleep(5000);
  int *recvBuff = kmalloc(800);
  sysrecv((PID_t *)2, recvBuff, sizeof(int));
  kprintf("Process 4 message received and time to sleep is %d\n", recvBuff);
  syssleep((unsigned int)recvBuff);
  kprintf("Process 4 sleep has stopped, exiting\n");
  for (int i = 0; i < 10000000; i++) {
  }
}

void proc3(void) {
  kprintf("Process 5 is now alive\n");
  syssleep(5000);
  int *recvBuff = kmalloc(800);
  sysrecv((PID_t *)2, recvBuff, sizeof(int));
  kprintf("Process 5 message received and time to sleep is %d\n", recvBuff);
  syssleep((unsigned int)recvBuff);
  kprintf("Process 5 sleep has stopped, exiting\n");
  for (int i = 0; i < 10000000; i++) {
  }
}

void proc4(void) {
  kprintf("Process 6 is now alive\n");
  syssleep(5000);
  int *recvBuff = kmalloc(800);
  sysrecv((PID_t *)2, recvBuff, sizeof(int));
  kprintf("Process 6 message received and time to sleep is %d\n", recvBuff);
  syssleep((unsigned int)recvBuff);
  kprintf("Process 6 sleep has stopped, exiting\n");
  for (int i = 0; i < 10000000; i++) {
  }
}

/* Send success test case */
// void sendTest1(void) {
//   char *testBuff = kmalloc(800);
//   memCpy(testBuff, "hello", 5);
//   int result = syssend(4, testBuff, 5);
//   kprintf("# of bytes sent is %d and info sent is %s\n", result,
//   testBuff); for (int i = 0; i < 10000000; i++) {
//   }
// }

/* Send fail test case: send to non-existing process */
// void sendTest2(void) {
//   char *testBuff = kmalloc(800);
//   memCpy(testBuff, "hello", 5);
//   int result = syssend(100, testBuff, 5);
//   kprintf("send result is %d \n", result);
// }

/* Send fail test case 2: send to self */
// void sendTest3(void) {
//   char *testBuff = kmalloc(800);
//   memCpy(testBuff, "hello", 5);
//   int result = syssend(3, testBuff, 5);
//   kprintf("send result is %d \n", result);
// }

/* Recv success test case */
// void recvTest1(void) {
//   char *testBuff = kmalloc(800);
//   memCpy(testBuff, "donald", 6);
//   PID_t *from_pid = (PID_t *)3;
//   int result = sysrecv(from_pid, testBuff, 5);
//   kprintf("# of bytes received is %d and result is %s\n", result,
//   testBuff); for (int i = 0; i < 10000000; i++) {
//   }
// }

/* Recv any success test case */
// void recvTest2(void) {
//   char *testBuff = kmalloc(800);
//   memCpy(testBuff, "donald", 6);
//   PID_t *from_pid = 0;
//   int result = sysrecv(from_pid, testBuff, 5);
//   kprintf("# of bytes received is %d and result is %s\n", result,
//   testBuff); for (int i = 0; i < 10000000; i++) {
//   }
// }

/* Recv fail test case 1: from process does not exist */
// void recvTest3(void) {
//   char *testBuff = kmalloc(800);
//   memCpy(testBuff, "donald", 6);
//   PID_t *from_pid = (PID_t *)100;
//   int result = sysrecv(from_pid, testBuff, 5);
//   kprintf("recv result is %d\n", result);
// }

/* Recv fail test case 2: from process is itself */
// void recvTest4(void) {
//   char *testBuff = kmalloc(800);
//   memCpy(testBuff, "donald", 6);
//   PID_t *from_pid = (PID_t *)3;
//   int result = sysrecv(from_pid, testBuff, 5);
//   kprintf("recv result is %d\n", result);
// }

void root(void) {
  /****************************/
  PID_t proc1_pid, proc2_pid, proc3_pid, proc4_pid;

  kprintf("Root process has been called\n");

  sysyield();
  sysyield();
  proc1_pid = syscreate(&proc1, 4096);
  kprintf("Root process has created process %u\n", proc1_pid);
  proc2_pid = syscreate(&proc2, 4096);
  kprintf("Root process has created process %u\n", proc2_pid);
  proc3_pid = syscreate(&proc3, 4096);
  kprintf("Root process has created process %u\n", proc3_pid);
  proc4_pid = syscreate(&proc4, 4096);
  kprintf("Root process has created process %u\n", proc4_pid);

  syssleep(4000);

  int *sendBuff = kmalloc(800);
  memCpy(sendBuff, (int *)10000, sizeof(int));
  syssend(5, sendBuff, sizeof(int));
  memCpy(sendBuff, (int *)7000, sizeof(int));
  syssend(4, sendBuff, sizeof(int));
  memCpy(sendBuff, (int *)20000, sizeof(int));
  syssend(3, sendBuff, sizeof(int));
  memCpy(sendBuff, (int *)27000, sizeof(int));
  syssend(6, sendBuff, sizeof(int));

  char *recvBuff = kmalloc(800);
  int result = sysrecv((PID_t *)6, recvBuff, 2);
  kprintf("Root process receive result from proc 4 is %d\n", result);

  result = syssend(5, sendBuff, sizeof(int));
  kprintf("Root process send result to proc 3 is %d\n", result);

  syskill(2);
  /* Tests for send and recv */
  // PID_t send_pid, recv_pid;

  /* 3 Tests for send */
  // send_pid = syscreate(&sendTest1, 4096);
  // send_pid = syscreate(&sendTest2, 4096);
  // send_pid = syscreate(&sendTest3, 4096);

  /* 4 Tests for receive */
  // recv_pid = syscreate(&recvTest1, 4096);
  // recv_pid = syscreate(&recvTest2, 4096);
  // recv_pid = syscreate(&recvTest3, 4096);
  //  recv_pid = syscreate(&recvTest4, 4096);

  // kprintf("SendTest pid = %u RecvTest pid = %u\n", send_pid, recv_pid);

  for (int i = 0; i < 10000000; i++) {
  }
}

void idleproc() {
  for (;;) {
    // kprintf("idle proc now running\n");
  }
}
