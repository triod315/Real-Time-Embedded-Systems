/*
* Assignment 1: Simple Thread
* Course: Real-Time Embedded Systems Concepts and Practices
* Date: 03/12/2023
* Names:
*   - Oleksandr Hryshchuk
* Description: This program creates a thread that prints "Hello World from Thread!" to the syslog.
*              The output is written to a text file for submission.
*/
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <sched.h>
#include <syslog.h>
#include <time.h>

#define NUM_THREADS 1

typedef struct {
    int threadIdx;
} threadParams_t;


// POSIX thread declarations and scheduling attributes
//
pthread_t threads[NUM_THREADS];
threadParams_t threadParams[NUM_THREADS];

void *hello_world_thread(void *threadp) {
    syslog(LOG_DEBUG, "Hello World from Thread!");
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
    system("logger [COURSE:1][ASSIGNMENT:1]: `uname -a` | tee /var/log/syslog"); 

    // create threads
    openlog("[COURSE:1][ASSIGNMENT:1]", LOG_NDELAY, LOG_DAEMON); 
    for(int i=0; i < NUM_THREADS; i++) {
        threadParams[i].threadIdx=i;

        pthread_create(&threads[i],                 // pointer to thread descriptor
                      (void *)0,                    // use default attributes
                      hello_world_thread,           // thread function entry point
                      (void *)&(threadParams[i])    // parameters to pass in
                     );
    }

    syslog(LOG_DEBUG, "Hello World from Main!"); 

    for(int i=0;i<NUM_THREADS;i++)
        pthread_join(threads[i], NULL);
    closelog();

    // write syslog to text file
    // 
    // note: this is not a good way to do this, but it works for this assignment
    //       a better way would be to use a pipe to read the syslog and write to a file
    delay(100000);
    system("cp /var/log/syslog assignment1.txt");

    printf("\nComplete\n");
    return 0;
}