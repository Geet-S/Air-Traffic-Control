#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>
#include <mqueue.h>

struct Flight {
    int Arriv;
    int dept;
    int planeID;
    int totalWeight;
    int planeType;
    int numOfPassengers;
    int currentStage;
    int isCompleted;
};

struct FlightMessage {
    long mtype;
    struct Flight flight;
};

int main(){
        
    key_t key;
    int msgid;
        
    key = ftok("AirTrafficController.txt", 65);
    if (key == -1) {
        perror("ftok");
        exit(1);
    }

    msgid = msgget(key, 0666 | IPC_CREAT);
    if (msgid == -1) {
        perror("msgget");
        exit(1);
    }

    int planeID;
    printf("Enter Plane ID: \n");
    scanf("%d", &planeID);
    printf("Plane ID entered: %d\n", planeID);

    int planeType;
    printf("Enter Type of Plane (1 for passenger, 0 for cargo): \n");
    scanf("%d", &planeType);
    printf("Type of Plane entered: %d\n", planeType);
    struct Flight f;
    f.planeID=planeID;
    f.currentStage=1;
    f.isCompleted=0;
    if(planeType==1){
        int num;
        int isParent=1;
        int isChild=-1;
        printf("Enter Number of Occupied Seats:\n");
        scanf("%d", &num);
        pid_t passengers= fork();
        if(passengers==-1){
            perror("Fork failed");
            exit(1);
        }
        if(passengers==0){
            isParent=-1;
            isChild=1;
        }
        if(passengers>0){
            printf("Enter Weight of Plane (in kgs):\n");
            scanf("%d", &f.totalWeight);
            printf("Total Weight entered: %d\n", f.totalWeight);
            printf("Enter Airport Number:\n");
            scanf("%d", &f.dept);
            printf("Airport Number entered: %d\n", f.dept);
            printf("Sending Arrival Message\n");
            struct FlightMessage message;
            message.mtype=1008;
            message.flight=f;
            if (msgsnd(msgid, &message, sizeof(struct FlightMessage) - sizeof(long), 0) == -1) {
                perror("msgsnd");
                exit(1);
            }
            printf("Arrival Message Sent\n");
        }
        if(passengers==0){
            int counter=0;
            while(counter<num){
                printf("Boarding Passenger %d\n", counter+1);
                counter++;
            }
            f.numOfPassengers=num;
            f.currentStage++;
            printf("Sending Arrival Message\n");
            struct FlightMessage message;
            message.mtype=1008;
            message.flight=f;
            if (msgsnd(msgid, &message, sizeof(struct FlightMessage) - sizeof(long), 0) == -1) {
                perror("msgsnd");
                exit(1);
            }
            printf("Arrival Message Sent\n");
        }
    }
    else if(planeType==0){
        printf("Enter Weight of Plane (in kgs):\n");
        scanf("%d", &f.totalWeight);
        printf("Total Weight entered: %d\n", f.totalWeight);
        printf("Enter Airport Number:\n");
        scanf("%d", &f.dept);
        printf("Airport Number entered: %d\n", f.dept);
        printf("Sending Arrival Message\n");
        struct FlightMessage message;
        message.mtype=1008;
        message.flight=f;
        if (msgsnd(msgid, &message, sizeof(struct FlightMessage) - sizeof(long), 0) == -1) {
            perror("msgsnd");
            exit(1);
        }
        printf("Arrival Message Sent\n");
    }
}

