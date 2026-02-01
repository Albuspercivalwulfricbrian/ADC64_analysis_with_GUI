#include "PolinomRoots.h"
#include <cmath>

//*************************************************************************
double polinom(int n, double x, double *k)
{
    double s = 1;
    for (int i = n - 1; i >= 0; i--)
        s = s * x + k[i];
    return s;
} // polinom

double dihot(int degree, double edgeNegativ, double edgePositiv, double *kf)
{
    for (;;)
    {
        double x = 0.5 * (edgeNegativ + edgePositiv);
        if (x == edgeNegativ || x == edgePositiv)
            return x;
        if (polinom(degree, x, kf) < 0)
            edgeNegativ = x;
        else
            edgePositiv = x;
    }
} // dihot

void stepUp(
    int level,
    double **A,
    double **B,
    int *currentRootsCount)
{
    double major = 0;
    for (int i = 0; i < level; i++)
    {
        double s = fabs(A[level][i]);
        if (s > major)
            major = s;
    }
    major += 1.0;

    currentRootsCount[level] = 0;

    for (int i = 0; i <= currentRootsCount[level - 1]; i++)
    {
        int signLeft, signRight;
        double edgeLeft, edgeRight;
        double edgeNegativ, edgePositiv;

        if (i == 0)
            edgeLeft = -major;
        else
            edgeLeft = B[level - 1][i - 1];

        double rb = polinom(level, edgeLeft, A[level]);

        if (rb == 0)
        {
            B[level][currentRootsCount[level]] = edgeLeft;
            currentRootsCount[level]++;
            continue;
        }

        if (rb > 0)
            signLeft = 1;
        else
            signLeft = -1;

        if (i == currentRootsCount[level - 1])
            edgeRight = major;
        else
            edgeRight = B[level - 1][i];

        rb = polinom(level, edgeRight, A[level]);

        if (rb == 0)
        {
            B[level][currentRootsCount[level]] = edgeRight;
            currentRootsCount[level]++;
            continue;
        }

        if (rb > 0)
            signRight = 1;
        else
            signRight = -1;
        if (signLeft == signRight)
            continue;

        if (signLeft < 0)
        {
            edgeNegativ = edgeLeft;
            edgePositiv = edgeRight;
        }
        else
        {
            edgeNegativ = edgeRight;
            edgePositiv = edgeLeft;
        }

        B[level][currentRootsCount[level]] = dihot(level, edgeNegativ, edgePositiv, A[level]);
        currentRootsCount[level]++;
    }
    return;
} // stepUp

void polynomRealRoots(
    double *rootsArray,
    int n,
    double *kf_,
    int &rootsCount)
{
    double *kf = new double[n + 1];

    double **A = new double *[n + 1];
    double **B = new double *[n + 1];

    int *currentRootsCount = new int[n + 1];

    for (int i = 1; i <= n; i++)
    {
        A[i] = new double[i];
        B[i] = new double[i];
    }

    for (int i = 0; i <= n; i++)
        kf[i] = kf_[n - i];

    for (int i = 0; i < n; i++)
        A[n][i] = kf[i] / kf[n];

    for (int i1 = n, i = n - 1; i > 0; i1 = i, i--)
    {
        for (int j1 = i, j = i - 1; j >= 0; j1 = j, j--)
        {
            A[i][j] = A[i1][j1] * j1 / i1;
        }
    }

    currentRootsCount[1] = 1;
    B[1][0] = -A[1][0];

    for (int i = 2; i <= n; i++)
        stepUp(i, A, B, currentRootsCount);

    rootsCount = currentRootsCount[n];
    for (int i = 0; i < rootsCount; i++)
        rootsArray[i] = B[n][i];

    // Cleanup
    for (int i = 1; i <= n; i++)
    {
        delete[] A[i];
        delete[] B[i];
    }
    delete[] A;
    delete[] B;
    delete[] currentRootsCount;
    delete[] kf;
} // polynomRealRoots

//*************************************************************************