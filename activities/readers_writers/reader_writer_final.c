/* Adapted from https://gist.github.com/rajabishek/6209a575f00b122fe490 */
#include<stdio.h>
#include<pthread.h>
#include<semaphore.h>
#include<unistd.h>

#define NUM_READERS 50
#define NUM_WRITERS 2

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t time_to_read = PTHREAD_COND_INITIALIZER;
pthread_cond_t time_to_write = PTHREAD_COND_INITIALIZER;

int data = 0;
int writer_active = 0;
int writers_waiting = 0;
int readers_active = 0;

void *reader(void *arg)
{
  int f;
  f = *((int*)arg);
  while(1){
    pthread_mutex_lock(&lock);
    while(writer_active || writers_waiting){
      pthread_cond_wait(&time_to_read, &lock);
    }
    readers_active++;
    pthread_mutex_unlock(&lock);
    //critical section
    printf("Data read by the reader %d is %d\n",f,data);
    sleep(1);
    //critical section
    pthread_mutex_lock(&lock);
    readers_active--;
    if(readers_active==0){
        pthread_cond_signal(&time_to_write);
    }
    pthread_mutex_unlock(&lock);
    // Please keep this here to inject some delay in the readers' arrival to
    // simulate readers processing the data.
    usleep(200);
  }
}

void *writer(void *arg)
{
  int f;
  f = *((int*) arg);
  while(1){
    pthread_mutex_lock(&lock);
    writers_waiting++;
    while(readers_active || writer_active){
      pthread_cond_wait(&time_to_write, &lock);
    }
    writers_waiting--;
    writer_active=1;
    pthread_mutex_unlock(&lock);
    //critical section
    printf("Writer %d starts to write..\n",f);
    data++;
    sleep(1);
    printf("Writer %d finishes to write %d\n",f,data);
    //critical section
    pthread_mutex_lock(&lock);
    writer_active=0;
    if(writers_waiting){
      pthread_cond_signal(&time_to_write);
    }else{
      pthread_cond_broadcast(&time_to_read);
    }
    pthread_mutex_unlock(&lock);
    // Please keep this here to slow down our writers and satisify our
    // assumptions from class
    sleep(1);
  }
}

int main()
{
  int i,b;
  pthread_t rtid[NUM_READERS],wtid[NUM_WRITERS];
  //create writer
  for(i=0;i<NUM_WRITERS;i++)
    pthread_create(&wtid[i],NULL,writer,(void *)&i);
  //create readers
  for(i=0;i<NUM_READERS;i++)
    pthread_create(&rtid[i],NULL,reader,(void *)&i);

  // this will never joing since our threads run forever.
  // Hit C-c to exit the program.
  pthread_join(wtid[0],NULL);

  return 0;
}
