/******************************************************************
 * The Main program with the two functions. A simple
 * example of creating and using a thread is provided.
 ******************************************************************/

#include "helper.h"
#include <iostream>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <queue>

//job class 
class Job{
public:
	int id;
	int duration;
	Job(int id, int duration){
		this->id = id;
		this->duration = duration;
	}
};

//global variables
int q_size;
int numProducers;
int numConsumers;
int n_jobs;

//time
struct timespec timeoutTime;



//semaphores
sem_t emptySem;
sem_t full;
sem_t mutex;


//queue
queue<Job*> jobQueue;

void *producer (void *id);
void *consumer (void *id);

int main (int argc, char **argv)
{
  if (argc !=5){
    
    cerr << "Incorrect number of arguments!" << endl;
    return 0;

  }

  //initialise variables
  q_size = atoi(argv[1]);
  n_jobs = atoi(argv[2]);
  numProducers = atoi(argv[3]);
  numConsumers = atoi(argv[4]);

  //initialise semaphores
  sem_init(&emptySem, 0, q_size);
  sem_init(&full, 0, 0);
  sem_init(&mutex, 0, 1);

  //initialise threads
  pthread_t producerid[numProducers];
  pthread_t consumerid[numConsumers];

  //create producer threads
  for(int i = 0; i < numProducers; i++){
    int* id = new int;
    *id = i + 1;
    pthread_create(&producerid[i], NULL, producer, (void*)id);
  }

  //create consumer threads
  for(int i = 0; i < numConsumers; i++){
    int* id = new int;
    *id = i + 1;
    pthread_create(&consumerid[i], NULL, consumer, (void*)id);
  }
  
    //join threads
  for(int i = 0; i < numProducers; i++){
    pthread_join(producerid[i], NULL);
  }
  for(int i = 0; i < numConsumers; i++){
    pthread_join(consumerid[i], NULL);
  }
    
  //destroy semaphores
  sem_destroy(&emptySem);
  sem_destroy(&full);
  sem_destroy(&mutex);
  
  return 0;

}

void *producer (void *parameter) 
{
  int id = *((int*)parameter);
  int jobsProduced = 0;
  bool timeout = false;
  timeoutTime.tv_sec = 20;
  
  while(jobsProduced < n_jobs){
    //produce job
    int jobId = rand() % 100;
    int jobDuration = rand() % 10 + 1;
    Job* newJob = new Job(jobId, jobDuration);

    //wait for empty slot and store timeout state
    timeout = sem_timedwait(&emptySem, &timeoutTime);

    //if operation times out then break loop
    if (timeout){
      cout << "Producer(" << id << ") stopped beacause of timeout" << endl;
      break;
    }
    
    //wait for empty mutex
    sem_wait(&mutex);
		
    //add job to queue
    jobQueue.push(newJob);
    cout << "Producer(" << id << "): Job id " << jobId << " duration " << jobDuration << endl;
    
    //signal full and mutex
    sem_post(&mutex);
    sem_post(&full);
    
    //sleep for 1-5 seconds
    sleep(rand() % 5 + 1);

    jobsProduced++;
  }
  
  cout << "Producer(" << id << "): No more jobs to generate." << endl;
  pthread_exit(0);
  
}





void *consumer (void *parameter) 
{
  int id = *((int*)parameter);
  int jobsConsumed = 0;
  timeoutTime.tv_sec = 20;

  //wait for full slot
  while(sem_timedwait(&full, &timeoutTime) == 0){
   
    sem_wait(&mutex);
		
    //get job from queue
    Job* job = jobQueue.front();
    jobQueue.pop();
    cout << "Consumer(" << id << "): Job id " << job->id << " executing sleep duration " << job->duration << endl;
		
    //signal empty and mutex
    sem_post(&mutex);
    sem_post(&emptySem);
		
    //sleep for job duration
    sleep(job->duration);

    jobsConsumed++;
  }
  
  cout << "Consumer(" << id << "): No more jobs left" << endl;
  pthread_exit(0);
	
}
