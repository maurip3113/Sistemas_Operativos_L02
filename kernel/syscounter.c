#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint counter = 0;

uint64
sys_count_value(void)
{
  return counter;
}

uint64
sys_count_inc(void)
{
  counter += 1;
  return counter;
}

uint64
sys_count_dec(void)
{
  counter -= 1;
  return counter;
}
