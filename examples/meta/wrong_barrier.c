
#include <mpi.h>

int main(int argc, char **argv) {

  int myrank, buffer;
  MPI_Status status;
  MPI_Request req;

  printf("Am Anfang\n");
  MPI_Init(&argc, &argv);

  printf("Bin da\n");
  MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
  printf("%d ist da\n",myrank);
  if(myrank==0) {
    buffer=31;
    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Send(&buffer, 1, MPI_INT, 1, 1, MPI_COMM_WORLD);
    MPI_Barrier(MPI_COMM_WORLD);
  } else if (myrank==1) {
    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Recv(&buffer, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, &status);
    printf("Prozess %d hat %d empfangen.\n",myrank,buffer);
  }
  MPI_Finalize();
}

