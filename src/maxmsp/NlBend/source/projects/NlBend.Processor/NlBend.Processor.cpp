#include "c74_min.h"
#include <Eigen/Dense>
#include <atomic>
#include <dlfcn.h>
#include <memory>

#include "NlBendProcessor.h"

using namespace c74::min;

using ftype = double;
// Function declarations
// Multichannel handling
long ph_multichanneloutputs(c74::max::t_object *x, long index, long count);
long ph_inputchanged(c74::max::t_object *x, long index, long count);

// Main class definition
class Processor : public object<Processor>, public vector_operator<> {
private:
  std::shared_ptr<NlBendProcessor<ftype>> proc;

  Eigen::Vector<ftype, -1> inVec, outVec;

  float sr;

public:
  MIN_DESCRIPTION{"Nonlinear model bending audio processor"};
  MIN_TAGS{"multichannel, audio, physical modelling, nonlinear"};
  MIN_AUTHOR{"Thomas Risse"};
  MIN_RELATED{"cycle~"};

  inlet<> m_inlet{this, "(multichannelsignal) input", "multichannelsignal"};
  outlet<> m_outlet{this, "(multichannelsignal) output", "multichannelsignal"};

  bool compatibleInput{false}, compatibleOutput{false};

  std::size_t inConnected{0};

  std::shared_ptr<NlBendProcessor<ftype>> getProc(){return proc;}

  Processor(const atoms &args = {}) {
    proc = std::make_shared<NlBendProcessor<double>>(44100);
    inVec = outVec = Eigen::Vector<ftype, -1>::Zero(1);
    try{
      proc->setLinearParameters(
        Eigen::Vector<ftype, -1>::Constant(1, 1),
        Eigen::Vector<ftype, -1>::Constant(1, 600),
        Eigen::Vector<ftype, -1>::Constant(1, 0.1)
      );
    } catch (const std::invalid_argument& ex) {
      cout << ex << endl;
    };
  };

  message<> dspsetup { this, "dspsetup",
    MIN_FUNCTION {
      sr = args[0];
      proc->ReinitDsp(sr);
      return {};
    }
  };

  void operator()(audio_bundle input, audio_bundle output) {
    auto out = output.samples();
    auto in = input.samples();

    for (auto i = 0; i < output.frame_count(); ++i) {
      for (auto j = 0; j < input.channel_count(); ++j) {
        inVec[j] = in[j][i];
      }
      proc->process(inVec, outVec);
      for (auto j = 0; j < output.channel_count(); ++j) {
        out[j][i] = outVec[j];
      }
    }
  };

message<> maxclass_setup{this, "maxclass_setup",
                         MIN_FUNCTION{c74::max::t_class *c = args[0];
c74::max::class_addmethod(c, (c74::max::method)ph_multichanneloutputs,
                          "multichanneloutputs", c74::max::A_CANT, 0);
c74::max::class_addmethod(c, (c74::max::method)ph_inputchanged, "inputchanged",
                          c74::max::A_CANT, 0);
return {};
}
}
;
}
; // Processor

MIN_EXTERNAL(Processor);

// Function definitions
// Multichannel handling
long ph_multichanneloutputs(c74::max::t_object *x, long index, long count) {
  minwrap<Processor> *ob = (minwrap<Processor> *)(x);
  std::size_t nout =
      ob->m_min_object.getProc()->getNouts();
  if (nout >= 1) {
    ob->m_min_object.compatibleOutput = true;
    return nout;
  } else {
    ob->m_min_object.compatibleOutput = false;
    return 1;
  }
}

long ph_inputchanged(c74::max::t_object *x, long index, long count) {
  minwrap<Processor> *ob = (minwrap<Processor> *)(x);
  ob->m_min_object.inConnected = count;
  if (count == ob->m_min_object.getProc()->getNins()) {
    ob->m_min_object.compatibleInput = true;
  } else {
    ob->m_min_object.compatibleInput = false;
  }
  return ob->m_min_object.compatibleInput;
}