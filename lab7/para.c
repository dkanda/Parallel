//@author David Kanda (dkanda)
//@date 4/8/2013
//@description Program to sort array of integers using threads and odd-even sorting
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "seq.h"

const int RMAX = 5; //random numbers range from 1 to this number

int n, npes; //n is length of array, npes is number of threads to use
int chunksize, my_rank,npes;
int *masterArray;

int *evenRanks;
int *oddRanks;
pthread_mutex_t minimum_value_lock;
pthread_barrier_t bar;


void SplitLower(int localArray[], int temp_B[], int temp_C[]);
void SplitUpper(int localArray[], int temp_B[], int temp_C[]);
void CreateRandomArray();
int  Compare(const void* a_p, const void* b_p);
void *Sort(void *threadid);
void PrintArray();
int main(int argc, char* argv[]) { 

  int t, i, rc;
  double start, finish;
  n = atoi(argv[1]); //get array length argument from command line
  npes = atoi(argv[2]); //get number of threads argument from command line
  chunksize = n/npes; //calculate how big each chunk a thread should get

  pthread_t threads[npes];
  struct timeval tempo1, tempo2; //struct to get execution time
  unsigned long elapsed_useconds; /* diff between counters in microseconds */
  pthread_mutex_init(&minimum_value_lock, NULL);
  pthread_barrier_init(&bar, NULL, npes);

  CreateRandomArray();//create an array of length n

  evenRanks = (int*) malloc(npes*sizeof(int));
  oddRanks = (int*) malloc(npes*sizeof(int));

  gettimeofday(&tempo1, NULL); //start the clock 

  //if more than one thread, create the threads
  if(npes != 1)
  {
    for(t=0; t<npes; t++)
    {
      rc = pthread_create(&threads[t], NULL, Sort, (void *)t);
      if (rc)
      {
        printf("ERROR: return error from pthread_create() is %d\n", rc);
        exit(-1);
      }
    }

    //kill the threads
    for(t=0; t<npes; t++)
    {
      rc = pthread_join(threads[t], NULL);
      if (rc)
      {
         printf("ERROR: return error from pthread_join() is %d\n", rc);
         exit(-1);
      }
    }
  }
  else//just one thread
  {
      qsort(masterArray, n, sizeof(int), Compare);
  }
  gettimeofday(&tempo2, NULL); //stop the clock
  //if n less than 40 elements, print er out
  if(n <=40)
    PrintArray();
  elapsed_useconds = 1000000*(tempo2.tv_sec - tempo1.tv_sec) + (tempo2.tv_usec - tempo1.tv_usec); 
  printf("Elapsed time = %ld microseconds\n", elapsed_useconds);//prints out the difference in times
  free(masterArray);

  return 0;
}  

//prints all the elements located in masterArray
void PrintArray()
{
   int i;
   for(i = 0; i < n; i++)
      printf("%d ", masterArray[i]);
   printf("\n");
}

//creates an array of length n of random numbers
void CreateRandomArray() 
{
  int i;
  masterArray = (int*) malloc(n*sizeof(int));
  srand (time(NULL));
  for (i = 0; i < n; i++)
    masterArray[i] = rand() % RMAX + 1;

}  

//function to compare size of integers
int Compare(const void* a_p, const void* b_p) {
   int a = *((int*)a_p);
   int b = *((int*)b_p);

   if (a < b)
      return -1;
   else if (a == b)
      return 0;
   else /* a > b */
      return 1;
}  /* Compare */


