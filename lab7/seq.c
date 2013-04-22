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
//#include <mpi.h> 
#include <stdlib.h>
#include "seq.h"

// void BubbleSort(int array[], int n);
// void Swap(int array[], int left, int right);
// void Quicksort(int array[], int left, int right);
// int Partition(int array[], int left, int right, int pivot_index);
// void display(int array[], int length);


void display(int array[], int length)
{
	int i;
	printf(">");
	for (i = 0; i < length; i++)
		printf(" %d", array[i]);
	printf("\n");
}

void BubbleSort(int array[], int start, int end)
{
	int i, j,temp;
	for(i = start; i <= end - 2; i++)
	{
		for(j = start; j <= end - 2 - i; j++)
		{
			if(array[j] > array[j+1])
				Swap(array, j, j+1);
		}
	}
}

void Swap(int array[], int left, int right)
{ 
	int temp;
	temp = array[left];
	array[left] = array[right];
	array[right] = temp;
}

