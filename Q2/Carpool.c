#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>
int Execute = 1;
int start=1;

sem_t load_full_semaphor, load_empty_semaphor, unload_full_semaphor, unload_empty_semaphor, mutex_run;

void run() 
{
    printf("The ride is running\n"); 
    sleep(2);
}

void load(int capacity) 
{
    for (int i = 0; i < capacity; ++i) 
    {
        sem_wait(&load_empty_semaphor);
        sem_post(&load_full_semaphor);
    }
}

void unload(int capacity)
{
    for (int i = 0; i < capacity; ++i) 
    {
        sem_wait(&unload_full_semaphor); 
        sem_post(&unload_empty_semaphor); 
    }
}

void board(int n) 
{
    printf("Passenger %d is boarding\n", n); 
    sleep(1);
}

void offboard(int n) 
{
    printf("Passenger %d is offboarding\n", n); 
    
    sleep(1);
}

void* car(void* args) 
{
    int capacity = *(int*)args;
    while (Execute) 
    {   
        load(capacity);
        sem_wait(&mutex_run);
        run();
        sem_post(&mutex_run);
        unload(capacity);
    }
}

void* passenger(void* args) 
{
    int position = *(int*)args;
    while (Execute) 
    {   
        sem_wait(&load_full_semaphor);
        board(position);
        sem_post(&mutex_run);
        sem_wait(&unload_empty_semaphor);

        sem_wait(&mutex_run);
        offboard(position);
        sem_post(&unload_full_semaphor);
        sem_post(&load_empty_semaphor);
    }
}

int main() 
{
    int num_passengers, max_capacity;
    printf("Enter the number of passengers waiting and the max capacity of the car: \n");
    scanf("%d %d", &num_passengers, &max_capacity);

    pthread_t Car_th;
    pthread_t* Passenger_th = (pthread_t*)malloc(num_passengers * sizeof(pthread_t));

    sem_init(&load_empty_semaphor, 0, max_capacity);
    sem_init(&load_full_semaphor, 0, 0);
    sem_init(&unload_empty_semaphor, 0, 0);
    sem_init(&unload_full_semaphor, 0, max_capacity);
    sem_init(&mutex_run, 0, 0);

    int position[num_passengers];

    for (int i = 0; i < num_passengers; ++i)
        position[i] = i;

    int* capacity = &max_capacity;

    if (pthread_create(&Car_th, NULL, &car, (void*)capacity) != 0)
        perror("Failed to Create Car Thread");

    for (int i = 0; i < num_passengers; ++i) {
        if (pthread_create(&Passenger_th[i], NULL, &passenger, (void*)&position[i]) != 0)
            perror("Failed to Create Passenger Thread");
    }
    start=0;
    //printf("Enter any character to stop execution: ");
    char exit;
    scanf(" %c", &exit); 
    Execute = 0;
    printf("Execution Stopped\n"); 

    for (int i = 0; i < num_passengers; ++i) 
    {
        if (pthread_join(Passenger_th[i], NULL) != 0)
            perror("Failed to Join Passenger Thread"); 
    }

    if (pthread_join(Car_th, NULL) != 0)
        perror("Failed to Join Car Thread");

    free(Passenger_th); 
    sem_destroy(&load_empty_semaphor);
    sem_destroy(&load_full_semaphor);
    sem_destroy(&unload_empty_semaphor);
    sem_destroy(&unload_full_semaphor);

    return 0;
}