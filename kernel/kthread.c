#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "defs.h"

extern struct proc proc[NPROC];

extern void forkret(void);

void kthreadinit(struct proc *p)
{
  initlock(&p->ktid_lock, "nextktid");

  for (struct kthread *kt = p->kthread; kt < &p->kthread[NKT]; kt++)
  {
    initlock(&kt->lock, "kthread");
    kt->state = KTUNUSED;
    kt->process = p;

    // WARNING: Don't change this line!
    // get the pointer to the kernel stack of the kthread
    kt->kstack = KSTACK((int)((p - proc) * NKT + (kt - p->kthread)));
  }
}

struct kthread *mykthread()
{
  push_off();
  struct cpu *c = mycpu();
  struct kthread *kt = c->kthread;
  pop_off();
  return kt;
}

int allocktid(struct proc* p)
{
  int ktid;
  acquire(&p->ktid_lock);
  ktid = p->nextktid;
  p->nextktid = p->nextktid + 1;
  release(&p->ktid_lock);
  return ktid;
}


struct kthread *allockthread(struct proc *p)
{
  struct kthread *kt;
  for (kt = p->kthread; kt < &p->kthread[NKT]; kt++) {
    acquire(&kt->lock);
    if (kt->state == KTUNUSED) {
      goto found;
    } else {
      release(&kt->lock);
    }
  }
  return 0;

found :
  kt->ktid = allocktid(p);
  kt->state = KTUSED;
  kt->process = p;
  
  if ((kt->trapframe = get_kthread_trapframe(p, kt)) == 0) {
    freekthread(kt);
    release(&kt->lock);
    return 0;
  }

  memset(&kt->context, 0, sizeof(kt->context));
  kt->context.ra = (uint64)forkret;
  kt->context.sp = kt->kstack + PGSIZE;

  return kt;
}

void freekthread(struct kthread *kt)
{
  kt->ktid = 0;
  kt->process = 0;
  kt->chan = 0;
  kt->killed = 0;
  kt->xstate = 0;
  kt->trapframe = 0;
  kt->state = KTUNUSED;
}

struct trapframe *get_kthread_trapframe(struct proc *p, struct kthread *kt)
{
  return p->base_trapframes + ((int)(kt - p->kthread));
}
