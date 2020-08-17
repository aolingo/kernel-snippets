
#include <kbd.h>
#include <stdarg.h>
#include <xeroskernel.h>
#include <xeroslib.h>

// Implement the upper and lower half of the keyboard device driver in this
// file.
#define KEY_UP 0x80 /* If this bit is on then it is a key   */
                    /* up event instead of a key down event */

/* Control code */
#define LSHIFT 0x2a
#define RSHIFT 0x36
#define LMETA 0x38

#define LCTL 0x1d
#define CAPSL 0x3a

/* scan state flags */
#define INCTL 0x01    /* control key is down          */
#define INSHIFT 0x02  /* shift key is down            */
#define CAPSLOCK 0x04 /* caps lock mode               */
#define INMETA 0x08   /* meta (alt) key is down       */
#define EXTENDED 0x10 /* in extended character mode   */

#define EXTESC 0xe0 /* extended character escape    */
#define NOCHAR 256

static int state;     /* the state of the keyboard */
static int eof = 0x4; /* global variable of eof indicator, default CTRL-D */
static int echo =
    ECHO_ON;            /* kbd echo state, default is on when kbd first opens*/
static char kbdbuff[4]; /* data buffer for kbd */

void enable_irq(unsigned int, int);

/*  Normal table to translate scan code  */
unsigned char kbcode[] = {0,    27,  '1', '2',  '3',  '4',  '5', '6', '7',  '8',
                          '9',  '0', '-', '=',  '\b', '\t', 'q', 'w', 'e',  'r',
                          't',  'y', 'u', 'i',  'o',  'p',  '[', ']', '\n', 0,
                          'a',  's', 'd', 'f',  'g',  'h',  'j', 'k', 'l',  ';',
                          '\'', '`', 0,   '\\', 'z',  'x',  'c', 'v', 'b',  'n',
                          'm',  ',', '.', '/',  0,    0,    0,   ' '};

/* captialized ascii code table to tranlate scan code */
unsigned char kbshift[] = {0,   0,   '!', '@', '#',  '$',  '%', '^', '&',  '*',
                           '(', ')', '_', '+', '\b', '\t', 'Q', 'W', 'E',  'R',
                           'T', 'Y', 'U', 'I', 'O',  'P',  '{', '}', '\n', 0,
                           'A', 'S', 'D', 'F', 'G',  'H',  'J', 'K', 'L',  ':',
                           '"', '~', 0,   '|', 'Z',  'X',  'C', 'V', 'B',  'N',
                           'M', '<', '>', '?', 0,    0,    0,   ' '};
/* extended ascii code table to translate scan code */
unsigned char kbctl[] = {0,  0,    0,    0,  0,  0,  0,  0,  0,  0,  0,  0,  31,
                         0,  '\b', '\t', 17, 23, 5,  18, 20, 25, 21, 9,  15, 16,
                         27, 29,   '\n', 0,  1,  19, 4,  6,  7,  8,  10, 11, 12,
                         0,  0,    0,    0,  28, 26, 24, 3,  22, 2,  14, 13};

static int extchar(unsigned char code) {
  state &= ~EXTENDED;
  return NOCHAR;
}

unsigned int kbtoa(unsigned char code) {
  unsigned int ch;

  if (state & EXTENDED) return extchar(code);
  if (code & KEY_UP) {
    switch (code & 0x7f) {
      case LSHIFT:
      case RSHIFT:
        state &= ~INSHIFT;
        break;
      case CAPSL:
        // kprintf("Capslock off detected\n");
        state &= ~CAPSLOCK;
        break;
      case LCTL:
        state &= ~INCTL;
        break;
      case LMETA:
        state &= ~INMETA;
        break;
    }

    return NOCHAR;
  }

  /* check for special keys */
  switch (code) {
    case LSHIFT:
    case RSHIFT:
      state |= INSHIFT;
      // kprintf("shift detected!\n");
      return NOCHAR;
    case CAPSL:
      state |= CAPSLOCK;
      // kprintf("Capslock ON detected!\n");
      return NOCHAR;
    case LCTL:
      state |= INCTL;
      return NOCHAR;
    case LMETA:
      state |= INMETA;
      return NOCHAR;
    case EXTESC:
      state |= EXTENDED;
      return NOCHAR;
  }

  ch = NOCHAR;

  if (code < sizeof(kbcode)) {
    if (state & CAPSLOCK)
      ch = kbshift[code];
    else
      ch = kbcode[code];
  }
  if (state & INSHIFT) {
    if (code >= sizeof(kbshift)) return NOCHAR;
    if (state & CAPSLOCK)
      ch = kbcode[code];
    else
      ch = kbshift[code];
  }
  if (state & INCTL) {
    if (code >= sizeof(kbctl)) return NOCHAR;
    ch = kbctl[code];
  }
  if (state & INMETA) ch += 0x80;
  return ch;
}

