/* circuitSatifiability.c solves the Circuit Satisfiability
 *  Problem using a brute-force sequential solution.
 *
 *   The particular circuit being tested is "wired" into the
 *   logic of function 'checkCircuit'. All combinations of
 *   inputs that satisfy the circuit are printed.
 *
 *   16-bit version by Michael J. Quinn, Sept 2002.
 *   Extended to 32 bits by Joel C. Adams, Sept 2013.
 */

#include <stdio.h>     // printf()
#include <limits.h>    // UINT_MAX
#include <mpi.h>
#include <math.h>

int checkCircuit(int, int);

double Log2(double n);

int main(int argc, char *argv[]) {
    int i, count = 0, np, pid, amount, buff;

    // mpi basic info
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &np);
    MPI_Comm_rank(MPI_COMM_WORLD, &pid);
    MPI_Status status;
    amount = (int) USHRT_MAX / np;

    if (pid == 0) {
        // Main node workflow:
        // Start the timer -> checkCircuit -> Sum up the result

        // Timing
        double startTime, totalTime;
        startTime = MPI_Wtime();

        // checkCircuit
        for (i = 0; i < amount; i++) {
            count += checkCircuit(pid, i);
        }

        // Sum up the result
        for (i = 0; i < Log2(np); i++) {
            MPI_Recv(&buff, 1, MPI_INT, (int) pow(2, i), 0, MPI_COMM_WORLD, &status);
            count += buff;
        }
        printf("\nA total of %d solutions were found.\n\n", count);

        // Timing
        totalTime = MPI_Wtime() - startTime;
        printf("Main process finished in time %f secs.\n", totalTime);
        fflush(stdout);
    } else {
        // Worker node workflow:
        // checkCircuit -> pass message by tree structure (and ultimately to main node)

        // checkCircuit
        int right_border = pid == np - 1 ? USHRT_MAX : amount * (pid + 1);
        for (i = amount * pid; i < right_border; i++) {
            count += checkCircuit(pid, i);
        }

        // Communication tree
        int current_base, next_base;
        for (i = 0; i < Log2(np); i++) {
            current_base = (int) pow(2, i);
            next_base = current_base * 2;

            if (pid % next_base != 0) {
                // node that should pass message
                MPI_Send(&count, 1, MPI_INT, pid - current_base, 0, MPI_COMM_WORLD);
                MPI_Finalize();
                return 0;
            } else if (pid + current_base < np) {
                // node that should receive
                MPI_Recv(&buff, 1, MPI_INT, pid + current_base, 0, MPI_COMM_WORLD, &status);
                count += buff;
            }
            // and node that should stay still
        }
    }

    MPI_Finalize();
    return 0;
}

/* EXTRACT_BIT is a macro that extracts the ith bit of number n.
 *
 * parameters: n, a number;
 *             i, the position of the bit we want to know.
 *
 * return: 1 if 'i'th bit of 'n' is 1; 0 otherwise
 */

#define EXTRACT_BIT(n, i) ( (n & (1<<i) ) ? 1 : 0)


/* checkCircuit() checks the circuit for a given input.
 * parameters: id, the id of the process checking;
 *             bits, the (long) rep. of the input being checked.
 *
 * output: the binary rep. of bits if the circuit outputs 1
 * return: 1 if the circuit outputs 1; 0 otherwise.
 */

#define SIZE 16

int checkCircuit(int id, int bits) {
    int v[SIZE];        /* Each element is a bit of bits */
    int i;

    for (i = 0; i < SIZE; i++)
        v[i] = EXTRACT_BIT(bits, i);

    if ((v[0] || v[1]) && (!v[1] || !v[3]) && (v[2] || v[3])
        && (!v[3] || !v[4]) && (v[4] || !v[5])
        && (v[5] || !v[6]) && (v[5] || v[6])
        && (v[6] || !v[15]) && (v[7] || !v[8])
        && (!v[7] || !v[13]) && (v[8] || v[9])
        && (v[8] || !v[9]) && (!v[9] || !v[10])
        && (v[9] || v[11]) && (v[10] || v[11])
        && (v[12] || v[13]) && (v[13] || !v[14])
        && (v[14] || v[15])) {
        printf("%d) %d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d \n", id,
               v[15], v[14], v[13], v[12],
               v[11], v[10], v[9], v[8], v[7], v[6], v[5], v[4], v[3], v[2], v[1], v[0]);
        fflush(stdout);
        return 1;
    } else {
        return 0;
    }
}

double Log2(double n) {
    return log(n) / log(2);
}