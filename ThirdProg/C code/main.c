#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <unistd.h>

#define NUM_ROOMS 25

pthread_mutex_t rooms_mutex = PTHREAD_MUTEX_INITIALIZER;

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

    pthread_mutex_lock(&rooms_mutex);  // acquire lock
    room_number = findRoom(c->budget);
    if (room_number == -1) {
        pthread_mutex_unlock(&rooms_mutex);  // release lock
        free(c);
        pthread_exit(NULL);
    }
    room_price = rooms[room_number].price;
    rooms[room_number].client = c->id;  // assign room to client
    c->budget -= room_price;  // deduct room price from budget
    printf("Client %d has occupied room %d for %d\n", c->id, room_number, room_price);
    pthread_mutex_unlock(&rooms_mutex);  // release lock

    usleep((rand() % 5 + 1) * 50000);  // simulate stay in hotel

    pthread_mutex_lock(&rooms_mutex);  // acquire lock
    rooms[room_number].client = -1;  // free room
    pthread_mutex_unlock(&rooms_mutex);  // release lock

    printf("Client %d has left room %d\n", c->id, room_number);

    free(c);
    pthread_exit(NULL);
}

int main() {
    int i;
    srand(time(NULL));
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
        arg->budget = (rand() % 10 + 1) * 1000;  // choose random budget between 1000 and 10000
        pthread_t pthread;
        pthread_create(&pthread, NULL, client, arg);
        usleep(1000);  // sleep to prevent threads from starting at exactly the same time
    }

    return 0;
}
