#ifndef POLINOMROOTS_H
#define POLINOMROOTS_H

double polinom(int n, double x, double *k);

double dihot(int degree, double edgeNegativ, double edgePositiv, double *kf);

void stepUp(int level, double **A, double **B, int *currentRootsCount);

void polynomRealRoots(double *rootsArray, int n, double *kf_, int &rootsCount);

#endif // POLINOMROOTS_H