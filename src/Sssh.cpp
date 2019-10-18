#include "Southpole.hpp"

struct Sssh : Module {
  enum ParamIds {
    NUM_PARAMS
  };
  enum InputIds {
    SH1_INPUT,
    SH2_INPUT,
    SH3_INPUT,
    SH4_INPUT,
    TRIG1_INPUT,
    TRIG2_INPUT,
    TRIG3_INPUT,
    TRIG4_INPUT,
    NUM_INPUTS
  };
  enum OutputIds {
    NOISE1_OUTPUT,
    NOISE2_OUTPUT,
    NOISE3_OUTPUT,
    NOISE4_OUTPUT,
    SH1_OUTPUT,
    SH2_OUTPUT,
    SH3_OUTPUT,
    SH4_OUTPUT,
    NUM_OUTPUTS
  };
  enum LightIds {
    SH_POS1_LIGHT,
    SH_NEG1_LIGHT,
    SH_POS2_LIGHT,
    SH_NEG2_LIGHT,
    SH_POS3_LIGHT,
    SH_NEG3_LIGHT,
    SH_POS4_LIGHT,
    SH_NEG4_LIGHT,
    NUM_LIGHTS
  };

  dsp::SchmittTrigger trigger[4];
  float sample[4] = {0.0, 0., 0., 0.};

  Sssh() {
    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
    //trigger.setThresholds(0.0, 0.7);
  }
  void onReset() override {

    for (unsigned int i = 0; i < 4; i++)
      sample[i] = 0.;
  }

  void process(const ProcessArgs &args) override;
};

void Sssh::process(const ProcessArgs &args) {

  float trig[4];

  for (unsigned int i = 0; i < 4; i++) {

    // Gaussian noise generator
    // TO DO: check correlation between calls
    float noise = 5.0 * random::normal();

    if (i == 0) {
      trig[0] = inputs[TRIG1_INPUT].getNormalVoltage(0);
    } else {
      trig[i] = inputs[TRIG1_INPUT + i].getNormalVoltage(trig[i - 1]);
    }

    if (trigger[i].process(trig[i])) {
      sample[i] = inputs[SH1_INPUT + i].getNormalVoltage(noise);
    }

    // lights
    lights[SH_POS1_LIGHT + 2 * i].setBrightness(fmaxf(0.0, sample[i] / 5.0));
    lights[SH_NEG1_LIGHT + 2 * i].setBrightness(fmaxf(0.0, -sample[i] / 5.0));

    // outputs
    outputs[NOISE1_OUTPUT + i].setVoltage(noise);
    outputs[SH1_OUTPUT + i].setVoltage(sample[i]);
  }
}

struct SsshWidget : ModuleWidget {

  SsshWidget(Sssh *module) {
    setModule(module);

    box.size = Vec(15 * 4, 380);

    {
      SvgPanel *panel = new SvgPanel();
      panel->box.size = box.size;
      panel->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Sssh.svg")));
      addChild(panel);
    }

    float y1 = 50;
    float yh = 80;

    for (unsigned int i = 0; i < 4; i++) {
      addInput(createInput<sp_Port>(Vec(5, y1 + i * yh), module, Sssh::SH1_INPUT + i));
      addInput(createInput<sp_Port>(Vec(34, y1 + i * yh), module, Sssh::TRIG1_INPUT + i));
      addOutput(createOutput<sp_Port>(Vec(5, 35 + y1 + i * yh), module, Sssh::NOISE1_OUTPUT + i));
      addOutput(createOutput<sp_Port>(Vec(34, 35 + y1 + i * yh), module, Sssh::SH1_OUTPUT + i));

      addChild(createLight<SmallLight<GreenRedLight>>(Vec(26, y1 + i * yh - 4), module, Sssh::SH_POS1_LIGHT + 2 * i));
    }
  }
};

Model *modelSssh = createModel<Sssh, SsshWidget>("Sssh");
