//
//  trap_reduce.c
//  csc450_lab1
//
//  Created by Philippe Nadon on 2019-10-07.
//  Copyright © 2019 Philippe Nadon. All rights reserved.
//

#include <stdio.h>
#include <mpi.h>

double f(double x_val) {
    return x_val * x_val;
}

void Get_input(
               int my_rank,
               int comm_sz,
               double* a_p,
               double* b_p,
               int* n_p) {
    int dest;
    
    if (my_rank == 0) {
        printf("Enter a, b, and n\n");
        scanf("%lf %lf %d", a_p, b_p, n_p);
        for (dest = 1; dest < comm_sz; dest++) {
            MPI_Send(a_p, 1, MPI_DOUBLE, dest, 0, MPI_COMM_WORLD);
            MPI_Send(b_p, 1, MPI_DOUBLE, dest, 0, MPI_COMM_WORLD);
            MPI_Send(n_p, 1, MPI_INT, dest, 0, MPI_COMM_WORLD);
        }
    } else {
            MPI_Recv(a_p, 1, MPI_DOUBLE, 0, 0,
                     MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Recv(b_p, 1, MPI_DOUBLE, 0, 0,
                     MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Recv(n_p, 1, MPI_INT, 0, 0,
                     MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
}

double Trap(
            double left_endpt,
            double right_endpt,
            int trap_count,
            double base_len) {
    double estimate, x;
    int i;
    
    estimate = (f(left_endpt) + f(right_endpt)) / 2.0;
    for (i = 1; i <= trap_count - 1; i++) {
        x = left_endpt + i * base_len;
        estimate += f(x);
    }
    estimate = estimate * base_len;
    
    return estimate;
}

int main(void) {
    int my_rank, comm_sz, n, local_n;
    double a, b, h, local_a, local_b;
    double local_int, total_int = 0;
    int source;
    
    MPI_Init(NULL, NULL);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
    
    Get_input(my_rank, comm_sz, &a, &b, &n);
    
    h = (b - a) / n;
    local_n = n / comm_sz;
    
    local_a = a + my_rank * local_n * h;
    local_b = local_a + local_n * h;
    local_int = Trap(local_a, local_b, local_n, h);
    
    // Data is automatically collected into total_int at Process 0,
    // from each process's local_int.
    MPI_Reduce(&local_int, &total_int, 1, MPI_DOUBLE, MPI_SUM, 0,
               MPI_COMM_WORLD);
    
    if (my_rank == 0) {
        printf("with n = %d trapezoids, our estimate\n", n);
        printf("of the integral from %f to %f = %.15e\n", a, b, total_int);
    }
    MPI_Finalize();
    return 0;
}
