#ifndef _PINFO_H_
#define _PINFO_H_

#include "types.h"

#define PINFO_MAXPROCS 64
#define PINFO_NAMELEN  16

// Per-process info returned by sys_getprocs()
struct procinfo {
  int      pid;
  int      state;        // enum procstate value
  uint64   sz;           // memory size in bytes
  uint64   pagefaults;   // cumulative page faults
  uint64   syscall_count;// cumulative syscalls made
  char     name[PINFO_NAMELEN];
};

// Returned by sys_getprocs(); nproc = number filled
struct procinfolist {
  int           nproc;
  struct procinfo procs[PINFO_MAXPROCS];
};


#endif // _PINFO_H_
