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

    // Определяем размер матрицы
    int n = 0;
    double tmp;
    while(fscanf(fA,"%lf",&tmp) != EOF) n++;
    n = (int)sqrt(n);
    fseek(fA,0,SEEK_SET);

    double *A = malloc(n*n*sizeof(double));
    double *b = malloc(n*sizeof(double));
    double *x = malloc(n*sizeof(double));
    for(int i=0;i<n;i++) x[i]=0.0;

    // Чтение A
    for(int i=0;i<n;i++)
        for(int j=0;j<n;j++)
            fscanf(fA,"%lf",&A[i*n+j]);

    // Чтение b
    for(int i=0;i<n;i++) fscanf(fb,"%lf",&b[i]);
    fclose(fA); fclose(fb);

    // Решение треугольной системы (нижняя)
    for(int i=0;i<n;i++){
        double sum = 0.0;
        #pragma omp parallel for reduction(+:sum)
        for(int j=0;j<i;j++) sum += A[i*n+j]*x[j];
        if(A[i*n+i]==0){ printf("Zero diagonal\n"); return 1; }
        x[i] = (b[i]-sum)/A[i*n+i];
    }

    // Вывод
    printf("Solution x:\n");
    for(int i=0;i<n;i++) printf("%f ", x[i]);
    printf("\n");

    // Сохранение
    FILE *fout = fopen(out_file,"w");
    for(int i=0;i<n;i++) fprintf(fout,"%f\n",x[i]);
    fclose(fout);

    free(A); free(b); free(x);
    return 0;
}

