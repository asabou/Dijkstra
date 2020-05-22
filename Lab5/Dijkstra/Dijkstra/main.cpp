#include <mpi.h>
#include <iostream>
#include <fstream>
#include <string.h>
#include <chrono>
#include <cassert>

#define MAX 10000

using namespace std::chrono;
using namespace std;

void initMatrix(int a[], int&n) {
	cout << "InitMatrix" << endl;
	for (int i = 1; i <= n; i++) {
		for (int j = 1; j <= n; j++) {
			if (i != j) {
				a[i*n+j] = MAX;
			}
		}
	}
}

int readDims(int rank) {
	int n;
	if (rank == 0) {
		//string filename = "C:\\Users\\alexandru.sabou\\university\\An3\\Sem2\\PTPP\\Lab5\\Dijkstra\\Dijkstra\\test_hard_input1.txt";
		string filename = "C:\\Users\\alexandru.sabou\\university\\An3\\Sem2\\PTPP\\Lab5\\Dijkstra\\Dijkstra\\test_easy_input1.txt";
		ifstream f(filename);
		int m, source, destination;
		f >> n >> m;
		f >> source >> destination;
	}
	MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
	return n;
}

void printMatrix(int *mat, int &n) {
	for (int i = 0; i < n; i++) {
		for (int j = 0; j < n; j++) {
			cout << mat[i*n + j] << " ";
		}
		cout << endl;
	}
}

void readMatrix(int localMat[], int n, int segment, MPI_Datatype myblock, int rank) {
	int *a = NULL;
	if (rank == 0) {
		//string filename = "C:\\Users\\alexandru.sabou\\university\\An3\\Sem2\\PTPP\\Lab5\\Dijkstra\\Dijkstra\\test_hard_input1.txt";
		string filename = "C:\\Users\\alexandru.sabou\\university\\An3\\Sem2\\PTPP\\Lab5\\Dijkstra\\Dijkstra\\test_easy_input1.txt";
		ifstream f(filename);
		int n, m, source, destination;
		f >> n >> m >> source >> destination;
		int x, y, cost;
		a = (int*)malloc(sizeof(int) * n * n);
		cout << "InitMatrix\n";
		for (int i = 0; i < n; i++) {
			for (int j = 0; j < n; j++) {
				if (i != j) {
					a[i*n + j] = MAX;
				}
				if (i == j) {
					a[i*n + j] = 0;
				}
			}
		}
		cout << "ReadMatrix\n";
		for (int i = 1; i <= m; i++) {
			f >> x >> y >> cost;
			a[x*n + y] = cost;
		}
		//cout << "PrintMatrix\n";
		/*for (int i = 0; i < n; i++) {
			for (int j = 0; j < n; j++) {
				cout << a[i*n + j] << " ";
			}
			cout << endl;
		}*/
	}
	MPI_Scatter(a, 1, myblock, localMat, n*segment, MPI_INT, 0, MPI_COMM_WORLD);
	if (rank == 0) {
		free(a);
	}
}

MPI_Datatype buildMyBloc(int&n, int&segment) {
	MPI_Aint lb, extent;
	MPI_Datatype myBlock, myBlockReturned, oldBlock;
	MPI_Type_contiguous(segment, MPI_INT, &myBlock);
	MPI_Type_get_extent(myBlock, &lb, &extent);
	MPI_Type_vector(n, segment, n, MPI_INT, &oldBlock);
	MPI_Type_create_resized(oldBlock, lb, extent, &myBlockReturned);
	MPI_Type_commit(&myBlockReturned);
	MPI_Type_free(&myBlock);
	MPI_Type_free(&oldBlock);
	return myBlockReturned;
}

void dijkstraInit(int localMat[], int localPred[], int localDist[], int localVisited[], int& rank, int&segment) {
	if (rank == 0) {
		localVisited[0] = 1;
	}
	else {
		localVisited[0] = 0;
	}
	for (int i = 1; i < segment; i++) {
		localVisited[i] = 0;
	}
	for (int i = 0; i < segment; i++) {
		localDist[i] = localMat[0 * segment + i];
		localPred[i] = 0;
	}

}

int findMinDist(int localDist[], int localVisited[], int &segment) {
	int returnedNode = -1;
	int minDist = MAX;
	for (int i = 0; i < segment; i++) {
		if (!localVisited[i]) {
			if (localDist[i] < minDist) {
				minDist = localDist[i];
				returnedNode = i;
			}
		}
	}
	return returnedNode;
}

void dijkstra(int localMat[], int localDist[], int localPred[], int &segment, int &n,int&rank) {
	int*localVisited = (int*)malloc(segment*(sizeof(int)));
	int localMins[2];
	int globalMins[2];
	dijkstraInit(localMat, localPred, localDist, localVisited, rank, segment);
	for (int i = 0; i < n - 1; i++) {
		int neighbour = findMinDist(localDist, localVisited, segment);
		if (neighbour != -1) {
			localMins[0] = localDist[i];
			localMins[1] = neighbour + rank * segment;
		}
		else {
			localMins[0] = MAX;
			localMins[1] = -1;
		}
		MPI_Allreduce(localMins, globalMins, 1, MPI_2INT, MPI_MINLOC, MPI_COMM_WORLD);
		int globalDist = globalMins[0];
		int globalNeighbor = globalMins[1];
		if (globalNeighbor == -1) {
			break;
		}
		if ((globalNeighbor / segment) == rank) {
			neighbour = globalNeighbor % segment;
			localVisited[neighbour] = 1;
		}
		for (int i = 0; i < segment; i++) {
			if (!localVisited[i]) {
				int dist = globalDist + localMat[globalNeighbor*segment + i];
				if (dist < localDist[i]) {
					localDist[i] = dist;
					localPred[i] = globalNeighbor;
				}
			}
		}
	}
	free(localVisited);
}

int main(int argc, char**argv) {
	int *localMat, *localDist, *localPred, *globalDist = NULL, *globalPred = NULL;
	int rank, size, segment,n;
	MPI_Datatype myBlock;
	MPI_Init(NULL, NULL);
	
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	n = readDims(rank);
	segment = n / size;

	localMat = (int*)malloc(sizeof(int) * n * segment);
	localDist = (int*)malloc(sizeof(int) * segment);
	localPred = (int*)malloc(sizeof(int) * segment);
	myBlock = buildMyBloc(n, segment);

	if (rank == 0) {
		globalDist = (int*)malloc(sizeof(int)*n);
		globalPred = (int*)malloc(sizeof(int)*n);
	}
	readMatrix(localMat, n, segment, myBlock, rank);
	dijkstra(localMat, localDist, localPred, segment, n,rank);
	MPI_Gather(localDist, segment, MPI_INT, globalDist, segment, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Gather(localPred, segment, MPI_INT, globalPred, segment, MPI_INT, 0, MPI_COMM_WORLD);

	if (rank == 0) {
		cout << "Fathers: ";
		for (int i = 0; i < n; i++) {
			cout << globalPred[i] << " ";
		}
		free(globalDist);
		free(globalPred);
	}
	free(localMat);
	free(localDist);
	free(localPred);
	MPI_Type_free(&myBlock);
	MPI_Finalize();
	return 0;						
}