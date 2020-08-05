/*
pi_threads.c

Computes pi using pthreads and mutex locks
author: Philippe Nadon
date: October 28, 2019
*/
#include <stdio.h> 
#include <stdlib.h> 
#include <math.h>
#include <pthread.h> 
double sum;
long n;
long thread_count;
pthread_mutex_t mutex;


void* Thread_sum(void* rank) {
  long my_rank = (long) rank;
  double factor, my_sum = 0.0;
  long long i;
  long long my_n = n / thread_count;
  long long my_first_i = my_n * my_rank;
  long long my_last_i = my_first_i + my_n;

  if (my_first_i % 2 == 0)
    factor = 1.0; 
  else
    factor = -1.0;

  for (i = my_first_i; i < my_last_i; i++, factor = -factor)
    my_sum += factor / (2 * i + 1);

  pthread_mutex_lock(&mutex); 
  sum += 4 * my_sum; 
  pthread_mutex_unlock(&mutex);

  return NULL;
}

int main(int argc, char *argv[]) {
  n = atol(argv[1]);
  thread_count = atol(argv[2]);

  pthread_t *rank = malloc(sizeof(pthread_t) * thread_count);
  pthread_mutex_init(&mutex, NULL);

  for( int i = 0; i < thread_count; i++) {
    pthread_create(&rank[i], NULL, Thread_sum, i);
  }
  for( int i = 0; i < thread_count; i++) {
    pthread_join(rank[i], NULL);
  }

  // size of n is approximately proportional to the number of accurate digits
  printf("Approximation: %2.20lf\n", sum);
  printf("       Actual: %2.20lf\n", M_PI);
  return 0;
}