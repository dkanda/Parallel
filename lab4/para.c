

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "seq.h"

const int RMAX = 100;

int n, npes;
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
void DoOddEvenPassing(int localArray[], int temp_B[], int temp_C[], 
   int phase, int even_partner, int odd_partner, int my_rank);
void PrintArray();
int main(int argc, char* argv[]) { 

   int t, i, rc;
   double start, finish;
   n = atoi(argv[1]);
   npes = atoi(argv[2]);
   chunksize = n/npes;

   pthread_t threads[npes];
   struct timeval tempo1, tempo2; //struct to get execution time
   unsigned long elapsed_useconds; /* diff between counters in microseconds */
   pthread_mutex_init(&minimum_value_lock, NULL);
   pthread_barrier_init(&bar, NULL, npes);

   CreateRandomArray();
   PrintArray();

   evenRanks = (int*) malloc(npes*sizeof(int));
   oddRanks = (int*) malloc(npes*sizeof(int));
   for(i = 0; i < npes; i++)
   {
      if(i%2 ==0)
      {
         oddRanks[i] = i - 1;
         evenRanks[i] = i + 1;
         if(i+1 ==npes) evenRanks[i] = -1;
      }
      else
      {
         oddRanks[i] = i+1;
         evenRanks[i] = i-1;
         if(i+1 == npes) oddRanks[i] = -1;
      }
   }

   // start = MPI_Wtime();
   // Sort(localArray, chunksize, my_rank, npes, comm);
   // finish = MPI_Wtime();
   // printf("Elapsed time = %e seconds\n", finish-start);

   gettimeofday(&tempo1, NULL); //start the clock 

   for(t=0; t<npes; t++)
   {

      rc = pthread_create(&threads[t], NULL, Sort, (void *)t);
      if (rc)
      {
         printf("ERROR: return error from pthread_create() is %d\n", rc);
         exit(-1);
      }

   }
   

   for(t=0; t<npes; t++)
   {
      rc = pthread_join(threads[t], NULL);
      if (rc)
      {
         printf("ERROR: return error from pthread_join() is %d\n", rc);
         exit(-1);
      }

   }
   gettimeofday(&tempo2, NULL); //stop the clock
   PrintArray();
   elapsed_useconds = 1000000*(tempo2.tv_sec - tempo1.tv_sec) + (tempo2.tv_usec - tempo1.tv_usec); 
   printf("Elapsed time = %ld microseconds\n", elapsed_useconds);//prints out the difference in times

   return 0;
}  

void PrintArray()
{
   int i;
   for(i = 0; i < n; i++)
      printf("%d ", masterArray[i]);
   printf("\n");
}
void CreateRandomArray() 
{
   int i;
   
	masterArray = (int*) malloc(n*sizeof(int));
    	srand (time(NULL));
       for (i = 0; i < n; i++)
       	masterArray[i] = rand() % RMAX + 1;

}  


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


void *Sort(void *threadid) {
   int phase, i;
   int *tempArrayB, *tempArrayC;  
   int *localArray;
   int threadRank;
   threadRank = threadid;

   tempArrayB = (int*) malloc(chunksize*sizeof(int));
   tempArrayC = (int*) malloc(chunksize*sizeof(int));

   localArray = (int*) malloc(chunksize*sizeof(int));
   //cope out what we need
   memcpy(localArray, masterArray + (threadRank*chunksize), chunksize*sizeof(int));

   //select qsort or Bubblesort here
   //qsort(localArray, chunksize, sizeof(int), Compare);
   BubbleSort(localArray, 0, chunksize);
   //put back sorted part
   memcpy(masterArray + (threadRank*chunksize), localArray, chunksize*sizeof(int));

   for (phase = 0; phase < npes; phase++)
   {
      DoOddEvenPassing(localArray, tempArrayB, tempArrayC, phase, 
             evenRanks[threadRank], oddRanks[threadRank], threadRank);

   }
   //memcpy(masterArray + (threadRank*chunksize), localArray, chunksize*sizeof(int));
   //pthread_barrier_wait(&bar);
   //printf("je suis %d\n", threadRank);
   for(i = 0; i < chunksize; i++)
      printf("%d ", localArray[i]);
   printf("\n");
   free(tempArrayB);
   free(tempArrayC);
}  


void DoOddEvenPassing(int localArray[], int temp_B[], int temp_C[], 
   int phase, int even_partner, int odd_partner, int my_rank) {
   printf("my rank &d\n", my_rank);
   //pthread_barrier_wait(&bar);

   if (phase % 2 == 0) //even
   {
      if (even_partner >= 0) {
         memcpy(temp_B, masterArray + (even_partner*chunksize), chunksize*sizeof(int));
         //MPI_Sendrecv(localArray, chunksize, MPI_INT, even_partner, 0, temp_B, chunksize, MPI_INT, even_partner, 0, comm, &status);
         if (my_rank % 2 != 0)
         {
            printf("upper\n");
            SplitUpper(localArray, temp_B, temp_C);
         }
         else
         {
            SplitLower(localArray, temp_B, temp_C);
            printf("lower\n");
         }

      }
   } 
   else //odd
   { 
      
      if (odd_partner >= 0) {
         memcpy(temp_B, masterArray + (odd_partner*chunksize), chunksize*sizeof(int));
         //MPI_Sendrecv(localArray, chunksize, MPI_INT, odd_partner, 0, temp_B, chunksize, MPI_INT, odd_partner, 0, comm, &status);
         if (my_rank % 2 != 0)
            SplitLower(localArray, temp_B, temp_C);
         else
            SplitUpper(localArray, temp_B, temp_C);
      }
   }
}  



void SplitLower(int localArray[], int temp_B[], int temp_C[]) {
   int a, b, c;
   
   a = 0;
   b = 0;
   c = 0;
   while (c < chunksize) {
      if (localArray[a] <= temp_B[b]) {
         temp_C[c] = localArray[a];
         c++; a++;
      } else {
         temp_C[c] = temp_B[b];
         c++; b++;
      }
   }
   for(i = 0; i < chunksize; i++)
      printf("%d ", localArray[i]);
   printf("\n");
   memcpy(localArray, temp_C, chunksize*sizeof(int));
} 


void SplitUpper(int localArray[], int temp_B[], int temp_C[]) {
   int a, b, c;
   
   a = chunksize-1;
   b = chunksize-1;
   c = chunksize-1;
   while (c >= 0) {
      if (localArray[a] >= temp_B[b]) {
         temp_C[c] = localArray[a];
         c--; a--;
      } else {
         temp_C[c] = temp_B[b];
         c--; b--;
      }
   }

   memcpy(localArray, temp_C, chunksize*sizeof(int));
}  

