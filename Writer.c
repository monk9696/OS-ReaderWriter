//Nicklaus Krems UID# 36935302

// CHANGELOG
// Nov 8, 2018 - Harsha Kuchampudi
//		- Updated include statements to include unistd.h for sleep
//		- Added function prototypes for reader and writer functions
//		- 
// Nov 5, 2018 - Nicklaus Krems
//		- Initially created files

#define _REENTRANT
#include <pthread.h>	// Thread library
#include <stdio.h>		// Standard input and output
#include <sys/types.h>	// Types header
#include <sys/ipc.h>	// Required for shmget
#include <sys/shm.h>	// Required for shmget
#include <sys/wait.h>	// Wait function call
#include <fcntl.h>		// Control
#include <semaphore.h>	// POSIX semaphores
#include <stdlib.h>		// Standard library
#include <unistd.h>		// For sleep function

// Define function prototypes
void *writer();
void *reader();

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

//create the writer funtion
void* writer(){

	sem_wait(&WriteMu);
	total->arr[1]++;
	if(total->arr[1] == 1){
		sem_wait(&Read);
	}
	sem_post(&WriteMu);
	sem_wait(&Write);
	
	sleep(2);
	total->arr[0]++;
	printf("Value updated to %d\n",total->arr[0]);

	sem_post(&Write);

	sem_wait(&WriteMu);
	total->arr[1]--;
	if(total->arr[1] == 0){
		sem_post(&Read);
	}
	sem_post(&WriteMu);

}
//create the reader function
void* reader(){

	sem_wait(&Read);
	sem_wait(&ReadMu);
	total->arr[2]++;
	if(total->arr[2] == 1){
		sem_wait(&Mu);
	}
	sem_post(&ReadMu);
	sem_post(&Read);

	sleep(3);
	printf("%d: Read in\n", total->arr[0]);
	

	sem_wait(&ReadMu);
	total->arr[2]--;
	if(total->arr[2] == 0){
		sem_post(&Mu);
	}
	sem_post(&ReadMu);
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
	//iffy probably the broken part
	//Defines 40 total thread Id's
	pthread_t	tid[40];

	//flush out the buffer
	fflush(stdout);



	//reading in the input file allowing for fine
	//tuning of reader and writer inputs
	FILE* fp = fopen("mydata.dat", "r");
	//define some desired variables
	char rw[1];
	char c[10];
	//sleep time
	int num = 0;
	//for loop definition
	int i = 0;
	//defines the maximum numof threads currently
	int count = 40;
	//set the chared memory to default values of 0
	total->arr[0] = 0;
	total->arr[1] = 0;
	total->arr[2] = 0;

	//loop through the file spiting out pthreads for each reader and writer as desired
	for(;i<40; i++){
		//chcek end of file
		if (fscanf(fp, "%s", &rw) != EOF)
		{
			//gather the sleep time
			fscanf(fp, "%s", &c);
			num = atoi(c);
			//printf("%s %d\n", &rw, num);

			//Identify the use case as a reader or writer and generate the thread
			if(rw[0] == 'r'){
				pthread_create(&tid[i], NULL, reader, NULL);
				//printf("Read done?\n");
				sleep(num);
			}else if(rw[0] == 'w'){
				pthread_create(&tid[i], NULL, writer, NULL);
				sleep(num);
			}else{
				printf("There is an error with the input file \n");
			}
		//break out of the loop early and define how many threads exist
		}else{
			count = i;
			break;
		}
	}
	//close the input file
	fclose(fp);
	//printf("writes, %d, total read/writes, %d", total->arr[0], count);

 	// Wait for the threads to finish 
 	for(i = 0; i<count; i++){
 		pthread_join(tid[i], NULL);
 	}
 	//output to see if proper test cases resulted
 	printf("writes, %d, total read/writes, %d", total->arr[0], count);

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

