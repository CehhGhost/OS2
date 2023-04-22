#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/sem.h>

#define NUM_ROOMS 25

union semun {
    int val;
    struct semid_ds* buf;
    unsigned short* array;
    struct seminfo* __buf;
};

typedef struct {
    int id;
    int budget;
} client_arg;

typedef struct {
    int client;
    int price;
} room_arg;

room_arg rooms[NUM_ROOMS];

int sem_mutex_id;

int findRoom(int budget) {
    int n;
    if (budget < 2000) {
        return -1;
    } else if (budget < 4000) {
        n = 10;
    } else if (budget < 6000) {
        n = 20;
    } else {
        n = 25;
    }
    for (int i = 0; i < n; ++i) {
        if (rooms[i].client == -1 && rooms[i].price <= budget) {
            return i;
        }
    }
    return -1;
}


void* client(void* arg) {
    client_arg* c = (client_arg*) arg;
    int room_price, room_number;
    struct sembuf sem_op;
    sem_op.sem_num = 0;
    sem_op.sem_flg = 0;
    sem_op.sem_op = -1;
    semop(sem_mutex_id, &sem_op, 1);
    room_number = findRoom(c->budget);
    if (room_number == -1) {
        sem_op.sem_op = 1;
        semop(sem_mutex_id, &sem_op, 1);
        free(c);
        pthread_exit(NULL);
    }
    room_price = rooms[room_number].price;
    rooms[room_number].client = c->id;
    c->budget -= room_price;
    printf("Client %d has occupied room %d for %d\n", c->id, room_number, room_price);
    sem_op.sem_op = 1;
    semop(sem_mutex_id, &sem_op, 1);
    usleep((rand() % 5 + 1) * 50000);
    sem_op.sem_op = -1;
    semop(sem_mutex_id, &sem_op, 1);
    rooms[room_number].client = -1;
    sem_op.sem_op = 1;
    semop(sem_mutex_id, &sem_op, 1);
    printf("Client %d has left room %d\n", c->id, room_number);
    free(c);
    pthread_exit(NULL);
}

int main() {
    int i;
    srand(time(NULL));
    sem_mutex_id = semget(IPC_PRIVATE, 1, IPC_CREAT | 0666);
    union semun sem_arg;
    sem_arg.val = 1;
    semctl(sem_mutex_id, 0, SETVAL, sem_arg);
    for (i = 0; i < NUM_ROOMS; ++i) {
        rooms[i].client = -1;
        if (i < 10) {
            rooms[i].price = 2000;
        } else if (i < 20) {
            rooms[i].price = 4000;
        } else {
            rooms[i].price = 6000;
        }
    }
    for (i = 0; findRoom(6000) != -1; i++) {
        client_arg* arg = malloc(sizeof(client_arg));
        arg->id = i;
        arg->budget = (rand() % 1000 + 5000);
pthread_t client_thread;
pthread_create(&client_thread, NULL, client, (void*) arg);
pthread_detach(client_thread);
usleep((rand() % 5 + 1) * 50000);
}
semctl(sem_mutex_id, 0, IPC_RMID, 0);
return 0;
}