//threaded sort function by using odd-even sorting
void *Sort(void *threadid) {
  int phase, i, j;
  int *tempArrayB, *tempArrayC;  
  int *localArray;
  int threadRank;
  threadRank = threadid;

  tempArrayB = (int*) malloc(chunksize*sizeof(int));
  tempArrayC = (int*) malloc(chunksize*sizeof(int));

  localArray = (int*) malloc(chunksize*2*sizeof(int));
  //copy out what we need
  pthread_mutex_lock(&minimum_value_lock);
  for(i = 0; i < chunksize; i++)
    localArray[i] = masterArray[(threadRank*chunksize) + i];

  //select qsort or Bubblesort here
  qsort(localArray, chunksize, sizeof(int), Compare);
  //BubbleSort(localArray, 0, chunksize);

  for(i = 0; i < chunksize; i++)
    masterArray[(threadRank*chunksize) + i] = localArray[i];
  
  pthread_mutex_unlock(&minimum_value_lock);
  pthread_barrier_wait(&bar);

  for(j = 0; j < npes; j++)
  {
    if(j%2 != 0 && threadRank != npes-1 && threadRank != 0) //odd, separate first and last nodes
    {
      pthread_mutex_lock(&minimum_value_lock); //enter critical section
      if(threadRank % 2 ==0) //rank is even
      {
        for(i = 0; i < chunksize*2; i++)
          localArray[i] = masterArray[((threadRank - 1)*chunksize) + i];

        SortAndKeepUpper(localArray);
        
        for(i = 0; i < chunksize; i++)
          masterArray[(threadRank*chunksize) + i] = localArray[i];
      }

      else//thread rank is odd
      {
        for(i = 0; i < chunksize*2; i++)
          localArray[i] = masterArray[(threadRank*chunksize) + i];

        SortAndKeepLower(localArray);
        
        for(i = 0; i < chunksize; i++)
          masterArray[(threadRank*chunksize) + i] = localArray[i];

      }

      pthread_mutex_unlock(&minimum_value_lock); //leave critical section
    }
    else //even
    {
      pthread_mutex_lock(&minimum_value_lock); //enter critical section

      if(threadRank % 2 ==0) //rank is even
      {
        for(i = 0; i < chunksize*2; i++)
          localArray[i] = masterArray[(threadRank*chunksize) + i];

        SortAndKeepLower(localArray);
        
        for(i = 0; i < chunksize; i++)
          masterArray[(threadRank*chunksize) + i] = localArray[i];
      }
      else//thread rank is odd
      {
        for(i = 0; i < chunksize*2; i++)
          localArray[i] = masterArray[((threadRank-1)*chunksize) + i];

        SortAndKeepUpper(localArray);
        
        for(i = 0; i < chunksize; i++)
          masterArray[(threadRank*chunksize) + i] = localArray[i];

      }

      pthread_mutex_unlock(&minimum_value_lock); //leave critical section
    }
    pthread_barrier_wait(&bar);
  }

  free(tempArrayB);
  free(tempArrayC);
}  

//given a list of 2*n elements, start at the smallest index and
//    find the smallest elements in the array until n elements 
//    are found 
SortAndKeepLower(int localArray[])
{
  int *tempA; 
  int a,b,c, i;
  tempA= (int*) malloc(chunksize*sizeof(int));
  a = 0;
  b = 0;
  c = 0;
  while (c < chunksize) {
    if (localArray[a] <= localArray[b+chunksize]) {
      tempA[c] = localArray[a];
      c++; a++;
    } else {
      tempA[c] = localArray[b+chunksize];
      c++; b++;
    }
  }
  for(i = 0; i < chunksize; i++)
    localArray[i] = tempA[i];

}

//given a list of 2*n elements, start at the largest index and
//    find the largest  elements in the array until n elements 
//    are found 
SortAndKeepUpper(int localArray[])
{
  int a, b, c, i;
  int *tempA; 
  a = 0;
  b = 0;
  c = 0;
  tempA= (int*) malloc(chunksize*sizeof(int));

  a = (chunksize * 2) - 1;
  b = chunksize-1;
  c = chunksize-1;
  while (c >= 0) {
    if (localArray[a] >= localArray[b]) {
      tempA[c] = localArray[a];
      c--; a--;
    } else {
      tempA[c] = localArray[b];
      c--; b--;
    }
  }
  for(i = 0; i < chunksize; i++)
    localArray[i] = tempA[i];
}
