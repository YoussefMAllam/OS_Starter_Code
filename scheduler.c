#include "headers.h"
#define PROCESS_EXEC "/home/youssefmallam/Downloads/OS_Starter_Code/process.out"

struct msgbuff{
    long mtype;
    struct processData data;
};

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

struct processData readyqueue[100];
int q_size=0;

void insert_process (struct processData p1){
    if(q_size==100){
        return;
    }
    readyqueue[q_size++]=p1;
}

struct processData peek_processs(){
    return readyqueue[0];
}

struct processData remove_process(){
    if (q_size == 0) {
        struct processData emptyProcess = {0}; // Return an empty process
        return emptyProcess;
    }

    struct processData earliestProcess = readyqueue[0]; // Get the first process
    
    // Shift all elements to the left
    for (int i = 1; i < q_size; i++) {
        readyqueue[i - 1] = readyqueue[i];
    }
    
    q_size--; // Decrease the queue size
    return earliestProcess; // Return the first process
}

int new_process(struct processData p1){
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
        sprintf(id, "%d", p1.id);
        sprintf(arrival_time, "%d", p1.arrivaltime);
        sprintf(priority,"%d",p1.priority);
        sprintf(running_time,"%d",p1.runningtime);
        p1.pid=getpid();
        char* arr[]={PROCESS_EXEC,id,arrival_time,priority,running_time, NULL};
        int proccess_init=execv(PROCESS_EXEC,arr);
        if(proccess_init==-1){
            perror("Error in execv for new process");
            return -1;
        }
        return 1;
    }
}

int stop_process(struct processData p1){
    kill(p1.pid,SIGSTOP);    
}

int continue_process(struct processData p1){
    kill(p1.pid,SIGCONT);
}

int main(int argc, char * argv[])
{
    printf("Scheduling type:%s\nQuantum:%s\n",argv[1],argv[2]);
    
    initClk();
    key_t id;
    id = ftok("keyfile",1);
    int msgid, received;
    msgid=msgget(id, 0666|IPC_CREAT);
    if(msgid==-1){
        perror("error in creatting message queue in scheduler");
    }
    struct msgbuff message;
    int scheduling_type=atoi(argv[1]);
    if(argv[1]=="1"){
        //SJF
        while(1){
        received=msgrcv(msgid, &message, sizeof(message.data), 1,!IPC_NOWAIT);
        if(received==-1){
            printf("error in receiving process data");
        }
        printf("received: %d\n",message.data.id);
    }
    }
    else if(argv[1]=="2"){
        //PHPF
        while(1){
        received=msgrcv(msgid, &message, sizeof(message.data), 1,!IPC_NOWAIT);
        if(received==-1){
            printf("error in receiving process data");
        }
        printf("received: %d\n",message.data.id);
    }
    }
    else if(scheduling_type==3){
        //RR
        printf("ana gowa 1\n");
        int quantum=atoi(argv[2]);
        while(1){
                received=msgrcv(msgid, &message, sizeof(message.data), 1,IPC_NOWAIT);
                enqueue(message.data);
                struct processData currentprocess = dequeue(); 
                                       
                currentprocess.runningtime=currentprocess.runningtime- quantum;

                enqueue(currentprocess);
                                 
                printf("ID:%d\tRemaining Time:%d\n",peek().id,peek().runningtime);        
        }
    //TODO implement the scheduler :)
    //upon termination release the clock resources
    
    //destroyClk(true);
}
