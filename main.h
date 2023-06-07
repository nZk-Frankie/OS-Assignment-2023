#include <stdio.h>
#include <ctype.h>
#include <time.h>
#include <stdint.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <semaphore.h>

typedef struct customerNode
{
    int customerNumber;
    char serviceType;
    int aHr;
    int aMin;
    int aSec;
}customerNode;

typedef struct customerQueue
{
    customerNode *data;
    int size;
    int MAX_SIZE;
    pthread_mutex_t mutex;
}customerQueue;

typedef struct tellerStats
{
    int tellerNames;
    int ti;
    int tw;
    int td;
    //time keeping purpose .. start time
    int sHr;
    int sMin;
    int sSec;
    //time keeping purpose ... finish time
    int fHr;
    int fMin;
    int fSec;
    customerQueue *queue;
    int nCustomer;
    pthread_mutex_t mutex;

}tellerStats;

typedef struct informationCustomerThread{
    int periodicTime;
    int space;
    customerQueue *queue;
}informationCustomerThread;

void *customer(void *arg);
void *Bank(void *arg);
customerNode dequeue(customerQueue *queue);
int validateInput(char mode);
