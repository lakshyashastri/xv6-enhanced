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

struct run
{
  struct run *next;
};

struct
{
  struct spinlock lock;
  struct run *freelist;
  unsigned char references[NREFERENCE];
} kmem;

void kinit()
{
  initlock(&kmem.lock, "kmem");
  freerange(end, (void *)PHYSTOP);
}

void freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char *)PGROUNDUP((uint64)pa_start);
  for (; p + PGSIZE <= (char *)pa_end; p += PGSIZE)
  {
    kfree(p);
    kmem.references[PA2REFERENCE_INDEX((uint64)p)] = 1; // have to convert to complete register 64 bits
  }
}

// Free the page of physical memory pointed at by pa,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void kfree(void *pa)
{
  struct run *r;

  if (((uint64)pa % PGSIZE) != 0 || (char *)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // if unitialized or corrupted
  acquire(&kmem.lock);
  if (kmem.references[PA2REFERENCE_INDEX((uint64)pa)] > 1)
  {
    kmem.references[PA2REFERENCE_INDEX((uint64)pa)]--;
    release(&kmem.lock);
    return;
  }
  kmem.references[PA2REFERENCE_INDEX((uint64)pa)]--;
  release(&kmem.lock);

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
  //check if the list is stillnull, then assign the first page
  if (r)
  {
    kmem.references[PA2REFERENCE_INDEX((uint64)r)]=1;
    kmem.freelist = r->next;
  }
  release(&kmem.lock);

  if (r)
    memset((char *)r, 5, PGSIZE); // fill with junk
  return (void *)r;
}

void
kincr_refcount(uint64 pa)
{
  if ((char*)pa < end || pa >= PHYSTOP)    //page address gets out of bounds
    panic("kincr_refcount");
  acquire(&kmem.lock);  //hold the lock so that other cpus don't access the same memory
  kmem.references[PA2REFERENCE_INDEX(pa)]++;
  release(&kmem.lock);
}

uint8
kcheck_and_decr_refcount(uint64 pa)
{
  if ((char*)pa < end || pa >= PHYSTOP)
    panic("kcheck_and_decr_refcount");
  acquire(&kmem.lock); // again hold to avoid multiple cpus increasing the count to make pages simultaneously
  uint8 cnt;
  if(kmem.references[PA2REFERENCE_INDEX(pa)] != 1) 
    cnt = kmem.references[PA2REFERENCE_INDEX(pa)]--;   //get hold but decerease the count 
  else
  {
    cnt=kmem.references[PA2REFERENCE_INDEX(pa)];
  }
  release(&kmem.lock);
  return cnt;
}
//not working yet, try looking at the kalloc function later

