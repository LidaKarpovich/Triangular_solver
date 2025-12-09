#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>

typedef struct { int i; double *A; double *x; double *sum; int n; int lower; } thread_arg_t;

void *thread_sum(void *arg) {
    thread_arg_t *t = (thread_arg_t*)arg;
    double s = 0.0;
    if(t->lower) {
        for(int j=0; j<t->i; j++) s += t->A[t->i*t->n + j] * t->x[j];
    } else {
        for(int j=t->i+1; j<t->n; j++) s += t->A[t->i*t->n + j] * t->x[j];
    }
    *(t->sum) = s;
    return NULL;
}

// Функция для определения типа треугольной матрицы
int is_lower_triangular(double *A, int n) {
    for(int i=0;i<n;i++)
        for(int j=i+1;j<n;j++)
            if(fabs(A[i*n+j]) > 1e-12) return 0;
    return 1;
}

int main(int argc, char *argv[]){
    if(argc!=4){ 
        printf("Usage: %s A.txt b.txt out.txt\n",argv[0]); 
        return 1;
    }

    char *A_file=argv[1], *b_file=argv[2], *out_file=argv[3];

    FILE *fA=fopen(A_file,"r");
    FILE *fb=fopen(b_file,"r");
    if(!fA||!fb){ perror("File open"); return 1;}

    // Определяем размер матрицы
    int n=0;
    double tmp;
    while(fscanf(fA,"%lf",&tmp)!=EOF) n++;
    n=(int)sqrt(n);
    fseek(fA,0,SEEK_SET);

    double *A=malloc(n*n*sizeof(double));
    double *b=malloc(n*sizeof(double));
    double *x=malloc(n*sizeof(double));
    for(int i=0;i<n;i++) x[i]=0.0;

    // Чтение A и b
    for(int i=0;i<n;i++)
        for(int j=0;j<n;j++)
            fscanf(fA,"%lf",&A[i*n+j]);
    for(int i=0;i<n;i++)
        fscanf(fb,"%lf",&b[i]);
    fclose(fA); fclose(fb);

    int lower = is_lower_triangular(A,n);
    if(lower) printf("Matrix type: lower triangular\n");
    else       printf("Matrix type: upper triangular\n");

    if(lower) {
        // Решение нижней матрицы
        for(int i=0;i<n;i++){
            double sum=0.0;
            pthread_t tid;
            thread_arg_t t={i,A,x,&sum,n,1};
            pthread_create(&tid,NULL,thread_sum,&t);
            pthread_join(tid,NULL);

            if(fabs(A[i*n+i])<1e-12){ printf("Zero diagonal\n"); return 1;}
            x[i]=(b[i]-sum)/A[i*n+i];
        }
    } else {
        // Решение верхней матрицы (обратный порядок)
        for(int i=n-1;i>=0;i--){
            double sum=0.0;
            pthread_t tid;
            thread_arg_t t={i,A,x,&sum,n,0};
            pthread_create(&tid,NULL,thread_sum,&t);
            pthread_join(tid,NULL);

            if(fabs(A[i*n+i])<1e-12){ printf("Zero diagonal\n"); return 1;}
            x[i]=(b[i]-sum)/A[i*n+i];
        }
    }

    // Вывод
    printf("Solution x:\n");
    for(int i=0;i<n;i++) printf("%f ",x[i]);
    printf("\n");

    // Сохранение в файл
    FILE *fout=fopen(out_file,"w");
    for(int i=0;i<n;i++) fprintf(fout,"%f\n",x[i]);
    fclose(fout);

    free(A); free(b); free(x);
    return 0;
}
