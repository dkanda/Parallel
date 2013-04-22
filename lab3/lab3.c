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
#define  n	256

int main(int argc, char *argv[]) 
{ 
    double starttime, endtime;
    int rank, size, i, j, chunksize, k; 
    pid_t mypid;
    MPI_Init(&argc, &argv); 
    MPI_Comm_rank(MPI_COMM_WORLD, &rank); 
    MPI_Comm_size(MPI_COMM_WORLD, &size); 
    mypid = getpid();

	chunksize = n/(size-1);

    float matA[n*n] ={0.0};
    float matB[n*n] ={0.0};
    float matC[n*n] ={0.0};

    //if this is the master node
    if(rank == 0)
    {
		int i,j; //variables
			
        //populate matrix with 2s in first and 1s in second    
        for(i = 0; i < n*n; i++)
        {
            matB[i] = 2.0;
            matA[i] = 1.0;
        }

        //start the clock
        starttime = MPI_Wtime(); 

        //send pieces to each node excep the last node
		for(i = 0; i< size-2; i++)
		{           
			MPI_Send(&(matA[i*chunksize*n]), chunksize*n, MPI_FLOAT, i+1, 1, MPI_COMM_WORLD);
			MPI_Send(&(matB), n*n, MPI_FLOAT, i+1, 2, MPI_COMM_WORLD);
		}
        //send piece to last node along with the remainder
        MPI_Send(&(matA[(size-2)*chunksize*n]), (n*n) - ((size-2)*chunksize*n), MPI_FLOAT, size-1, 1, MPI_COMM_WORLD);
        MPI_Send(&(matB), n*n, MPI_FLOAT, size-1, 2, MPI_COMM_WORLD);

        //receive from all the nodes except the last one
	    for(i = 0; i< size-2; i++)
        {
            MPI_Recv(&(matC[i*chunksize*n]), n*chunksize, MPI_FLOAT, i + 1, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
        //receive from the last node
        MPI_Recv(&(matC[(size-2)*chunksize*n]), (n*n) - ((size-2)*chunksize*n), MPI_FLOAT, size-1, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        //stop the clock
        endtime   = MPI_Wtime(); 
        printf("with %d cores\n", size);
        printf("That took %f seconds\n",endtime-starttime); 
        //print the resultant matrix
         printf("Matrix C\n");
        for (i = 0; i < n*n; i++) {
            if(i%n ==0)
                printf("\n");
            printf("%4.2f ", matC[i]);
         }
         printf("\n");
    }

    else if(rank != 0 && rank != size-1) //slave nodes
    {
    	//receive pieces
    	MPI_Recv(&(matA[(rank-1)*chunksize*n]), n*chunksize, MPI_FLOAT, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    	MPI_Recv(&(matB), n*n, MPI_FLOAT, 0, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        //perform matrix multiplication
    	for(i=(rank-1)*chunksize; i<((rank-1)*chunksize)+chunksize; i++)
    	{
            for(j=0; j<n; j++)
            {
                for(k=0; k<n; k++)
	    		{	    		
	    			matC[i*n+j] += matA[(i*n) + k]*matB[(k*n) + j];
	    		}
            }
    		
    	}
        //return chunk
        MPI_Send(&(matC[(rank-1)*chunksize*n]), chunksize*n, MPI_FLOAT, 0, 1, MPI_COMM_WORLD);

    }
    else//the last node in the network
    {
        //receive pieces plus remainder
        MPI_Recv(&(matA[(size-2)*chunksize*n]), (n*n) - ((size-2)*chunksize*n), MPI_FLOAT, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(&(matB), n*n, MPI_FLOAT, 0, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        
        //perform matrix multiplication
        for(i=(size-2)*chunksize; i<n; i++)
        {
            for(j=0; j<n; j++)
            {
                for(k=0; k<n; k++)
                {               
                    matC[i*n+j] += matA[(i*n) + k]*matB[(k*n) + j];
                }
            }
            
        }
        //return chunk 
        MPI_Send(&(matC[(size-2)*chunksize*n]), (n*n) - ((size-2)*chunksize*n), MPI_FLOAT, 0, 1, MPI_COMM_WORLD);
    }
    MPI_Finalize(); 
    return 0; 
} 
