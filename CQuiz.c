// Wyatt Shanahan Copyright 2024
// CQuiz
// A C-based Quiz game

// Problem 1
#include <errno.h> // included for error checking
#include <fcntl.h> // included for using files
#include <signal.h> // included for signals
#include <stdio.h> // included for io use
#include <stdlib.h> // included for built in functionality
#include <string.h> // included for string functions
#include <sys/stat.h> // included for functionality
#include <sys/time.h> // included for functionality
#include <time.h> // included for timers
#include <unistd.h> // included for  built in functionality
#define BUF_SIZE 1024 // included for use in arrays

int checkError(int val, const char *msg){  // function to check errors in timers and non read/write functions
    if (val == -1){ // if an error occurred, then val is -1
        if (errno == EINTR) return -1; // if a specific signalling error, then return -1
        perror(msg); // print out the error message
        exit(EXIT_FAILURE); // exit with a failure flag
    }
    return val; // if no errors, then return val
}

int wrError(int val, const char *msg){ // function to check errors in read/write functions
    if (val == 0){ // since read/write return 0 if failed, then val == 0 is used to detect errors
        perror(msg); // the message is outputted using perror
        exit(EXIT_FAILURE); // the program exits with a failure flag
    }
    return val; // if no error, then val is returned
}
static int timedOut; // this integer represents a C Boolean value that is set to “true” if the SIGALRM is received
// Problem 2
int PrintS(const char *str) // function to write strings to the terminal
{
    return wrError(write(STDOUT_FILENO, str, strlen(str)),"writeS"); //runs write through wrError, writes to the terminal
}
// Problem 3
int PrintInt(const int val) // function to write integers to the terminal
{
    char num [32]; // char array to store the converted int
    sprintf(num,"%d",val); // converts the integer and stores it in char num
    return wrError(write(STDOUT_FILENO, num, strlen(num)),"writeI"); //runs write through wrError, writes to the terminal
}
// Problem 4
void signalHandler(int sig) { // signalHandler takes int sig, which is a signal, as an arugment
    int exitval; // integer to assign checkError value to when checking for an exit
    char ans[BUF_SIZE]; // char array to store answer in
    char* yes; // char* for doing string comparison with
    yes = "y\n"; // assigning the value to compare against for exiting
    if (sig == SIGALRM) { // if SIGALRM is raised then use this
        timedOut = 1; // times out the program to tell it to move to the next question
    }
    else if (sig == SIGINT) { // if the signal is SIGINT then this is used
        PrintS("\nDo you wish to exit? (y/n) "); // the user is asked if they wish to exit
        exitval = wrError(read(STDIN_FILENO, ans, BUF_SIZE), "read"); //the user input is read in, wrError is used for error detection
        if (strcmp(ans, yes) == 0) { // if the strings are the same then this executes
            PrintS("Exiting the program.\n"); // the user is informed that the program will exit
            exit(EXIT_SUCCESS); // the program is exited with a success flag
        }
        else{ // otherwise nothing further is done and the program returns
        }
    }
}
// Problem 5
int readLine(int fd, char *line) // function to read each line unto a newline character
{
    char data; // stores each char read in
    int cnt = 0; // counts the number of characters read
    int status; // used to detect EOF
    while (1) { // loops until broken at EOF or \n
        status = checkError(read(fd, &data, sizeof(char)),
                            "read"); // reads in characters and checks for any read errors
        if (status == 0) { // if nothing is read in then an EOF has occurred
            return 0; // return 0, which eventually exits the program
        }
        if (data == '\n') { // if the character read in is a newline then execute the following lines
            break; // break the while loop
        }
        line[cnt] = data; // assigns the char stored in data to the end of the array line
        cnt++; // count is incremented by one
    }
    line[cnt] = 0; // sets the end of line to 0 to make it a c-string
}

int readQA(int questFd, int ansFd, char *quest, char *ans) // function to read in information, provided
{
    if (readLine(questFd,quest) == 0) return 0; // reads in questions stored in questFd
    if (readLine(ansFd, ans) == 0) return 0; // reads in answers stored in ansFd
    return 1; // returns 1 on completion
}

