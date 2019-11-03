#include "rack.hpp"

using namespace rack;

extern Plugin *pluginInstance;

extern Model *modelSmoke;
extern Model *modelSplash;

// GUI COMPONENTS

struct sp_Port : app::SvgPort {
  sp_Port() {
    setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/sp-Port20.svg")));
  }
};

struct sp_BlackKnob : app::SvgKnob {
  sp_BlackKnob() {
    minAngle = -0.83 * M_PI;
    maxAngle = 0.83 * M_PI;
    setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/sp-knobBlack.svg")));
  }
};

struct sp_SmallBlackKnob : app::SvgKnob {
  sp_SmallBlackKnob() {
    minAngle = -0.83 * M_PI;
    maxAngle = 0.83 * M_PI;
    setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/sp-knobBlack.svg")));
  }
};

struct sp_Trimpot : app::SvgKnob {
  sp_Trimpot() {
    minAngle = -0.83 * M_PI;
    maxAngle = 0.83 * M_PI;
    setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/sp-trimpotBlack.svg")));
  }
};
