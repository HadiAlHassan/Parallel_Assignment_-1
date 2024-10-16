#include <mpi.h>
#include <stdio.h>
#include <math.h>

// Function to evaluate the curve (y = f(x))
float f(float x) {
    return x * x ; // Example: y = x^2
}

// Function to compute the area of a trapezoid
float trapezoid_area(float a, float b, float d) { 
    float area = 0;
    for (float x = a; x < b; x+=d) {
        area += f(x) + f(x+d);
    }
    
    return area * d / 2.0f;
}

int main(int argc, char** argv) {
    int rank, size;
    float a = 0.0f, b = 1.0f;  // Limits of integration
    int n;
    float start, end, local_area, total_area;

    // Assume n is given
    n = 10000000;
    
    double seq_start, seq_end, seq_time;
    double seq_area = 0;
    float delta = (b - a) / n; // delta 

    // Initialize MPI
    MPI_Init(&argc, &argv); 
    MPI_Comm_rank(MPI_COMM_WORLD, &rank); 
    MPI_Comm_size(MPI_COMM_WORLD, &size); 
    double parallel_start, parallel_end, parallel_duration;

    if (rank == 0) {
        // Sequential implementation 
        printf("Number of intervals is: %d \n", n);

        seq_start = MPI_Wtime();
        seq_area = trapezoid_area(a, b, delta);
        seq_end = MPI_Wtime();
        seq_time = seq_end - seq_start;

        printf("\n");
        printf("----------Sequential Code---------- \n");
        printf("Execution time %f\n", seq_area);
        printf("The time took to complete the operation is: %f  \n", seq_time);
    }

    parallel_start = MPI_Wtime();
    MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD); // Broadcast the number of intervals

    float region = (b - a)/ size;
    start = a + rank * region;
    end = start + region;
    
    // Each process calculates its local area
    local_area = trapezoid_area(start, end, delta);
    
    // Reduce all local areas to the total area on the root process
    MPI_Reduce(&local_area, &total_area, 1, MPI_FLOAT, MPI_SUM, 0, MPI_COMM_WORLD);
    parallel_end = MPI_Wtime();
    parallel_duration = parallel_end - parallel_start;
    
    if (rank == 0) {
        // Only rank 0 prints the results
        printf("\n");
        printf("----------Parallel Code----------\n");
        printf("The total area under the curve is: %f\n", total_area);
        printf("Execution time is: %f\n", parallel_duration);
        printf("\n");

        float speed_up =  seq_time / parallel_duration;
        printf("The speedup factor is %f \n", speed_up);
        printf("The efficiency is %f%%\n\n", (speed_up / size) * 100);
    }
    
    MPI_Finalize(); 
    return 0;
}
