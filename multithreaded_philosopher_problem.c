#include <unistd.h>
#include <pthread.h>
#include <stdio.h> 
#include <stdlib.h>

struct Philosopher
{
	int i;
	int N;
	int meals_eaten;
	pthread_mutex_t* fork1;
	pthread_mutex_t* fork2;
};

void* run(void* arg)
{
	struct Philosopher p = *((struct Philosopher*)arg);

	while (p.meals_eaten != 3) 
	{
		if (pthread_mutex_trylock(p.fork1) == 0)
		{
			if (pthread_mutex_trylock(p.fork2) == 0)
			{
				sleep(1);	
				printf("Philosopher (Thread %d) ate their meal #%d.\n", p.i+1, p.meals_eaten+1);
				p.meals_eaten++;

				pthread_mutex_unlock(p.fork1);
				pthread_mutex_unlock(p.fork2);
			}
		}
		else
		{
			pthread_mutex_unlock(p.fork1);
			// The following print statement prints VERY often if un-commented
			//
			//printf("--Philosopher (Thread %d) had to put their first fork down.\n", p.i+1);
		}
	}
	if (p.meals_eaten == 3)
	{
		printf("Philosopher (THREAD %d) ATE 3 MEALS; DONE\n", p.i+1);
		pthread_exit(NULL);
	}
	return NULL;
}


int main(int argc, char* argv[])
{
	int N = atoi(argv[1]);
	
	pthread_t threads[N];
	pthread_mutex_t forks[N];

	struct Philosopher philosophers[N];	

	for (int x = 0; x < N; x++)
	{
		pthread_mutex_t fork;
		
		forks[x] = fork;
		
		pthread_mutex_init(&forks[x], NULL);
	}

	for (int x = 0; x < N; x++)
	{
		struct Philosopher p;
		p.i = x;
		p.N = N;
		p.meals_eaten = 0;
		p.fork1 = &forks[x];
		p.fork2 = &forks[((x+1) % N)];
	
		philosophers[x] = p;
	}

	for (int x = 0; x < N; x++)
		pthread_create(&threads[x], 0, run, &philosophers[x]);

	for (int x = 0; x < N; x++)
		pthread_join(threads[x],NULL);
	
	for (int x = 0; x < N; x++)
		pthread_mutex_destroy(&forks[x]);
}
