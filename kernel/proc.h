#include "kthread.h"

enum procstate { UNUSED, USED, ZOMBIE };

// Per-process state

struct proc {
  struct spinlock lock;
  
  int nextktid;
  struct spinlock ktid_lock;

  // p->lock must be held when using these:
  enum procstate state;        // Process state
  int exec_on;                 // To make sure only 1 thread can exec at a time.
  int exited;                  // To make sure only 1 exit.
  int killed;                  // If non-zero, have been killed
  int xstate;                  // Exit status to be returned to parent's wait
  int pid;                     // Process ID

  struct kthread kthread[NKT];        // kthread group table
  struct trapframe *base_trapframes;  // data page for trampolines

  // wait_lock must be held when using this:
  struct proc *parent;         // Parent process

  // these are private to the process, so p->lock need not be held.
  uint64 sz;                   // Size of process memory (bytes)
  pagetable_t pagetable;       // User page table
  struct file *ofile[NOFILE];  // Open files
  struct inode *cwd;           // Current directory
  char name[16];               // Process name (debugging)
};
