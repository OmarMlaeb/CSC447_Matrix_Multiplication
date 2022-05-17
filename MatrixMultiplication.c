#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <mpi.h>
#include <stddef.h>

int rowA, colA;
int rowB, colB;

int get_random_number() {
	return rand() % (100 + 1);
}

void SerialMatrixMultiplication(int* matrix1, int* matrix2, int* resultMatrix) {
	for (int i = 0; i < rowA; ++i){
		for (int j = 0; j < colB; ++j){
			resultMatrix[i*colB + j] = 0;
			for (int x = 0; x < colA; x++){
				resultMatrix[i*colB + j] += matrix1[i*colA + x] * matrix2[x*colB + j];
		}
	}
}

	//printf("RESULTS Serial WAY:\n");
	//for(int i = 0; i < rowA*colB; i++)
	//	printf("%d ", resultMatrix[i]);
	//printf("\n");
	
}

// function to display the matrix
void displayResult(int* resultMatrix) {
	printf("\nResultant Matrix:\n");
	for (int i = 0; i < rowA; ++i) {
		for (int j = 0; j < colB; ++j) {
			printf("%d ", resultMatrix[i*colB + j]);
			if (j == colB - 1){
				printf("\n");
			}
		}
	}
}

int* OwnPartOfMatrixA;
int* matrixB;
int* finalMatrix;
int* matrixA;
int tasksPerProcess;
double rowsPerProcess_d;
int rowsPerProcess;

int main(int argc, char *argv[]){
	int MatrixProperties[4];

	clock_t t;
	int rank,size;
	MPI_Init(&argc, &argv); //initialize MPI operations
	MPI_Comm_rank(MPI_COMM_WORLD, &rank); //get the rank
	MPI_Comm_size(MPI_COMM_WORLD, &size); //get number of processes

//==============================================================START==============================================================//

	if(rank == 0){		
		printf("Scan the row of matrix one:\n");
		scanf("%d", &rowA);
		printf("\nScan the col of matrix one:\n");
		scanf("%d", &colA);
		printf("\nScan the row of matrix one:\n");
		scanf("%d", &rowB);
		printf("\nScan the col of matrix one:\n");
		scanf("%d", &colB);
		
		t = clock();

		if (colA != rowB){
			printf("\nThe column of the first matrix is not equal to the row of the second matrix\n");
			return 0;
		}
	
	}

	MPI_Barrier(MPI_COMM_WORLD);
	double start_time = MPI_Wtime();

	if(rank == 0){

		OwnPartOfMatrixA = (int*) malloc((rowA*colA)*sizeof(int));
		matrixB = (int*) malloc((rowB*colB)*sizeof(int));
		
		finalMatrix = (int*) malloc((rowA*colB)*sizeof(int)); 
		
		for (int i = 0; i < rowA; ++i){
			for (int j = 0; j < colA; ++j){

				OwnPartOfMatrixA[i*colA + j] = get_random_number();	

			}
		}

		for (int i = 0; i < rowB; ++i){
			for (int j = 0; j < colB; ++j){
				
				matrixB[i*colB + j] = get_random_number();
				
			}
		}

		MatrixProperties[0] = rowA;
		MatrixProperties[1] = colA;
		MatrixProperties[2] = rowB;
		MatrixProperties[3] = colB;
		
		tasksPerProcess = (rowA*colB) / size;
		rowsPerProcess = rowA / size;
		rowsPerProcess_d = (double)rowA / (double)size;

	}
	
	MPI_Bcast(MatrixProperties, 4, MPI_INT, 0, MPI_COMM_WORLD);

	rowA = MatrixProperties[0];
	colA = MatrixProperties[1];
	rowB = MatrixProperties[2];
	colB = MatrixProperties[3];
	
	tasksPerProcess = (rowA*colB) / size;
	rowsPerProcess = rowA / size;
	rowsPerProcess_d = (double)rowA / (double)size;
	
	int* RC = (int*) malloc((rowA*colB*2)*sizeof(int));
	int R = 0;
	int C = 0;
	for(int i = 0; i < rowA*colB*2; i += 2){
		RC[i] = R;
		RC[i+1] = C;
		
		C++;
		if(C == colA){
			C = 0;
			R++;
		}
		
	}
	
	
	
	if(rank != 0){
		
		matrixB = (int*) malloc((rowB*colB)*sizeof(int));
		OwnPartOfMatrixA = (int*) malloc((rowA*colA)*sizeof(int));
		
	}
	
	MPI_Bcast(OwnPartOfMatrixA, rowA*colA, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(matrixB, rowB*colB , MPI_INT, 0, MPI_COMM_WORLD);

	int* localResults = (int*) malloc(tasksPerProcess*sizeof(int)); 
		
	int index = 0;
	int a = (rank*(tasksPerProcess*2));
	
	for(int i = 0; i < tasksPerProcess; i++){
		int r = RC[a];
		int c = RC[a+1];
		a += 2;
		localResults[index] = 0;
		for(int j = 0; j < colA; j++){

			localResults[index] += OwnPartOfMatrixA[r*colA +j]*matrixB[j*colA + c];
		
		}

		index++;
	}
	
	if(rank == 0){
		int max = rowA * colB;
		int remainder = max % size;
		int a = (size)*(tasksPerProcess*2);
		for(int i = max - remainder; i < max; i++){
			int r = RC[a];
			int c = RC[a+1];
			a += 2;
			finalMatrix[i] = 0;
			
			for(int j = 0; j < colA; j++)
				finalMatrix[i] += OwnPartOfMatrixA[r*colA + j]*matrixB[j*colA + c];
		
		}
		
	}
	
	MPI_Gather(localResults, tasksPerProcess , MPI_INT , finalMatrix , tasksPerProcess , MPI_INT , 0 , MPI_COMM_WORLD);
	//Gather statement From: A_partition s to final matrix 
	
	MPI_Barrier(MPI_COMM_WORLD);
	double end_time = MPI_Wtime();
	
	if(rank == 0){
		printf("Parallel time: %fs\n", end_time-start_time);
	
		//t = clock() - t;
		//double time_taken = ((double)t)/CLOCKS_PER_SEC; 
		//printf("%f\n", time_taken);
	
		//printf("RESULTS PARALLEL WAY:\n");
		//for(int i=0;i<rowA*colB;i++){
		//	printf("%d ",finalMatrix[i]);
		//}
		//printf("\n");

		start_time = MPI_Wtime();
		SerialMatrixMultiplication(OwnPartOfMatrixA, matrixB, finalMatrix);
		end_time = MPI_Wtime();
		
		printf("Serial time: %fs\n", end_time-start_time);
	}
	
	MPI_Finalize();
	return 0;
}
