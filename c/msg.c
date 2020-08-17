#include <xeroskernel.h>
#include <xeroslib.h>

/* Kernel handler for syssend
@params *buffer is the buffer holding the msg to be sent
@params buffer_len is the length of buffer
returns one of the following:
- the # of bytes sent
- 0 if send process is blocked
- -1 if dest process terminates while sending
- -13 for any other problems that may occur */
int send(pcb *srcPcb, pcb *dstPcb, void *buffer, int buffer_len) {
  /***********************************************/

  /* Case where dst process is waiting for send from the src process
  or dst process is receiving any*/
  if ((dstPcb->state == STATE_BLOCKED_RECV &&
       dstPcb->waitingPid == srcPcb->pid) ||
      dstPcb->waitingPid == -1) {
    // kprintf("sending right now \n");
    int minLen =
        (buffer_len < dstPcb->buffer_len) ? buffer_len : dstPcb->buffer_len;
    memCpy(dstPcb->buffer, buffer, minLen);
    dstPcb->ret = minLen;  // set ret value of blocked recv to be # bytes sent
    ready(dstPcb);
    ready(srcPcb);
    return minLen;
  }
  /* Case where the dst process is not receiving atm, so block send process
  and update the senderQueue */
  else {
    // kprintf("waiting for receive, block send\n");
    srcPcb->state = STATE_BLOCKED_SEND;
    srcPcb->waitingPid = dstPcb->pid;
    srcPcb->buffer_len = buffer_len;
    srcPcb->buffer = buffer;
    // Add sender process to the end of the dst process' sender queue
    if (dstPcb->senderQueue == NULL) {
      dstPcb->senderQueue = srcPcb;
    } else {
      pcb *temp = dstPcb->senderQueue;
      while (temp->senderNext != NULL) {
        temp = temp->senderNext;
      }
      temp->senderNext = srcPcb;
    }
    // TODO send is blocked, return 0 to indicate
    return 0;
  }
}

/* Kernel handler for sysrecv
@params *buffer is the buffer holding the msg to be sent
@params buffer_len is the length of buffer
returns one of the following:
- the # of bytes received
- 0 if the recv process is blocked
- -2 if src process terminates during recv operation
- -13 for any other problems that may occur */
int recv(pcb *srcPcb, pcb *fromPcb, void *buffer, int buffer_len) {
  /***********************************************/

  // Case where from process is waiting for recv from the src process
  if (fromPcb->state == STATE_BLOCKED_SEND &&
      fromPcb->waitingPid == srcPcb->pid) {
    // kprintf("send is present, so recv right away\n");
    int minLen =
        (buffer_len < fromPcb->buffer_len) ? buffer_len : fromPcb->buffer_len;
    memCpy(buffer, fromPcb->buffer, minLen);
    fromPcb->ret = minLen;  // set ret value of blocked send to be # bytes sent
    ready(fromPcb);
    ready(srcPcb);
    // Update the recving process' sender queue after receiving send
    if (srcPcb->senderQueue->pid == fromPcb->pid) {
      srcPcb->senderQueue = fromPcb->senderNext;
    } else {
      pcb *temp = srcPcb->senderQueue;
      while (temp->senderNext->pid != fromPcb->pid) {
        temp = temp->senderNext;
      }
      temp->senderNext = fromPcb->senderNext;
    }
    return minLen;
  }
  // Case where the from process has not send yet, so block recv process
  else {
    // kprintf("waiting for send \n");
    srcPcb->state = STATE_BLOCKED_RECV;
    srcPcb->waitingPid = fromPcb->pid;
    srcPcb->buffer_len = buffer_len;
    srcPcb->buffer = buffer;
    return 0;
  }
}

/* Kernel handler for receiveAny
@params *buffer is the buffer holding the msg to be sent
@params buffer_len is the length of buffer
returns one of the following:
- the # of bytes received
- 0 if the recv process is blocked
- -2 if src process terminates during recv operation
- -13 for any other problems that may occur */
int recvAny(pcb *srcPcb, PID_t *from_pid, void *buffer, int buffer_len) {
  // Case where from process is waiting for recv from the src process
  if (srcPcb->senderQueue != NULL) {
    // kprintf("receivingAny right now \n");
    pcb *temp = srcPcb->senderQueue;
    int minLen =
        (buffer_len < temp->buffer_len) ? buffer_len : temp->buffer_len;
    memCpy(buffer, temp->buffer, minLen);
    temp->ret = minLen;
    // Set location pointed by from_pid to send process
    from_pid = (PID_t *)temp->pid;
    ready(temp);
    ready(srcPcb);
    srcPcb->senderQueue = temp->senderNext;
    return minLen;
  }
  // Case where there is no process sending, block recvAny
  else {
    // kprintf("recvAny waiting for send \n");
    srcPcb->state = STATE_BLOCKED_RECV;
    srcPcb->buffer_len = buffer_len;
    srcPcb->buffer = buffer;
    srcPcb->waitingPid = -1;  // waitingPid = -1 means it's recvAny
    return 0;
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