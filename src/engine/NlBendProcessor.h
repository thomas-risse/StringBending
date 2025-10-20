#ifndef NL_BEND_PROCESSOR_H
#define NL_BEND_PROCESSOR_H

#include <cmath>
#include <vector>
#include <tuple>

#include <Eigen/Dense>

template <class T>
class NlBendProcessor {
    private:
        // Number of modes
        int Nmodes{1};

        // Linear part: system "matrices" (diagonal)
        Eigen::Vector<T, -1> M, K, R;

        // Nonlinear part: function parametrization: How can we do that?

        // Time-scheme parameters
        float sr;
        T dt;

        // System state (modal coordinates)
        Eigen::Vector<T, -1> qlast, qnow, qnext;
        T r;

        // Intermediate vectors
        Eigen::Vector<T, -1> RHS, LHS;

        // Displacement (spatial coordinates)
        Eigen::Vector<T, -1> qspat;

        // Nonlinear function evaluation
        Eigen::Vector<T, -1> g;
        T V;

        // Input vector
        Eigen::Vector<T, -1> Gp;

        void setModalMatrices();

        // Input and output dimensions
        int Nins{1}, Nouts{1};


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

        // Control signals
        void setControlPosition(T pos);
        
        // Control SAV parameter
        T lambda0{0};
        // Discretization parameters
        int getNmodes() {return Nmodes;};

        int getNins(){return Nins;}
        int getNouts(){return Nouts;}
};

#endif
