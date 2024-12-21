#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <pthread.h>

struct Plane {
    int Arriv;
    int dept;
    int pid;
    int tweight;
    int type;
    int numofpass;
    int stage;
    int terminate;
};

pthread_mutex_t lock[11]; // for 10 regular runways + 1 backup runway
int lock1[11] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};

int Runway[11] = {0}; // holds load capacity of runways
int numorunway; // global variable

key_t key;
int msgid;

struct MyMessage {
    long mtype;
    struct Plane plane;
};

int bestfit(int runway[], int value, int numorunway) {
    int minDiff = value + 1; 
    int bestIndex = -1;

    for (int i = 0; i < numorunway; i++) {
        if (lock1[i] && runway[i] >= value) {
            int diff = runway[i] - value;
            if (diff < minDiff) {
                minDiff = diff;
                bestIndex = i;
            }
        }
    }

    return bestIndex;
}

void depature(struct Plane p, int msgid, int Runway[], int numorunway) {
    int index = bestfit(Runway, p.tweight, numorunway);
    if (index == -1) {
        pthread_mutex_lock(&lock[10]);
        lock1[10] = 0;
        sleep(3);
        printf("Plane %d has completed boarding/loading and taken off from the backup runway of Airport No. %d.\n", p.pid, p.dept);
        sleep(2);
        struct MyMessage m;
        m.mtype = 1008;
        p.stage++;
        m.plane = p;
        if (msgsnd(msgid, &m, sizeof(struct MyMessage) - sizeof(long), 0) == -1) {
            perror("msgsnd");
            exit(1);
        }
        lock1[10] = 1;
        pthread_mutex_unlock(&lock[10]);
    } else {
        pthread_mutex_lock(&lock[index]);
        lock1[index] = 0;
        sleep(3);
        printf("Plane %d has completed boarding/loading and taken off from Runway No. %d of Airport No. %d.\n", p.pid, index, p.dept);
        sleep(2);
        struct MyMessage m;
        m.mtype = 1008;
        p.stage++;
        m.plane = p;
        if (msgsnd(msgid, &m, sizeof(struct MyMessage) - sizeof(long), 0) == -1) {
            perror("msgsnd");
            exit(1);
        }
        lock1[index] = 1;
        pthread_mutex_unlock(&lock[index]);
    }
}

void* deptthread(void* arg) {
    struct Plane* p = (struct Plane*)arg;
    depature(*p, msgid, Runway, numorunway);
    pthread_exit(NULL);
}

void arrival(struct Plane p, int msgid, int Runway[], int numorunway) {
    int index = bestfit(Runway, p.tweight, numorunway);
    if (index == -1) {
        pthread_mutex_lock(&lock[10]);
        lock1[10] = 0;
        sleep(3);
        printf("Plane %d has landed on the backup runway of Airport No. %d and has completed deboarding/unloading.\n", p.pid, p.dept);
        sleep(2);
        struct MyMessage m;
        m.mtype = 1008;
        p.stage++;
        m.plane = p;
        if (msgsnd(msgid, &m, sizeof(struct MyMessage) - sizeof(long), 0) == -1) {
            perror("msgsnd");
            exit(1);
        }
        lock1[10] = 1;
        pthread_mutex_unlock(&lock[10]);
    } else {
        pthread_mutex_lock(&lock[index]);
        lock1[index] = 0;
        sleep(3);
        printf("Plane %d has landed on Runway No. %d of Airport No. %d and has completed deboarding/unloading.\n", p.pid, index, p.dept);
        sleep(2);
        struct MyMessage m;
        m.mtype = 1008;
        p.stage++;
        m.plane = p;
        if (msgsnd(msgid, &m, sizeof(struct MyMessage) - sizeof(long), 0) == -1) {
            perror("msgsnd");
            exit(1);
        }
        lock1[index] = 1;
        pthread_mutex_unlock(&lock[index]);
    }
}

void* arrvthread(void* arg) {
    struct Plane* p = (struct Plane*)arg;
    arrival(*p, msgid, Runway, numorunway);
    pthread_exit(NULL);
}

int main() {
    printf("Enter Airport Number:\n");
    int Airportid;
    scanf("%d", &Airportid);

    printf("Enter number of Runways:\n");
    scanf("%d", &numorunway);

    int add = Airportid + 2000;

    struct MyMessage message;

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

    // Initialize mutexes
    for (int i = 0; i <= numorunway; i++) {
        if (pthread_mutex_init(&lock[i], NULL) != 0) {
            perror("pthread_mutex_init");
            exit(1);
        }
    }

    printf("Enter loadCapacity of Runways (give as a space separated list in a single line, end with 'x'):\n");
    for (int i = 0; i < numorunway; i++) {
        scanf("%d", &Runway[i]);
    }

    pthread_t tid[100];
    int n = 0;

    while (1) {
        if (msgrcv(msgid, &message, sizeof(struct MyMessage) - sizeof(long), add, 0) == -1) {
            perror("msgrcv");
            exit(1);
        }

        struct Plane p = message.plane;

        if (p.terminate == 1) {
            break;
        }

        if (p.stage == 2) { // Departure stage
            pthread_create(&tid[n], NULL, deptthread, (void*)&p);
            n++;
        } else if (p.stage == 4) { // Arrival stage
            pthread_create(&tid[n], NULL, arrvthread, (void*)&p);
            n++;
        }
    }

    for (int i = 0; i < n; i++) {
        pthread_join(tid[i], NULL);
    }

    // Destroy mutexes
    for (int i = 0; i <= numorunway; i++) {
        pthread_mutex_destroy(&lock[i]);
    }

    return 0;
}
