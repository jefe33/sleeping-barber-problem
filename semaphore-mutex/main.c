#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>
#include <stdbool.h>
#include <getopt.h>


struct Queue{
    int id;
    sem_t client_turn;
    sem_t cut_redy;
    struct Queue* next;
};

struct Queue *clients_queue = NULL;
struct Queue *resigned_clients_queue = NULL;

sem_t client_ready;

pthread_mutex_t queue_charis = PTHREAD_MUTEX_INITIALIZER;

int num_of_queue_chairs;
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


int main(int arg,char ** argc){
    
    int c;
    int option_index = 0;
    opterr = 0;
    static struct option long_options[]= {
    {"debug", no_argument,NULL ,1},
    {"chairs", required_argument,NULL ,2},
    {0,0,0,0}
    };
    while(c != -1){
    c = getopt_long(arg, argc, "", long_options,&option_index);
    //printf("%d\n",c);
    switch (c){
       case 1:
       debug = 1;
       printf("DEBUG ON\n");
       break;
       case 2:
       printf("Number of chairs: %s\n",optarg);
       num_of_queue_chairs = atoi(optarg);
       break;
       case 63:
       printf("bledny argument\n");
       exit(-1);
       }
    }
         
    srand(time(0));

    int status;
    int turns = rand() % 5 + 5;
    
    sem_init(&client_ready, 0, 0);
    pthread_t barber_thread, client_thread;

    status = pthread_create(&barber_thread, NULL, barber, NULL);
    if (status!=0)
	{
		printf("Barber thread couldn't start!'\n");
		exit(status);
	}
    
    while(turns > 0) {
        for (int i = 0; i < num_of_queue_chairs + turns; i++) {
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
    sem_init(&new_client->client_turn, 0, 0);
    sem_init(&new_client->cut_redy, 0, 0);

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
        sem_post(&client_ready);
        // odblokowanie dostepu do poczekalni
        pthread_mutex_unlock(&queue_charis);

        // czekanie na swoja kolej do strzyzenia
        sem_wait(&client_in_queue->client_turn);
        // przejscie do strzyzenia i czekanie na jego koniec
        sem_wait(&client_in_queue->cut_redy);
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
        // czekanie na klienta
        sem_wait(&client_ready);
        // zablokowanie dostepu do krzesel w poczekalni w celu zmiany stanu poczeklani
        pthread_mutex_lock(&queue_charis);
        // zmniejszenie liczby osob w kolejce
        num_of_clients_in_queue--;
        // usuniecie osoby z kolejki do ciecia
        struct Queue* client_getting_cut = remove_from_queue(&clients_queue);
        // poproszenie klienta na krzeslo do strzyzenia
        sem_post(&client_getting_cut->client_turn);
        serverd_clinet_id = client_getting_cut->id;
        display();
        // odblokoowanie dostepu do krzesel
        pthread_mutex_unlock(&queue_charis);
        // przeprowadzenie ciecia
        doCut();
        // poinformowanie klienta o koncu strzyzenia
        sem_post(&client_getting_cut->cut_redy);
        // zwolnienie pamieci po kliencie
        free(client_getting_cut);
        // zmiana id obslugiwanego klienta na 0 w celu zasygnalizowania ze zaden klient nie jest strzyzony
        serverd_clinet_id = 0;
    }
}
