#include "c74_min.h"
#include <Eigen/Dense>

#include "NlBendProcessor.h"

using namespace c74::min;

// Function declarations
// Multichannel handling
long ph_multichanneloutputs(c74::max::t_object *x, long index, long count);
long ph_inputchanged(c74::max::t_object *x, long index, long count);

// Main class definition
class OutputMap : public object<OutputMap>, public mc_operator<> {
private:
  int Nins{1};

  float sr, dt;

  float posListSmoothed{0.5}, alpha{1};
  float fcSmoothing{100.0};

public:
  MIN_DESCRIPTION{"Many to many output mapping for NlBend processor listening."};
  MIN_TAGS{"multichannel, audio"};
  MIN_AUTHOR{"Thomas Risse"};
  MIN_RELATED{"cycle~"};

  inlet<> m_inlet{this, "(multichannelsignal) input", "multichannelsignal"};
  outlet<> m_outlet{this, "(multichannelsignal) output", "multichannelsignal"};

  attribute<number, threadsafe::no, limit::clamp> posList { this, "posList", 0.5,
	  range { 0.0, 1.0 }
  };

  OutputMap(const atoms &args = {}) {
    if (args.size() > 0){
      Nouts = args[0];
    }
  };

  // Number of modes
  attribute<number, threadsafe::no, limit::clamp> Nouts { this, "Nouts", 1,
	  range { 1, 1000 }
  };

  int getNouts(){
    return Nouts;
  }

  void setNins(int valIn){
    Nins = valIn;
  }

  void setNouts(int valOut){
    Nouts = valOut;
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
      posListSmoothed = alpha * posList + (1-alpha) * posListSmoothed;
      for (auto j = 0; j < output.channel_count(); ++j) {
        out[j][i] = 0;
        for (auto k = 0; k < input.channel_count(); ++k) {
          out[j][i] += sin(M_PI * float(k+1) * posListSmoothed) * in[k][i] ;
        }
      }
    }
  };

  // Allow setting the number of output channels independently
  message<> setNoutsMsg { this, "Nouts",
    MIN_FUNCTION {
        if (args.size() >= 1) {
          int val = static_cast<int>(args[0]);
          if (val < 1) val = 1;
          setNouts(val);
        }
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

MIN_EXTERNAL(OutputMap);

// Function definitions
// Multichannel handling
long ph_multichanneloutputs(c74::max::t_object *x, long index, long count) {
  minwrap<OutputMap> *ob = (minwrap<OutputMap> *)(x);
  // Return the object's configured number of output channels (independent of input)
  if (ob->m_min_object.Nouts < 1) ob->m_min_object.Nouts = 1;
  return ob->m_min_object.Nouts;
  
}

long ph_inputchanged(c74::max::t_object *x, long index, long count) {
  minwrap<OutputMap> *ob = (minwrap<OutputMap> *)(x);
  ob->m_min_object.setNins(count);
  // Return 1 to indicate acceptance of new input channel count
  return 1;
}