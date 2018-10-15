#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"

int
sys_fork(void)
{
  return fork();
}

int
sys_exit(void)
{
  exit();
  return 0;  // not reached
}

int
sys_wait(void)
{
  return wait();
}

int
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int
sys_getpid(void)
{
  return myproc()->pid;
}

int
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

int
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

// return how many clock tick interrupts have occurred
// since start.
int
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

int
sys_getppid(void)
{
  return myproc()->parent->pid;
}

int
sys_getdate(void) {
  struct rtcdate *dp;
  if (argptr(0, (char **)&dp, sizeof(dp)) < 0)
    return -1;
  acquire(&tickslock);
  cmostime(dp);
  release(&tickslock);
  return 0;
}

uint
get_unix_time(struct rtcdate *dp) {
  uint month = dp->month;
  uint year = dp->year;
  if (month < 3) {
    month += 12;
    year--;
  }
  return (365 * year + (year / 4) - (year / 100) + (year / 400) + (306 * (month + 1) / 10) - 428 + dp->day - 719163) *
         86400 + (dp->hour * 3600) + (dp->minute * 60) + dp->second;
}

int
sys_sleep_sec(void) {
  int n;
  int i = 0;
  struct rtcdate t0;
  struct rtcdate t;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);

  cmostime(&t0);
  cmostime(&t);

  while(get_unix_time(&t) - get_unix_time(&t0) < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    while(i++ < 10)
      sleep(&ticks, &tickslock);
    i = 0;
    cmostime(&t);
  }
  release(&tickslock);
  return 0;
}
