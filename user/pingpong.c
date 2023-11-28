#include "../kernel/types.h"
#include "../kernel/stat.h"
#include "../user/user.h"

#define MAX_SEM 64


//Open the first available semaphore, returning its index or -1 on error.
static int open_first(int value){
  int i = 0;
  while(sem_open(i,value) == 0 && i < MAX_SEM){
    i ++;
  }
  if(i == MAX_SEM){
    i = -1;
  }
  return i;
}

//Checks if the number of round is positive
static void positive_rounds(int rounds){
  if(rounds == 0){
    printf("ERROR: EL número de rondas debe ser positivo\n");
    exit(1);
  }
}
//Checks if the user has provided a number of rounds
static void minimun_args(int argc){
  if(argc == 1){
    printf("ERROR: Se necesita proveer un número de rondas\n");
    exit(1);
  }
}

void 
main(int argc, char *argv[])
{
  int N = atoi(argv[1]);
  minimun_args(argc);
  positive_rounds(N);

  //A semaphore for the mutex lock. We use it to ensure mutal exclusion when we have to print it the output
  //This has to be a unique semaphore, so we use the first one on the array for all the theard. 
  //We only open the sem_mutex in the first thread and close it when no more theards are running.
  int semMutex_id;
  int open = sem_open(0,1);
  if (open == 1 ){
    semMutex_id = 0;
  }
  
  //We use syscalls in syscaounter.c to tracker the amount of threads running.
  sem_down(semMutex_id);
  count_inc();
  sem_up(semMutex_id);

  //A semaphore for each process. The parent has value 1 because it is the first to be executed. 
  //This is because one has to wait for the other to finish.
  int semFather_id = open_first(1);
  int semChild_id = open_first(0);
  int pid;
  pid = fork();

  if(pid < 0){
    printf("fork failed\n");
    exit(1);
  }
  else if(pid == 0){
    for(unsigned int i = 0; i < N; i++){
      sem_down(semChild_id); //wait for the sign that father has finished.
      sem_down(semMutex_id);
      printf("      Pong\n");
      sem_up(semMutex_id);
      sem_up(semFather_id); //the son has finished, send the signal to the father.
    }
    exit(0);
  }
  else{
    for(unsigned int i = 0; i < N; i++){
      sem_down(semFather_id); //wait for the sign that son has finished.
      sem_down(semMutex_id);
      printf("Ping\n");
      sem_up(semMutex_id);
      sem_up(semChild_id); //the father has finished, send the signal to the son.
    }
    wait(0);
  }

  sem_close(semFather_id);
  sem_close(semChild_id);

  //The thread finish the pingpong program, so we decrement the counter.
  sem_down(semMutex_id);
  count_dec();
  sem_up(semMutex_id);

  if (count_value() == 0){ //NO more threads are running, so we can close the mutex sem.
    sem_close(semMutex_id);
  }
  

}
