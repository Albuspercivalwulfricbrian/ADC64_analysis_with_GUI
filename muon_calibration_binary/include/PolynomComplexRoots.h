/* Copyright (C) 2020 Facility for Antiproton and Ion Research in Europe,
   Darmstadt SPDX-License-Identifier: GPL-3.0-only
   Authors: Pierre-Alain Loizeau [committer] */

#ifndef POLYNOMCOMPLEXROOTS_H
#define POLYNOMCOMPLEXROOTS_H

#include <cstddef>

// Forward declarations for internal helper functions
int roots(float *a, int n, float *wr, float *wi);
void deflate(float *a, int n, float *b, float *quad, float *err);
void find_quad(float *a, int n, float *b, float *quad, float *err, int *iter);
void diff_poly(float *a, int n, float *b);
void recurse(float *a, int n, float *b, int m, float *quad, float *err, int *iter);
void get_quads(float *a, int n, float *quad, float *x);

/**
 * @brief Calculate complex roots of a polynomial
 *
 * @param wr Array to store real parts of roots
 * @param wi Array to store imaginary parts of roots
 * @param n Degree of polynomial
 * @param a Polynomial coefficients (a[0] to a[n])
 * @param numr Number of roots found (output)
 */
void polynomComplexRoots(float *wr, float *wi, int n, float *a, int &numr);

#endif // POLYNOMCOMPLEXROOTS_H