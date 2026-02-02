#ifndef POLYNOMREALROOTS_H
#define POLYNOMREALROOTS_H

/**
 * @brief Calculate real roots of a polynomial
 *
 * @param rootsArray Array to store real roots
 * @param n Degree of polynomial
 * @param kf_ Polynomial coefficients (kf_[0] to kf_[n])
 * @param rootsCount Number of real roots found (output)
 */
void polynomRealRoots(float *rootsArray, int n, float *kf_, int &rootsCount);

#endif // POLYNOMREALROOTS_H