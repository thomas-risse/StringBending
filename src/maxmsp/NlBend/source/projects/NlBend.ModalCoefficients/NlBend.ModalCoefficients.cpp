#include "c74_min.h"

#include "ModalCoefficients.h"

using namespace c74::min;


class ModalCoefficients : public object<ModalCoefficients> {
private:
  c74::min::atoms Ans, Fns, T60s;

public:
  outlet<>	outputAn	{ this, "(list) Modal amplitudes" };
  outlet<>	outputFn	{ this, "(list) Modal frequencies" };
  outlet<>	outputT60	{ this, "(list) Modal decay times" };


	ModalCoefficients(const atoms& args = {}) {
    if (args.size()>0){
      Nmodes = args[0];
    } else {
      Nmodes = 10;
    }
  }

  // A set of high level parameters
  attribute<number, threadsafe::no, limit::clamp> f0 { this, "f0", 440,
	  range { 1e-3, 10000 }
  };

  attribute<number, threadsafe::no, limit::clamp> beta { this, "beta", 0.0,
	  range { -1.0, 5.0 }
  };

  attribute<number, threadsafe::no, limit::clamp> T60_0 { this, "T60_0", 5.0,
	  range { 0.0, 100000.0 }
  };

  attribute<number, threadsafe::no, limit::clamp> b1 { this, "b1", 1e-3,
	  range { 0.0, 1.0 }
  };

  // Number of modes
  attribute<number, threadsafe::no, limit::clamp> Nmodes { this, "Nmodes", 10,
	  range { 1, 1000 },
    setter { MIN_FUNCTION {
      if (args.size()>0){
        int N = args[0];
        // Resize vectors and initialize with default values
        Ans.resize(N, 1.0);
        Fns.resize(N);
        frequenciesInharmonic(Fns, f0, beta);
        T60s.resize(N);
        T60sQuadraticLoss(T60s, Fns, float(f0), float(T60_0), float(b1));
      }
      return args;
	  }}
  };

  message<> bang { this, "bang", "Outputs modal coefficients.",
	MIN_FUNCTION {
    frequenciesInharmonic(Fns, f0, beta);
    T60sQuadraticLoss(T60s, Fns, float(f0), float(T60_0), float(b1));
    outputAn.send(Ans);
    outputFn.send(Fns);
    outputT60.send(T60s);
		return {};
    }
  };

};

MIN_EXTERNAL(ModalCoefficients);
