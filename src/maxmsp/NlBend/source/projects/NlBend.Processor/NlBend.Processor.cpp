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
class Processor : public object<Processor>, public mc_operator<> {
private:
  std::shared_ptr<NlBendProcessor<ftype>> proc;

  Eigen::Vector<ftype, -1> inVec, outVec;

  int Nmodes{1};

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
    if (args.size() > 0){
		  Nmodes = static_cast<int>(args[0]);
    }
    proc = std::make_shared<NlBendProcessor<double>>(44100, Nmodes);
    inVec = Eigen::Vector<ftype, -1>::Zero(proc->getNins());
    outVec = Eigen::Vector<ftype, -1>::Zero(proc->getNouts());
    try{
      proc->setLinearParameters(
        Eigen::Vector<ftype, -1>::Constant(Nmodes, 1),
        Eigen::Vector<ftype, -1>::Constant(Nmodes, 600),
        Eigen::Vector<ftype, -1>::Constant(Nmodes, 0.1)
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

  message<> setAmps { this, "Amps",
    MIN_FUNCTION {
      // Check size
      if (args.size() != Nmodes){
        cout << "Wrong vector size" << endl;
        return{};
      } else {
        // Convert atoms to Eigen vector
        Eigen::Vector<ftype, -1> input_vec;
        input_vec = Eigen::Vector<ftype, -1>::Zero(args.size());
        int i = 0;
        for (auto& a : args) {
            input_vec(i) = a;
            i+=1;
        }
        proc->setAmps(input_vec);
        return {};
      }
    }
  };

  message<> setFreqs { this, "Freqs",
    MIN_FUNCTION {
      // Check size
      if (args.size() != Nmodes){
        cout << "Wrong vector size" << endl;
        return{};
      } else {
        // Convert atoms to Eigen vector
        Eigen::Vector<ftype, -1> input_vec;
        input_vec = Eigen::Vector<ftype, -1>::Zero(args.size());
        int i = 0;
        for (auto& a : args) {
            input_vec(i) = a;
            i+=1;
        }
        proc->setFreqs(input_vec);
        return {};
      }
    }
  };

  message<> setDecays { this, "Decays",
    MIN_FUNCTION {
      // Check size
      if (args.size() != Nmodes){
        cout << "Wrong vector size" << endl;
        return{};
      } else {
        // Convert atoms to Eigen vector
        Eigen::Vector<ftype, -1> input_vec;
        input_vec = Eigen::Vector<ftype, -1>::Zero(args.size());
        int i = 0;
        for (auto& a : args) {
            input_vec(i) = a;
            i+=1;
        }
        proc->setDecays(input_vec);
        return {};
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
  };
}; // Processor

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
  return 1 ;
}