// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
} kmem;

struct {
  struct spinlock lock;
  int count[PHYSTOP / PGSIZE];
} kref;

void
kref_set(void *pa, int val)
{
  if (((uint64)pa % PGSIZE) != 0 || (char *)pa < end || (uint64)pa >= PHYSTOP)
    panic("kref_set");
  acquire(&kref.lock);
  kref.count[(uint64)pa / PGSIZE] = val;
  release(&kref.lock);
}

void
kref_inc(void *pa)
{
  if (((uint64)pa % PGSIZE) != 0 || (char *)pa < end || (uint64)pa >= PHYSTOP)
    panic("kref_inc");
  acquire(&kref.lock);
  kref.count[(uint64)pa / PGSIZE]++;
  release(&kref.lock);
}

int
kref_get(void *pa)
{
  if (((uint64)pa % PGSIZE) != 0 || (char *)pa < end || (uint64)pa >= PHYSTOP)
    panic("kref_get");
  acquire(&kref.lock);
  int c = kref.count[(uint64)pa / PGSIZE];
  release(&kref.lock);
  return c;
}

void
kinit()
{
  initlock(&kmem.lock, "kmem");
  initlock(&kref.lock, "kref");
  freerange(end, (void *)PHYSTOP);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char *)PGROUNDUP((uint64)pa_start);
  for (; p + PGSIZE <= (char *)pa_end; p += PGSIZE) {
    kref_set(p, 1);
    kfree(p);
  }
}

// Free the page of physical memory pointed at by pa,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;

  if (((uint64)pa % PGSIZE) != 0 || (char *)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  acquire(&kref.lock);
  kref.count[(uint64)pa / PGSIZE]--;
  int c = kref.count[(uint64)pa / PGSIZE];
  release(&kref.lock);

  if (c < 0)
    panic("kfree: ref < 0");

  if (c > 0)
    return;

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run *)pa;

  acquire(&kmem.lock);
  r->next = kmem.freelist;
  kmem.freelist = r;
  release(&kmem.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;

  acquire(&kmem.lock);
  r = kmem.freelist;
  if (r)
    kmem.freelist = r->next;
  release(&kmem.lock);

  if (r) {
    kref_set((void *)r, 1);
    memset((char *)r, 5, PGSIZE); // fill with junk
  }
  return (void *)r;
}

// count the amount of free memory in bytes
uint64
count_free_mem(void)
{
  struct run *r;
  uint64 count = 0;
  acquire(&kmem.lock);
  r = kmem.freelist;
  while (r) {
    count++;
    r = r->next;
  }
  release(&kmem.lock);
  return count * PGSIZE;
}
