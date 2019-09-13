#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <omp.h>

void prime(int a, int n);

int main(int agrc, char** argv){

	if(agrc != 3){
		printf("the input is not correct\n");
		exit(1);
	}

	int a = atoi(argv[1]); 
	if(a < 0){
		printf("a is not correct\n");
		exit(1);
	}

	int b = atoi(argv[2]);
	if(b < 0){
		printf("b is not correct\n");
		exit(1);
	}


	prime(a, b);
	return 0;
}


void prime(int a, int n){

	FILE * file; 
	char output[100] ="";
	sprintf(output,"%d.txt",a);
	file = fopen(output, "w");

	if(!file){
		printf("Cannot create the file %s\n", output);
		exit(1);
	}

	double tstart = 0.0, ttaken = 0.0;
	tstart = omp_get_wtime();

	int* arr;
	arr = (int*)malloc((a-1) * sizeof(int));

	if(!arr){
		printf("Cannot allocate a!\n");
		exit(1);
	}

	int limit = (int)((a + 1)/2);

	# pragma omp parallel for num_threads(n)
	for(int i = 0; i < a - 1; i++){
		arr[i] = i + 2; 
	}


	# pragma omp parallel num_threads(n)
	for(int i = 0; i < limit; i++){
		if(arr[i] != -1){
			# pragma omp for 
			for(int j = i + 1; j < a - 1; j++){
				if(arr[j] != -1 && arr[j] % arr[i] == 0){
					arr[j] = -1; 
				}
			}
		}
	}

	ttaken = omp_get_wtime() - tstart;


	int counter = 1; 
	int last = 0;
	for(int i = 0; i < a - 1; i++){
		if(arr[i] > 0){
			if(i == 0){
				fprintf(file, "%d, %d, %d\n", counter, arr[i], 0); 
			}else{
				fprintf(file, "%d, %d, %d\n", counter, arr[i], arr[i] - last); 
			}
			
			last = arr[i]; 
			counter ++; 
		}
	}


	fclose(file);

	printf("Time take for the main part: %f\n", ttaken);

}

