/* File:     vector_add.c
 *
 * Purpose:  Implement vector addition with random vectors of size 100,000
 *
 * Compile:  gcc -g -Wall -o vector_add vector_add.c
 * Run:      ./vector_add
 *
 * Output:   A small part of the sum vector z = x+y
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define VECTOR_SIZE 100000

void Allocate_vectors(double** x_pp, double** y_pp, double** z_pp, int n);
void Generate_random_vector(double a[], int n);
void Print_part_of_vector(double b[], int n, char title[]);
void Vector_sum(double x[], double y[], double z[], int n);

int main(void) {
   int n = VECTOR_SIZE;
   double *x, *y, *z;

   Allocate_vectors(&x, &y, &z, n);
   
   Generate_random_vector(x, n);
   Generate_random_vector(y, n);
   
   Vector_sum(x, y, z, n);

   Print_part_of_vector(z, n, "A part of the sum is");

   free(x);
   free(y);
   free(z);

   return 0;
}  /* main */

void Allocate_vectors(double**  x_pp, double**  y_pp, double**  z_pp, int n) {
   *x_pp = malloc(n*sizeof(double));
   *y_pp = malloc(n*sizeof(double));
   *z_pp = malloc(n*sizeof(double));
   if (*x_pp == NULL || *y_pp == NULL || *z_pp == NULL) {
      fprintf(stderr, "Can't allocate vectors\n");
      exit(-1);
   }
}  /* Allocate_vectors */

void Generate_random_vector(double  a[], int n) {
   srand(time(NULL)); // Seed the random number generator
   for (int i = 0; i < n; i++)
      a[i] = (rand() % 1000) / 10.0;  // Random number between 0 and 99.9
}  /* Generate_random_vector */

void Print_part_of_vector(double b[], int n, char title[]) {
   int i;
   printf("%s\n", title);
   for (i = 0; i < 10 && i < n; i++)  // Print only the first 10 elements
      printf("%.2f ", b[i]);
   printf("\n");
}  /* Print_part_of_vector */

void Vector_sum(double  x[], double  y[], double  z[], int n) {
   for (int i = 0; i < n; i++)
      z[i] = x[i] + y[i];
}  /* Vector_sum */
