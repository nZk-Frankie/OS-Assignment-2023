#include "main.h"

#define NUM_TELLER 4
pthread_cond_t conditionArrayFULL = PTHREAD_COND_INITIALIZER;
pthread_cond_t conditionEmpty = PTHREAD_COND_INITIALIZER;
pthread_mutex_t fileMutex;

FILE *r_log;

int main(int argc, char *argv[])
{
   r_log = fopen("r_log.txt","w");
   //create and initialise mutex;
   pthread_mutex_t mutex;
   pthread_mutex_init(&mutex,NULL);
   pthread_mutex_init(&fileMutex,NULL);
   //this customerThread will be responsible for enqueue in customer on a fifo queue in a set of interval.
   pthread_t customerThread;
   //this is the teller tread
   pthread_t teller[NUM_TELLER];

   customerQueue *queue = malloc(sizeof(customerQueue));

   queue->size=0;

   informationCustomerThread *infoCustThread = malloc(sizeof(informationCustomerThread));
   tellerStats *tellerObject = malloc(sizeof(tellerStats)*NUM_TELLER);

   //time keeping purpose
   time_t timeEpoch;
   struct tm *startTime, *finishTime;
   startTime = (struct tm*)malloc(sizeof(struct tm));
   finishTime = (struct tm*)malloc(sizeof(struct tm));

   int lengthQueue, periodicTime, tw, td, ti;
   if (argc == 6)
   {
      lengthQueue = atoi(argv[1]);
      periodicTime = atoi(argv[2]);
      tw = atoi(argv[3]);
      td = atoi(argv[4]);
      ti = atoi(argv[5]);

      queue->mutex = mutex;

      customerNode *data = malloc(sizeof(customerNode)*lengthQueue);

      infoCustThread->periodicTime = periodicTime;
      infoCustThread->queue = queue;
      infoCustThread->space = lengthQueue;

      queue->data = data;
      queue->MAX_SIZE = lengthQueue;

      for(int i = 0; i<NUM_TELLER; i++)
      {
         time(&timeEpoch);
         localtime_r(&timeEpoch,startTime);
         tellerObject[i].sHr = startTime->tm_hour;
         tellerObject[i].sMin = startTime->tm_min;
         tellerObject[i].sSec = startTime->tm_sec;
         tellerObject[i].tellerNames = i+1;
         tellerObject[i].mutex = mutex;
         tellerObject[i].ti = ti;
         tellerObject[i].tw = tw;
         tellerObject[i].td = td;
         tellerObject[i].queue = queue;
         pthread_create(&teller[i],NULL,Bank,&tellerObject[i]);
      }

      pthread_create(&customerThread, NULL, customer,(void*)infoCustThread);

      for(int i = 0; i<NUM_TELLER; i++)
      {
         pthread_join(teller[i],NULL);
         //when the thread has joined then we can call time again
         time(&timeEpoch);
         localtime_r(&timeEpoch,finishTime);
         tellerObject[i].fHr = finishTime->tm_hour;
         tellerObject[i].fMin = finishTime->tm_min;
         tellerObject[i].fSec = finishTime->tm_sec;

         //write it into the file
         fprintf(r_log,"--------------------------\n");
         fprintf(r_log,"Termination: teller-%d\n",tellerObject[i].tellerNames);
         fprintf(r_log,"#served customers: %d\n",tellerObject[i].nCustomer);
         fprintf(r_log,"Start time: %02d:%02d:%02d\n",tellerObject[i].sHr,tellerObject[i].sMin,tellerObject[i].sSec);
         fprintf(r_log,"Termination time: %02d:%02d:%02d\n",tellerObject[i].fHr,tellerObject[i].fMin,tellerObject[i].fSec);
         fprintf(r_log,"--------------------------\n");

      }
      printf("Teller Statistic\n");
      printf("Teller-1 serves %d customers\n",tellerObject[0].nCustomer);
      printf("Teller-2 serves %d customers\n",tellerObject[1].nCustomer);
      printf("Teller-3 serves %d customers\n",tellerObject[2].nCustomer);
      printf("Teller-4 serves %d customers\n",tellerObject[3].nCustomer);
      int total = tellerObject[0].nCustomer + tellerObject[1].nCustomer+tellerObject[2].nCustomer+tellerObject[3].nCustomer;
      printf("Total Customers Served: %d\n",total);


      pthread_join(customerThread,NULL);

      free(data);
   }
   else
   {
      printf("Insufficient Arguments\n");
   }

   //destroy the mutex
   pthread_mutex_destroy(&mutex);
   pthread_mutex_destroy(&fileMutex);

   //free the remaining memory allocated
   free(queue);
   free(tellerObject);
   free(infoCustThread);
   //free the time keeping struct
   free(startTime);
   free(finishTime);
   return 0;
}

