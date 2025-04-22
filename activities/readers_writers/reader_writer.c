/* Adapted from https://gist.github.com/rajabishek/6209a575f00b122fe490 */
#include<stdio.h>
#include<pthread.h>
#include<semaphore.h>
#include<unistd.h>

#define NUM_READERS 4
#define NUM_WRITERS 2

int data = 0;

void *reader(void *arg)
{
  int f;
  f = *((int*)arg);
  while(1){

    //critical section
    printf("Data read by the reader %d is %d\n",f,data);
    //critical section

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

    //critical section
    printf("Writer %d starts to write..\n",f);
    data++;
    sleep(1);
    printf("Writer %d finishes to write %d\n",f,data);
    //critical section

    // Please keep this here to slow down our writers and satisify our
    // assumptions from class
    sleep(1);
  }
}

int main()
{
  int i,b;
  pthread_t rtid[5],wtid[5];
  int rids[i];
  int wids[i];
  //create writer
  for(i=0;i<NUM_WRITERS;i++) {
    rids[i] = i;
    pthread_create(&wtid[i],NULL,writer,(void *)&rids[i]);
  }
  //create readers
  for(i=0;i<NUM_READERS;i++) {
    wids[i] = i;
    pthread_create(&rtid[i],NULL,reader,(void *)&wids[i]);
  }

  // this will never joing since our threads run forever.
  // Hit C-c to exit the program.
  pthread_join(wtid[0],NULL);

  return 0;
}
