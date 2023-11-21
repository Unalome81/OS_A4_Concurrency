#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <math.h> // Add this for fmin function

#define Max_Cars 5

int bridge_state = 0; // left cars = 0, right cars = 1

sem_t left_full_semaphor, left_empty_semaphor, right_full_semaphor, right_empty_semaphor, mutex;

int num_left, num_right;
int left_passed = 0, right_passed = 0; // shared resource

void passing(int dir);

void *left(void *args);
void *right(void *args);

void left_load();
void right_load();

void *Bridge(void* args);

void passing(int dir)
{
    if (dir > 0)
    {
        printf("Car from right direction of position %d is passing\n", dir-1);
    }
    else
    {
        printf("Car from left direction of position %d is passing\n", -1 *  dir);
    }
    sleep(1);
}

void left_load()
{
    int count_left = (Max_Cars < num_left - left_passed)? Max_Cars : num_left - left_passed;
    for (int i = 0; i < count_left; ++i)
    {   
        sem_wait(&left_empty_semaphor);
        sem_post(&left_full_semaphor);
    }
}

void right_load()
{
    int count_right = (Max_Cars < num_right - right_passed)? Max_Cars : num_right - right_passed;
    for (int i = 0; i < count_right; ++i)
    {   
        sem_wait(&right_empty_semaphor);
        sem_post(&right_full_semaphor);
    }
}

void *left(void *args)
{
    int pos = *(int *)args;
    pos *= -1;

    sem_wait(&left_full_semaphor);
    passing(pos);
    sem_post(&left_empty_semaphor);

    sleep(1);

    sem_wait(&mutex);
    left_passed++; 
    sem_post(&mutex);

    free(args); 

    return NULL;
}

void *right(void *args)
{
    int pos = *(int *)args;

    sem_wait(&right_full_semaphor);
    passing(pos);
    sem_post(&right_empty_semaphor);

    sem_wait(&mutex);
    right_passed++; 
    sem_post(&mutex);

    free(args); 
    return NULL;
}

void *Bridge(void* args)
{
    while (left_passed < num_left && right_passed < num_right)
    {
        left_load();
        sleep(1);
        right_load();
        sleep(1);
    }


    while (left_passed < num_left)
    {
        left_load();
        sleep(1);
    }

    while (right_passed < num_right)
    {
        right_load();
        sleep(1);
    }
    free(args); 
    return NULL;
}

int main()
{
    printf("Enter the number of left cars and right cars: \n");
    scanf("%d %d", &num_left, &num_right);

    pthread_t *left_th = (pthread_t *)malloc(num_left * sizeof(pthread_t));
    pthread_t *right_th = (pthread_t *)malloc(num_right * sizeof(pthread_t));
    pthread_t Bridge_th;
    sem_init(&left_full_semaphor, 0, 0);
    sem_init(&left_empty_semaphor, 0, Max_Cars);
    sem_init(&right_full_semaphor, 0, 0);
    sem_init(&right_empty_semaphor, 0, Max_Cars);
    sem_init(&mutex, 0, 1);
    for (int i = 0; i < num_left; ++i)
    {
        int *pos = malloc(sizeof(int));
        *pos = i;
        if (pthread_create(&left_th[i], NULL, &left, (void *)pos) != 0)
            perror("Failed to Create Thread");
    }
    if (pthread_create(&Bridge_th, NULL, &Bridge,NULL) != 0)
        perror("Failed to Create Car Thread");
    for (int i = 0; i < num_right; ++i)
    {
        int *pos = malloc(sizeof(int));
        *pos = i+1;
        if (pthread_create(&right_th[i], NULL, &right, (void *)pos) != 0)
            perror("Failed to Create Thread");
    }
    for (int i = 0; i < num_left; ++i)
    {
        if (pthread_join(left_th[i], NULL) != 0)
            perror("Failed to Join Thread");
    }

    for (int i = 0; i < num_right; ++i)
    {
        if (pthread_join(right_th[i], NULL) != 0)
            perror("Failed to Join Thread");
    }
    if (pthread_join(Bridge_th, NULL) != 0)
        perror("Failed to Join Bridge Thread");
    sem_destroy(&left_empty_semaphor);
    sem_destroy(&left_full_semaphor);
    sem_destroy(&right_empty_semaphor);
    sem_destroy(&right_full_semaphor);
    sem_destroy(&mutex);

    free(left_th);
    free(right_th);
    return 0;
}