void *customer(void* arg)
{
   informationCustomerThread *infoCustThread = (informationCustomerThread*)arg;
   int periodicTime = (int)infoCustThread->periodicTime;
   int queueSpace = (int)infoCustThread->space;
   customerQueue *Queue = infoCustThread->queue;

   time_t rawtime;
   struct tm *accurateTime = (struct tm*)malloc(sizeof(struct tm));
   char mode;
   int position;

   FILE *inputFile = fopen("c_file","r");
   if (inputFile != NULL)
   {
      while(!feof(inputFile))
      {
         if(fscanf(inputFile,"%d %c", &position, &mode) == 2)
         {
            if(validateInput(mode)==1)
            {
               pthread_mutex_lock(&Queue->mutex);
               //check if the full
               while(Queue->size==queueSpace)
               {
                  //wait until a customer is served
                  pthread_cond_wait(&conditionArrayFULL,&Queue->mutex);
               }

               pthread_mutex_lock(&fileMutex);

               time(&rawtime);
               localtime_r(&rawtime,accurateTime);

               fprintf(r_log,"--------------------------\n");
               fprintf(r_log, "Customer: %d -> %c\n",position,mode);
               fprintf(r_log, "Arrival Time: %02d:%02d:%02d\n",accurateTime->tm_hour, accurateTime->tm_min, accurateTime->tm_sec);
               fprintf(r_log,"--------------------------\n");
               fprintf(r_log,"\n");
               pthread_mutex_unlock(&fileMutex);
               (Queue->data[Queue->size]).customerNumber = position;
               (Queue->data[Queue->size]).serviceType = mode;
               (Queue->data[Queue->size]).aHr= accurateTime->tm_hour;
               (Queue->data[Queue->size]).aMin= accurateTime->tm_min;
               (Queue->data[Queue->size]).aSec= accurateTime->tm_sec;
               Queue->size++;

               pthread_cond_signal(&conditionEmpty);
               pthread_mutex_unlock(&Queue->mutex);
            }
            else
            {
               printf("Invalid Transaction\n");
            }
         }
         else
         {
            printf("Invalid Input\n");
         }
      }
      sleep(periodicTime);
   }
   else
   {
      printf("Cannot Find Any File. Exiting\n");
   }

   return NULL;
}


void *Bank(void *arg)
{
   time_t rawtime;
   static struct tm *accurateTime,*accurateFinish;
   accurateTime = (struct tm*)malloc(sizeof(struct tm));
   accurateFinish = (struct tm*)malloc(sizeof(struct tm));
   tellerStats *teller = (tellerStats*)arg;
   customerQueue *queue = teller->queue;
   int numberofCustomerServed = 0;
   //pthread_t ID = pthread_self();

   //the prefix "a" will represent arrival time;
   int aHr,aMin,aSec;
   //the prefix "r" will represent response time
   int rHr,rMin,rSec;
   //the prefix "c" will represent completion time
   int cHr,cMin,cSec;

   customerNode customer;
   pthread_mutex_lock(&teller->mutex);
   while(queue->size == 0)
   {
      pthread_cond_wait(&conditionEmpty,&teller->mutex);
   }
   pthread_mutex_unlock(&teller->mutex);
   while(queue->size > 0)
   {

      pthread_mutex_lock(&teller->mutex);
      customer = queue->data[0];

      aHr = customer.aHr;
      aMin = customer.aMin;
      aSec = customer.aSec;

      //budget-style dequeue
      for(int i=0; i<queue->size-1;i++)
      {
         queue->data[i] = queue->data[i+1];
      }
      queue->size--;
      pthread_cond_signal(&conditionArrayFULL);
      pthread_mutex_unlock(&teller->mutex);


      //lock the file
      pthread_mutex_lock(&fileMutex);
      time(&rawtime);
      localtime_r(&rawtime,accurateTime);
      rHr = accurateTime->tm_hour;
      rMin = accurateTime->tm_min;
      rSec = accurateTime->tm_sec;

      //writing into the file
      fprintf(r_log,"--------------------------\n");
      fprintf(r_log,"Teller: %d\n",teller->tellerNames);
      fprintf(r_log,"Customer: %d\n",customer.customerNumber);
      fprintf(r_log,"Arrival Time: %02d:%02d:%02d\n",aHr,aMin,aSec);
      fprintf(r_log,"Response Time: %02d:%02d:%02d\n",rHr,rMin,rSec);
      fprintf(r_log,"--------------------------\n");
      fprintf(r_log,"\n");

      pthread_mutex_unlock(&fileMutex);

      //serving simulation
      switch(tolower(customer.serviceType))
      {
         case 'd':
            sleep(teller->td);
            break;
         case 'i':
            sleep(teller->ti);
            break;
         case 'w':
            sleep(teller->tw);
            break;
         default:
            sleep(1);
            break;
      }

      //after simulation, the thread will once again write it on the file
      pthread_mutex_lock(&fileMutex);
      time(&rawtime);
      localtime_r(&rawtime,accurateFinish);

      cHr =  accurateFinish->tm_hour;
      cMin = accurateFinish->tm_min;
      cSec = accurateFinish->tm_sec;

      fprintf(r_log,"--------------------------\n");
      fprintf(r_log,"Teller: %d\n",teller->tellerNames);
      fprintf(r_log,"Customer: %d\n",customer.customerNumber);
      fprintf(r_log,"Arrival Time: %02d:%02d:%02d\n",aHr,aMin,aSec);
      fprintf(r_log,"Response Time: %02d:%02d:%02d\n",rHr,rMin,rSec);
      fprintf(r_log,"Completion Time: %02d:%02d:%02d\n",cHr,cMin,cSec);
      fprintf(r_log,"--------------------------\n");
      fprintf(r_log,"\n");
      pthread_mutex_unlock(&fileMutex);
      numberofCustomerServed++;

   }
   teller->nCustomer=numberofCustomerServed;


   return NULL;
}

int validateInput(char mode)
{
   int result;
   mode = tolower(mode);
   if(mode == 'w' || mode == 'd' || mode == 'i')
   {
      result = 1;
   }
   else
   {
      result = 0;
   }

   return result;
}


