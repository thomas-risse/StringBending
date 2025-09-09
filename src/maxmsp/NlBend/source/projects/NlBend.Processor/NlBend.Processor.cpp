#include "c74_min.h"
#include "NlBendProcessor.h"
#include <atomic>
#include <memory>
using namespace c74::min;


class Processor : public object<Processor>, public sample_operator <1, 1> {
private:
  std::shared_ptr<NlBendProcessor<double>> proc;

public:
  MIN_DESCRIPTION	{"Audio processor for nonlinear model bending."};
  MIN_TAGS		{"audio"};
  MIN_AUTHOR		{"Thomas Risse"};

  inlet<> input { this, "(signal) excitation"};

  Processor(const atom& args = {}) {
      proc = std::make_shared<NlBendProcessor<double>>(44100);
  }

  message<> dspsetup { this, "dspsetup",
    MIN_FUNCTION {
      //number samplerate = args[0];
      //int vectorsize = args[1];
      sr = args[0];
      cout << "sr =" << sr << endl;
      proc->reinitDsp(sr);
      return {};
    }
  };

  samples<1> operator()(sample input) {
      auto [out] = proc->process(double(input));
      return {{out}};
  }
};


MIN_EXTERNAL(Processor);
