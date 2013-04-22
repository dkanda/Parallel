//@author David Kanda, Zach Elvis Costello, Nick Interrante
//@date 4/22/13
//@description Program to multiply matrix A and B and put result in
//  matrix C. Program utilizes nonblocking send and recieve on slave end
//  and threaded calculations.

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "mpi.h"

#define n 16 

void *Multiply(void *threadid);

int nthreads;
pthread_mutex_t minimum_value_lock;
pthread_barrier_t bar;
int* matC;
int* matB;
int* matA;
int* recv_value;
int* send_value;

int main( int argc, char *argv[] )
{
    int rank, size, t, rc;
    MPI_Status status;
    MPI_Request send_request;
    MPI_Request recv_request;
    double start, finish;

    nthreads = atoi(argv[1]);
    pthread_t threads[nthreads];
    pthread_mutex_init(&minimum_value_lock, NULL);
    pthread_barrier_init(&bar, NULL, nthreads);   

    MPI_Init( &argc, &argv );

    MPI_Comm_rank( MPI_COMM_WORLD, &rank );
    MPI_Comm_size( MPI_COMM_WORLD, &size );
   
    

    matA= (int*) calloc(n*n, sizeof(int*));
    matB= (int*) calloc(n*n, sizeof(int*));
    matC = (int *) calloc(n*n, sizeof(int*));
    recv_value = (int *) calloc(n, sizeof(int*));
    send_value = (int *) calloc(n, sizeof(int*));

    //is root node
    if(!rank)
    {
        int i, j;
        //populate arrays
        for(i = 0; i < n*n; i++)
        {
            matA[i] = i;
            matB[i] = i;
            matC[i] = 0;
        }

	start = MPI_Wtime();
        //send matrix B to all nodes
        MPI_Bcast(matB, n*n, MPI_INT, 0,  MPI_COMM_WORLD );

       
        //send inital piece to all nodes
        for(i = 0; i < size - 1; i++){
            for(j=0; j < n; j++)
                send_value[j] = matA[i*n + j];
            MPI_Send (send_value, n, MPI_INT, i + 1, i, MPI_COMM_WORLD);
    	}

        //keep sending to all nodes until it is all gone
        for(i = size - 1; i < n + (size - 1); i++ )
        {
            
            MPI_Recv (recv_value, n, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            for(j=0; j < n; j++)
                matC[(status.MPI_TAG)*n + j] = recv_value[j];
            //printf("the tag returning is %d\n", status.MPI_TAG);
            if(i < n)
            {
                for(j=0; j < n; j++)
                    send_value[j] = matA[i*n + j];
                MPI_Send (send_value, n, MPI_INT, status.MPI_SOURCE, i, MPI_COMM_WORLD);
            }
            
        }
        //tell them to stop
        for(i = 0; i < size - 1; i++)
            MPI_Send(&i, 0, MPI_INT, i+1, 32768, MPI_COMM_WORLD);

        finish = MPI_Wtime();
         printf("Elapsed time = %e seconds\n", finish-start);
        //print the array
        if(n <= 16)
	{
        for (i = 0; i < n*n; i++)
        {
            if(i%n ==0)
                printf("\n");
            printf("%d ", matC[i]);
        }
	}
    }

    //is slave node
    else if(rank)
    {
        int firstTime = 1;
        int j, k, i;
        //get matrix B
        MPI_Bcast(matB, n*n, MPI_INT, 0,  MPI_COMM_WORLD );
        //get the initial piece
        MPI_Irecv (recv_value, n, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &recv_request);
        //MPI_Wait (&recv_request, &status);

        while(1)
        {
            //clear send buffer
            for (i = 0; i < n; i++)
                send_value[i] = 0;
            //stop everything
            MPI_Wait (&recv_request, &status);
            if(status.MPI_TAG == 32768)
                break;
            

            if(nthreads > 1)
              {
                for(t=0; t<nthreads; t++)
                {
                  rc = pthread_create(&threads[t], NULL, Multiply, (void *)t);
                  if (rc)
                  {
                    printf("ERROR: return error from pthread_create() is %d\n", rc);
                    exit(-1);
                  }
                }

                //kill the threads
                for(t=0; t<nthreads; t++)
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
                //computation
                for(j=0; j < n; j++)
                    for(k=0; k < n; k++)       
                        send_value[j] += recv_value[k] * matB[(k*n) + j];
              }
        
            MPI_Isend (send_value, n, MPI_INT, 0, status.MPI_TAG, MPI_COMM_WORLD, &send_request);
            //MPI_Wait (&send_request, &status);
            MPI_Irecv (recv_value, n, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &recv_request);
            //MPI_Wait (&recv_request, &status);
        }   
    }
    MPI_Finalize ();
    return 0;
}


void *Multiply(void *threadid) {
  int l,m,amount;
  int threadRank;
  threadRank = threadid;
  amount = n/nthreads;
    
    for(l = (threadRank*amount); l < (threadRank*amount) + amount; l++)
    {
        for(m=0; m < n; m++)
        {     
            pthread_mutex_lock(&minimum_value_lock);  
            send_value[l] += recv_value[m] * matB[(m*n) + l];
            pthread_mutex_unlock(&minimum_value_lock);
        }
    
        //pthread_barrier_wait(&bar);
    }

}  
   
