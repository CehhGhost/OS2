#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <unistd.h>
#include <semaphore.h>
#include <fcntl.h>

#define NUM_ROOMS 25
#define SEM_MUTEX "rooms_mutex"

typedef struct {
    int id;
    int budget;
} client_arg;

typedef struct {
    int client;
    int price;
} room_arg;

room_arg rooms[NUM_ROOMS];

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
    sem_t* sem_mutex = sem_open(SEM_MUTEX, O_CREAT, 0666, 1);
    sem_wait(sem_mutex);
    room_number = findRoom(c->budget);
    if (room_number == -1) {
        sem_post(sem_mutex);
        free(c);
        sem_close(sem_mutex);
        pthread_exit(NULL);
    }
    room_price = rooms[room_number].price;
    rooms[room_number].client = c->id;
    c->budget -= room_price;
    printf("Client %d has occupied room %d for %d\n", c->id, room_number, room_price);
    sem_post(sem_mutex);
    usleep((rand() % 5 + 1) * 200000);
    sem_wait(sem_mutex);
    rooms[room_number].client = -1;
    sem_post(sem_mutex);
    printf("Client %d has left room %d\n", c->id, room_number);
    free(c);
    sem_close(sem_mutex);
    pthread_exit(NULL);
}

int main() {
    int i;
    srand(time(NULL));
    sem_t* sem_mutex = sem_open(SEM_MUTEX, O_CREAT, 0666, 1);
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
        arg->budget = (rand() % 10 + 1) * 1000;
        pthread_t pthread;
        pthread_create(&pthread, NULL, client, arg);
        usleep(1000);
    }
    sem_close(sem_mutex);
    sem_unlink(SEM_MUTEX);
    return 0;
}
