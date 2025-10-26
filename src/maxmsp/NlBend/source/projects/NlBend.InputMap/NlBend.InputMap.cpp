#include "c74_min.h"
#include <Eigen/Dense>

#include "NlBendProcessor.h"

using namespace c74::min;

using ftype = double;
// Function declarations
// Multichannel handling
long ph_multichanneloutputs(c74::max::t_object *x, long index, long count);
long ph_inputchanged(c74::max::t_object *x, long index, long count);

// Main class definition
class InputMap : public object<InputMap>, public mc_operator<> {
private:
  int Nins{1}, Nouts{1};

  float sr;

  float posEx{0.5};

public:
  MIN_DESCRIPTION{"Many to many input mapping for NlBend processor excitation."};
  MIN_TAGS{"multichannel, audio"};
  MIN_AUTHOR{"Thomas Risse"};
  MIN_RELATED{"cycle~"};

  inlet<> m_inlet{this, "(multichannelsignal) input", "multichannelsignal"};
  outlet<> m_outlet{this, "(multichannelsignal) output", "multichannelsignal"};

  bool compatibleInput{false}, compatibleOutput{false};

  std::size_t inConnected{0};

  InputMap(const atoms &args = {}) {
    if (args.size() > 0){
		  Nouts = args[0];
    }
  };

  int getNouts(){
    return Nouts;
  }

  void setNins(int valIn){
    Nins = valIn;
  }

  message<> dspsetup { this, "dspsetup",
    MIN_FUNCTION {
      return {};
    }
  };

  void operator()(audio_bundle input, audio_bundle output) {
    auto out = output.samples();
    auto in = input.samples();

    for (auto i = 0; i < output.frame_count(); ++i) {
      for (auto j = 0; j < output.channel_count(); ++j) {
        out[j][i] = sin(M_PI * float(j+1) * posEx) * in[0][i] ;
      }
    }
  };

  message<> setExPos { this, "PosEx",
    MIN_FUNCTION {
        posEx = args[0];
        return {};
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

MIN_EXTERNAL(InputMap);

// Function definitions
// Multichannel handling
long ph_multichanneloutputs(c74::max::t_object *x, long index, long count) {
  minwrap<InputMap> *ob = (minwrap<InputMap> *)(x);
  return ob->m_min_object.getNouts();
  
}

long ph_inputchanged(c74::max::t_object *x, long index, long count) {
  minwrap<InputMap> *ob = (minwrap<InputMap> *)(x);
  ob->m_min_object.setNins(count);
  ob->m_min_object.compatibleInput = true;
  return ob->m_min_object.compatibleInput;
}