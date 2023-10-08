/* File:     mpi_vector_add.c
 *
 * Purpose:  Implement parallel vector addition using a block
 *           distribution of the vectors. This version also
 *           illustrates the use of MPI_Scatter and MPI_Gather.
 *
 * Compile:  mpicc -g -Wall -o mpi_vector_add mpi_vector_add.c
 * Run:      mpiexec -n <comm_sz> ./vector_add
 *
 * Input:    The order of the vectors, n, and the vectors x and y
 * Output:   The sum vector z = x+y
 *
 * Notes:     
 * 1.  The order of the vectors, n, should be evenly divisible
 *     by comm_sz
 * 2.  DEBUG compile flag.    
 * 3.  This program does fairly extensive error checking. When
 *     an error is detected, a message is printed and the processes
 *     quit. Errors detected are incorrect values of the vector
 *     order (negative or not evenly divisible by comm_sz), and
 *     malloc failures.
 *
 * IPP:  Section 3.4.6 (pp. 109 and ff.)
 */
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

void Check_for_error(int local_ok, char fname[], char message[], 
      MPI_Comm comm);
void Read_n(int* n_p, int* local_n_p, int my_rank, int comm_sz, 
      MPI_Comm comm);
void Allocate_vectors(double** local_x_pp, double** local_y_pp,
      double** local_z_pp, int local_n, MPI_Comm comm);
void Read_vector(double local_a[], int local_n, int n, char vec_name[], 
      int my_rank, MPI_Comm comm);
void Print_vector(double local_b[], int local_n, int n, char title[], 
      int my_rank, MPI_Comm comm);
void Parallel_vector_sum(double local_x[], double local_y[], 
      double local_z[], int local_n);

//-------------------------------------------------------------------/
int main(void) {
   int n, local_n;
   int comm_sz, my_rank;
   double *local_x, *local_y, *local_z;
   MPI_Comm comm;

   MPI_Init(NULL, NULL);
   comm = MPI_COMM_WORLD;
   MPI_Comm_size(comm, &comm_sz);
   MPI_Comm_rank(comm, &my_rank);

   Read_n(&n, &local_n, my_rank, comm_sz, comm);
   
   Allocate_vectors(&local_x, &local_y, &local_z, local_n, comm);
   Read_vector(local_x, local_n, n, "x", my_rank, comm);
   Read_vector(local_y, local_n, n, "y", my_rank, comm);
   
   Parallel_vector_sum(local_x, local_y, local_z, local_n);
   Print_vector(local_z, local_n, n, "The sum is", my_rank, comm);

   free(local_x);
   free(local_y);
   free(local_z);

   MPI_Finalize();
   return 0;
}  /* main */

void Read_n(int* n_p, int* local_n_p, int my_rank, int comm_sz, MPI_Comm comm) {
   *n_p = 100000;
   *local_n_p = *n_p / comm_sz;
}

void Read_vector(double local_a[], int local_n, int n, char vec_name[], int my_rank, MPI_Comm comm) {
   double* a = NULL;
   int i;

   if (my_rank == 0) {
      a = malloc(n*sizeof(double));
      for (i = 0; i < n; i++) {
         a[i] = (double) rand() / RAND_MAX;
      }
      MPI_Scatter(a, local_n, MPI_DOUBLE, local_a, local_n, MPI_DOUBLE, 0, comm);
      free(a);
   } else {
      MPI_Scatter(a, local_n, MPI_DOUBLE, local_a, local_n, MPI_DOUBLE, 0, comm);
   }
}

//-------------------------------------------------------------------/
void Check_for_error(
      int       local_ok   /* in */, 
      char      fname[]    /* in */,
      char      message[]  /* in */, 
      MPI_Comm  comm       /* in */) {
   int ok;

   MPI_Allreduce(&local_ok, &ok, 1, MPI_INT, MPI_MIN, comm);
   if (ok == 0) {
      int my_rank;
      MPI_Comm_rank(comm, &my_rank);
      if (my_rank == 0) {
         fprintf(stderr, "Proc %d > In %s, %s\\n", my_rank, fname, message);
         fflush(stderr);
      }
      MPI_Finalize();
      exit(-1);
   }
}  /* Check_for_error */

//-------------------------------------------------------------------/
void Allocate_vectors(
      double**  local_x_pp  /* out */, 
      double**  local_y_pp  /* out */, 
      double**  local_z_pp  /* out */, 
      int       local_n     /* in  */, 
      MPI_Comm  comm        /* in  */) {

   int local_ok = 1;

   *local_x_pp = malloc(local_n*sizeof(double));
   *local_y_pp = malloc(local_n*sizeof(double));
   *local_z_pp = malloc(local_n*sizeof(double));

   if (*local_x_pp == NULL || *local_y_pp == NULL || 
       *local_z_pp == NULL) local_ok = 0;
   Check_for_error(local_ok, "Allocate_vectors", 
      "Can't allocate local arrays", comm);
}  /* Allocate_vectors */


//-------------------------------------------------------------------/
void Print_vector(
      double    local_b[]  /* in */, 
      int       local_n    /* in */, 
      int       n          /* in */, 
      char      title[]    /* in */, 
      int       my_rank    /* in */,
      MPI_Comm  comm       /* in */) {

   double* b = NULL;
   int i;
   int local_ok = 1;
   char* fname = "Print_vector";

   if (my_rank == 0) {
      b = malloc(n*sizeof(double));
      if (b == NULL) local_ok = 0;
      Check_for_error(local_ok, fname, "Can't allocate temporary vector", 
            comm);
      MPI_Gather(local_b, local_n, MPI_DOUBLE, b, local_n, MPI_DOUBLE,
            0, comm);
      printf("%s\\n", title);
      for (i = 0; i < n; i++)
         printf("%f ", b[i]);
      printf("\\n");
      free(b);
   } else {
      Check_for_error(local_ok, fname, "Can't allocate temporary vector", 
            comm);
      MPI_Gather(local_b, local_n, MPI_DOUBLE, b, local_n, MPI_DOUBLE, 0,
         comm);
   }
}  /* Print_vector */


//-------------------------------------------------------------------/
void Parallel_vector_sum(
      double  local_x[]  /* in  */, 
      double  local_y[]  /* in  */, 
      double  local_z[]  /* out */, 
      int     local_n    /* in  */) {
   int local_i;

   for (local_i = 0; local_i < local_n; local_i++)
      local_z[local_i] = local_x[local_i] + local_y[local_i];
}  /* Parallel_vector_sum */
