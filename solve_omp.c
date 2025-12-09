#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>

int main(int argc, char *argv[]) {
    if(argc != 4) {
        printf("Usage: %s A.txt b.txt out.txt\n", argv[0]);
        return 1;
    }

    char *A_file = argv[1];
    char *b_file = argv[2];
    char *out_file = argv[3];

    FILE *fA = fopen(A_file,"r");
    FILE *fb = fopen(b_file,"r");
    if(!fA || !fb){ perror("File open"); return 1; }

    int n = 0;
    double tmp;
    while(fscanf(fA,"%lf",&tmp) != EOF) n++;
    n = (int)sqrt(n);
    fseek(fA,0,SEEK_SET);

    double *A = malloc(n*n*sizeof(double));
    double *b = malloc(n*sizeof(double));
    double *x = malloc(n*sizeof(double));
    for(int i=0;i<n;i++) x[i]=0.0;

    for(int i=0;i<n;i++)
        for(int j=0;j<n;j++)
            fscanf(fA,"%lf",&A[i*n+j]);

    for(int i=0;i<n;i++) fscanf(fb,"%lf",&b[i]);
    fclose(fA); fclose(fb);

    // Определяем тип треугольной матрицы
    int is_lower = 1;
    int is_upper = 1;
    for(int i=0;i<n;i++){
        for(int j=0;j<i;j++) if(fabs(A[i*n+j])>1e-12) is_upper=0;
        for(int j=i+1;j<n;j++) if(fabs(A[i*n+j])>1e-12) is_lower=0;
    }

    if(!is_lower && !is_upper){
        printf("Matrix is not triangular!\n");
        free(A); free(b); free(x);
        return 1;
    }

    if(is_lower) printf("Matrix type detected: lower\n");
    if(is_upper) printf("Matrix type detected: upper\n");

    // Решение треугольной системы
    if(is_lower){
        for(int i=0;i<n;i++){
            double sum = 0.0;
            #pragma omp parallel for reduction(+:sum)
            for(int j=0;j<i;j++) sum += A[i*n+j]*x[j]; // параллельное суммирование
            if(fabs(A[i*n+i])<1e-12){ printf("Zero diagonal\n"); free(A); free(b); free(x); return 1; }
            x[i] = (b[i]-sum)/A[i*n+i]; // последовательное вычисление x[i]
        }
    } else {
        for(int i=n-1;i>=0;i--){
            double sum = 0.0;
            #pragma omp parallel for reduction(+:sum)
            for(int j=i+1;j<n;j++) sum += A[i*n+j]*x[j]; // параллельное суммирование
            if(fabs(A[i*n+i])<1e-12){ printf("Zero diagonal\n"); free(A); free(b); free(x); return 1; }
            x[i] = (b[i]-sum)/A[i*n+i]; // последовательное вычисление x[i]
        }
    }

    printf("Solution x:\n");
    for(int i=0;i<n;i++) printf("%f ", x[i]);
    printf("\n");

    FILE *fout = fopen(out_file,"w");
    for(int i=0;i<n;i++) fprintf(fout,"%f\n",x[i]);
    fclose(fout);

    free(A); free(b); free(x);
    return 0;
}
