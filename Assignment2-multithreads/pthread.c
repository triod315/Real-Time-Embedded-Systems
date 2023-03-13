/*
* Assignment 2: Multiple Threads
* Course: Real-Time Embedded Systems Concepts and Practices
* Date: 03/12/2023
* Names:
*   - Oleksandr Hryshchuk
* Description: This program creates 128 threads that print the sum of the numbers from 0 to the thread index.
*              The output is written to the syslog.
*              The output is also written to a text file for submission.
*/
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <sched.h>
#include <syslog.h>
#include <time.h>

// notes for the assignment:
// before printing to syslog clear it with:
// echo > /dev/null | sudo tee /var/log/syslog 

// for the first line you need to print uname -a, use logger command on the command line
// logger [COURSE:X][ASSIGNMENT:Y]: `uname -a`

#define NUM_THREADS 128

typedef struct {
    int threadIdx;
} threadParams_t;


// POSIX thread declarations
pthread_t threads[NUM_THREADS];
threadParams_t threadParams[NUM_THREADS];

/*
* Thread function
* This function is called by each thread to print the sum of the numbers from 0 to the thread index.
* The output is written to the syslog.
*  
* @param threadp - pointer to thread parameters
*/
void *counter_thread(void *threadp) {
    int sum = 0, i;
    threadParams_t *threadParams = (threadParams_t *)threadp;

    for(i = 1; i <= (threadParams->threadIdx); i++)
        sum=sum+i;
 
    syslog(LOG_DEBUG, "Thread idx=%d, sum[0...%d]=%d\n", 
           threadParams->threadIdx,
           threadParams->threadIdx, sum);
}

/*
* Delay function
* This function is used to delay the program for a specified number of milliseconds.
* This is used to allow the syslog to be written to the text file.
*/
void delay(unsigned int mseconds)
{
    clock_t goal = mseconds + clock();
    while (goal > clock());
}

int main (int argc, char *argv[]) {
    // clear syslog and create first line
    system("echo > /dev/null | tee /var/log/syslog");
    system("logger [COURSE:1][ASSIGNMENT:2]: `uname -a` | tee /var/log/syslog"); 

    openlog("[COURSE:1][ASSIGNMENT:2]", LOG_NDELAY, LOG_DAEMON); 
    for(int i = 0; i < NUM_THREADS; i++) {
        threadParams[i].threadIdx=i;

        pthread_create(&threads[i],                 // pointer to thread descriptor
                      (void *)0,                    // use default attributes
                      counter_thread,               // thread function entry point
                      (void *)&(threadParams[i])    // parameters to pass in
                     );
   } 

   for(int i = 0; i < NUM_THREADS; i++)
       pthread_join(threads[i], NULL);
    closelog();

    // write syslog to text file
    // 
    // note: this is not a good way to do this, but it works for this assignment
    //       a better way would be to use a pipe to read the syslog and write to a file
    delay(100000);
    system("cp /var/log/syslog assignment2.txt");

    printf("\nComplete\n");
    return 0;
}