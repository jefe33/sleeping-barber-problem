#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>

#define N 3

struct Queue{
    int id;
    sem_t client;
    struct Queue* next;
} *head = NULL;

void* func(void *);
struct Queue* add_to_queue();
struct Queue* remove_from_queue(); 
void print_queue();
void* client(void*);
void* barber(void*);
void display();
void doCut();

sem_t client_ready, hair_cutting;
pthread_mutex_t queue_charis = PTHREAD_MUTEX_INITIALIZER;
int free_queue_chairs = N;
int total_id = 0;

int main(){
    srand(time(0));
    // struct Queue* head = NULL;
    sem_init(&client_ready, 0, 0);
    sem_init(&hair_cutting, 0, 0);
    pthread_t barber_thread, client_thread;
    pthread_create(&barber_thread, NULL, barber, NULL);
    while (1) {
        for (int i = 0; i < 10; i++) {
            printf("client nr: %d\n", i);
            pthread_create(&client_thread, NULL, client, NULL);
            // if (i % (rand() % 5 + 1) == 0){
            //     sleep(1);
            // }
        }
            sleep(3);
    }
    // for (int i = 0; i < N; i++) {
    //     pthread_create(&threads[i], NULL, func, NULL);
    // }
}

void* func(void* ptr){
    printf("watek");
    return NULL;
}

struct Queue* add_to_queue(){
    struct Queue* new_client;
    new_client = malloc(sizeof(struct Queue));
    new_client->id = total_id++;
    new_client->next = NULL;
    sem_init(&new_client->client, 0, 0);
    if(!head){
        head = new_client;
        return head;
        
    }else {
        struct Queue* tmp;
        tmp = head;
        while(tmp->next){
            tmp = tmp->next;
        }
        tmp->next = new_client;
        return new_client;
    }
}


struct Queue* remove_from_queue(){
    struct Queue* tmp;
    tmp = head;
    head = tmp->next;
    return tmp;
}

void display(){
    printf("number of free seats: %d\n", free_queue_chairs);
}

void print_queue(){
    struct Queue* tmp = head;
    while (tmp->next) {
        printf("%i ", tmp->id);
        tmp = tmp->next;
    }
    if (tmp){
        printf("%i ", tmp->id);
    }
    printf("\n");
}

void doCut(){
    int j;
    for (int i = 0; i < rand() % 3000; i++) {
        j = i / 32451;
        j = j % 124214;
    }
    sem_post(&hair_cutting);
}


void* client(void* ptr){
    // lock queue chairs to add new client
    // sleep(1);

    // printf("przed ifem\n");
    pthread_mutex_lock(&queue_charis);

    if (free_queue_chairs) {
        // printf("enter the void\n");
        free_queue_chairs--;
        struct Queue* client_in_queue = add_to_queue();
        // signal that client is ready to cut
        display();
        sem_post(&client_ready);
        // unlock queue chairs
        pthread_mutex_unlock(&queue_charis);
        // wait to get cut
        sem_wait(&client_in_queue->client);
        // get cut and wait for end of cut
        sem_wait(&hair_cutting);
        return NULL;
    }else {
        pthread_mutex_unlock(&queue_charis);
        return NULL;
    }
}

void* barber(void* ptr){
    while (1) {
        // check if there are clients in queue
        sem_wait(&client_ready); // TODO add sleeping
        print_queue();
        // l
        pthread_mutex_lock(&queue_charis);
        free_queue_chairs++;
        struct Queue* client_getting_cut = remove_from_queue();
        sem_post(&client_getting_cut->client);
        // do cut
        pthread_mutex_unlock(&queue_charis);
        doCut();
        free(client_getting_cut);
    }
}