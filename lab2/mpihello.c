//@ author David Kanda (dkanda)
//@ date 3/1/2013
//@ description MPI test program hello world
#include <stdio.h> 
#include <mpi.h> 
int main(int argc, char *argv[]) 
{ 
  int    namelen; 
  char   processor_name[MPI_MAX_PROCESSOR_NAME]; 
  int rank, size; 
  MPI_Init(&argc, &argv); //initialize
  MPI_Comm_rank(MPI_COMM_WORLD, &rank); //get node rank
  MPI_Comm_size(MPI_COMM_WORLD, &size); //get number of nodes on network
  MPI_Get_processor_name(processor_name,&namelen); 
  printf("Hello world! I am %d of %d on %s \n", rank, size, processor_name); 
  MPI_Finalize(); //disconnect
  return 0; 
} 
