#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>

#define N 10

struct Queue{
    sem_t client;
    struct Queue* next;
    struct Queue* previous;
} *head = NULL;

void* func(void *);
struct Queue* add_to_queue();
void remove_from_queue(); 
void print_queue();
void* client(void*);
void* barber(void*);

sem_t client_ready;
pthread_mutex_t queue_charis = PTHREAD_MUTEX_INITIALIZER;
int free_queue_chairs = N;

int main(){
    // struct Queue* head = NULL;
    sem_init(&client_ready, 0, 0);
    pthread_t barber, client;
    pthread_create(&barber, NULL, func, NULL);
    while (1) {
        int i = rand()%3;
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
    if(!head){
        head = malloc(sizeof(struct Queue));
        // head->client = PTHREAD_MUTEX_INITIALIZER;
        head->next = NULL;
        head->previous = NULL;
        sem_init(&head->client, 0, 0);
        return head;
        
    }else {
        struct Queue* tmp;
        tmp = head;
        while(tmp->next){
            tmp = tmp->next;
        }

        struct Queue* new_client;
        new_client = malloc(sizeof(struct Queue));
        new_client->next = NULL;
        new_client->previous = tmp;
        sem_init(&new_client->client, 0, 0);
        tmp->next = new_client;
        return new_client;
    }
}


void remove_from_queue(){
    if (head){
        struct Queue* tmp;
        tmp = head;
        head = tmp->next;
        if(head){
            head->previous = NULL;
        }
        free(tmp);
    }
}

void print_queue(){
    struct Queue* tmp = head;
    // while (tmp->next) {
    //     printf("%i", tmp->client);
    //     tmp = tmp->next;
    // }
    // if (tmp){
    //     printf("%i", tmp->client);
    // }

}

void* client(void* ptr){
    // lock queue chairs to add new client
    pthread_mutex_lock(&queue_charis);
    if (free_queue_chairs) {
        free_queue_chairs--;
        struct Queue* client_in_queue = add_to_queue();
        // signal that client is ready to cut
        sem_post(&client_ready);
        // unlock queue chairs
        pthread_mutex_unlock(&queue_charis);
        // wait to get cut
        sem_wait(&client_in_queue->client);
        // do cut

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
        // l
        pthread_mutex_lock(&queue_charis);
        free_queue_chairs++;
        sem_post(&head->client);
        remove_from_queue();
        pthread_mutex_unlock(&queue_charis);
        
    }
}