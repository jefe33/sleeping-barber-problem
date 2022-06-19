#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>

#define N 10

struct Queue{
    int id;
    pthread_cond_t client_turn;
    pthread_cond_t cut_redy;
    struct Queue* next;
};

struct Queue *clients_queue = NULL;
struct Queue *resigned_clients_queue = NULL;

pthread_cond_t client_ready = PTHREAD_COND_INITIALIZER;

pthread_mutex_t queue_charis = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t barber_chair = PTHREAD_MUTEX_INITIALIZER;

int num_of_queue_chairs = N;
int num_of_clients_in_queue = 0;
int total_id = 1;
int serverd_clinet_id = 0;
int num_of_resigned_clients = 0;
int debug = 0;


void* client(void*);
void* barber(void*);
void doCut();
struct Queue* add_to_queue(struct Queue**);
struct Queue* remove_from_queue(struct Queue**); 
void print_queue();
void display();
void displayDebug();


int main(){
    srand(time(0));

    int status;
    int turns = rand() % 5 + 5;
    
    pthread_t barber_thread, client_thread;

    status = pthread_create(&barber_thread, NULL, barber, NULL);
    if (status!=0)
	{
		printf("Barber thread couldn't start!'\n");
		exit(status);
	}
    
    while(turns > 0) {
        for (int i = 0; i < N + turns; i++) {
            status = pthread_create(&client_thread, NULL, client, NULL);
            if (status != 0)
            {
                printf("Customer thread couldn't start!\n");
                exit(status);
            }
            usleep(rand() % 3);
        }
        usleep(500000);
        turns--;
    }
}


struct Queue* add_to_queue(struct Queue** head){
    struct Queue* new_client;
    new_client = malloc(sizeof(struct Queue));
    new_client->id = total_id++;
    new_client->next = NULL;
    pthread_cond_init(&new_client->client_turn, NULL);
    pthread_cond_init(&new_client->cut_redy, NULL);

    if(*head == NULL){
        *head = new_client;
        
    }else {
        struct Queue* tmp = *head;
        
        while(tmp->next){
            tmp = tmp->next;
        }
        tmp->next = new_client;
    }
    return new_client;
}


struct Queue* remove_from_queue(struct Queue** head){
    struct Queue* tmp = *head;
    *head = tmp->next;
    return tmp;
}

void displayDebug()
{
	printf("Waiting: ");	
	print_queue(clients_queue);
	printf("Resigned: ");	
	print_queue(resigned_clients_queue);
}

void display(){
    if (serverd_clinet_id != 0){
		printf("Res: %d wRoom: %d/%d In: %d\n", num_of_resigned_clients, num_of_clients_in_queue, num_of_queue_chairs, serverd_clinet_id);
	}else{
		printf("Res: %d wRoom: %d/%d In: -\n", num_of_resigned_clients, num_of_clients_in_queue, num_of_queue_chairs);
	}
    if (debug){
        displayDebug();
    }
}


void print_queue(struct Queue* head){
    if (head){
        struct Queue* tmp = head;
        while (tmp->next) {
            printf("%i ", tmp->id);
            tmp = tmp->next;
        }
        if (tmp){
            printf("%i ", tmp->id);
        }
    }
    printf("\n");
}

void doCut(){
    int k;
    for (int i = 0; i < rand() % 300000; i++) {
        for (int j = 1; j < rand() % 100000; j++) {
            k = i / j;
            k *= (i * j) % 3;
        }
    }
}


void* client(void* ptr){
    // zablokowanie dostepu do poczekalni
    pthread_mutex_lock(&queue_charis);
    if (num_of_clients_in_queue < num_of_queue_chairs) {
        // zwiekszenie liczby klientow w kolejce
        num_of_clients_in_queue++;
        // dodanie nowej osoby do kolejki klientow czekajacych na strzyzenie
        struct Queue* client_in_queue = add_to_queue(&clients_queue);
        display();
        // poinformowanie fryzjera ze client jest gotowy na strzyzenie
        pthread_cond_signal(&client_ready);

        // odblokowanie dostepu do poczekalni
        pthread_mutex_unlock(&queue_charis);

        // czekanie na swoja kolej do strzyzenia
        pthread_mutex_lock(&barber_chair);
        while (serverd_clinet_id != client_in_queue->id) {
            pthread_cond_wait(&client_in_queue->client_turn, &barber_chair);
        }
        pthread_mutex_unlock(&barber_chair);

        // przejscie do strzyzenia i czekanie na jego koniec
        pthread_mutex_lock(&barber_chair);
        pthread_cond_wait(&client_in_queue->cut_redy, &barber_chair);
        pthread_mutex_unlock(&barber_chair);
        return NULL;
    }else {
        // jesli brak wolnych krzesel
        // zwiekszenie liczby zrezygnowanych klientow
        num_of_resigned_clients++;
        // dodanie nowej osoby do kolejki zrezygnowanych klientow
        add_to_queue(&resigned_clients_queue);
        display();
        // odblokowanie dostepu do poczekalni
        pthread_mutex_unlock(&queue_charis);
        return NULL;
    }
}

void* barber(void* ptr){
    for(;;) {
        // zablokowanie dostepu do krzesel w poczekalni w celu zmiany stanu poczeklani
        pthread_mutex_lock(&queue_charis);
        // czekanie na klienta
        while (num_of_clients_in_queue == 0) {
            pthread_cond_wait(&client_ready, &queue_charis);
        }
        // zmniejszenie liczby osob w kolejce
        num_of_clients_in_queue--;
        // usuniecie osoby z kolejki do ciecia
        struct Queue* client_getting_cut = remove_from_queue(&clients_queue);
        serverd_clinet_id = client_getting_cut->id;
        display();
        // odblokoowanie dostepu do krzesel
        pthread_mutex_unlock(&queue_charis);

        // poproszenie klienta na krzeslo do strzyzenia
        pthread_mutex_lock(&barber_chair);
        pthread_cond_signal(&client_getting_cut->client_turn);
        pthread_mutex_unlock(&barber_chair);

        // przeprowadzenie ciecia
        doCut();

        // poinformowanie klienta o koncu strzyzenia
        pthread_mutex_lock(&barber_chair);
        // zmiana id obslugiwanego klienta na 0 w celu zasygnalizowania ze zaden klient nie jest strzyzony
        serverd_clinet_id = 0;
        pthread_cond_signal(&client_getting_cut->cut_redy);
        pthread_mutex_unlock(&barber_chair);

        // zwolnienie pamieci po kliencie
        free(client_getting_cut);
    }
}