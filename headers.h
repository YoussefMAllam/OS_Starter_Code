#include <stdio.h>      //if you don't use scanf/printf change this include
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>



typedef short bool;
#define true 1
#define false 1

#define PROCESS_EXEC "/home/youssefmallam/Downloads/OS_Starter_Code/process.out"

#define SHKEY 300


///==============================
//don't mess with this variable//
int * shmaddr;                 //
//===============================



int getClk()
{
    return *shmaddr;
}


/*
 * All process call this function at the beginning to establish communication between them and the clock module.
 * Again, remember that the clock is only emulation!
*/
void initClk()
{
    int shmid = shmget(SHKEY, 4, 0444);
    while ((int)shmid == -1)
    {
        //Make sure that the clock exists
        printf("Wait! The clock not initialized yet!\n");
        sleep(1);
        shmid = shmget(SHKEY, 4, 0444);
    }
    shmaddr = (int *) shmat(shmid, (void *)0, 0);
}


/*
 * All process call this function at the end to release the communication
 * resources between them and the clock module.
 * Again, Remember that the clock is only emulation!
 * Input: terminateAll: a flag to indicate whether that this is the end of simulation.
 *                      It terminates the whole system and releases resources.
*/

void destroyClk(bool terminateAll)
{
    shmdt(shmaddr);
    if (terminateAll)
    {
        killpg(getpgrp(), SIGINT);
    }
}

struct processData 
{
    int arrivaltime;
    int priority;
    int runningtime;
    int id;
    pid_t pid;
};

struct msgbuff{
    long mtype;
    struct processData data;
};


///////////////////////////////////////////////////Process Control/////////////////////////////////////////////
int new_process(struct msgbuff* m1){
    //printf("Scheduler PID: %d\n",getpid());
    int pid=fork();
    if(pid==-1){
        perror("Error in fork");
        return -1;
    }
    else if(pid==0){
        char id[3];
        char arrival_time[5];
        char priority[3];
        char running_time[6];
        sprintf(id, "%d", m1->data.id);
        sprintf(arrival_time, "%d", m1->data.arrivaltime);
        sprintf(priority,"%d",m1->data.priority);
        sprintf(running_time,"%d",m1->data.runningtime);
        printf("Copied");
        m1->data.pid=getpid();
        //printf("New Process PID:%d\n",m1->data.pid);
        char* arr[]={PROCESS_EXEC,id,arrival_time,priority,running_time, NULL};
        int proccess_init=execv(PROCESS_EXEC,arr);
        if(proccess_init==-1){
            perror("Error in execv for new process");
            return -1;
        }
        return getpid();
    }
}

int stop_process(struct processData p1){
    kill(p1.pid,SIGSTOP);    
}

int continue_process(struct processData p1){
    kill(p1.pid,SIGCONT);
}

//////////////////////////////////////////////Circular Queue//////////////////////////////////////////////////////

struct CircularQueue {
    struct processData rrprocesses[100];
    int front;
    int rear;
};

struct CircularQueue queue;

// Initialize the circular queue
void initQueue() {
    queue.front = 0;
    queue.rear = 0;
}

int isEmpty() {
    return queue.front == queue.rear;
}

int isFull() {
    return (queue.rear + 1) % 100 == queue.front;
}

void enqueue(struct processData process) {
    if (!isFull()) {
        queue.rrprocesses[queue.rear] = process;
        queue.rear = (queue.rear + 1) % 100;
    }
}

struct processData dequeue() {
    if (!isEmpty()) {
        struct processData process = queue.rrprocesses[queue.front];
        queue.front = (queue.front + 1) % 100;
        return process;
    }
    struct processData emptyProcess = {0}; // Return an empty process if queue is empty
    return emptyProcess;
}

struct processData peek() {
    if (!isEmpty()) {
        struct processData process = queue.rrprocesses[queue.front];
        return process;
    }
    struct processData emptyProcess = {0}; // Return an empty process if queue is empty
    return emptyProcess;
}
///////////////////////////////////////////////////////SJF PriQ////////////////////////////////////////////////////
struct processData sjf_priorityQueue[100];

int sjf_queueSize = 0;


void sjf_enqueue(struct processData process) {
    if (sjf_queueSize >= 100) return; // Queue is full

    // Insert process in sorted order based on arrival time
    int i;
    for (i = sjf_queueSize - 1; (i >= 0 && sjf_priorityQueue[i].runningtime > process.runningtime); i--) {
        sjf_priorityQueue[i + 1] = sjf_priorityQueue[i];
    }
    printf("%d\n",process.id);
    sjf_priorityQueue[i + 1] = process; 
    sjf_queueSize++;
}


// Function to remove the process with the earliest arrival time
struct processData sjf_dequeue() {
    if (sjf_queueSize == 0) {
        struct processData emptyProcess = {0}; // Return an empty process
        return emptyProcess;
    }

    struct processData earliestProcess = sjf_priorityQueue[0]; // Get the first process
    
    // Shift all elements to the left
    for (int i = 1; i < sjf_queueSize; i++) {
        sjf_priorityQueue[i - 1] = sjf_priorityQueue[i];
    }
    
    sjf_queueSize--; // Decrease the queue size
    return earliestProcess; // Return the first process
}
struct processData sjf_peek() {
    if (sjf_queueSize == 0) {
        struct processData emptyProcess = {0}; // Return an empty process
        return emptyProcess;
    }
    return sjf_priorityQueue[0]; // Return the first process without removing it
}

struct processData currently_running_sjf;

void handler_sjf(int sig_num){
    struct processData removed=currently_running_sjf;
    printf("Process %d terminated at time %d\n",removed.id,getClk());
    signal (SIGUSR1,handler_sjf);
    currently_running_sjf=sjf_dequeue();
}

void printsjfQueue() {

    printf("Queue Size:%d Current Clock: %d\n",sjf_queueSize, getClk());
    printf("Current Process Queue:\n");
    printf("ID\tArrival Time\tRunning Time\tPriority\n");
    
    printf("Currently Running\n%d\t%d\t\t%d\t\t%d\n", 
            currently_running_sjf.id, 
            currently_running_sjf.arrivaltime, 
            currently_running_sjf.runningtime, 
            currently_running_sjf.priority);
    printf("Waiting:\n");
    for (int i = 0; i < sjf_queueSize; i++) {
        printf("%d\t%d\t\t%d\t\t%d\n", 
            sjf_priorityQueue[i].id, 
            sjf_priorityQueue[i].arrivaltime, 
            sjf_priorityQueue[i].runningtime, 
            sjf_priorityQueue[i].priority);
    }
}