int main(int argc, char *argv[]) // main driver function, runs the program
{
    int numRead = 0; // initialises numRead to 0
    int numWrite = 0; // intiialises numWrite to 0
    int question = 1; // initialises question to 1
    int correct = 0; // initialises correct to 0
    char buf[BUF_SIZE]; // intiialises buf to BUF_SIZE
    char quest[BUF_SIZE]; // initialises quest to BUF_SIZE
    char ans[BUF_SIZE]; // initialises ans to BUF_SIZE
    int questFd, ansFd; // declares questFd and ansFd

    // Problems 6 and 7
    struct sigaction sa; // declares struct for signal handler
    struct itimerval timer20 = {20,0,20,0}; // declares struct for 20s timer
    struct itimerval timer0 = {20,0,0,0}; // declares struct for 0s timer
    sa.sa_handler = signalHandler; // declares the signal handler to be signalHandler
    sa.sa_flags = 0; // sets sa flags to 0
    sigemptyset(&sa.sa_mask); // clears the signal set/mask
    checkError(sigaddset(&sa.sa_mask, SIGALRM),"sigadd"); // adds SIGALRM to sigset/mask
    checkError(sigaddset(&sa.sa_mask, SIGINT),"sigadd"); // adds SIGINT to the sigset/mask
    checkError(sigaction(SIGALRM,&sa,NULL),"sigaction"); // sets sigaction for SIGALRM
    checkError(sigaction(SIGINT,&sa,NULL),"sigaction"); // sets sigaction for SIGINT
    // Problem 8
    questFd = checkError(open("quest.txt", O_RDONLY, S_IRUSR | S_IWUSR), "open"); //opens quest.txt to questFd
    ansFd = checkError(open("ans.txt", O_RDONLY, S_IRUSR), "open"); // opens ans.txt to ansFd

    readQA(questFd, ansFd, quest, ans); //reads questions and answers from questFd and ansFd
    while (1) // loops until broken
    {
        /* lines 122 through 127 output the current question */
        PrintS("#");
        PrintInt(question);
        PrintS(" ");
        PrintS(quest);
        PrintS("? ");
        timedOut = 0; // question hasn't timed out, so timedOut is 0
        // Problem 9
        checkError(setitimer(ITIMER_REAL, &timer20, NULL), "setitimer"); // sets the timer to 20 seconds
        // Problem 10
        numRead = checkError(read(STDIN_FILENO, buf,1024),"read"); // reads in the user input and assigns it to numRead
        if (numRead == 0) break; // if nothing read, then break
        if (numRead == -1) // if an error occurred, then execute this branch
        {
            if (errno == EINTR) // if the error was EINTR, then execute this branch
            {
                if (timedOut) // if the program timed out, then execute this
                {
                    PrintS("\nTime's up, next question\n"); // write to the terminal that time is up
                    if (readQA(questFd, ansFd, quest, ans) == 0) break; // if there is no next question, then break
                    question++; // increment question by one
                }
                continue; // keep looping
            }
            perror("read"); // output an error of read
            exit(EXIT_FAILURE); // exit with exit flag failure
        }
        // Problem 11
        checkError(setitimer(ITIMER_REAL, &timer0, NULL), "setitimer"); // sets the timer to 0 seconds, done to disable it between questions
        buf[numRead-1]= 0; // sets the last character of buf to 0, making it a c-string
        if (strcmp(buf,ans) == 0) // compares the answer and user input, if they're the same then this branch executes
        {
            correct++; // increment correct by one
            PrintS("\ncorrect\n"); // output correct to let the user know their answer was correct
        } else // if the answer is incorrect
        {
            PrintS(ans); // output the correct answer
            PrintS("\nwrong\n"); // output wrong to inform the user that they were incorrect
        }
        if (readQA(questFd, ansFd, quest, ans) == 0) break; // read next question, break if no more questions
        question++; // increment question
    }
    // lines 164-167 print out the final score
    PrintS("final score is ");
    PrintInt(correct);
    PrintS(" out of ");
    PrintInt(question);
    // Problem 12
    checkError(close(questFd), "close"); // closes the questions file
    checkError(close(ansFd), "close"); // closes the answers file
}
