//
// Created by Emre Cankaya on 01.06.2021.
//
#include <pthread.h>
#include <semaphore.h>
#define THREAD_NUM 5

sem_t sem; //name of semaphore

void semaphores(){
    pthread_t th[THREAD_NUM];    //example with threads                                                                                            pshared = 1 when multiple process with multiple threads
    sem_init(&sem,0,1);          //create semaphore(reference,pshared,initial value),initial value can control how many processes work at a time , pshared = 0 cannot be shared between processes
    sem_wait(&sem);              //decrement semaphore value ,if value = 0 ,process is set to wait
    sem_post(&sem);              //increment semaphore value
    sem_destroy(&sem);           //destroys semaphore ,frees memory
    //sem_try
}