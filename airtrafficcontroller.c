#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>
#include <mqueue.h>

struct ATCPlane {
    int Arriv;
    int dept;
    int planeID;
    int totalWeight;
    int planeType;
    int numOfPassengers;
    int currentStage;
    int isTerminated;
};

struct ATCMessage {
    long mtype;
    struct ATCPlane plane;
};

int main(){

    key_t key;
    int msgid;
    struct ATCMessage message;

    key = ftok("AirTrafficController.txt", 65);
    if (key == -1) {
        perror("ftok");
        exit(1);
    }

    msgid = msgget(key, 0666);
    if (msgid == -1) {
        perror("msgget");
        exit(1);
    }

    struct ATCPlane p=message.plane;
    int endflag=1;
    int loop=1;
    int stats=0;
    while(loop){
        if ((msgrcv(msgid, &message, sizeof(struct ATCMessage) - sizeof(long), 1010, IPC_NOWAIT) == -1)) {
            endflag=0;
        }
        if(endflag!=0){
            if((msgrcv(msgid, &message, sizeof(struct ATCMessage) - sizeof(long), 1008, 0) == -1)){
                exit(1);
            }
            struct ATCPlane p=message.plane;
            if(p.currentStage==1){
                p.currentStage++;
                stats++;
                struct ATCMessage ms;
                ms.plane=p;
                ms.mtype=p.Arriv+2000;
                if (msgsnd(msgid, &ms, sizeof(struct ATCMessage) - sizeof(long), 0) == -1) {
                    perror("msgsnd");
                    exit(1);
                }   
            }
            if(p.currentStage==3){
                p.currentStage++;
                struct ATCMessage ms;
                ms.plane=p;
                ms.mtype=p.dept+2000;
                if (msgsnd(msgid, &ms, sizeof(struct ATCMessage) - sizeof(long), 0) == -1) {
                    perror("msgsnd");
                    exit(1);
                }   
            }
            if(p.currentStage==5){
                p.currentStage++;
                stats--;
                struct ATCMessage ms;
                ms.plane=p;
                ms.mtype=p.planeID;
                if (msgsnd(msgid, &ms, sizeof(struct ATCMessage) - sizeof(long), 0) == -1) {
                    perror("msgsnd");
                    exit(1);
                }   
            }
            
        }else{
            if(stats==0){
                return 0;
            }else{
                if((msgrcv(msgid, &message, sizeof(struct ATCMessage) - sizeof(long), 1008, 0) == -1)){
                    exit(1);
                }
                struct ATCPlane p=message.plane;
                if(p.currentStage==1){
                    p.currentStage=7;
                    
                    struct ATCMessage ms;
                    ms.plane=p;
                    ms.mtype=p.planeID;
                    if (msgsnd(msgid, &ms, sizeof(struct ATCMessage) - sizeof(long), 0) == -1) {
                        perror("msgsnd");
                        exit(1);
                    }   
                }
                if(p.currentStage==3){
                    p.currentStage++;
                    struct ATCMessage ms;
                    ms.plane=p;
                    ms.mtype=p.dept+2000;
                    if (msgsnd(msgid, &ms, sizeof(struct ATCMessage) - sizeof(long), 0) == -1) {
                        perror("msgsnd");
                        exit(1);
                    }   
                }
                if(p.currentStage==5){
                    p.currentStage++;
                    stats--;
                    struct ATCMessage ms;
                    ms.plane=p;
                    ms.mtype=p.planeID;
                    if (msgsnd(msgid, &ms, sizeof(struct ATCMessage) - sizeof(long), 0) == -1) {
                        perror("msgsnd");
                        exit(1);
                    }   
                }
            }

        }


    }
}