void kbdHandler(pcb *p) {
  // check if there's data to read from kbd (port 0x60)
  if (inb(0x64) & 0x01) {
    unsigned char scanCode = inb(0x60);
    unsigned int asciiCode = kbtoa(scanCode);
    if (asciiCode == eof) {
      // close keyboard
      for (int i = 0; i < 4; i++) {
        if (!kbdbuff[i]) {
          kbdbuff[i] = eof;
          break;
        }
      }
      kbdClose(p, 0);
    } else if (asciiCode == 0x09) {
      // tab key to toggle echo
      echo = !echo;
    } else {
      // add typed data into buffer
      for (int i = 0; i < 4; i++) {
        if (!kbdbuff[i]) {
          kbdbuff[i] = eof;
          break;
        }
      }
      if (echo) {
        // print char since echo is on
        kprintf("%c", asciiCode);
      }
    }
  }
}

int kbdOpen(pcb *p, void *device) {
  for (int i = 0; i < MAX_FDT; i++) {
    // Find the next available fd in the process' fd table
    fdStruct *fdStruct = &p->fdt[i];
    if (fdStruct->status == CLOSE) {
      fdStruct->status = OPEN;
      fdStruct->device = device;
      enable_irq(1, 0);
      return i;
    }
  }
  // Open failed since all fds (0 to 3) are already used
  return -1;
}

int kbdClose(pcb *p, int fd) {
  echo = ECHO_ON;    // reset kbd echo back to on
  enable_irq(1, 1);  // turn off keyboard int
  fdStruct *fdStruct = &p->fdt[fd];
  fdStruct->device = NULL;
  fdStruct->status = CLOSE;
  return 0;
}

int kbdRead(pcb *p, int fd, void *buff, int bufflen) {
  int isEmpty = 1;
  for (int i = 0; i < 4; i++) {
    if (kbdbuff[i]) {
      isEmpty = 0;
    }
  }
  if (!isEmpty) {
    int bytesRead = 0;
    for (int i = 0; i < 4; i++) {
      if (kbdbuff[i]) {
        char *input = &kbdbuff[i];
        if (kbdbuff[i] == eof) {
          memCpy(buff, input, 1);
          kbdbuff[i] = 0;
          bytesRead++;
          kbdClose(p, fd);
          ready(p);
          break;
        } else if (kbdbuff[i] == 0x0D) {
          // return has been pressed, return sysread
          return bytesRead;
        } else {
          // save buffer and block sysread
          memCpy(buff, input, 1);
          kbdbuff[i] = 0;
          bytesRead++;
          fdStruct *fdStruct = &p->fdt[fd];
          memCpy(fdStruct->dataBuffer, input, 1);
          p->state = STATE_BLOCKED;
          return -3;
        }
      }
    }
    return bytesRead;
  }
  return -1;
}

int kbdWrite(pcb *p, int fd, void *buff, int bufflen) {
  // Keyboard device doesn't support write
  return -1;
}

int kbdIoctl(int fd, unsigned long command, int arg3) {
  switch (command) {
    case 53:
      // update eof to be new character specified by arg3
      eof = arg3;
      return 0;
      break;
    case 55:
      // turn echo off
      echo = ECHO_OFF;
      kprintf("kbd echo is now off, press Tab to toggle echo mode \n");
      return 0;
      break;
    case 56:
      // turn echo on
      echo = ECHO_ON;
      kprintf("kbd echo is now on \n");
      return 0;
      break;
    default:
      kprintf("invalid command for kbdIoctl encountered\n");
      return -1;
  }
}
