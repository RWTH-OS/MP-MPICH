
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "mpi.h"

#ifndef boolean
typedef int boolean;
#define true 1 
#define false 0
#endif

static volatile int N;						/* Größe der Matrix (Option: s<N>  */
static volatile int P;						/* Anzahl Prozesse                 */ 
static volatile int N_DIV_P;			/* Anzahl Zeilen pro Prozeß        */
static volatile boolean DISPLAY;	/* Ausgabe der Matrix? (Option: d) */

double	**A;				/* Matrix                                        */
double	*tmpRow;		/* eine Matrixzeile, zum Austausch zw. Prozessen */
double	*maxValue;  /* Max-Werte der einzelnen Prozesse              */
int			*pivot;     /* die Pivot-Zeilen (für Auswert.reihenfolge)    */
boolean *marked;    /* schon markierte Zeilen                        */

static volatile int myid; /* MPI-Id */

/* displayResult() gibt die globale Matrix auf dem Bildschirm aus */
void displayResult()
{ int i,j;
	MPI_Barrier(MPI_COMM_WORLD);
	if (myid==0)
	{	for (i=0; i<N; i++)
		{ int row;
			printf("\n(");
			for (row=0; row<N; row++)
				if (pivot[row]==i)
					break;
			for (j=0; j<N; j++)
		    printf("%.2f ",A[row][j]);
			printf(")(1)=(%.2f)",A[row][N]);
		}
		printf("\n");
	}
}

/* generateMat() erstellt eine zufällige, per Gauss lösbare Matrix */
void generateMat()
{	int i,j,k;
	if (myid==0)
	{	for (i=0; i<N; i++) /* Einheitsmatrix erstellen */
		{	for (j=0; j<N; j++)
		 		A[i][j] = 0.0;
			A[i][i] = (rand()%10)+1;	
			A[i][N] = A[i][i];
		}
		for (k=0; k<N*5; k++) /* beliebig paarweise Zeilen addieren */
		{	int a=abs(rand())%N,b=abs(rand())%N;
			double c=(rand()%100)/10;
			if (a!=b)
				for (j=0; j<=N; j++)
					A[a][j] += c*A[b][j];
		}
	}
}

/* allocateMat() fordert den dynamischen Speicher an... */
void allocateMat()
{	int i,size = (myid==0?N:N_DIV_P);
	A = (double**) malloc(size*sizeof(double*));
	A[0] = (double*) malloc(size*(N+1)*sizeof(double));
	for (i=1; i<size; i++)
		A[i] = A[i-1]+(N+1);
	tmpRow = (double*) malloc((N+1)*sizeof(double));
	maxValue = (double*) malloc(P*sizeof(double));
	pivot = (int*) malloc(size*sizeof(int));
	marked = (boolean*) malloc(N_DIV_P*sizeof(boolean));
}

/* deallocateMat() gibt ihn wieder frei... */
void deallocateMat()
{	free(A[0]);
	free(A);
	free(tmpRow);
	free(maxValue);
	free(pivot);
	free(marked);
}

/* distributeMat() verteilt A auf die einzelnen Prozesse */
void distributeMat()
{	MPI_Scatter(&A[0][0],N_DIV_P*(N+1),MPI_DOUBLE,
			        &A[0][0],N_DIV_P*(N+1),MPI_DOUBLE,0,MPI_COMM_WORLD);	
}

/* collectMat() sammelt die verteilten Zeilen sowie die Pivot-Werte
 *              in der globalen Matrix A vom Rootprozeß 0 */
void collectMat()
{ MPI_Gather(&A[0][0],N_DIV_P*(N+1),MPI_DOUBLE,
			       &A[0][0],N_DIV_P*(N+1),MPI_DOUBLE,0,MPI_COMM_WORLD);	
	MPI_Gather(&pivot[0],N_DIV_P,MPI_INT,
			       &pivot[0],N_DIV_P,MPI_INT,0,MPI_COMM_WORLD);	
}

