// mmm.c
// Parallelized matrix multiplication
// author: Philippe Nadon
// date: Oct 21, 2019

#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>
#include <time.h> 

double get_rand_double() {
  return (double)rand()/RAND_MAX*2.0-1.0;
}

int main( int argc, char **argv) {
  int my_rank, comm_sz;
  time_t timer;
  int m = atoi(argv[1]); int n = atoi(argv[2]); int k = atoi(argv[3]);

  // a_trans is merely the A-matrix, but transposed for memory locality.
  double *a_trans = malloc(sizeof(double) * k * m);
  double *b = malloc(sizeof(double) * k * n);
  double *c = malloc(sizeof(double) * m * n);
  double *sub_c = malloc(sizeof(double) * m * n);

  // Initialize transposed A-matrix, and B-matrix with random float64
  for(int i = 0; i < m * k; i++) {
    a_trans[i] = get_rand_double();
  }
  for(int i = 0; i < n * k; i++) {
    b[i] = get_rand_double();
  }
  for( int i = 0; i < m; i++) {
    for( int j = 0; j < n; j++) {
      c[i * n + j] = (double)0;
    }
  }

  MPI_Init(NULL, NULL);
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
  // sub-arrays of a & b initialized here, due to dependence on # of procs
  double *sub_a = malloc(sizeof(double) * k * m / comm_sz);
  double *sub_b = malloc(sizeof(double) * k * n / comm_sz);

  MPI_Scatter(
    a_trans, m*k / comm_sz, MPI_DOUBLE, 
    sub_a, m*k / comm_sz, MPI_DOUBLE, 0, MPI_COMM_WORLD);

  MPI_Scatter(
    b, n * k / comm_sz, MPI_DOUBLE, 
    sub_b, n * k / comm_sz, MPI_DOUBLE, 0, MPI_COMM_WORLD);

  if(my_rank == 0) {
    timer = clock(); // begin timing here
  }
  for( int i = 0; i < m; i++) {
    for( int j = 0; j < n; j++) {
      sub_c[i * n + j] = 0;
      for( int sub_row = 0; sub_row < k / comm_sz; sub_row++) {
        sub_c[i * n + j] += sub_a[sub_row * m + i] * sub_b[sub_row * n + j];
      }
    }
  }
  MPI_Barrier(MPI_COMM_WORLD);
  MPI_Reduce(sub_c, c, m * n, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
  MPI_Barrier(MPI_COMM_WORLD);
  if( my_rank == 0) {
    printf("%d,%d,%d,%d,%ld\n", comm_sz, m, k, n, clock() - timer);
  }
  MPI_Finalize();
}