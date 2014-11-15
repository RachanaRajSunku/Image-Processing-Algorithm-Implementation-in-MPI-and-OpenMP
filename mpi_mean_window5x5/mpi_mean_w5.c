#include <mpi.h>
#include <stdio.h>

#define SIZE 1024			/* Size of image matrix */
#define WINDOW_SIZE 5
#define FROM_MASTER 1
#define FROM_WORKER 2

int A[SIZE][SIZE];
int B[SIZE][SIZE];

void fill_matrix(int R[SIZE][SIZE])
{
  int i, j;
  for (i=0; i<SIZE; i++){
    for (j=0; j<SIZE; j++)
      R[i][j] = rand()%10;
      //R[i][j]=j;
  }
}

void print_matrix(int R[SIZE][SIZE])
{
  int i, j;
  for (i=0; i<SIZE; i++) {
    printf("\n ");
    for (j=0; j<SIZE; j++)
      printf("%2d\t ", R[i][j]);
  }
}

int main(int argc, char *argv[])
{
  int proc_id, num_procs, rows, offset, rows_send,extra_row, chunk_size, m,i, j, k;
  //int tag = 666;		/* any value will do */
  int master_type,recv_type,rows_recv;
  int sum=0;
  int mean =0;
  double start, stop, elapsed_time;
  MPI_Status status;
  
  MPI_Init (&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &proc_id);	/* id of processor */
  MPI_Comm_size(MPI_COMM_WORLD, &num_procs);		/* num of processors */

  int num_workers=num_procs-1;
  if(proc_id == 0){
	fill_matrix(A);
	//print_matrix(A);
	start = MPI_Wtime();
	chunk_size = SIZE/num_workers;
	extra_row = SIZE%num_workers;
	offset = 0;
	master_type = FROM_MASTER;

	for(i=1;i<=num_workers;i++){
		rows = (i<=extra_row)? chunk_size+1 : chunk_size;
		rows_send = (i<2)? rows : rows+(WINDOW_SIZE-1); 
		MPI_Send(&offset,1,MPI_INT,i,master_type,MPI_COMM_WORLD);
		MPI_Send(&rows_send,1,MPI_INT,i,master_type,MPI_COMM_WORLD);
		int count = rows_send*SIZE;
		MPI_Send(&A[offset][0],count,MPI_INT,i,master_type,MPI_COMM_WORLD);
		offset = offset+rows_send-(WINDOW_SIZE-1);
	}
 	 recv_type = FROM_WORKER;

        for(i=1;i<=num_workers;i++){
		//printf("Received resultant matrix from worker processes\n");
                MPI_Recv(&offset,1,MPI_INT,i,recv_type,MPI_COMM_WORLD,&status);
                MPI_Recv(&rows_recv,1,MPI_INT,i,recv_type,MPI_COMM_WORLD,&status);
                int count = rows_recv*SIZE;
                MPI_Recv(&B[offset][0],count,MPI_INT,i,recv_type,MPI_COMM_WORLD,&status);
	} 	
	//printf("\nMatrix B is:\n");	
	//print_matrix(B);
	
	stop = MPI_Wtime();

	elapsed_time=stop-start;
	printf("\nElapsed time is %1.2f",elapsed_time);
  }

  if(proc_id > 0){
  int recv_type = FROM_MASTER;
	MPI_Recv(&offset,1,MPI_INT,0,recv_type,MPI_COMM_WORLD,&status);
	/*if(proc_id == num_workers)
		printf("\nOffset received is %d\n",offset);*/
        MPI_Recv(&rows_recv,1,MPI_INT,0,recv_type,MPI_COMM_WORLD,&status);
	/*if(proc_id == num_workers)
                printf("Number of rows received is %d\n",rows_recv);*/
        int count = rows_recv*SIZE;
        MPI_Recv(&A,count,MPI_INT,0,recv_type,MPI_COMM_WORLD,&status);
	/*if(proc_id == num_workers){
	for(i=0;i<rows_recv;i++){
        	for(j=0;j<SIZE;j++)
                	printf("%d\t", A[i][j]);
        printf("\n");
	}
	}*/
  
  /* printf("Process rank is: %d\n",proc_id);
  printf("%d\n", offset);
  printf("%d\n", rows_recv);
  for(i=0;i<count;i++){
	for(j=0;j<SIZE;j++)
		printf("%d\t", A[i][j]);
        printf("\n");
   }
  */
	for(i=0;i<rows_recv;i++){
		for(j=0;j<SIZE;j++)
			B[i][j] = A[i][j];
	}
	
	for(k=0;k<(rows_recv-(WINDOW_SIZE-1));k++){
		for(m=0;m<(SIZE-(WINDOW_SIZE-1));m++){
			sum=0;
			mean=0;
        		for(i=0;i<WINDOW_SIZE;i++){
                		for(j=0;j<WINDOW_SIZE;j++)
                        	sum+=A[i+k][j+m];
                        }	
			mean = sum/(WINDOW_SIZE*WINDOW_SIZE);
			B[k+2][m+2]=mean;
                }
        }
	
	if(proc_id == 1){
		//printf("I am worker process 1\n");
		rows_send = rows_recv-2;
		master_type = FROM_WORKER;
                MPI_Send(&offset,1,MPI_INT,0,master_type,MPI_COMM_WORLD);
		//printf("Offset being sent is %d\n",offset);
                MPI_Send(&rows_send,1,MPI_INT,0,master_type,MPI_COMM_WORLD);
		//printf("Number of rows being sent is %d\n",rows_send);
                int count = rows_send*SIZE;
                MPI_Send(&B,count,MPI_INT,0,master_type,MPI_COMM_WORLD);
		/*printf("Matrix B is:");
		for(i=0;i<rows_send;i++){
        		for(j=0;j<SIZE;j++)
               	 		printf("%d\t", B[i][j]);
        		printf("\n"); 
		}*/
        }
	if(proc_id > 1){
		//printf("We are other worker processes\n");
		offset=offset+2;
		master_type = FROM_WORKER;
		rows_send = (proc_id > (num_workers-1))? rows_recv-2 : rows_recv -(WINDOW_SIZE-1);
		MPI_Send(&offset,1,MPI_INT,0,master_type,MPI_COMM_WORLD);
		/*if(proc_id == num_workers)
                printf("Offset being sent is %d\n",offset);*/
                MPI_Send(&rows_send,1,MPI_INT,0,master_type,MPI_COMM_WORLD);
		/*if(proc_id == num_workers)
                printf("Number of rows being sent is %d\n",rows_send);*/
                int count = rows_send*SIZE;
                MPI_Send(&B,count,MPI_INT,0,master_type,MPI_COMM_WORLD);
		/*if(proc_id == num_workers){
                printf("Matrix B is:");
		for(i=0;i<rows_send;i++){
                        for(j=0;j<SIZE;j++)
                                printf("%d\t", B[i][j]);
                        printf("\n");
		}
		}*/
	}

  }
  MPI_Finalize();
}
