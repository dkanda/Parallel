//@ author David Kanda (dkanda)
//@ date 3/1/2013
//@ description MPI program to take two matrices of size n*n and multiply
//      them together and put the result in a third matrix
//      The first matrix is divided into chunks, then sent to the different 
//      nodes, the final node takes a chuck as well as whatever is left over 
//      all nodes are sent the second matrix
//      once a node is done calculating, the chucks are sent back to the master node
//      to be recompiled and printed
#include <sys/types.h>
#include <stdio.h> 
#include <mpi.h> 
#include <stdlib.h>
#include <time.h>
#include "seq.h"


int main(int argc, char *argv[])
{
	double starttime, endtime;
	int* inputArray; //the matrix
	int* outputArray;
	int* finalArray;

	//printf("%s\n", argv[1]);

	int rank, size, i, j, chunksize, k, n, isEven; 
	pid_t mypid;
	MPI_Init(&argc, &argv); 
	MPI_Comm_rank(MPI_COMM_WORLD, &rank); 
	MPI_Comm_size(MPI_COMM_WORLD, &size); 
	n = atoi(argv[1]); //determines the size of the inputArray and vector
	inputArray = (int *) calloc(n, sizeof(int )); //create a 1 dimension inputArray of length n for vector
	outputArray = (int *) calloc(n, sizeof(int )); //create a 1 dimension inputArray of length n for vector
	finalArray = (int *) calloc(n, sizeof(int )); //create a 1 dimension inputArray of length n for vector    
	
	chunksize = n/size;
	printf("n is %d and chunksize is %d\n",n , chunksize);
	if(!rank)
	{
		
		srand (time(NULL));
		//fill inputArray with random ints
		for(i = 0; i<n; i++)
		{
			 inputArray[i] = rand() % 100 + 1;
			 if(n <= 40)
			 	printf("%d ", inputArray[i]);
		}
		printf("\n");
		starttime = MPI_Wtime(); 
	}

	

	for(k = 0; k < n; k++)
	{
		printf("k is now %d\n", k);
			
		if(k%2==0)
		{
			MPI_Scatter(inputArray, chunksize, MPI_INT, outputArray, chunksize, MPI_INT, 0, MPI_COMM_WORLD);
			//Quicksort(outputArray, 0, chunksize - 1);
			BubbleSort(outputArray, 0, chunksize);
			MPI_Gather(outputArray, chunksize, MPI_INT, finalArray, chunksize, MPI_INT, 0, MPI_COMM_WORLD);
		}
		else
		{
			MPI_Scatter(&(inputArray[1]), chunksize, MPI_INT, outputArray, chunksize, MPI_INT, 0, MPI_COMM_WORLD);
			if(rank != size - 1)
				//Quicksort(outputArray, 0, chunksize - 1);
				BubbleSort(outputArray, 0, chunksize);
			MPI_Gather(outputArray, chunksize, MPI_INT, &(finalArray[1]), chunksize, MPI_INT, 0, MPI_COMM_WORLD);
			printf("%d is done here \n", rank);
		   if(!rank)
		   	{
		   		finalArray[0] = inputArray[0];
		   		finalArray[n-1] = inputArray[n-1];
		   		inputArray = finalArray;
		   	}
		}
	}

	if(!rank)
    {
    	endtime = MPI_Wtime(); 
    	printf("That took %f seconds\n",endtime-starttime);
    }
	 
    free(inputArray);
    free(outputArray);
    free(finalArray);

	if(!rank && n <= 40)
	{
			 for(i = 0; i < n; i++)
			 	printf("%d ", finalArray[i]);
			 printf("\n");
	}
	MPI_Finalize(); 
	return 0;
}
