#include <stdio.h>
#include <stdlib.h>
#include <math.h>

double **create(long long n) {
    long long i, j;
    double **matrix = malloc(n*sizeof(double*));
    if (matrix==NULL) {
        return NULL; }
    for (i=0; i<n; i++) {
        matrix[i] = malloc(n*sizeof(double));
        if (matrix[i]==NULL) {
            free(matrix);
            return NULL;
        }
    }
    return matrix;
}

void free_matrix(long long n, double **matrix) {
    long long i;
    for (i=0; i<n; i++) {
        free(matrix[i]);
    }
    free(matrix);
    return;
}

int check(long long n, double**matrix) {
    long long i, j, rang=0;
    int f=0;
    for (i=0; i<n; i++) {
        for (j=0; j<n; j++) {
            if (f==0 && matrix[i][j]!=0) {
                rang++;
                f = 1;
            }
        }
        f = 0;
    }
    if (rang==n) {
        return 0;
    }
    return 1;

}

void permutation(long long n, double *line1, double *line2) {
    double *tmp = malloc(n*sizeof(double));
    long long i;
    for (i=0; i<n; i++) {
        tmp[i] = line1[i];
        line1[i] = line2[i];
        line2[i] = tmp[i];
    }
    free(tmp);
    return;
}

double Gauss_method(long long n, double **matrix) {
    double det = 1, number;
    long long i, j, k, leading_el;
    for (i=0; i<n; i++) {
        leading_el = i;
        for (j=i+1; j<n; j++) {
            if (fabs(matrix[leading_el][i]) < fabs(matrix[j][i])) {
                leading_el = j;
            }
        }
        if (matrix[leading_el][i]==0) {
            return 0;
        }
        if (leading_el!=i) {
            permutation(n, matrix[leading_el], matrix[i]);
            det *= -1;
        }
        for (j=i+1; j<n; j++) {
            number = -1*matrix[j][i]/matrix[i][i];
            for (k=i; k<n; k++) {
                matrix[j][k] += matrix[i][k]*number;
            }
        }
    }
    for (i=0; i<n; i++) {
        det *= matrix[i][i];
    }
    return det;
}

double find_determinant(long long n, double **matrix) {
    if (check(n, matrix)) {
        return 0;
    }
    double det = Gauss_method(n, matrix);
    return det;
}

int main(void) {
    long long n, i, j;
    scanf("%lld", &n);
    if (n<=0) {
        printf("Размер матрицы должен быть положительным");
        return 0;
    }
    double **matrix = create(n);
    for (i=0; i<n; i++) {
        for (j=0; j<n; j++) {
            scanf("%lf", &matrix[i][j]);
        }
    }
    double determinant = find_determinant(n, matrix);
    printf("%lf", determinant);
    free_matrix(n, matrix);
    return 0;
}
