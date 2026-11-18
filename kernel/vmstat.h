// vmstat.h - struct for vmstat syscall
// holds page fault info and free mem

#ifndef _VMSTAT_H_
#define _VMSTAT_H_

#include "types.h"

struct vmstat {
  uint64 cow_faults;    // total cow faults
  uint64 demand_faults; // total lazy page faults
  uint64 free_pages;    // how many pages are free rn
};

#endif
