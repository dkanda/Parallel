//author David Kanda (dkanda)
//program to perform column major data access on nxn array
// and put result in vector of n items
//date modified 2/19/2012

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

int main(){
	struct timeval tempo1, tempo2; //struct to get execution time
	unsigned long elapsed_useconds; /* diff between counters in microseconds */
	float** array; //the matrix
	float* vector; //the vector
	int n, i,j; //variables
	n = 10000; //determines the size of the array and vector
	 
	array = (float **) calloc(n, sizeof(float *)); //create one row of the matrix
	for (i = 0; i < n; i++)  //make this a 2 dimensional array
	{
		array[i] = (float *) calloc(n, sizeof(float));
	}

	vector = (float *) calloc(n, sizeof(float )); //create a 1 dimension array of length n for vector

	gettimeofday(&tempo1, NULL); //start the clock
	//perform column major data access
	for (i = 0; i < n; i++) {
		vector[i] = 0;
	    for (j = 0; j < n; j++) {
	    	vector[i] = array[j][i] + vector[i];
	    }
	}
	gettimeofday(&tempo2, NULL); //stop the clock
	free(array); //frees array from memory
	elapsed_useconds = 1000000*(tempo2.tv_sec - tempo1.tv_sec) + (tempo2.tv_usec - tempo1.tv_usec); 
	//gets the difference between the two time
	printf("Elapsed time = %ld microseconds\n", elapsed_useconds);//prints out the difference in times

	return 0;
}
