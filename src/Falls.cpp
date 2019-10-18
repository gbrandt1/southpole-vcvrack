#include "Southpole.hpp"
#include <string.h>

#define NUMP 6

struct Falls : Module {
  enum ParamIds {
    RANGE_PARAM,
    GAIN1_PARAM,
    GAIN2_PARAM,
    GAIN3_PARAM,
    GAIN4_PARAM,
    GAIN5_PARAM,
    GAIN6_PARAM,
    GAIN7_PARAM,
    GAIN8_PARAM,
    NUM_PARAMS
  };
  enum InputIds {
    IN1_INPUT,
    IN2_INPUT,
    IN3_INPUT,
    IN4_INPUT,
    IN5_INPUT,
    IN6_INPUT,
    IN7_INPUT,
    IN8_INPUT,
    NUM_INPUTS
  };
  enum OutputIds {
    OUT1_OUTPUT,
    OUT2_OUTPUT,
    OUT3_OUTPUT,
    OUT4_OUTPUT,
    OUT5_OUTPUT,
    OUT6_OUTPUT,
    OUT7_OUTPUT,
    OUT8_OUTPUT,
    NUM_OUTPUTS
  };
  enum LightIds {
    OUT1_POS_LIGHT,
    OUT1_NEG_LIGHT,
    OUT2_POS_LIGHT,
    OUT2_NEG_LIGHT,
    OUT3_POS_LIGHT,
    OUT3_NEG_LIGHT,
    OUT4_POS_LIGHT,
    OUT4_NEG_LIGHT,
    OUT5_POS_LIGHT,
    OUT5_NEG_LIGHT,
    OUT6_POS_LIGHT,
    OUT6_NEG_LIGHT,
    OUT7_POS_LIGHT,
    OUT7_NEG_LIGHT,
    OUT8_POS_LIGHT,
    OUT8_NEG_LIGHT,
    NUM_LIGHTS
  };

  Falls() {
    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
    for (int i = 0; i < NUMP; i++)
      configParam(Falls::GAIN1_PARAM + i, -1.0, 1.0, 0.0, "");
    configParam(Falls::RANGE_PARAM, 0.0, 1.0, 0.0, "");
  }
  void process(const ProcessArgs &args) override;
};

void Falls::process(const ProcessArgs &args) {

  float range = params[RANGE_PARAM].getValue() > 0.5 ? 10. : 1.;

  float out = 0.0;

  for (int i = 0; i < NUMP; i++) {
    float g = params[GAIN1_PARAM + i].getValue() * range;
    g = clamp(g, -range, range);
    //if (inputs[IN1_INPUT + i].isConnected()) {
    out += g * inputs[IN1_INPUT + i].getNormalVoltage(1.);
    //} else {
    //    out += g;
    //}
    lights[OUT1_POS_LIGHT + 2 * i].setSmoothBrightness(fmaxf(0.0, out / 5.0), args.sampleTime);
    lights[OUT1_NEG_LIGHT + 2 * i].setSmoothBrightness(fmaxf(0.0, -out / 5.0), args.sampleTime);
    if (outputs[OUT1_OUTPUT + i].isConnected()) {
      outputs[OUT1_OUTPUT + i].setVoltage(out);
      out = 0.0;
    }
  }
}

struct FallsWidget : ModuleWidget {

  FallsWidget(Falls *module) {
    setModule(module);

    box.size = Vec(15 * 4, 380);

    setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Falls.svg")));

    const float y1 = 32;
    const float yh = 49;

    const float x1 = 4.;
    const float x2 = 20.;
    const float x3 = 36.;

    for (int i = 0; i < NUMP; i++) {
      addInput(createInput<sp_Port>(Vec(x1, y1 + i * yh), module, Falls::IN1_INPUT + i));
      addChild(createLight<MediumLight<GreenRedLight>>(Vec(x2 + 6, y1 + i * yh), module, Falls::OUT1_POS_LIGHT + 2 * i));
      addOutput(createOutput<sp_Port>(Vec(x3, y1 + i * yh), module, Falls::OUT1_OUTPUT + i));
      addParam(createParam<sp_SmallBlackKnob>(Vec(x2, y1 + i * yh + 18), module, Falls::GAIN1_PARAM + i));
    }

    addParam(createParam<sp_Switch>(Vec(x2, y1 + NUMP * yh), module, Falls::RANGE_PARAM));
  }
};

Model *modelFalls = createModel<Falls, FallsWidget>("Falls");
