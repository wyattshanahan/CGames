// Wyatt Shanahan Copyright 2024
// ThreadGuess
// A game where two threads compete to guess a pseudo-random number

// Problem 1 - include statements
#include <errno.h> // used for error handling
#include <pthread.h> // used for thread support
#include <stdio.h> // used for io
#include <stdlib.h> // used for standard functionality
#include <string.h> // used for string support
#include <time.h> // used for the random number generator
#include <unistd.h> // used for functionality

// Problem 2 - Globab Variables
static int guess[2]; // used to store guesses
static int dirs[2]; // used to store referee responses
static int sgn[4]; // used to store flags for condition variables
int scr[2]; //stores thread scores ([0] for thread 1, [1] for thread 2

// Problem 3 - Declare mutexes and cond variables
pthread_mutex_t mtx1 = PTHREAD_MUTEX_INITIALIZER; // initialise Mutex 1
pthread_mutex_t mtx2 = PTHREAD_MUTEX_INITIALIZER; // initialise Mutex 2
pthread_mutex_t mtx3 = PTHREAD_MUTEX_INITIALIZER; // initialise Mutex 3
pthread_cond_t cnd1 = PTHREAD_COND_INITIALIZER; // initialise the 1st condition variable
pthread_cond_t cnd2 = PTHREAD_COND_INITIALIZER; // initialise the 2nd condition variable
pthread_cond_t cnd3 = PTHREAD_COND_INITIALIZER; // initialise the 3rd condition variable

int checkThread(int val, const char *msg) //  function to check for thread errors
{
    if (val == 0) return 0; // if no error, then return 0
    errno = val; // else errno is the error value
    perror(msg); // print error message
    exit(EXIT_FAILURE); // exit with failure
}

int checkError(int val, const char *msg) //  function to check for function errors
{
    if (val == -1) // if val is -1, then an error has occured
    {
        if (errno == EINTR) return val; // if the error is EINTR, then return val
        perror(msg); // otherwise print the error message
        exit(EXIT_FAILURE); // exit with failure
    }
    return val; // if no error, then return val
}

int rngRand(int first, int last) // pseudo-randon number generator
{
    int rng = (last - first) + 1; // find integer of the range
    double perc = ((double) rand()) / (((double) RAND_MAX)+1); // double percentage is a random number divided by RAND_MAX + 1
    int offst = rng * perc; // the offset is the range multiplied by the percent
    return first + offst; // return the number
}

void * player1(void * args) // Thread 1, which guesses based on taking the average of min and max
{
    // Problem 4 - Thread 2
    checkThread(pthread_detach(pthread_self()), "pthread_detach"); // detatch the thread
    while(1){ // infinitely loop
        int min = 0; // set min to 0
        int max = 101; // set max to 101 (allows for average to actually reach 100)
        int currGuess; // declares currGuess
        checkThread(pthread_mutex_lock(&mtx3), "mutex3_lock"); // locks mutex 3
        while(sgn[2] == 0){ // while sgn[2] is 0
            checkThread(pthread_cond_wait(&cnd3,&mtx3),"cond3_wait"); // wait for cnd3 to be set
        }
        checkThread(pthread_mutex_unlock(&mtx3),"mutex3_unlock"); // unlock mutex 3
        sgn[2] = 0; // reset sgn[2] to 0
        while(1){ // infinitely loop
            currGuess = (min + max) / 2; // currGuess is the average of min and max
            guess[0] = currGuess; // set guess[0] as currGuess
            checkThread(pthread_mutex_lock(&mtx1),"mutex1_lock"); // lock mutex 1
            while(sgn[0] == 0){ // while sgn[0] is 0
                checkThread(pthread_cond_wait(&cnd1, &mtx1), "cond1_wait"); // wait for cnd1 to update
            }
            sgn[0] = 0; // reset sgn[0]
            checkThread(pthread_mutex_unlock(&mtx1), "mutex1_unlock"); // unlock mutex1
            if (dirs[0] == -1){ // if dirs is -1, then the guess was too small
                min = currGuess; // set min to currGuess
            }
            else if (dirs[0] == 1){ // if dirs is 1, then the guess was too large
                max = currGuess; // set min to currGuess
            }
            else if (dirs[0] == 0){ // if dirs is 0, then the guess or other thread's guess was correct
                break; // break thread
            }
        }
    }
}

void * player2(void *args) // thread 2, guesses by random number
{
    // Problem 5 - Thread 2
    checkThread(pthread_detach(pthread_self()), "pthread_detach"); // detatches the thread
    while(1){ // infinitely loops
        int min = 0; // intiialises min to 0
        int max = 100; // initialises max to 100
        int currGuess; // currGuess stores guesses
        checkThread(pthread_mutex_lock(&mtx3), "mutex3_lock"); // locks mutex 3
        while(sgn[3] == 0){ // while sgn[3] is 0
            checkThread(pthread_cond_wait(&cnd3,&mtx3),"cond3_wait"); // wait for a change in cnd3
        }
        checkThread(pthread_mutex_unlock(&mtx3),"mutex3_unlock"); // unlock mutex 3
        sgn[3] = 0; // reset sgn[3] to 0
        while(1){ // infinitely loops
            srand(time(NULL)); // re-seed the pseudo-random number generator
            rngRand(0,13); // generate a number to prevent overlap with the referee generation
            rngRand(0,2); // generate a number to prevent overlap with the referee generation
            currGuess = rngRand(min, max); // generate a number between min and max as the guess
            guess[1] = currGuess; // set guess[1] as the current guess
            checkThread(pthread_mutex_lock(&mtx2),"mutex2_lock"); // lock mutex 2
            while(sgn[1] == 0){ // while sgn[1] is 0
                checkThread(pthread_cond_wait(&cnd2, &mtx2), "cond2_wait"); // wait for cnd2
            }
            sgn[1] = 0; // reset sgn[1] to 0
            checkThread(pthread_mutex_unlock(&mtx2), "mutex2_unlock"); // unlock mutex 2
            if (dirs[1] == -1){ // if dirs is -1 then the guess was too small
                min = currGuess; // set min as currGuess
            }
            else if (dirs[1] == 1){ // if dirs is 1 then the guess was too large
                max = currGuess; // set max as currGuess
            }
            else if (dirs[1] == 0){ // if dirs is 0 then one of the threads guessed correctly
                break; // breaks the loop
            }
        }
    }
}

