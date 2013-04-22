//author David Kanda (dkanda)
//program to perform row major data access on nxn array
// and put result into vector of n items
//date modified 2/19/2012

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

int main(){
	struct timeval tempo1, tempo2;
	unsigned long elapsed_useconds; /* diff between microseconds counter */
	float** array; //the matrix
	float* vector; //the vector
	int n, i,j;
	n = 10000; //determines the size of the array and vector

	array = (float **) calloc(n, sizeof(float *)); //create one row of the matrix
	for (i = 0; i < n; i++)  //attach the other rows to this row
	{
		array[i] = (float *) calloc(n, sizeof(float));
	}

	vector = (float *) calloc(n, sizeof(float )); //create a 1 dimension array for vector

	gettimeofday(&tempo1, NULL); //start the clock
	//perform row major data access
	for (i = 0; i < n; i++) {
	    for (j = 0; j < n; j++) {
	     	vector[i] = array[i][j] + vector[i];
	    }
	}

	gettimeofday(&tempo2, NULL);
	free(array); //frees array from memory
	elapsed_useconds = 1000000*(tempo2.tv_sec - tempo1.tv_sec) + (tempo2.tv_usec - tempo1.tv_usec);
	printf("Elapsed time = %ld microseconds\n", elapsed_useconds);
	return 0;
}
