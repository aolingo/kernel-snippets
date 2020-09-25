# kernel-snippets
Code snippets from the course project for a simple Intel x86 kernel that can be run on the Bochs IA-32 emulator (only inculded c code written by my partner and I).

## Table of Contents

| Component        | File           | Description  |
|:-------------|:-------------:|:-----|
| Kernel initialization     | [init.c](c/init.c) | initialize kernel data structures (eg. mem mgr lists, process table, interrupt table and etc)|
| Memory manager     | [mem.c](c/mem.c) | to allocate and free memory from the free space (free blocks tracked using a linked list)|
| Dispatcher      | [disp.c](c/disp.c)      |  process system calls, schedules the next ready process and calls ctsw to switch into process |
| Context switcher | [ctsw.c](c/ctsw.c)      |    context switch between the kernel and process (and vice versa) |
| Process creation | [create.c](c/create.c)      |    create a process and add it to the ready queue |
| System calls| [syscall.c](c/syscall.c)      |    application part of a system call |
| Root process| [user.c](c/user.c)      |    a simple shell and a couple test programs for processes, signals and devices |
| Signalling system| [signal.c](c/signal.c)      |    supports 32 signals (# from 0-31), allows kernel to asynchronously signal an application; signals are delivered in priority order, higher signum = higher priority|
| Inter-process communication| [msg.c](c/msg.c) , [sleep.c](c/sleep.c)      |  allows processes to send and receive from each other  |
| Device drivers and devices| [di_calls.c](c/di_calls.c) , [kbd.c](c/kbd.c) , [zerorand.c](c/zerorand.c)     |  include device independent interface, support for 3 devices: keyboard and the zero and random virtual devices  |
| Header files| [xeroskernel.h](h/xeroskernel.h)  , [di_calls.h](h/di_calls.h) , [kbd.h](h/kbd.h) , [zerorand.h](h/zerorand.h)     |  relevant header files  |
