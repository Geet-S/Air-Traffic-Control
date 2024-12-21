#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/msg.h>

#define ATC_QUEUE_KEY 1085
#define MAXLEN 1024

struct CleanupMessage {
    long mtype;
    char mtext[MAXLEN];
};

int main() {
    int msgid;
    struct CleanupMessage msg;

    msgid = msgget(ATC_QUEUE_KEY, 0666);
    if (msgid == -1) {
        perror("msgget");
        exit(EXIT_FAILURE);
    }

    int terminate = 0;
    while (terminate != 1) {
        printf("Do you want the Air Traffic Control System to terminate? (Y for Yes and N for No)\n");
        scanf("%s", msg.mtext);

        if ((strcmp(msg.mtext, "Y") == 0) || (strcmp(msg.mtext, "y")) == 0) {
            printf("Sending termination request to Air Traffic Control System...\n");
            terminate = 1;
            break;
        } 
        else if ((strcmp(msg.mtext, "N") == 0) || (strcmp(msg.mtext, "n")) == 0) {
            printf("Cleanup process will continue running.\n");
        } 
        else {
            printf("Invalid msg.mtext. Please enter Y or N.\n");
        }
    }

    printf("Cleaning up IPC constructs...\n");
    printf("Cleanup process terminated.\n");

    return 0;
}

