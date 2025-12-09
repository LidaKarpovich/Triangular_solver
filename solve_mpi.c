#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <math.h>

int main(int argc,char *argv[]){
    MPI_Init(&argc,&argv);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);
    MPI_Comm_size(MPI_COMM_WORLD,&size);

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


    // Определение типа матрицы
    int is_lower = 1, is_upper = 1;
    if(rank==0){
        for(int i=0;i<n;i++){
            for(int j=0;j<n;j++){
                if(i<j && fabs(A[i*n+j])>1e-12) is_lower = 0;
                if(i>j && fabs(A[i*n+j])>1e-12) is_upper = 0;
            }
        }
        if(is_lower && !is_upper) printf("Matrix type detected: lower triangular\n");
        else if(is_upper && !is_lower) printf("Matrix type detected: upper triangular\n");
        else { 
            printf("Matrix is not triangular\n"); 
            MPI_Abort(MPI_COMM_WORLD,1);
        }
    }
    MPI_Bcast(&is_lower,1,MPI_INT,0,MPI_COMM_WORLD);

    if(is_lower){
        for(int i=0;i<n;i++){
            int owner = i % size;  // кто считает x[i]
            double xi;
            if(rank==owner){
                double sum=0.0;
                for(int j=0;j<i;j++) sum+=A[i*n+j]*x[j];
                if(fabs(A[i*n+i])<1e-12){ if(rank==0) printf("Zero diagonal\n"); MPI_Abort(MPI_COMM_WORLD,1);}
                xi = (b[i]-sum)/A[i*n+i];
            }
            MPI_Bcast(&xi,1,MPI_DOUBLE,owner,MPI_COMM_WORLD);
            x[i] = xi;
        }
    } else {
        for(int i=n-1;i>=0;i--){
            int owner = i % size;
            double xi;
            if(rank==owner){
                double sum=0.0;
                for(int j=i+1;j<n;j++) sum+=A[i*n+j]*x[j];
                if(fabs(A[i*n+i])<1e-12){ if(rank==0) printf("Zero diagonal\n"); MPI_Abort(MPI_COMM_WORLD,1);}
                xi = (b[i]-sum)/A[i*n+i];
            }
            MPI_Bcast(&xi,1,MPI_DOUBLE,owner,MPI_COMM_WORLD);
            x[i] = xi;
        }
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
