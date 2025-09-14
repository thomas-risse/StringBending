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
  qnow = Eigen::Vector<T, 1>::Zero(Nmodes);
  qnext = qnow;
  qlast = qnow;
  qspat = qnow;

  r = 0;

  RHS = LHS = qnow;
  Gp = qnow;
  Gp(0) = 1;

  // Reinit g vector
  g = qnow;

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
  K = Amps * Omega * Omega;
  R = 2 * 6.9 * Decays.cwiseInverse() * Amps;
};

template <class T>
void NlBendProcessor<T>::setLinearParameters(Eigen::Vector<T, -1> Amps, Eigen::Vector<T, -1> Omega, Eigen::Vector<T, -1> Decays){
  if (Amps.size() != Nmodes || Omega.size() != Nmodes || Decays.size() != Nmodes) {
    throw std::invalid_argument("Input vectors must have the same size as Nmodes");
  }
  this->Amps = Amps;
  this->Omega = Omega;
  this->Decays = Decays;
  setModalMatrices();
};

template <class T>
void NlBendProcessor<T>::setAmps(Eigen::Vector<T, -1> Amps){
  if (Amps.size() != Nmodes) {
    throw std::invalid_argument("Input vectors must have the same size as Nmodes");
  }
  this->Amps = Amps;
  setModalMatrices();
};


template <class T>
void NlBendProcessor<T>::setFreqs(Eigen::Vector<T, -1> Freqs){
  if (Freqs.size() != Nmodes) {
    throw std::invalid_argument("Input vectors must have the same size as Nmodes");
  }
  this->Omega = Freqs * 2 * M_PI;
  setModalMatrices();
};

template <class T>
void NlBendProcessor<T>::setDecays(Eigen::Vector<T, -1> Decays){
  if (Decays.size() != Nmodes) {
    throw std::invalid_argument("Input vectors must have the same size as Nmodes");
  }
  this->Decays = Decays;
  setModalMatrices();
};

template <class T>
void NlBendProcessor<T>::computeVAndVprime(){


};

template <class T>
void NlBendProcessor<T>::computeV(){


};

template <class T>
void NlBendProcessor<T>::process(Eigen::Ref<const Eigen::Vector<T, -1>> input, Eigen::Ref<Eigen::Vector<T, -1>> out){
  // Linear part
  RHS = (- K * dt * dt + 2 * M)* qnow 
    - (M - R * dt / 2) * qlast
    + input * dt;
  LHS = M + R * dt / 2;

  qnext = (RHS.array() / LHS.array()).matrix();

  r = r + 0.5 * g.dot(qnext - qlast);

  qlast = qnow;
  qnow = qnext;

  out = qnow;
};

template <class T>
void NlBendProcessor<T>::setControlPosition(T pos){
  pos = std::clamp(pos, T(0.01), T(0.99));
  for (int i=0; i<Nmodes; i++){
    Gp(i) = std::sin(M_PI * (2 * i + 1) * pos);
  }
};

template class NlBendProcessor<double>;
template class NlBendProcessor<float>;