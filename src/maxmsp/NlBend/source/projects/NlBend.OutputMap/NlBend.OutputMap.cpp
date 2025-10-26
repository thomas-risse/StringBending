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
class OutputMap : public object<OutputMap>, public mc_operator<> {
private:
  int Nins{1}, Nouts{1};

  float sr;

  float posList{0.5};

public:
  MIN_DESCRIPTION{"Many to many output mapping for NlBend processor listening."};
  MIN_TAGS{"multichannel, audio"};
  MIN_AUTHOR{"Thomas Risse"};
  MIN_RELATED{"cycle~"};

  inlet<> m_inlet{this, "(multichannelsignal) input", "multichannelsignal"};
  outlet<> m_outlet{this, "(multichannelsignal) output", "multichannelsignal"};

  bool compatibleInput{false}, compatibleOutput{false};

  std::size_t inConnected{0};

  OutputMap(const atoms &args = {}) {
    if (args.size() > 0){
		  Nins = args[0];
    }
    if (args.size() > 1){
      Nouts = args[1];
    }
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
      return {};
    }
  };

  void operator()(audio_bundle input, audio_bundle output) {
    auto out = output.samples();
    auto in = input.samples();

    for (auto i = 0; i < output.frame_count(); ++i) {
      for (auto j = 0; j < output.channel_count(); ++j) {
        out[j][i] = 0;
        for (auto k = 0; k < input.channel_count(); ++k) {
          out[j][i] += sin(M_PI * float(k+1) * posList) * in[k][i] ;
        }
      }
    }
  };

  message<> setExPos { this, "PosList",
    MIN_FUNCTION {
        posList = args[0];
        return {};
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
  if (ob->m_min_object.getNouts() < 1) ob->m_min_object.setNouts(1);
  ob->m_min_object.compatibleOutput = true;
  return ob->m_min_object.getNouts();
  
}

long ph_inputchanged(c74::max::t_object *x, long index, long count) {
  minwrap<OutputMap> *ob = (minwrap<OutputMap> *)(x);
  ob->m_min_object.setNins(count);
  ob->m_min_object.compatibleInput = true;
  // Return 1 to indicate acceptance of new input channel count
  return 1;
}