//Nicklaus Krems UID# 36935302
#define _REENTRANT
#include <pthread.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <semaphore.h>
#include <stdlib.h>

//Define our shmkey
#define SHMKEY ((key_t) 1400)

//define our read write counts
typedef struct
{
	int arr[3];
} shared_mem;

//declare the couts
shared_mem *total;

//define our semaphore names
sem_t ReadMu, WriteMu, Mu, Read, Write;



//create the producer funtion
void* writer(){

	printf("Writer\n");
	/*sem_wait(&WriteMu);
	total->arr[1]++;
	if(total->arr[1] == 1){
		sem_wait(&Read);
	}
	sem_post(&WriteMu);
	sem_wait(&Write);
	
	sleep(2);
	total->arr[0]++;
	printf("Read Updated%d\n",total->arr[0]);

	sem_post(&Write);

	sem_wait(&WriteMu);
	total->arr[1]--;
	if(total->arr[1] == 0){
		sem_post(&Read);
	}
	sem_post(&WriteMu);
*/
}
//create the consumer function
void* reader(){

	printf("Read\n");
	/*sem_wait(&Read);
	sem_wait(&ReadMu);
	total->arr[2]++;
	if(total->arr[2] == 1){
		sem_wait(&Mu);
	}
	sem_post(&ReadMu);
	sem_post(&Read);

	printf("%d ", total->arr[0]);
	sleep(3);

	sem_wait(&ReadMu);
	total->arr[2]--;
	if(total->arr[2] == 0){
		sem_post(&Mu);
	}
	sem_post(&ReadMu);*/
}



int main(){
	//declares the needed variables for shared memory
	int   shmid, ID,status;
	char *shmadd;
	shmadd = (char *) 0;

	//sets the area of shared memory
	if ((shmid = shmget (SHMKEY, sizeof(int), IPC_CREAT | 0666)) < 0)
    {
    	perror ("shmget");
		exit (1);
	}
	//allows shared memory to be accessed by the forks
	if ((total = (shared_mem *) shmat (shmid, shmadd, 0)) == (shared_mem *) -1) {
    	perror ("shmat");
		exit (0);
	}

	//initiate the semaphores
	sem_init(&ReadMu, 0, 1);
	sem_init(&WriteMu, 0, 1);
	sem_init(&Mu, 0, 1);
	sem_init(&Read, 0, 1);
	sem_init(&Write, 0, 1);
	//define the pthreads



	pthread_t	tid[40];     // process id for thread 1 
	pthread_attr_t	attr[1];     // attribute pointer array 

	//flush out the buffer
	fflush(stdout);
	// Required to schedule thread independently.
	pthread_attr_init(&attr[0]);
	pthread_attr_setscope(&attr[0], PTHREAD_SCOPE_SYSTEM);  
	// end to schedule thread independently 

	//Creating pthreads
	//printf("Pre pro con \n");

	FILE* fp = fopen("mydata.dat", "r");
	char rw[1];
	char c[10];
	int num = 0;
	int i = 0;
	int count = 40;
	total->arr[0] = 0;
	total->arr[1] = 0;
	total->arr[2] = 0;

	for(;i<40; i++){
		if (fscanf(fp, "%s", &rw) != EOF)
		{
			fscanf(fp, "%s", &c);
			num = atoi(c);
			//printf("%s %d\n", &rw, num);

			if(rw[0] == 'r'){
				pthread_create(&tid[i], &attr[0], reader, NULL);
				sleep(num);
			}else if(rw[0] == 'w'){
				pthread_create(&tid[i], &attr[0], writer, NULL);
				sleep(num);
			}else{
				printf("There is an error with the input file \n");
			}

		}else{
			count = i;
			break;
		}
	}
	fclose(fp);
	printf("writes, %d, total read/writes, %d", total->arr[0], count);

 	// Wait for the threads to finish 
 	for(i = 0; i<count; i++){
 		pthread_join(tid[i], NULL);
 	}


	//clear the shared memory	
	if (shmdt(total) == -1){
      perror ("shmdt");
      exit (-1);
    }

    shmctl(shmid, IPC_RMID, NULL); 

    //destroy the semaphores
	sem_destroy(&ReadMu);
	sem_destroy(&WriteMu);
	sem_destroy(&Mu);
	sem_destroy(&Read);
	sem_destroy(&Write);

	//kill the pthreads and exit the program
	pthread_exit(NULL);

}

