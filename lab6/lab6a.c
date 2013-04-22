#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>

#define NUM_THREADS 5

void *PrintHello(void *threadid)
{
printf("\nHello World! from thread %d\n", threadid);
pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
pthread_t threads[NUM_THREADS];
int rc, t;

	for(t=0; t<NUM_THREADS; t++)
	{

		rc = pthread_create(&threads[t], NULL, PrintHello, (void *)t);
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

return 0;

}
