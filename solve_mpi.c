#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <math.h>

int main(int argc,char *argv[]){
    MPI_Init(&argc,&argv);
    int rank; MPI_Comm_rank(MPI_COMM_WORLD,&rank);

    if(argc!=4){
        if(rank==0) printf("Usage: %s A.txt b.txt out.txt\n",argv[0]);
        MPI_Finalize(); return 1;
    }

    char *A_file=argv[1],*b_file=argv[2],*out_file=argv[3];
    int n;
    double *A,*b,*x;

    if(rank==0){
        FILE *fA=fopen(A_file,"r");
        FILE *fb=fopen(b_file,"r");
        if(!fA||!fb){ perror("File open"); MPI_Abort(MPI_COMM_WORLD,1);}
        double tmp; n=0;
        while(fscanf(fA,"%lf",&tmp)!=EOF) n++;
        n=(int)sqrt(n); fseek(fA,0,SEEK_SET);

        A=(double*)malloc(n*n*sizeof(double));
        b=(double*)malloc(n*sizeof(double));
        x=(double*)malloc(n*sizeof(double));
        for(int i=0;i<n;i++) x[i]=0.0;

        for(int i=0;i<n;i++)
            for(int j=0;j<n;j++)
                fscanf(fA,"%lf",&A[i*n+j]);
        for(int i=0;i<n;i++) fscanf(fb,"%lf",&b[i]);
        fclose(fA); fclose(fb);
    }

    MPI_Bcast(&n,1,MPI_INT,0,MPI_COMM_WORLD);
    if(rank!=0){
        A=(double*)malloc(n*n*sizeof(double));
        b=(double*)malloc(n*sizeof(double));
        x=(double*)malloc(n*sizeof(double));
        for(int i=0;i<n;i++) x[i]=0.0;
    }

    MPI_Bcast(A,n*n,MPI_DOUBLE,0,MPI_COMM_WORLD);
    MPI_Bcast(b,n,MPI_DOUBLE,0,MPI_COMM_WORLD);

    for(int i=0;i<n;i++){
        double sum=0.0;
        for(int j=0;j<i;j++) sum+=A[i*n+j]*x[j];
        if(A[i*n+i]==0){ if(rank==0) printf("Zero diagonal\n"); MPI_Abort(MPI_COMM_WORLD,1);}
        x[i]=(b[i]-sum)/A[i*n+i];
    }

    if(rank==0){
        printf("Solution x:\n");
        for(int i=0;i<n;i++) printf("%f ",x[i]);
        printf("\n");

        FILE *fout=fopen(out_file,"w");
        for(int i=0;i<n;i++) fprintf(fout,"%f\n",x[i]);
        fclose(fout);
    }

    free(A); free(b); free(x);
    MPI_Finalize();
    return 0;
}

