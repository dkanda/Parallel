//@ author David Kanda (dkanda)
//@ date 3/1/2013
//@ description MPI test program that compares pid numbers between 
// nodes to determine which is the smallest node and then each
// nodes prints its discovered smallest node
#include <sys/types.h>
#include <stdio.h> 
#include <mpi.h> 
int main(int argc, char *argv[]) 
{ 
  
  //struct contains pid and node rank 
    typedef struct Data_s {
          int pid;
          int rank;
    } Data;

    int rank, size, i; 
    pid_t mypid;
    MPI_Init(&argc, &argv); 
    MPI_Comm_rank(MPI_COMM_WORLD, &rank); 
    MPI_Comm_size(MPI_COMM_WORLD, &size); 
    mypid = getpid();

    /********************************************/
    //create an MPI datatype to handle a struct 
    const int nitems=2;
    int blocklengths[2] = {1,1};
    MPI_Datatype types[2] = {MPI_INT, MPI_INT};
    MPI_Datatype mpi_data_struct;
    MPI_Aint     offsets[2];
    offsets[0] = offsetof(Data, pid);
    offsets[1] = offsetof(Data, rank);
    MPI_Type_create_struct(nitems, blocklengths, offsets, types, &mpi_data_struct);
    MPI_Type_commit(&mpi_data_struct);
    /********************************************/
    //assign values to struct elements
    Data smallest, incoming;
    smallest.pid = mypid;
    smallest.rank = rank;
    incoming.pid = 0;
    incoming.rank = 0;
    //loop through world/2
    for(i=1; i<size;i = i + 2)
    {
      if(rank % 2 == 0)//for even nodes, each even node sends to every odd, then receives from every odd 
      {
        MPI_Send(&smallest, 1, mpi_data_struct, (rank+i) % size, 1, MPI_COMM_WORLD);
        MPI_Recv(&incoming, 1, mpi_data_struct, (rank-i + size) %size, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE); 
      //if the received pid is smaller than the smallest, replace it and send out new smallest
        if(incoming.pid < smallest.pid)
          smallest = incoming;
      }
      else//for odd nodes, each odd node receives from every even, then sends to every even 
      {
        MPI_Recv(&incoming, 1, mpi_data_struct, (rank-i + size) %size, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        //if the received pid is smaller than the smallest, replace it and send out new smallest
        if(incoming.pid < smallest.pid)
          smallest = incoming;
        MPI_Send(&smallest, 1, mpi_data_struct, (rank+i) % size, 1, MPI_COMM_WORLD);
      }
    }

    //print results
    printf("Process %d: my ID is %d, the smallest process ID %d is from %d \n", rank, mypid, smallest.pid, smallest.rank);
    MPI_Type_free(&mpi_data_struct);
    MPI_Finalize(); 
    return 0; 
} 
