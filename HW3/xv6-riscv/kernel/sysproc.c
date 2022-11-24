#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
uint64
sys_exit(void)
{
  int n;
  if(argint(0, &n) < 0)
    return -1;
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  if(argaddr(0, &p) < 0)
    return -1;
  return wait(p);
}

uint64
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}


// for mp3
uint64
sys_thrdstop(void)
{
  //uint64 current_tick = sys_uptime();
  int delay, thrdstop_context_id;
  uint64 handler, handler_arg;
  if (argint(0, &delay) < 0)
    return -1;
  if (argint(1, &thrdstop_context_id) < 0)
    return -1;
  if (argaddr(2, &handler) < 0)
    return -1;
  if (argaddr(3, &handler_arg) < 0)
    return -1;
  //if needed, init
  struct proc *current_process = myproc();
  if (thrdstop_context_id == -1){  //assign a new ID
    for (int i = 0; i < MAX_THRD_NUM; i++){
      if (current_process->context_isused[i] == 0){
        current_process->context_isused[i] = 1;
        thrdstop_context_id = i;
        break;
      }
    }
    if (thrdstop_context_id < 0){
      return -1;  //no more space
    }
    //thrdstop_context_id = current_process->totalIDcount;
    //current_process->totalIDcount = current_process->totalIDcount+1;
  }
  //printf("win\n");
  current_process -> totalticks = 0;
  current_process -> delay_val = delay;
  current_process -> context_id = thrdstop_context_id;
  current_process -> thrdstop_handler_ptr = handler;
  current_process -> handler_arg = handler_arg; 
  return thrdstop_context_id;
}

// for mp3
uint64
sys_cancelthrdstop(void)
{
  int thrdstop_context_id, is_exit;
  if (argint(0, &thrdstop_context_id) < 0)
    return -1;
  if (argint(1, &is_exit) < 0)
    return -1;
  //if (thrdstop_context_id < 0){
  //  return -1;
  //}
  struct proc *curpros = myproc();
  uint64 returnval = curpros -> totalticks;
  curpros->context_id = thrdstop_context_id;
  curpros->ISexit = is_exit;
  //struct proc *current_process = myproc();
  //if (is_exit == 0){
  //in proc.c
  //  storecontext(thrdstop_context_id);
  //}
  //else{
  //  current_process->context_id = -1;
    //remove
  //  current_process->context_isused[thrdstop_context_id] = 0; 
  //}
  //return 0;
  return returnval;
}

// for mp3
uint64
sys_thrdresume(void)
{
  int  thrdstop_context_id;
  if (argint(0, &thrdstop_context_id) < 0)
    return -1;
  struct proc *curprocess = myproc();
  curprocess->context_id = thrdstop_context_id;
  //if (thrdstop_context_id > -1){
    //restorecontext(thrdstop_context_id);  //in proc.c
  //}
  return 0;
}