void * referee(void *args) // referee function
{
    // Problem 6 - Referee Thread
    int target; // target stores the game target
    int breakout = 0; // initialise breakout to 0
    struct timespec sleepTime = {1,0}; // sets up a timespec to sleep for 1 second
    for (int i=0; i<10; i++){ // loop for 10 iterations, for 10 games
        checkThread(pthread_mutex_lock(&mtx3), "mutex3_lock"); // locks mutex 3
        target = rngRand(1,100); // generate a pseudo-random number
        sgn[2] = sgn[3] = 1; // set sgn[2] and sgn[3] to 1
        checkThread(pthread_cond_broadcast(&cnd3),"cnd3_broadcast"); // broadcast cnd3
        checkThread(pthread_mutex_unlock(&mtx3),"mutex3_unlock"); // unlock mutex3
        printf("\nGame Statistics:\n Game Number: %d\n Thread 1 Score: %d\n Thread 2 Score: %d\n Game Target: %d\n\n",i + 1,scr[0],scr[1],target); //output game stats
        while(1){ //infinitely loop
            checkError(nanosleep(&sleepTime, NULL), "nanosleep"); // sleep for 1 second
            checkThread(pthread_mutex_lock(&mtx1), "mutex1_lock"); // locks mutex 1
            checkThread(pthread_mutex_lock(&mtx2), "mutex2_lock"); // locks mutex 2
            sgn[0] = sgn[1] = 0; // set sgn[0] and sgn[1] to 0
            for (int j = 0; j < 2; j++){ // loop for 2 iterations, checks both guesses
                int curr = guess[j]; // curr is the guess at index j
                if (curr < target){ // if curr is smaller than target
                    dirs[j] = -1; // set dirs to -1
                }
                else if (curr > target){ // if curr is larger than target
                    dirs[j] = 1; // set dirs to 1
                }
                else if (curr == target){ // if curr is equal to targer
                    scr[j] = scr[j] + 1; // increment score by 1
                    dirs[j] = 0; // set dirs to 0
                }
            }
            if (dirs[0] == 0 || dirs[1] == 0){ // test if either is 0, if so then one of the threads has won
                dirs[0] = dirs[1] = 0; // set both dirs values to 0
                breakout = 1; // tells the referee to break the loop and start a new game
            }
            printf("Thread 1 guessed: %d \nThread 2 guessed: %d\n",guess[0],guess[1]); // output the guesses
            sgn[0] = 1; // set sgn[0] to 1
            sgn[1] = 1; // set sgn[1] to 1
            checkThread(pthread_cond_broadcast(&cnd1),"cnd1_broadcast"); // broadcast cnd1
            checkThread(pthread_cond_broadcast(&cnd2),"cnd2_broadcast"); // broadcast cnd2
            checkThread(pthread_mutex_unlock(&mtx1), "mutex1_unlock"); // unlocks mutex 1
            checkThread(pthread_mutex_unlock(&mtx2), "mutex2_unlock"); // unlocks mutex 2
            if (breakout == 1){ // if breakout is 1
                breakout = 0; // reset breakout
                break; // break the infinite loop, start a new game
            }
        }
    }
    printf("Final Game Statistics:\n Thread 1 Score: %d\n Thread 2 Score: %d\n",scr[0],scr[1]); // print out game statistic text
    if (scr[0] > scr[1]){ // if thread 1 has a larger score
        printf(" Thread 1 Wins\n"); // print that thread 1 wins
    }
    else if (scr[0] < scr[1]){ // if thread 2 has a larger score
        printf(" Thread 2 Wins\n"); // print that thread 2 wins
    }
    else if (scr[0] == scr[1]){ // if thread 1 and thread 2's scores are equal
        printf(" The threads tied and neither won.\n"); // print that the threads tied
    }
    pthread_exit(NULL); // exit the thread
}


int main (int argc, char *argv[]) // main driver thread
{
    srand(time(NULL)); // seeds rand()
    scr[0] = 0, scr[1] = 0; // initiaises scores to 0
    sgn[0] = 0, sgn[1] = 0, sgn[2] = 0, sgn[3] = 0; // initialises sgn array elements to 0
    pthread_t threads[3]; // creates threads
    checkThread(pthread_create(&threads[2], NULL, referee, NULL), "pthread_create"); // creates thread 2 with consumer
    checkThread(pthread_create(&threads[0], NULL, player1, NULL), "pthread_create"); // creates thread 0 with producer 1
    checkThread(pthread_create(&threads[1], NULL, player2, NULL), "pthread_create"); // creates thread 1 with producer 2
    checkThread(pthread_join(threads[2], NULL), "pthread_join"); // joins the referee thread together
    exit(EXIT_SUCCESS); // exit with success
}
