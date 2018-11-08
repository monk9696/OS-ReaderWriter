//Nicklaus Krems UID# 36935302

// CHANGELOG
// Nov 8, 2018 - Harsha Kuchampudi
//		- Updated include statements to include unistd.h for sleep
//		- Added function prototypes for reader and writer functions
//		- Cleaned variable names and added comments where necessary
//		- Added pThread attributes and set pThread scope
//		- Fixed compiler warning when compiled with -Wall
//		- Added Makefile for simpler building
//		- Using ftok() for SHMKEY to prevent collisions
//		- Fixed fscanf() so that references are not used
//		- Minor misc fixes
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

//define our read write counts
typedef struct
{
	int readerCount;	// Stores the number of readers
	int writerCount;	// Stores the number of writers	
	int totalCount;		// Stores the total count
} shared_mem;

shared_mem *total;		// Shared memory to communicate
key_t SHMKEY;			// Shared memory key
sem_t sem_reader;		// Lock for the reader
sem_t sem_writer; 		// Lock for the writer
sem_t readcount_lock;	// Mutex to protect readerCount
sem_t writecount_lock;	// Mutex to protect writerCount
pthread_t tid[40];		// Store the thread ids
pthread_attr_t attr;    // Store thread attributes

// Function: Writer
// Description: Writer implementation
void *writer(){

	sem_wait(&writecount_lock);
	total -> writerCount++;
	if (total -> writerCount == 1)
		sem_wait(&sem_reader);
	sem_post(&writecount_lock);
	sem_wait(&sem_writer);
	
	// CRITICAL SECTION START
	sleep(2);
	total -> totalCount++;
	printf("Value updated to %d\n",total -> totalCount);
	// CRITICAL SECTION END

	sem_post(&sem_writer);
	sem_wait(&writecount_lock);
	total -> writerCount--;
	if (total -> writerCount == 0)
		sem_post(&sem_reader);
	sem_post(&writecount_lock);

	// Return NULL
	return NULL;

}

// Function: Reader
// Description: Reader implementation
void *reader(){

	sem_wait(&sem_reader);
	sem_wait(&readcount_lock);
	total -> readerCount++;
	if (total -> readerCount == 1)
		sem_wait(&sem_writer);
	sem_post(&readcount_lock);
	sem_post(&sem_reader);

	// CRITICAL SECTION START
	sleep(3);
	printf("%d: Read in\n", total -> totalCount);
	// CRITICAL SECTION END

	sem_wait(&readcount_lock);
	total -> readerCount--;
	if (total -> readerCount == 0)
		sem_post(&sem_writer);
	sem_post(&readcount_lock);

	// Return NULL
	return NULL;

}

// Function: main
// Description: Main application logic
int main(){

	// Shared memory ID and address for allocating
	// and attaching shared memory	
	int shared_mem_id;
	char *shared_mem_addr = NULL;

	// Create and set the shared memory key
	SHMKEY = ftok(".", 'a');
	// Allocate the shared memory segment and make sure
	// that it is created properly
	if ((shared_mem_id = shmget (SHMKEY, sizeof(total), IPC_CREAT | 0666)) < 0)
    {
    	perror ("shmget");
		exit (1);
	}
	// Attach the shared memory
	if ((total = (shared_mem *) shmat (shared_mem_id, shared_mem_addr, 0)) == (shared_mem *) -1) {
    	perror ("shmat");
		exit (0);
	}

	// Initialize the shared memory values
	total -> readerCount = 0;
	total -> writerCount = 0;
	total -> totalCount = 0;

	// Initialize the global semaphores so that they all have a 
	// value of 1. Additionally, the second parameter identifies
	// that the semaphores will be shared between threads
	sem_init(&sem_reader, 0, 1);
	sem_init(&sem_writer, 0, 1);
	sem_init(&readcount_lock, 0, 1);
	sem_init(&writecount_lock, 0, 1);
	
	// Set thread attributes
    pthread_attr_init(&attr);
    pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);

	//flush out the buffer
	fflush(stdout);

	// Reading in the input file allowing for fine
	// tuning of reader and writer inputs
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

	//loop through the file spiting out pthreads for each reader and writer as desired
	for(;i<40; i++){
		//chcek end of file
		if (fscanf(fp, "%s", rw) != EOF)
		{
			//gather the sleep time
			fscanf(fp, "%s", c);
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

 	// Output to see if proper test cases resulted
 	printf("writes, %d, total read/writes, %d\n", total -> totalCount, count);

	// Clear the shared memory	
	if (shmdt(total) == -1){
      perror ("shmdt");
      exit (-1);
    }
    shmctl(shared_mem_id, IPC_RMID, NULL); 

    // Destroy the created semaphores
	sem_destroy(&sem_reader);
	sem_destroy(&sem_writer);
	sem_destroy(&readcount_lock);
	sem_destroy(&writecount_lock);

	// Kill the pthreads and exit the program
	pthread_exit(NULL);

	// Exit successfully
	exit(0);

}

