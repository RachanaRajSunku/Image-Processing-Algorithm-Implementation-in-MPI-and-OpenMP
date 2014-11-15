#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define WINDOW_SIZE 5
int A[SIZE][SIZE],B[SIZE][SIZE];
int window[25];

int main(){
	int i,j,k,m,num_threads,thread_id;   /* Declaration of Loop variables, thread id and number of threads */
	//int chunk=50;    /* Set chunksize */
	int median=0;
	struct timeval start,stop;

/* Matrix initialization */	
	for(i=0;i<SIZE;i++){
                for(j=0;j<SIZE;j++){
                        A[i][j]=rand()%10;
                        //A[i][j]=j;
                        B[i][j]=A[i][j];
                        //printf("Thread %d working on row\n",thread_id);
                }
        }

#pragma omp parallel shared(A,B,num_threads) private(thread_id,i,j,k,m)
{	
	thread_id=omp_get_thread_num();
	if(thread_id==0){
		num_threads=omp_get_num_threads();
		printf("The number of threads are:%d\n",num_threads);
}
gettimeofday(&start,0);

#pragma omp for schedule(static)
	for(k=0;k<(SIZE-4);k++){
		printf("Thread %d working on %d row\n",thread_id,k);
		for(m=0;m<(SIZE-4);m++){
			int l=0;
			median=0;
        		for(i=0;i<WINDOW_SIZE;i++){
                		for(j=0;j<WINDOW_SIZE;j++){
                        	window[l]=A[i+k][j+m];
				l++;
				}
                        }
			for(i=24;i>1;i--){
				for(j=0;j<i;j++){
					if(window[j]> window[j+1]){
						int temp = window[j];
						window[j] = window[j+1];
						window[j+1] = temp;
					}
				}
			}
			median = window[12];
			B[k+2][m+2]=median;
                }
        }
gettimeofday(&stop,0);
}
printf("Execution time=%.6f\n",(stop.tv_sec+stop.tv_usec*1e-6)-(start.tv_sec+start.tv_usec*1e-6));
printf("The resultant matrix is:");
for(i=0;i<SIZE;i++){
	printf("\n");
	for(j=0;j<SIZE;j++){
		printf("%d\t",B[i][j]);
	}
}
return 0;
} 

