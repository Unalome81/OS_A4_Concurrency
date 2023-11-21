#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

#define Num_Philosophers 5
#define Num_Forks 5
#define Num_Bowls 2
#define Execution_Time 1000000
#define Eating_Time 30
#define Thinking_Time 30

void* philosopher(void* pos);
void eating(int pos);
void thinking(int pos);

pthread_mutex_t mutex;
pthread_cond_t condition;

int Fork_state[Num_Philosophers]; // Shared Resources
int Bowls_left = 0;

int Eating_Count[Num_Philosophers]; // To check the fairness of the logic
int Thinking_Count[Num_Philosophers];

bool Execute = true;

void* philosopher(void* pos)
{
    int position = *(int*)pos;
    int right = (position + 1) % Num_Philosophers;
    int left = (position + Num_Philosophers - 1) % Num_Philosophers;

    while(Execute)
    {
        pthread_mutex_lock(&mutex);

        while (Fork_state[left] == 1 || Fork_state[right] == 1 || Bowls_left == 0) 
        {
            thinking(position);
            pthread_cond_wait(&condition, &mutex);
        }

        // Acquiring forks and bowl
        Fork_state[left] = 1;
        Fork_state[right] = 1;
        Bowls_left--;

        pthread_mutex_unlock(&mutex);

        eating(position);

        pthread_mutex_lock(&mutex);

        // Releasing forks and bowl
        Fork_state[left] = 0;
        Fork_state[right] = 0;
        Bowls_left++;

        // Signalling other philosophers that resources are available
        pthread_cond_broadcast(&condition);

        pthread_mutex_unlock(&mutex);
    }
}

void eating(int pos)
{
    printf("Philosopher %d is eating\n", pos);
    ++Eating_Count[pos];
    usleep(Eating_Time);
}

void thinking(int pos)
{
    printf("Philosopher %d is thinking\n", pos);
    ++Thinking_Count[pos];
    usleep(Thinking_Time);
}

int main()
{
    pthread_t Philosophers[Num_Philosophers];
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&condition, NULL);

    for (int i = 0; i < Num_Philosophers; ++i)
        Fork_state[i] = 0;

    Bowls_left = Num_Bowls;

    for (int i = 0; i < Num_Philosophers; ++i) {
        Eating_Count[i] = 0;
        Thinking_Count[i] = 0;
    }

    int position[Num_Philosophers];

    for (int i = 0; i < Num_Philosophers; ++i)
        position[i] = i;

    for (int i = 0; i < Num_Philosophers; ++i) {
        if (pthread_create(&Philosophers[i], NULL, &philosopher, (void*)&position[i]) != 0)
            perror("Failed to Create Thread");
    }

    usleep(Execution_Time);
    Execute = false;

    for (int i = 0; i < Num_Philosophers; ++i) {
        if (pthread_join(Philosophers[i], NULL) != 0)
            perror("Failed to Join Thread");
    }

    printf("                %-20s%-20s\n", "Thinking Count", "Eating Count");

    for (int i = 0; i < Num_Philosophers; ++i) {
        printf("Philosopher %d - %-20d%-20d\n", (i + 1), Thinking_Count[i], Eating_Count[i]);
    }

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&condition);
    return 0;
}