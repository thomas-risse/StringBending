#ifndef MODAL_COEFFICIENTS_H
#define MODAL_COEFFICIENTS_H

#include <vector>

// Set of utility functions to set modal coefficients from
// high-level parameters.

template <typename ftype, typename vecType>
void frequenciesInharmonic(
  vecType &out,
  const ftype &f0,
  const ftype &beta
){
  int i=1;
  for (auto &el: out ){
    el = f0 * i * (1 + (i-1) * beta);
    i++;
  }
}

template <typename ftype, typename vecType>
void T60sQuadraticLoss(
  vecType &out,
  const vecType &freqs,
  const ftype &f0,
  const ftype &T60_0,
  const ftype &b2 = 1e-5
){
  int i=1;
  ftype b1 = 2 * 6.9 / T60_0 - b2;
  ftype R;
  for (auto &el: out ){
    R = b1 + b2 * ftype(freqs.at(i-1)) * ftype(freqs.at(i-1)) / (f0*f0);
    el = 2 * 6.9 / R;
    i++;
  }
}


#endif // MODAL_COEFFICIENTS_H