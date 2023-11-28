#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"


#define MAX_SEM 64 // equal to NPROC  
#define NULL	0

//typedef unsigned int pthread_cond_t;      

typedef struct s_sem_t {
  int value;               //State variable to track the value of the sem. 1 to use as a lock; 0 to use for ordering   
  struct spinlock lock;    //Mutual exclusion lock: Hold(0) and not hold(1)
  //pthread_cond_t cond;     //Conditional variable which signals a procesos to continue. Related with wakeup and sleep
} *sem_t;

sem_t semaphores[MAX_SEM];
void sem_init () {
  for(unsigned int id=0;id<MAX_SEM;id++){
    semaphores[id] = NULL;
}
}
// Initializes a semaphore to an arbitrary value.
uint64
sys_sem_open(void)
{
  int id;
  int value;
  argint(0, &id);
  argint(1, &value);
  if(id < 0 || id > MAX_SEM || value < 0 || semaphores[id] != NULL){
    return 0; 
  }
  else {
    semaphores[id] = kalloc();                    //allocate memory for the sem (4Kb).  
    semaphores[id]->value = value;
    initlock(&semaphores[id]->lock, "sem lock");  //Mutex_init(&sem->lock);
    //semaphores[id]->cond = 1;                     //Cond_init(&sem->cond); 
}
  return 1;
}

// Decrease the value of a semaphore.
// If its value is zero, the processes blocks.
uint64
sys_sem_down(void)
{
  int id;
  argint(0, &id);
  acquire(&semaphores[id]->lock);        //(crit sect begin)                  
  while (semaphores[id]->value == 0) {
    sleep(&semaphores[id], &semaphores[id]->lock);         //semaphores[id]->cond indica la señal de que el proceso siga durmierdo, 
                                                                 //hasta que se modifique por wakeup en sys_sem_up
  }
  semaphores[id]->value--;
  release(&semaphores[id]->lock);        //(crit sect end)

  return 1;
}


// Increase the value of a semaphore.
// If its value is zero, blocked processes are unlocked
uint64
sys_sem_up(void)
{ 
  int id;
  argint(0, &id);
  acquire(&semaphores[id]->lock);         //(crit sect begin)   
  semaphores[id]->value++;
  wakeup(&semaphores[id]);          //semaphores[id]->cond indica que el proceso durmiendo puede continuar
  release(&semaphores[id]->lock);         //(crit sect end)

  return 1;
}

// Closes a semaphore of the system.
uint64
sys_sem_close(void)
{
  int id;
  int result;
  argint(0, &id);
  if(semaphores[id] != NULL){
    kfree(semaphores[id]);
    semaphores[id] = NULL;
    result = 1;
  }
  else {
    result = 0; // Error; semaphore already closed
  }

  return result;
}  

