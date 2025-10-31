#include "NlBendProcessor.h"
#include "iostream"

template <class T>
NlBendProcessor<T>::NlBendProcessor(float sampleRate, int Nmodes){
  
  this->Nmodes = Nmodes;
  this->Nins = Nmodes;
  this->Nouts = Nmodes;


  ReinitDsp(sampleRate);
};

template <class T>
void NlBendProcessor<T>::ReinitDsp(float sampleRate){
  sr = sampleRate;
  dt = 1 / (float(sr));

  // Reinit state
  qnow = Eigen::Vector<T, -1>::Zero(Nmodes);
  qnext = qnow;
  qlast = qnow;
  qspat = qnow;

  r = 0;

  RHS = LHS = qnow;

  // Reinit nonlinear variables
  g = qnow;
  dqV = qnow;
  V = 0;

  // Reinit system matrices
  Eigen::Vector<T, -1> Mcopy, Kcopy, Rcopy;
  Mcopy = M;
  Kcopy = K;
  Rcopy = R;

  M = Eigen::Vector<T, -1>::Ones(Nmodes);
  K = M;
  R = M;

  if (Mcopy.size() != 0){
    int maxsize = std::max(Mcopy.size(), M.size());
    M.head(maxsize) = Mcopy.head(maxsize);
    K.head(maxsize) = Kcopy.head(maxsize);
    R.head(maxsize) = Rcopy.head(maxsize);
  }
};

template <class T>
void NlBendProcessor<T>::setModalMatrices(){
  M = Amps;
  K = Amps.cwiseProduct(Omega).cwiseProduct(Omega);
  R = 2 * 6.9 * Decays.cwiseInverse().cwiseProduct(Amps);
};

template <class T>
void NlBendProcessor<T>::setLinearParameters(Eigen::Vector<T, -1> Amps, Eigen::Vector<T, -1> Omega, Eigen::Vector<T, -1> Decays){
  if (Amps.size() != Nmodes || Omega.size() != Nmodes || Decays.size() != Nmodes) {
    throw std::invalid_argument("Input vector must have the same size as Nmodes");
  }
  this->Amps = Amps;
  // Omega is clamped to ensure stability of the scheme
  this->Omega = ClipEigen(Omega, T(0), T(2 * sr));
  this->Decays = Decays;
  setModalMatrices();
};

template <class T>
void NlBendProcessor<T>::setAmps(Eigen::Vector<T, -1> Amps){
  if (Amps.size() != Nmodes) {
    throw std::invalid_argument("Input vector must have the same size as Nmodes");
  }
  this->Amps = Amps;
  setModalMatrices();
};


template <class T>
void NlBendProcessor<T>::setFreqs(Eigen::Vector<T, -1> Freqs){
  if (Freqs.size() != Nmodes) {
    throw std::invalid_argument("Input vector must have the same size as Nmodes");
  }
  this->Omega = ClipEigen(2 * M_PI * Freqs, T(0), T(2 * sr));
  setModalMatrices();
};

template <class T>
void NlBendProcessor<T>::setDecays(Eigen::Vector<T, -1> Decays){
  if (Decays.size() != Nmodes) {
    throw std::invalid_argument("Input vector must have the same size as Nmodes");
  }
  this->Decays = Decays;
  setModalMatrices();
};

template <class T>
void NlBendProcessor<T>::computeVAndVprime(){
  switch (nlMode){
    case MODEWISE:
      V = qnow.array().pow(4).sum() / 4;
      dqV = qnow.array().pow(3).matrix();
    case SUM:
      V = pow(qnow.sum(), 4) / 4;
      dqV = pow(qnow.sum(), 3) * Eigen::VectorX<T>::Ones(Nmodes);
  };
};

template <class T>
void NlBendProcessor<T>::computeV(){
  switch (nlMode){
    case MODEWISE:
      V = 1e5* ((qnow+qlast)/2).array().pow(4).sum() / 4;
    case SUM:
      V = pow(qnow.sum(), 4) / 4;
  };
};

template <class T>
void NlBendProcessor<T>::process(Eigen::Ref<const Eigen::Vector<T, -1>> input, Eigen::Ref<Eigen::Vector<T, -1>> out){
  // Nonlinear part
  computeVAndVprime();
  g = dqV / (sqrt(2*V) + NUM_EPS);

  if (controlTerm){
    computeV();
    epsilon = r - sqrt(2*V);
    g -= lambda0 * epsilon * dt * ((qnow-qlast).array()>0).select(Eigen::Vector<T, -1>::Ones(Nmodes), -Eigen::Vector<T, -1>::Ones(Nmodes)) / ((qnow-qlast).template lpNorm<1>() + NUM_EPS);
  }

  // Linear part
  RHS = (- K * dt * dt + 2 * M).cwiseProduct(qnow) 
    - (M - R * dt / 2).cwiseProduct(qlast)
    + input * dt;
  LHS = M + R * dt / 2;

  // Nonlinear part
  RHS += pow(dt, 2) * (0.25 * g * g.dot(qlast) - g * r);

  // Solve using Shermann-Morrisson
  auto inter = (M + dt * R/2).cwiseInverse(); // TODO: Precompute (and allocate) vector
  qnext = inter.cwiseProduct(RHS) - 0.25 * pow(dt, 2) *(inter.cwiseProduct(g) * inter.cwiseProduct(g).dot(RHS)) 
    / (1 + 0.25 * pow(dt, 2) * inter.cwiseProduct(g).dot(g));
  
  r = r + 0.5 * g.dot(qnext - qlast);

  qlast = qnow;
  qnow = qnext;

  out = qnow;
};

template class NlBendProcessor<double>;
template class NlBendProcessor<float>;