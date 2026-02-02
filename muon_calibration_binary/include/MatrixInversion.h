#ifndef MATRIXINVERSION_H
#define MATRIXINVERSION_H

#include <complex>

// calculate the cofactor of element (row,col)
int GetMinor(std::complex<float> **src, std::complex<float> **dest, int row, int col, int order);

std::complex<float> CalcDeterminant(std::complex<float> **mat, int order);

void MatrixInversion(std::complex<float> **A, int order, std::complex<float> **Y);

#endif // MATRIXINVERSION_H