#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>
#include <stdbool.h>


/*** Skeleton for Lab 1 ***/

/***** Globals ******/
float **a; /* The coefficients */
float *x;  /* The unknowns */
float *b;  /* The constants */
float err; /* The absolute relative error */
int num = 0;  /* number of unknowns */


/****** Function declarations */
void check_matrix(); /* Check whether the matrix will converge */
void get_input();  /* Read input from file */

/********************************/



/* Function definitions: functions are ordered alphabetically ****/
/*****************************************************************/

/* 
   Conditions for convergence (diagonal dominance):
   1. diagonal element >= sum of all other elements of the row
   2. At least one diagonal element > sum of all other elements of the row
 */
void check_matrix()
{
  int bigger = 0; /* Set to 1 if at least one diag element > sum  */
  int i, j;
  float sum = 0;
  float aii = 0;
  
  for(i = 0; i < num; i++)
  {
    sum = 0;
    aii = fabs(a[i][i]);
    
    for(j = 0; j < num; j++)
       if(j != i)
         sum += fabs(a[i][j]);
       
    if(aii < sum)
    {
      printf("The matrix will not converge.\n");
      exit(1);
    }
    
    if(aii > sum)
      bigger++;
    
  }
  
  if(!bigger )
  {
     printf("The matrix will not converge\n");
     exit(1);
  }
}


/******************************************************/
/* Read input from file */
/* After this function returns:
 * a[][] will be filled with coefficients and you can access them using a[i][j] for element (i,j)
 * x[] will contain the initial values of x
 * b[] will contain the constants (i.e. the right-hand-side of the equations
 * num will have number of variables
 * err will have the absolute error that you need to reach
 */
void get_input(char filename[])
{
  FILE * fp;
  int i,j;  
 
  fp = fopen(filename, "r");
  if(!fp)
  {
    printf("Cannot open file %s\n", filename);
    exit(1);
  }

 fscanf(fp,"%d ",&num);
 fscanf(fp,"%f ",&err);

 /* Now, time to allocate the matrices and vectors */

 // make process 0 reads a matrix 
 
  a = (float**)malloc(num * sizeof(float*));
  if(!a){
    printf("Cannot allocate a!\n");
    exit(1);
  }

  for(i = 0; i < num; i++){
    a[i] = (float *)malloc(num * sizeof(float)); 
    if(!a[i]){
      printf("Cannot allocate a[%d]!\n",i);
      exit(1);
    }
  }
 

 // end of reading A matrix in process 0
 
 x = (float *) malloc(num * sizeof(float));
 if(!x){
  printf("Cannot allocate x!\n");
  exit(1);
 }


 b = (float *) malloc(num * sizeof(float));
 if(!b)
  {
   printf("Cannot allocate b!\n");
   exit(1);
  }



 /* Now .. Filling the banks */ 

 /* The initial values of Xs */
 for(i = 0; i < num; i++)
  fscanf(fp,"%f ", &x[i]);

 for(i = 0; i < num; i++)
 {
   for(j = 0; j < num; j++)
     fscanf(fp,"%f ",&a[i][j]);
   
   /* reading the b element */
   fscanf(fp,"%f ",&b[i]);
 }
 
 fclose(fp); 
  
}


/************************************************************/


int main(int argc, char *argv[]){

 int i;
 int nit = 0; /* number of iterations */
 FILE * fp;
 char output[100] ="";
  
 if( argc != 2)
 {
   printf("Usage: ./gsref filename\n");
   exit(1);
 }
  
 /* Read the input file and fill the global data structure above */ 
 get_input(argv[1]);
 
 /* Check for convergence condition */
 /* This function will exit the program if the coffeicient will never converge to 
  * the needed absolute error. 
  * This is not expected to happen for this programming assignment.
  */
 check_matrix();
 
 
 
 /* Writing results to file */
 sprintf(output,"%d.sol",num);
 fp = fopen(output,"w");
 if(!fp)
 {
   printf("Cannot create the file %s\n", output);
   exit(1);
 }

    // initialization
    int comm_sz; 
    int my_rank; 

    MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz); 
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank); 

    float *local_x; 
    int flag = 0; 
 
    int all_flag; 
    float local_diff; 
    int counter = 0; 
    int each = (int)(num/comm_sz);



    // receive_count: the number of elements the master process will receive from the other processes
    // displs: accumulutive sum of each element in receive_count
    // they are for Allgatherv 

    int* receive_count, *displs; 
    receive_count = (int *)malloc(comm_sz * sizeof(int)); 
    displs = (int *)malloc(comm_sz * sizeof(int)); 
 

    int sum_buffer = 0; 
    if(each * comm_sz == num){
      for(int i = 0; i < comm_sz; i++){
        receive_count[i] = each; 
        displs[i] = sum_buffer; 
        sum_buffer += receive_count[i];
      }
    }else{
      for(int i = 0; i < comm_sz; i++){ 
        if(i < num - each * comm_sz){
          receive_count[i] = each + 1; 
        }else{
          receive_count[i] = each; 
        }
        displs[i] = sum_buffer; 
        sum_buffer += receive_count[i];
      }
    } 
 
    local_x = (float *)malloc((each+1) * sizeof(float));

    // start the iteration
    do{
      flag = 0; 
      local_diff = 0; 

      // for each process: calculate: receive_count[my_rank] to receive_count[my_rank + 1] -1 
      // each process can have different amount of calculations if the total number of calculations are not divisible
      // by number of processes
      for(int i = 0; i < receive_count[my_rank]; i++){
        float buffer = 0, deno = 0; 
        for(int j = 0; j < num; j++){
          if(j == (displs[my_rank] + i)){
            deno = a[j][j]; 
          }else{
            buffer += a[displs[my_rank] + i][j] * x[j]; 
          }
         }
        local_x[i] = (b[displs[my_rank] + i] - buffer)/deno; 

        local_diff = ((local_x[i] - x[displs[my_rank] + i])/local_x[i]); 

        if(local_diff > err || local_diff < -err) {
          flag = 1;
        } 
      }

      // gather the result into process 0
      MPI_Allgatherv(local_x, receive_count[my_rank], MPI_FLOAT, x, receive_count, displs, MPI_FLOAT, MPI_COMM_WORLD); 
      if(my_rank == 0){
        counter ++;
      }

      // to make all the processes stop executing at the same loop
      // is a synchronization point
      MPI_Allreduce(&flag, &all_flag, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);

    }while(all_flag > 0); 

    if(my_rank == 0){
      nit = counter; 
    }

    if(my_rank == 0){
      printf("total number of iterations: %d\n", nit);
    }

    free(receive_count); 
    free(local_x); 
 
    MPI_Finalize();

    for(i = 0; i < num; i++)
      fprintf(fp,"%f\n",x[i]);

    fclose(fp);

    for(int i = 0; i < num; i++){
      free(a[i]); 
    }

    free(a); 
    free(b); 
    free(x); 

    exit(0);

}
