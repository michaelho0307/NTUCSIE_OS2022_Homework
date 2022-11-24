#include "param.h"
#include "types.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "defs.h"
#include "proc.h"

/* NTU OS 2022 */
/* Page fault handler */
int handle_pgfault(uint64 va) {
  /* Find the address that caused the fault */ /*move to usertrap*/
  //always on a legal address
  /* TODO */
  struct proc *p = myproc();
  pte_t *currentp = walk(p->pagetable,va,0);
  if (*currentp & PTE_S) {
    uint64 blockNO = PTE2BLOCKNO(*currentp);
    *currentp |= PTE_V;
    *currentp &= ~PTE_S;
    void* page = kalloc();
    begin_op();
    read_page_from_disk(ROOTDEV,(char*)page,blockNO);
    bfree_page(ROOTDEV,blockNO);
    end_op();
    *currentp = PA2PTE(page) | PTE_FLAGS(*currentp);
    return 0;
  }
  char *getmem = kalloc();
  if (getmem != 0){ //legal
    memset(getmem,0,PGSIZE);
    int mappage_success = mappages(p->pagetable,PGROUNDDOWN(va),PGSIZE,(uint64)getmem,PTE_U|PTE_R|PTE_W|PTE_X);
    if (mappage_success != 0){
        kfree(getmem);
        p->killed = 1;
    }
  }
  else{
    p->killed = 1;
  }
  //panic("not implemented yet\n");
  return 0;
}
