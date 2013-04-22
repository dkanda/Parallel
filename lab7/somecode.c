//@author David Kanda (dkanda)
//@date 4/10/2013
//@description program to perform matrix-matrix multiplication (A x B = C). A,
//    B and C are nxn matrices. To execute, write "Lab6b n nthreads"

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

int NUM_THREADS;

int n;

int* matC;
int* matB;
int* matA;



void *Hazlo(void *threadid)
{
	int numRowsToDo, i, j, k, threadIndex, remainder;
	threadIndex = threadid;	

	numRowsToDo = n/NUM_THREADS;
       remainder= n%NUM_THREADS;
	for(i= threadIndex*numRowsToDo; i<(threadIndex*numRowsToDo)+numRowsToDo; i++)
       	for(j=0; j<n; j++)
			for(k=0; k<n; k++)	    
				matC[i*n+j] += matA[(i*n) + k]*matB[(k*n) + j];

	if(remainder && threadIndex == NUM_THREADS - 1)
	{
		for(i = n - remainder; i< n; i++)
        		for(j=0; j<n; j++)
              		for(k=0; k<n; k++)	    
					matC[i*n+j] += matA[(i*n) + k]*matB[(k*n) + j];
    			
  	}
	pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
	pthread_t threads[NUM_THREADS];
	n = atoi(argv[1]);
	NUM_THREADS = atoi(argv[2]);
	int rc, t, i;
	
	struct timeval tempo1, tempo2; //struct to get execution time
	unsigned long elapsed_useconds; /* diff between counters in microseconds */

	//allocate some memory for the matrices 
	matA= (int*) calloc(n*n, sizeof(int*));
	matB= (int*) calloc(n*n, sizeof(int*));
	matC= (int*) calloc(n*n, sizeof(int*));

	for(i = 0; i < n*n; i++)
	{
		matA[i] = i;
		matB[i] = i*2;
	}

	
	gettimeofday(&tempo1, NULL); //start the clock 

	for(t=0; t<NUM_THREADS; t++)
	{

		rc = pthread_create(&threads[t], NULL, Hazlo, (void *)t);
		if (rc)
		{
			printf("ERROR: return error from pthread_create() is %d\n", rc);
			exit(-1);
		}

	}
	

	for(t=0; t<NUM_THREADS; t++)
	{
		rc = pthread_join(threads[t], NULL);
		if (rc)
		{
			printf("ERROR: return error from pthread_join() is %d\n", rc);
			exit(-1);
		}

	}
	gettimeofday(&tempo2, NULL); //stop the clock
	free(matA); //frees array from memory
	free(matB); //frees array from memory
	elapsed_useconds = 1000000*(tempo2.tv_sec - tempo1.tv_sec) + (tempo2.tv_usec - tempo1.tv_usec); 
	printf("Elapsed time = %ld microseconds\n", elapsed_useconds);//prints out the difference in times

	 

	if(n <= 40)
	{
		for(i = 0; i < n*n; i++)
		{
			if(i%n == 0)
				printf("\n");
			printf("%d ", matC[i]);
		}	
		printf("\n");
	}

return 0;

}