/* Der eigentliche Gauss-Algorithmus... */ 
int gaussMIMD()
{	int i,j,k;	
	/* initialisiere marked[] zu Beginn auf false */
	for (i=0; i<N_DIV_P; i++)		
		marked[i] = false;
	/* über alle Spalten iterieren... */
	for (i=0; i<N-1; i++)			
	{	/* Jeder Prozeß sucht seinen lokalen maximalen Wert in Spalte i          */ 
		double tmp = -1.0;	/* temp. Max-Wert, -1 falls alle Spalten markiert    */
		int picked = 0;			/* Zeile, die den Max-Wert enthält                   */
		int winner = 0;			/* Prozeß, der den globalen Max-Wert enthält         */
		for (j=0; j<N_DIV_P; j++)							/* alle lokalen Zeilen durchlaufen */
			if (!marked[j] && fabs(A[j][i])>tmp)/* neuer Max-Wert?                 */
			{	tmp = fabs(A[j][i]);							/* -> neuen Max-Wert setzen        */
				picked = j;												/*    & Zeile merken...            */
			}
		/* Nun wird der globale maximale Wert ermittelt...                       */
		maxValue[myid] = tmp; /* Zunächst alle lokalen Max-Werte im über Feld    */ 
		/* maxValue[] many-to-many verschicken...                                */
		MPI_Gather(&maxValue[myid],1,MPI_DOUBLE,
			         &maxValue[0],1,MPI_DOUBLE,0,MPI_COMM_WORLD);
		MPI_Bcast(&maxValue[0],P,MPI_DOUBLE,0,MPI_COMM_WORLD);
		for (j=1; j<P; j++)   /* ...dann globalen Max-Wert suchen                */
			if (maxValue[winner]<maxValue[j])
				winner = j;
		if (maxValue[winner]==0.0) /* Max-Wert=0? -> Kann nicht sein, Gleichungs-*/
			return 1;								 /*                system nicht lösbar!        */
		/* Der Gewinner markiert seine Zeile als benutzt und schickt deren Inhalt*/
		/* one-to-many an die anderen Prozesse                                   */
		if (myid==winner)
		{	marked[picked] = true;
			pivot[picked] = i;         /* Reihenfolge für spätere Auswertung merken*/
			for (j=i; j<=N; j++)			 /* Zeile initialisieren                     */
				tmpRow[j] = A[picked][j];		
		}
		/* Zeile verschicken/empfangen */
		MPI_Bcast(&tmpRow[i],N+1-i,MPI_DOUBLE,winner,MPI_COMM_WORLD);
		/* Nun eliminiert jeder Prozeß seine lokalen unmarkierten Zeilen         */
		for (j=0; j<N_DIV_P; j++)				/* über alle lokalen Zeilen iterieren    */
			if (!marked[j])								/* nur von den noch-nicht-Pivot-Zeilen   */
			{	tmp = A[j][i]/tmpRow[i];		/* die Gewinner-Zeile abziehen...        */
				A[j][i] = 0.0;
				for (k=i+1; k<=N; k++)
				 	A[j][k] -= tmpRow[k]*tmp;
			}
	}
	/* Ermittle letzte Zeile, die noch nicht Pivot-Zeile war... */
	for (i=0; i<N_DIV_P; i++)
		if (!marked[i])
		{	pivot[i] = N-1; /* ...und setze die letzte Pivot-Nr. */
			break;
		}
	return 0;
}

int main(int argc, char *argv[])
{	int i,res;
	double start,end;
	DISPLAY=false;
	N=16;
	for (i=0; i<argc; i++)
	{	if (argv[i][0]=='s')
		 	N = atoi(&argv[i][1]);
		else if (argv[i][0]=='d')
			DISPLAY=true;
	}
	MPI_Init(&argc,&argv);
	MPI_Comm_rank(MPI_COMM_WORLD,&myid);
	MPI_Comm_size(MPI_COMM_WORLD,&P);
	N_DIV_P = N/P;
	if (N%P==0)
	{	if (myid==0)
		{	printf("\nNumber of processes: %d\n",P);
			printf("Matrix size        : %dx%d\n",N,N);
			printf("Partition size     : %d\n",N_DIV_P);
		}
		allocateMat();
		generateMat();
		distributeMat();
		start = MPI_Wtime();	
		res = gaussMIMD(); 
		end = MPI_Wtime();
		if (res && (myid==0)) 
			printf("\nUnsolvable!\n");
		else
		{	collectMat();
			if (DISPLAY)
			 	displayResult();
			printf("P%d Elapsed time (s) : %f\n",myid,end-start);
		}
		deallocateMat();
	}
	else
		printf("\nN mod P is not 0!!!!\n");
	MPI_Finalize();
	return 1;
}


