#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "mpi.h"

#define NUM_TOSS 10000000

long long int circle(int amount);

double Log2(double n);


int main(int argc, char **argv) {
    int np, pid, i, amount;
    long long int number_in_circle = 0, buff;

    // Basic info
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &np);
    MPI_Comm_rank(MPI_COMM_WORLD, &pid);
    MPI_Status status;
    amount = (int) NUM_TOSS / np;

    // Main node
    if (pid == 0) {
        long double pi;

        // Timing
        double startTime, totalTime;
        startTime = MPI_Wtime();

        number_in_circle = circle(amount);

        // Communication tree
        for (i = 0; i < Log2(np); i++) {
            MPI_Recv(&buff, 1, MPI_LONG_LONG_INT, (int) pow(2, i), 0, MPI_COMM_WORLD, &status);
            number_in_circle += buff;
        }

        // The final result
        pi = (long double) 4 * number_in_circle / NUM_TOSS;
        printf("PI: %Lg\n", pi);

        // Timing
        totalTime = MPI_Wtime() - startTime;
        printf("Main process finished in time %f secs.\n", totalTime);
    } else {
        number_in_circle = circle(amount);

        // Communication tree
        int current_base, next_base;
        for (i = 0; i < Log2(np); i++) {
            current_base = (int) pow(2, i);
            next_base = current_base * 2;
            if (pid % next_base != 0) {
                // node that should pass message
                MPI_Send(&number_in_circle, 1, MPI_LONG_LONG_INT, pid - current_base, 0, MPI_COMM_WORLD);
                MPI_Finalize();
                return 0;
            } else if (pid + current_base < np) {
                // node that should receive
                MPI_Recv(&buff, 1, MPI_LONG_LONG_INT, pid + current_base, 0, MPI_COMM_WORLD, &status);
                number_in_circle += buff;
            }
            // and node that should stay still
        }
    }

    MPI_Finalize();
    return 0;
}

long long int circle(int amount) {
    // rand() is not really random
    // But it's fast and easy
    long long int number_in_circle = 0, toss;
    double x, y, distance_squared;

    for (toss = 0; toss < amount; toss++) {
        // Spice the rand() a little
        // x,y will be any double between [-1,1]
        srand(rand());
        x = (((double) rand() / RAND_MAX) - 0.5) * 2;

        // Because rand() here has different seed form above,
        // so the seed for y would be different from x
        srand(rand());
        y = (((double) rand() / RAND_MAX) - 0.5) * 2;
        distance_squared = x * x + y * y;
        if (distance_squared <= 1) number_in_circle++;
    }

    return number_in_circle;
}

double Log2(double n) {
    return log(n) / log(2);
}