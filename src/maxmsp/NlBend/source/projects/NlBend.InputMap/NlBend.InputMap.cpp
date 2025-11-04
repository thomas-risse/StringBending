#include "c74_min.h"
#include <Eigen/Dense>

#include "NlBendProcessor.h"

using namespace c74::min;

// Function declarations
// Multichannel handling
long ph_multichanneloutputs(c74::max::t_object *x, long index, long count);
long ph_inputchanged(c74::max::t_object *x, long index, long count);

// Main class definition
class InputMap : public object<InputMap>, public mc_operator<> {
private:
  int Nins{1}, Nouts{1};

  float sr, dt;

  float posExSmoothed{0.5}, alpha{1};
  float fcSmoothing{100.0};

public:
  MIN_DESCRIPTION{"Many to many input mapping for NlBend processor excitation."};
  MIN_TAGS{"multichannel, audio"};
  MIN_AUTHOR{"Thomas Risse"};
  MIN_RELATED{"cycle~"};

  inlet<> m_inlet{this, "(multichannelsignal) input", "multichannelsignal"};
  outlet<> m_outlet{this, "(multichannelsignal) output", "multichannelsignal"};

  attribute<number, threadsafe::no, limit::clamp> posEx { this, "posEx", 0.5,
	  range { 0.0, 1.0 }
  };

  InputMap(const atoms &args = {}) {
    if (args.size() > 0){
		  Nouts = args[0];
    }
  };

  // Number of modes
  attribute<number, threadsafe::no, limit::clamp> Nmodes { this, "Nmodes", 10,
	  range { 1, 1000 },
    setter { MIN_FUNCTION {
      if (args.size()>0){
        Nouts = args[0];  
      }
      return args;
	  }}
  };

  int getNouts(){
    return Nouts;
  }

  void setNins(int valIn){
    Nins = valIn;
  }

  message<> dspsetup { this, "dspsetup",
    MIN_FUNCTION {
      sr = args[0];
      dt = 1/sr;
      alpha = 2 * M_PI * dt * fcSmoothing / (2 * M_PI * dt * fcSmoothing + 1);
      return {};
    }
  };

  void operator()(audio_bundle input, audio_bundle output) {
    auto out = output.samples();
    auto in = input.samples();

    for (auto i = 0; i < output.frame_count(); ++i) {
      posExSmoothed = alpha * posEx + (1-alpha) * posExSmoothed;
      for (auto j = 0; j < output.channel_count(); ++j) {
        out[j][i] = sin(M_PI * float(j+1) * posExSmoothed) * in[0][i] ;
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
  return 1;
}