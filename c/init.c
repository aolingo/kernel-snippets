/* initialize.c - initproc */

#include <i386.h>
#include <kbd.h>
#include <xeroskernel.h>
#include <xeroslib.h>
#include <zerorand.h>

extern int entry(void); /* start of kernel image, use &start    */
extern int end(void);   /* end of kernel image, use &end        */
extern long freemem;    /* start of free memory (set in i386.c) */
extern char *maxaddr;   /* max memory address (set in i386.c)	*/

devsw devtable[MAX_DEV];

/*------------------------------------------------------------------------
 *  The idle process
 *------------------------------------------------------------------------
 */
static void idleproc(void) {
  int i;
  //    kprintf("I");
  for (i = 0;; i++) {
    sysyield();
  }
}

/************************************************************************/
/***				NOTE:				      ***/
/***								      ***/
/***   This is where the system begins after the C environment has    ***/
/***   been established.  Interrupts are initially DISABLED.  The     ***/
/***   interrupt table has been initialized with a default handler    ***/
/***								      ***/
/***								      ***/
/************************************************************************/

/*------------------------------------------------------------------------
 *  The init process, this is where it all begins...
 *------------------------------------------------------------------------
 */
void initproc(void) /* The beginning */
{
  // kprintf(
  //     "\n\nCPSC 415, 2019W1\n32 Bit Xeros -12.0.0 - even before beta
  //     \nLocated " "at: %x to %x\n", &entry, &end);

  /* Your code goes here */

  // kprintf("Max addr is %d %x\n", maxaddr, maxaddr);

  kmeminit();
  // kprintf("memory inited\n");

  dispatchinit();
  // kprintf("dispatcher inited\n");

  contextinit();
  // kprintf("context inited\n");

  devsw *zero = kmalloc(sizeof(devsw));
  zero->dvnum = 0;
  zero->dvopen = &zeroOpen;
  zero->dvclose = &zeroClose;
  zero->dvread = &zeroRead;
  zero->dvwrite = &zeroWrite;
  zero->dvioctl = &zeroIoctl;
  devtable[0] = *zero;
  devsw *rand = kmalloc(sizeof(devsw));
  rand->dvnum = 1;
  rand->dvopen = &randOpen;
  rand->dvclose = &randClose;
  rand->dvread = &randRead;
  rand->dvwrite = &randWrite;
  rand->dvioctl = &randIoctl;
  devtable[1] = *rand;
  devsw *kbd = kmalloc(sizeof(devsw));
  kbd->dvnum = 2;
  kbd->dvopen = &kbdOpen;
  kbd->dvclose = &kbdClose;
  kbd->dvread = &kbdRead;
  kbd->dvwrite = &kbdWrite;
  kbd->dvioctl = &kbdIoctl;
  devtable[2] = *kbd;
  // kprintf("device 0 in table has num of %d\n", devtable[0].dvnum);
  // kprintf("device table inited\n");

  // WARNING THE FIRST PROCESS CREATED MUST BE THE IDLE PROCESS.
  // See comments in create.c

  // Note that this idle process gets a regular time slice but
  // according to the A2 specs it should only get a time slice when
  // there are no other processes available to run. This approach
  // works, but will give the idle process a time slice when other
  // processes are available for execution and thereby needlessly waste
  // CPU resources that could be used by user processes. This is
  // somewhat migigated by the immediate call to sysyield()
  // kprintf("Creating Idle Process\n");

  create(idleproc, PROC_STACK);
  // create(test1, PROC_STACK);
  // create(test2, PROC_STACK);
  // create(test3, PROC_STACK);
  // create(test4, PROC_STACK);
  // create(invalidDeviceTest, PROC_STACK);
  // create(zeroDeviceTest, PROC_STACK);
  // create(randomDeviceTest, PROC_STACK);
  // create(keyboardTest, PROC_STACK);
  create(initProgram, PROC_STACK);

  // kprintf("create inited\n");

  dispatch();

  kprintf("Returned to init, you should never get here!\n");

  /* This code should never be reached after you are done */
  for (;;)
    ; /* loop forever */
}
