/* user.c : User processes
 */

#include <xeroskernel.h>
#include <xeroslib.h>

void busy(void) {
  int myPid;
  char buff[100];
  int i;
  int count = 0;

  myPid = sysgetpid();

  for (i = 0; i < 10; i++) {
    sprintf(buff, "My pid is %d\n", myPid);
    sysputs(buff);
    if (myPid == 2 && count == 1) syskill(3, 9);
    count++;
    sysyield();
  }
}

void sleep1(void) {
  int myPid;
  char buff[100];

  myPid = sysgetpid();
  sprintf(buff, "Sleeping 1000 is %d\n", myPid);
  sysputs(buff);
  syssleep(1000);
  sprintf(buff, "Awoke 1000 from my nap %d\n", myPid);
  sysputs(buff);
}

void sleep2(void) {
  int myPid;
  char buff[100];

  myPid = sysgetpid();
  sprintf(buff, "Sleeping 2000 pid is %d\n", myPid);
  sysputs(buff);
  syssleep(2000);
  sprintf(buff, "Awoke 2000 from my nap %d\n", myPid);
  sysputs(buff);
}

void sleep3(void) {
  int myPid;
  char buff[100];

  myPid = sysgetpid();
  sprintf(buff, "Sleeping 3000 pid is %d\n", myPid);
  sysputs(buff);
  syssleep(3000);
  sprintf(buff, "Awoke 3000 from my nap %d\n", myPid);
  sysputs(buff);
}

void producer(void) {
  /****************************/

  int i;
  char buff[100];

  // Sping to get some cpu time
  for (i = 0; i < 100000; i++)
    ;

  syssleep(3000);
  for (i = 0; i < 20; i++) {
    sprintf(buff, "Producer %x and in hex %x %d\n", i + 1, i, i + 1);
    sysputs(buff);
    syssleep(1500);
  }
  for (i = 0; i < 15; i++) {
    sysputs("P");
    syssleep(1500);
  }
  sprintf(buff, "Producer finished\n");
  sysputs(buff);
  sysstop();
}

void consumer(void) {
  /****************************/

  int i;
  char buff[100];

  for (i = 0; i < 50000; i++)
    ;
  syssleep(3000);
  for (i = 0; i < 10; i++) {
    sprintf(buff, "Consumer %d\n", i);
    sysputs(buff);
    syssleep(1500);
    sysyield();
  }

  for (i = 0; i < 40; i++) {
    sysputs("C");
    syssleep(700);
  }

  sprintf(buff, "Consumer finished\n");
  sysputs(buff);
  sysstop();
}

void root(void) {
  /****************************/

  char buff[100];
  int pids[5];
  int proc_pid, con_pid;
  int i;

  sysputs("Root has been called\n");

  // Test for ready queue removal.

  proc_pid = syscreate(&busy, 1024);
  con_pid = syscreate(&busy, 1024);
  sysyield();
  syskill(proc_pid, 9);
  sysyield();
  syskill(con_pid, 9);

  for (i = 0; i < 5; i++) {
    pids[i] = syscreate(&busy, 1024);
  }

  sysyield();

  syskill(pids[3], 9);
  sysyield();
  syskill(pids[2], 9);
  syskill(pids[4], 9);
  sysyield();
  syskill(pids[0], 9);
  sysyield();
  syskill(pids[1], 9);
  sysyield();

  syssleep(8000);
  ;

  kprintf("***********Sleeping no kills *****\n");
  // Now test for sleeping processes
  pids[0] = syscreate(&sleep1, 1024);
  pids[1] = syscreate(&sleep2, 1024);
  pids[2] = syscreate(&sleep3, 1024);

  sysyield();
  syssleep(8000);
  ;

  kprintf("***********Sleeping kill 2000 *****\n");
  // Now test for removing middle sleeping processes
  pids[0] = syscreate(&sleep1, 1024);
  pids[1] = syscreate(&sleep2, 1024);
  pids[2] = syscreate(&sleep3, 1024);

  syssleep(110);
  syskill(pids[1], 9);
  syssleep(8000);
  ;

  kprintf("***********Sleeping kill last 3000 *****\n");
  // Now test for removing last sleeping processes
  pids[0] = syscreate(&sleep1, 1024);
  pids[1] = syscreate(&sleep2, 1024);
  pids[2] = syscreate(&sleep3, 1024);

  sysyield();
  syskill(pids[2], 9);
  syssleep(8000);
  ;

  kprintf("***********Sleeping kill first process 1000*****\n");
  // Now test for first sleeping processes
  pids[0] = syscreate(&sleep1, 1024);
  pids[1] = syscreate(&sleep2, 1024);
  pids[2] = syscreate(&sleep3, 1024);

  syssleep(100);
  syskill(pids[0], 9);
  syssleep(8000);
  ;

  // Now test for 1 process

  kprintf("***********One sleeping process, killed***\n");
  pids[0] = syscreate(&sleep2, 1024);

  sysyield();
  syskill(pids[0], 9);
  syssleep(8000);
  ;

  kprintf("***********One sleeping process, not killed***\n");
  pids[0] = syscreate(&sleep2, 1024);

  sysyield();
  syssleep(8000);
  ;

  kprintf("***********Three sleeping processes***\n");  //
  pids[0] = syscreate(&sleep1, 1024);
  pids[1] = syscreate(&sleep2, 1024);
  pids[2] = syscreate(&sleep3, 1024);

  // Producer and consumer started too
  proc_pid = syscreate(&producer, 4096);
  con_pid = syscreate(&consumer, 4096);
  sprintf(buff, "Proc pid = %d Con pid = %d\n", proc_pid, con_pid);
  sysputs(buff);

  processStatuses psTab;
  int procs;

  syssleep(500);
  procs = sysgetcputimes(&psTab);

  for (int j = 0; j <= procs; j++) {
    sprintf(buff, "%4d    %4d    %10d\n", psTab.pid[j], psTab.status[j],
            psTab.cpuTime[j]);
    kprintf(buff);
  }

  syssleep(10000);
  procs = sysgetcputimes(&psTab);

  for (int j = 0; j <= procs; j++) {
    sprintf(buff, "%4d    %4d    %10d\n", psTab.pid[j], psTab.status[j],
            psTab.cpuTime[j]);
    kprintf(buff);
  }

  sprintf(buff, "Root finished\n");
  sysputs(buff);
  sysstop();

  for (;;) {
    sysyield();
  }
}

