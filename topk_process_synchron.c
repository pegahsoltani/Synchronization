#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <limits.h>

/*Uses shared memory on arrays for synchronization*/

// globala variables
char** globalArgv;
int K;
int childIndex = 0;
sem_t* myLock;

// definitions
#define MAX_VALUE 4*K
#define SemName "/Sem"
#define ShmName "/SharedMemory"

int main(int argc, char* argv[]){
  // shared memory file descriptor
    int shmNum;
  // pointer to the shared memory
    int* array;
  // creating the shared memory, giving read and write permissions to this location
    shmNum = shm_open(ShmName, O_CREAT|O_RDWR, 0666);
  // semaphore value
  unsigned int lockVal = 1;
  // just for testing
  // initializing the semaphore
  myLock = sem_open(SemName, O_CREAT, 0644, lockVal);
  globalArgv = argv;
  K =atoi(argv[1]); // input number
  int N = atoi(argv[2]); // the number of files to be processed
  int argCount = 3;
  int number;
  // mapping the shared shared memory
  ftruncate(shmNum , K*sizeof(int));
  array = mmap(NULL, K*sizeof(int), PROT_WRITE, MAP_SHARED, shmNum, 0);
  memset(array, INT_MIN, K*sizeof(int));

  // each child process will open a file
  for(int i = 0 ; i<N ; i++){
    int fork_id = 0; // each child's process id
    fork_id = fork();
    if(fork_id == 0){
      // each child should open a file and sort it
      FILE* inputFile = fopen(argv[i + 3] , "r");
      argCount++;
      if(inputFile == NULL){
        exit(1);
      }

      // each child process reads a file
      while(fscanf(inputFile, "%d", &number) == 1){

        sem_wait(myLock);


        if(number<=array[0]){
          sem_post(myLock);
          continue;
        }
        array[0] = number;
        for(int m=1 ; m<K ; m++){
          if(number>array[m]){
            array[m-1] = array[m];
          }
          else{
            array[m-1] = number;
            break;
          }
          if(m == K - 1){
            array[m] = number;
          }
        }
        // unlock
        sem_post(myLock);
      }
      exit(0);
    }
  } // end of child
  // parent
  for (int i = 0; i < N; i++) {
    wait(NULL);
  }
  sem_close(myLock);
  sem_unlink(SemName);
  FILE* output = fopen(argv[N+3] , "w");
  for(int i = 0 ; i<K ; i++){
    fprintf(output, "%d\n" , array[i]);
  //  printf("%d\t" , array[i]);
  }

  return 0;
}
