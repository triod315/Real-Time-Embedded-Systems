/*
* Assignment 3: Linux Real-Time Threads
* Course: Real-Time Embedded Systems Concepts and Practices
* Date: 03/13/2023
* Names:
*   - Oleksandr Hryshchuk
* Description:
*   This program creates 128 threads and runs them on the different cores.
*   The threads are created with the SCHED_FIFO policy and the maximum priority.
*   The threads are created with the pthread_create() function.
*   The threads are joined with the pthread_join() function.
*/
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <sched.h>
#include <syslog.h>
#include <time.h>
#include <string.h>
#include <unistd.h>

#define COUNT  1000

typedef struct
{
    int threadIdx;
} threadParams_t;

#define NUM_THREADS 128
#define SCHED_POLICY SCHED_FIFO

// POSIX thread declarations and scheduling attributes
//
pthread_t threads[NUM_THREADS];
threadParams_t threadParams[NUM_THREADS];

// scheduling attributes
pthread_attr_t fifo_sched_attr;
struct sched_param fifo_param;

/*
* This function is used to print the current scheduler policy for the process.
*/
void print_scheduler(void) {
    int sched_type = sched_getscheduler(getpid());
    char *sched = malloc(256);
    
    switch (sched_type)
    {
    case SCHED_FIFO:
        strcpy(sched, "SCHED_FIFO");
        break;
    case SCHED_OTHER:
        strcpy(sched, "SCHED_OTHER");
        break;
    default:
        strcpy(sched, "UNKNOWN");
        break;
    }

    printf("Pthread policy is: %s\n", sched);
    free(sched);
}

/*
* This function is used to set the scheduler policy for the process.
* It sets the scheduler policy to SCHED_FIFO and sets the priority to the maximum.
*/
void set_scheduler(void) {
    int max_priority;   // priority values are 1-99 by default
    int rc;

    printf("Before policy adjustment:\n");
    print_scheduler();

    // set the scheduler policy
    pthread_attr_init(&fifo_sched_attr);
    pthread_attr_setinheritsched(&fifo_sched_attr, PTHREAD_EXPLICIT_SCHED);
    pthread_attr_setschedpolicy(&fifo_sched_attr, SCHED_POLICY);

    // get the maximum priority
    max_priority = sched_get_priority_max(SCHED_POLICY);
    fifo_param.sched_priority = max_priority;

    if ((rc = sched_setscheduler(getpid(), SCHED_POLICY, &fifo_param)) < 0) {
        perror("sched_setscheduler");
	}
    
    // set the scheduler policy and priority for the thread
    pthread_attr_setschedparam(&fifo_sched_attr, &fifo_param);

    printf("After policy adjustment:\n");
    print_scheduler();
}

/*
* This function is used to count the sum of numbers from 0 to the thread index.
* It is used to simulate a thread that is doing some work.
*/
void *counter_thread(void *threadp) {
    int sum = 0, i;
    threadParams_t *threadParams = (threadParams_t *)threadp;

    for(i = 1; i <= (threadParams->threadIdx); i++)
        sum=sum+i;
 
    syslog(LOG_DEBUG, "Thread idx=%d, sum[0...%d]=%d, running on core:%d\n", 
           threadParams->threadIdx,
           threadParams->threadIdx, sum, sched_getcpu());
}

void delay(unsigned int mseconds) {
    clock_t goal = mseconds + clock();
    while (goal > clock());
}

int main (int argc, char *argv[]) {

    // clear syslog and write first line
    system("echo > /dev/null | tee /var/log/syslog");
    system("logger [COURSE:1][ASSIGNMENT:3]: `uname -a` | tee /var/log/syslog");
    // open for logging
    openlog("[COURSE:1][ASSIGNMENT:3]", LOG_NDELAY, LOG_DAEMON);

    set_scheduler();
    
    // create threads
    for(int i = 0; i < NUM_THREADS; i++) {
        threadParams[i].threadIdx=i;

        pthread_create(&threads[i],                 // pointer to thread descriptor
                      &fifo_sched_attr,             // use fifo
                      counter_thread,               // thread function entry point
                      (void *)&(threadParams[i])    // parameters to pass in
                     );
    } 

    // wait for threads to complete
    for(int i = 0; i < NUM_THREADS; i++)
        pthread_join(threads[i], NULL);
    
    closelog();

    delay(100000);
    system("cp /var/log/syslog assignment3.txt");

    printf("\nComplete\n");
    return 0;
}