#include <stdio.h>
#include <stdlib.h>
#include <mpi.h> 

void Produit_MatVec(double *Mat, double *Vec,double* V_out,int m, int n)
{
	int i,j;
     double s;

	for (i=0;i<m;++i)
	{
	s=0.0;
		for (j=0;j<n;++j)
		{
		s =s+Vec[j]*Mat[i*n+j];
		}
       V_out[i]=s;
	}
}
void PrintMat(double *Mat, int n, int m)
{
int i,j;
   
	for (i=0;i<n;++i)
	{
		for (j=0;j<m;++j)
		{
			printf("%f ",Mat[i*m+j]);
		}
		printf("\n");
	}
}

void PrintVec_i(double *Vec,int n)
{
int j;
for (j=0;j<n;++j)
	{
	printf("%f\n",Vec[j]); // %d 
	}
	printf("\n");
}

int main(int argc, char **argv)
{
int nc,nl,i,j,debut,debut_b;
int rang, nproc,root;
int ntot, nloc, ideb, ifin,Q,R;
double *A,*x,*b;

double *blocMat; // bloc ou sous-matrice que chaque processus va traiter
double *b_proc; // bloc du vecteur resultat par chaque processus

// données pour le scatterv
int *sendcounts;    // array describing how many elements to send to each process
int *displs;        // array describing the displacements where each segment begins

// données pour le Gatherv
int *recvcounts;
int *displs_Gather;

MPI_Status sta;

nl=7;
nc=9;
debut,ntot, nloc, ideb, ifin,Q,R=0;




x=malloc(nc*sizeof(double));


MPI_Init(&argc, &argv);
MPI_Comm_rank(MPI_COMM_WORLD, &rang);
MPI_Comm_size(MPI_COMM_WORLD, &nproc);

i,j,root=0;

// le processus de rang 0 initialise la matrice A et le vecteur x
if (rang==root){
A=(double*) malloc(nl*nc*sizeof(double));
b=malloc(nl*sizeof(double));
for (i=0;i<nl;i++) {
	for (j=0;j<nc;j++) {
		A[i*nc+j]=i;
	}
}

for (j=0;j<nc;j++) {
		x[j]=1.0;
		
	}

for (j=0;j<nl;j++) {
		
		b[j]=0.0;
	}
	
   }
if (rang==root){

printf("ma matrice A :\n");
PrintMat(A, nl, nc)	;
printf("\n");
printf("le vecteur x :\n");	
PrintVec_i(x,nc);
printf("\n");
}/**/



/*
// tester si un processus donner a reçu le vecteur x
if (rang==nproc-1){
printf("rang:%d, le vecteur x :\n",rang);	
PrintVec_i(x,nc);
printf("\n");
}
*/
/*l'étape suivante consiste à distribuer les données de la matrices aux différents processus */
/*calcul du nombre debloc à traiter pour chaque processus*/
    ntot = nl*nc;
    Q = nl / nproc;
    R = nl % nproc;
    
    sendcounts =malloc(nproc*sizeof(int));// tableau d'entiers qui contient le nombre d'éléments à traiter par chaque processus 
    displs =malloc(nproc*sizeof(int));// tableau d'entiers qui contient le debut de bolc à traiter pour chaque processus
    recvcounts=malloc(nproc*sizeof(int)); // tableau d'entiers qui contient le nombre d'éléments déja traiter par chaque processus 
    displs_Gather=malloc(nproc*sizeof(int));//tableau d'entiers qui contient le debut du vecteur resultat traiter /chaque processus 
       for (i=0; i<nproc;i++)
      {
          
      	displs[i] = 0.0; 
      	sendcounts[i] = 0.0; 
      	recvcounts[i] = 0.0; 
      	displs_Gather[i] = 0.0; 	
      }
    
  // on envisage deux cas :   
 if (rang < R) { // si le reste est inférieur au rang, on distribue ce reste sur les nloc 
	nloc = Q+1;  // nombre d'éléments à traiter par chaque processus
	ideb = rang * nloc;// calcul de la position de debut dans la martrice
	ifin = ideb + nloc;
	
     blocMat =(double*) malloc(nloc*nc*sizeof(double));//bloc matrice à traiter par chaque processus
     b_proc= malloc((nloc)*sizeof(double));//bloc vecteur dans lequel chaque processus calcul une partie du produit A*x 
	
       for (i=0;i<nloc;i++) 
       {
		for (j=0;j<nc;j++) 
		{
			blocMat[i*nc+j]=0.0;
		}
	 }
	// chaque processus va calculer son display et l'envoyer au proc 0
		debut=ideb*nc; 
		debut_b=ideb;
		
		if (rang == 0) 
		{
			displs[0]=ideb*nc; 
      		displs_Gather[0]=ideb;
 
         for( i = 1 ; i < nproc ; i++ ) 
         { // ==> on va recevoir de p-1 processus .
            MPI_Recv(&debut, 1, MPI_INT, i, 1000, MPI_COMM_WORLD, &sta);// recevoir de n'importe qui.
            MPI_Recv(&debut_b, 1, MPI_INT, i, 1000, MPI_COMM_WORLD, &sta);// recevoir de n'importe qui.
            displs[i]= debut;
            displs_Gather[i]=debut_b; 
         }
         } 
         else 
         {

        MPI_Send(&debut,1, MPI_INT,root, 1000, MPI_COMM_WORLD);
        MPI_Send(&debut_b,1, MPI_INT,root, 1000, MPI_COMM_WORLD);      
        }
		
       for (i=0; i<nproc;i++)
      {
          sendcounts[i]= nloc*nc; 
          recvcounts[i] =nloc; // pour le Gatherv           		
      }
     } 
     
    /********************************************************************************************************************/   
   /********************************************************************************************************************/   
  /********************************************************************************************************************/  
 /********************************************************************************************************************/  
    // rang >= R
    else { 
	nloc = Q;
	ideb = R * (Q+1) + (rang - R) * Q;
	ifin = ideb + nloc;
 
	blocMat= (double*) malloc(nc*(nloc+1)*sizeof(double));//bloc matrice à traiter par chaque processus
	b_proc= malloc((nloc+1)*sizeof(double));//bloc vecteur dans lequel chaque processus calcul une partie du produit A*x

	for (i=0;i<nloc+1;i++) 
     {
		for (j=0;j<nc;j++) {
			blocMat[i*nc+j]=0.0;
		}
	}
	for (i=0;i<nloc+1;i++) 
     {
     	b_proc[i]=0.0;
     }
		// chaque processus va calculer son display et l'envoyer au proc 0
		debut=ideb*nc; 
		debut_b=ideb;
		
		if (rang == 0) 
		{
			displs[0]=ideb*nc; 
      		displs_Gather[0]=ideb;
      		 
      		
        for( i = 1 ; i < nproc ; i++ ) 
        { // ==> on va recevoir de p-1 processus .
            MPI_Recv(&debut, 1, MPI_INT, i, 1000, MPI_COMM_WORLD, &sta);// recevoir de n'importe qui son debut de bloc matrice
            MPI_Recv(&debut_b, 1, MPI_INT,i, 1000, MPI_COMM_WORLD, &sta);// recevoir de n'importe qui.
            displs[i]= debut;
            displs_Gather[i]=debut_b; 
        }
        
	  }
       else 
       {
        MPI_Send(&debut,1, MPI_INT,root, 1000, MPI_COMM_WORLD);
        MPI_Send(&debut_b,1, MPI_INT,root, 1000, MPI_COMM_WORLD);      
       }
	  /**/for (i=0; i<nproc;i++)
       {   
          sendcounts[i]= nloc*nc; //ok         
          recvcounts[i] =nloc; // pour le Gatherv
          
       }
     }
     

	
      MPI_Scatterv(A,sendcounts,displs,MPI_DOUBLE,blocMat,(nc*(nloc+1)),MPI_DOUBLE,root,MPI_COMM_WORLD);
      MPI_Barrier(MPI_COMM_WORLD);
      // diffuser le vecteur x par le processus 0
      MPI_Bcast(x,nc,MPI_DOUBLE,root,MPI_COMM_WORLD);
      Produit_MatVec(blocMat,x,b_proc,nloc,nc) ;
      MPI_Barrier(MPI_COMM_WORLD);
      MPI_Gatherv(b_proc,nloc,MPI_DOUBLE,b,recvcounts,displs_Gather,MPI_DOUBLE,root,MPI_COMM_WORLD);
      MPI_Barrier(MPI_COMM_WORLD);
      
      if(rang==root){
       printf("Rang 0: Le resultat final du produit A*b, X vaut:\n");	
       PrintVec_i(b,nl);
       }
     
       printf("nproc=%d,rang=%d,ntot=%d,nloc=%d,ideb=%d,ifin=%d\n", nproc, rang, ntot, nloc, ideb, ifin);
       MPI_Barrier(MPI_COMM_WORLD);
       printf("rang:%d, taille du sous-bloc:%d, ***ideb=%d, nc=%d ****,deplacement=%d\n",rang,nc*nloc,ideb,nc,ideb*nc);	
       MPI_Barrier(MPI_COMM_WORLD);
       
       if(rang==root)	
	  PrintMat(blocMat,nloc,nc);// affchage du sous bloc du processus root par exemple
       MPI_Barrier(MPI_COMM_WORLD);

       
       // chaque prov applique un produit matrice vecteur
       
       
       

if(rang==root){

free(b);
free(A);
}
free(x);
free(sendcounts);
free(displs);
free(blocMat);
free(recvcounts);
free(displs_Gather);
free(b_proc);
MPI_Finalize();


return 0;
}     

