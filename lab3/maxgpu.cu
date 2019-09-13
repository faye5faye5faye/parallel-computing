#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <cuda.h>
#include <locale.h>

unsigned int getmax(unsigned int *, unsigned int);
unsigned int getmaxcu(unsigned int *, unsigned int);




__global__ void helper(unsigned int * d, unsigned int * comp, int size) {
  int tid; 
  tid = threadIdx.x;

  int i; 
  i = blockIdx.x * blockDim.x + threadIdx.x;

  if(i < size){
    for(uint stride = 1; stride < blockDim.x; stride *= 2){
      if(i % (2 * stride) == 0){
        if(comp[i] < comp[i + stride]){
          comp[i] = comp[i + stride]; 
        }
      } 
      __syncthreads();  
    }


    if(tid == 0){
      if(comp[blockIdx.x * blockDim.x] > d[blockIdx.x]){
        d[blockIdx.x] = comp[blockIdx.x * blockDim.x]; 
      }
    }
  }
} 


unsigned int getmaxcu(unsigned int num[], unsigned int size){
  uint *x, *y;

  cudaMalloc((void**)&x, size*sizeof(uint));
  cudaMalloc((void**)&y, size*sizeof(uint));

  cudaMemcpy(x, num, sizeof(uint) * size, cudaMemcpyHostToDevice); 
  cudaMemcpy(y, num, sizeof(uint) * size, cudaMemcpyHostToDevice); 

  int a = (int)((size - size % 1024)/1024) + 1;

  helper<<<a, 1024>>>(x, y, size);

  cudaMemcpy(num, x, sizeof(uint) * size, cudaMemcpyDeviceToHost); 

  uint max = 0; 
  for(int i = 0; i < a + 1; i++){
    max = max > num[i] ? max : num[i]; 
  }

  cudaFree(x); 
  cudaFree(y); 

  return(max);
}


int main(int argc, char *argv[])
{
    unsigned int size = 0;  // The size of the array
    unsigned int i;  // loop index
    unsigned int * numbers; //pointer to the array
    
    if(argc !=2)
    {
       printf("usage: maxseq num\n");
       printf("num = size of the array\n");
       exit(1);
    }
   
    size = atol(argv[1]);

    numbers = (unsigned int *)malloc(size * sizeof(unsigned int));
    if( !numbers ){
       printf("Unable to allocate mem for an array of size %u\n", size);
       exit(1);
    }    

    srand(time(NULL)); // setting a seed for the random number generator
    // Fill-up the array with random numbers from 0 to size-1 
    for( i = 0; i < size; i++)
       numbers[i] = rand()  % size;    
   
    printf("The maximum number in the array is: %u\n", getmaxcu(numbers, size));

    free(numbers);
    exit(0);
}