/* Test for syssignalhandler */
void test1(void) {
  sysputs("Test1 has been called\n");
  syssleep(50000);
  // kprintf("address of sysstop is %x\n", &sysstop);
  // int ret1 = syssignal(4, &sysstop);
  // kprintf("addr of old handler ret1 is %x\n", ret1);
  // int ret2 = syssignal(40, 0);
  // kprintf("return value of ret2 is %x\n", ret2);
  // int ret3 = syssignal(4, 0);
  // kprintf("addr of old handler ret3 is %x\n", ret3);
}

/* Test for syswait, syskill and signals */
void test2(void) {
  sysputs("Test2 has been called\n");
  // syssignal(4, &hello);
  // syssignal(25, &bye);
  // int ret3 = syswait(1);
  // kprintf("syswait result for ret3 is %d\n", ret3);
}

void test3(void) {
  sysputs("Test3 has been called\n");
  syskill(1, 31);
}

void test4(void) {
  sysputs("Test4 has been called\n");
  syskill(2, 25);
}

void invalidDeviceTest(void) {
  sysputs("Invalid Device tests are being called\n");
  int ret1 = sysopen(99);
  kprintf("return value of sysopen is %d\n", ret1);
  int ret2 = syswrite(99, 0, 0);
  kprintf("return value of syswrite is %d\n", ret2);
  int fd = sysopen(1);
  int ret3 = sysioctl(fd, 99, 3);
  kprintf("return value of sysioctl is %d\n", ret3);
}

void zeroDeviceTest(void) {
  sysputs("Zero Device test has been called\n");
  int fd = sysopen(0);
  int* sendBuff = kmalloc(800);
  int* readBuff = kmalloc(800);
  memset(readBuff, 8, 40);
  kprintf("Writing 16 bytes to zero dev\n");
  int ret1 = syswrite(fd, sendBuff, 16);
  kprintf("Number of bytes written: %d\n", ret1);
  kprintf("memset 40 bytes to readBuff\n");
  kprintf("First 20 bytes of readBuff before read:\n");
  for (int i = 0; i < 5; i++) {
    kprintf("%d\n", *(readBuff + i));
  }
  kprintf("First 20 bytes of readBuff after read:\n");
  sysread(fd, readBuff, 16);
  for (int i = 0; i < 5; i++) {
    kprintf("%d\n", *(readBuff + i));
  }
}

void keyboardTest() {
  kprintf("testing keyboard\n");
  // int fd = sysopen(2);
  // int writeBuff = kmalloc(800);
  // int ret = syswrite(fd, writeBuff, 16);
  // kprintf("ret value is: %d", ret);
}

void randomDeviceTest() {
  sysputs("Random Device test has been called\n");
  int fd = sysopen(1);
  int* readBuff = kmalloc(800);
  memset(readBuff, 0, 40);
  sysread(fd, readBuff, 16);
}

void initProgram() {
  char* username = "cs415";
  char* password = "hungryForKnowledge";
  int fd;
  char temp1 = NULL;
  char temp2 = NULL;
  char* user = &temp1;
  char* pw = &temp2;
  kprintf("Welcome to Xeros 415 - A not very secure Kernel\n");
  fd = sysopen(2);
  // kprintf("fd for kbd is %d\n", fd);
  kprintf("Username: \n");
  sysread(fd, user, 4);
  sysioctl(fd, 55);
  kprintf("Password: \n");
  sysread(fd, pw, 4);
  // sysclose(fd);
  if (strcmp(user, username) == 0 && strcmp(pw, password) == 0) {
    // create shell process if user and pw are correct
    int pid = create(&shell, PROC_STACK);
    syswait(pid);
  }
}

void shell() {
  for (;;) {
    kprintf(">\n");
    // TODO
    // char* ps = "ps";
    // char* ex = "ex";
    // char* k = "k";
    // char* a = "a";
    // char* t = "t";
    // char* R = "R";
  }
}

/* Handler functions to be used for signal tests */
void hello(void) { kprintf("hello world\n"); }

void bye(void) { kprintf("goodbye world\n"); }
