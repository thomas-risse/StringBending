#ifndef NL_BEND_PROCESSOR_H
#define NL_BEND_PROCESSOR_H

#include <cmath>
#include <vector>
#include <tuple>

#include <Eigen/Dense>

#include "EigenUtility.h"

enum NLMODE {
    MODEWISE,
    SUM
};

template <class T>
class NlBendProcessor {
    private:
        // Numerical epsilon value
        constexpr static T NUM_EPS{1e-12};
        // Number of modes
        int Nmodes{1};

        // Linear part: system "matrices" (diagonal)
        Eigen::Vector<T, -1> M, K, R;

        // Nonlinear part: function parametrization: 
        // How can we do that?
        NLMODE nlMode = MODEWISE;


        // Time-scheme parameters
        float sr;
        T dt;
        bool controlTerm{true};
        T lambda0{0};

        // System state (modal coordinates)
        Eigen::Vector<T, -1> qlast, qnow, qnext;
        T r;

        // Intermediate vectors
        Eigen::Vector<T, -1> RHS, LHS;

        // Displacement (spatial coordinates)
        Eigen::Vector<T, -1> qspat;

        // Nonlinear function evaluation
        Eigen::Vector<T, -1> g, dqV;
        T V;

        // Drift variable
        T epsilon;

        // Input and output dimensions
        int Nins{1}, Nouts{1};
        
        void setModalMatrices();

    public:
        NlBendProcessor(float sampleRate, int Nmodes = 1);

        void ReinitDsp(float sampleRate);

        void computeVAndVprime();

        void computeV();

        void process(Eigen::Ref<const Eigen::Vector<T, -1>> input, Eigen::Ref<Eigen::Vector<T, -1>> out);

        // Higher level modal parameters
        Eigen::Vector<T, -1> Amps, Omega, Decays;
        void setLinearParameters (Eigen::Vector<T, -1> Amps, Eigen::Vector<T, -1> Omega, Eigen::Vector<T, -1> Decays);
        void setAmps (Eigen::Vector<T, -1> Amps);
        void setFreqs (Eigen::Vector<T, -1> Freqs);
        void setDecays (Eigen::Vector<T, -1> Decays);

        T getOmega(int i){
            return this->K(i);
        }

        // Discretization parameters
        int getNmodes() {return Nmodes;};

        int getNins(){return Nins;}
        int getNouts(){return Nouts;}
};

#endif
