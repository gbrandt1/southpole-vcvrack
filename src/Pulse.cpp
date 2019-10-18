#include "Southpole.hpp"

struct Pulse : Module {
  enum ParamIds {
    TRIG_PARAM,
    REPEAT_PARAM,
    RESET_PARAM,
    RANGE_PARAM,
    DELAY_PARAM,
    TIME_PARAM,
    AMP_PARAM,
    //		OFFSET_PARAM,
    SLEW_PARAM,
    NUM_PARAMS
  };
  enum InputIds {
    TRIG_INPUT,
    CLOCK_INPUT,
    //		REPEAT_INPUT,
    //		RESET_INPUT,
    DELAY_INPUT,
    TIME_INPUT,
    AMP_INPUT,
    //		OFFSET_INPUT,
    SLEW_INPUT,
    NUM_INPUTS
  };
  enum OutputIds {
    CLOCK_OUTPUT,
    GATE_OUTPUT,
    EOC_OUTPUT,
    NUM_OUTPUTS
  };
  enum LightIds {
    EOC_LIGHT,
    GATE_LIGHT,
    NUM_LIGHTS
  };

  dsp::SchmittTrigger clock;
  dsp::SchmittTrigger trigger;
  dsp::SchmittTrigger triggerBtn;
  dsp::PulseGenerator clkPulse;
  dsp::PulseGenerator eocPulse;

  unsigned long delayt = 0;
  unsigned long gatet = 0;
  unsigned long clockt = 0;
  unsigned long clockp = 0;

  unsigned long delayTarget = 0;
  unsigned long gateTarget = 0;

  float level = 0;

  bool reset = true;
  bool repeat = false;
  bool range = false;
  bool gateOn = false;
  bool delayOn = false;

  float amp;
  float slew;

  static const int ndurations = 12;
  const float durations[ndurations] = {
      1 / 256., 1 / 128., 1 / 64., 1 / 32., 1 / 16., 1 / 8., 3. / 16., 1 / 4., 1 / 3., 1 / 2., 3. / 4., .99
      //,2.,3.,4. //,5.,6.,7.,8.,12.,16.
  };

  Pulse() {
    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

    configParam(Pulse::TRIG_PARAM, 0.0, 1.0, 0., "");
    configParam(Pulse::RESET_PARAM, 0.0, 1.0, 0.0, "");
    configParam(Pulse::REPEAT_PARAM, 0.0, 1.0, 0.0, "");
    configParam(Pulse::RANGE_PARAM, 0.0, 1.0, 0.0, "");
    configParam(Pulse::TIME_PARAM, 0.0, 1.0, 0.0, "");
    configParam(Pulse::DELAY_PARAM, 0.0, 1.0, 0.0, "");
    configParam(Pulse::AMP_PARAM, 0.0, 1.0, 1.0, "");
    configParam(Pulse::SLEW_PARAM, 0.0, 1.0, 0., "");
  }

  void process(const ProcessArgs &args) override;
};

void Pulse::process(const ProcessArgs &args) {

  bool triggered = false;

  reset = params[RESET_PARAM].getValue();
  repeat = params[REPEAT_PARAM].getValue();
  range = params[RANGE_PARAM].getValue();

  if (triggerBtn.process(params[TRIG_PARAM].getValue())) {
    triggered = true;
  }

  if (trigger.process(inputs[TRIG_INPUT].getNormalVoltage(0.))) {
    triggered = true;
    //printf("%lu\n", gateTarget);
  }

  if (clock.process(inputs[CLOCK_INPUT].getNormalVoltage(0.))) {
    triggered = true;
    clkPulse.trigger(1e-3);
    clockp = clockt;
    clockt = 0;
  }

  float dt = 1e-3 * args.sampleRate;
  int sr = (int)args.sampleRate;

  amp = clamp(params[AMP_PARAM].getValue() + inputs[AMP_INPUT].getNormalVoltage(0.) / 10.0f, 0.0f, 1.0f);
  slew = clamp(params[SLEW_PARAM].getValue() + inputs[SLEW_INPUT].getNormalVoltage(0.) / 10.0f, 0.0f, 1.0f);
  slew = pow(2., (1. - slew) * log2(sr)) / sr;
  if (range)
    slew *= .1;

  float delayTarget_ = clamp(params[DELAY_PARAM].getValue() + inputs[DELAY_INPUT].getNormalVoltage(0.) / 10.0f, 0.0f, 1.0f);
  float gateTarget_ = clamp(params[TIME_PARAM].getValue() + inputs[TIME_INPUT].getNormalVoltage(0.) / 10.0f, 0.0f, 1.0f);

  if (inputs[CLOCK_INPUT].isConnected()) {
    clockt++;

    delayTarget = clockp * durations[int((ndurations - 1) * delayTarget_)];
    gateTarget = clockp * durations[int((ndurations - 1) * gateTarget_)];
    if (gateTarget < dt)
      gateTarget = dt;

  } else {
    unsigned int r = range ? 10 : 1;
    delayTarget = r * delayTarget_ * sr;
    gateTarget = r * gateTarget_ * sr + dt;
  }

  if (triggered && (reset || !gateOn || !delayOn)) {
    delayt = 0;
    delayOn = true;
    gateOn = false;
  }

  if (delayOn) {
    if (delayt < delayTarget) {
      delayt++;
    } else {
      delayOn = false;
      gateOn = true;
      gatet = 0;
    }
  }

  if (gateOn) {

    if (gatet < gateTarget) {
      gatet++;
    } else {
      eocPulse.trigger(1e-3);
      gateOn = false;
      if (repeat) {
        delayt = 0;
        delayOn = true;
      }
    }

    if (level < 1.)
      level += slew;
    if (level > 1.)
      level = 1.;

  } else {
    if (level > 0.)
      level -= slew;
    if (level < 0.)
      level = 0.;
  }

  outputs[CLOCK_OUTPUT].setVoltage(10. * clkPulse.process(1.0 / args.sampleRate));
  outputs[EOC_OUTPUT].setVoltage(10. * eocPulse.process(1.0 / args.sampleRate));
  outputs[GATE_OUTPUT].setVoltage(clamp(10.f * level * amp, -10.f, 10.f));

  lights[EOC_LIGHT].setSmoothBrightness(outputs[EOC_OUTPUT].getVoltage(), args.sampleTime);
  lights[GATE_LIGHT].setSmoothBrightness(outputs[GATE_OUTPUT].getVoltage(), args.sampleTime);
}

struct PulseWidget : ModuleWidget {

  PulseWidget(Module *module) {
    setModule(module);

    box.size = Vec(15 * 4, 380);

    setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Pulse.svg")));

    const float x1 = 5.;
    const float x2 = 35.;
    const float y1 = 40.;
    const float yh = 35.;

    addInput(createInput<sp_Port>(Vec(x1, y1 + 0 * yh), module, Pulse::CLOCK_INPUT));
    addOutput(createOutput<sp_Port>(Vec(x2, y1 + 0 * yh), module, Pulse::CLOCK_OUTPUT));

    addInput(createInput<sp_Port>(Vec(x1, y1 + 1 * yh), module, Pulse::TRIG_INPUT));
    addParam(createParam<TL1105>(Vec(x2, y1 + 1 * yh), module, Pulse::TRIG_PARAM));

    addParam(createParam<sp_Switch>(Vec(x1, y1 + 1.75 * yh), module, Pulse::RESET_PARAM));
    addParam(createParam<sp_Switch>(Vec(x1, y1 + 2.25 * yh), module, Pulse::REPEAT_PARAM));
    addParam(createParam<sp_Switch>(Vec(x1, y1 + 2.75 * yh), module, Pulse::RANGE_PARAM));

    addInput(createInput<sp_Port>(Vec(x1, y1 + 4 * yh), module, Pulse::TIME_INPUT));
    addParam(createParam<sp_SmallBlackKnob>(Vec(x2, y1 + 4 * yh), module, Pulse::TIME_PARAM));

    addInput(createInput<sp_Port>(Vec(x1, y1 + 5 * yh), module, Pulse::DELAY_INPUT));
    addParam(createParam<sp_SmallBlackKnob>(Vec(x2, y1 + 5 * yh), module, Pulse::DELAY_PARAM));

    addInput(createInput<sp_Port>(Vec(x1, y1 + 6 * yh), module, Pulse::AMP_INPUT));
    addParam(createParam<sp_SmallBlackKnob>(Vec(x2, y1 + 6 * yh), module, Pulse::AMP_PARAM));

    //addInput(createInput<sp_Port>		   (Vec(x1, y1+7*yh), module, Pulse::OFFSET_INPUT));
    //addParam(createParam<sp_SmallBlackKnob>(Vec(x2, y1+7*yh), module, Pulse::OFFSET_PARAM));

    addInput(createInput<sp_Port>(Vec(x1, y1 + 7 * yh), module, Pulse::SLEW_INPUT));
    addParam(createParam<sp_SmallBlackKnob>(Vec(x2, y1 + 7 * yh), module, Pulse::SLEW_PARAM));

    addOutput(createOutput<sp_Port>(Vec(x1, y1 + 8.25 * yh), module, Pulse::EOC_OUTPUT));
    addOutput(createOutput<sp_Port>(Vec(x2, y1 + 8.25 * yh), module, Pulse::GATE_OUTPUT));

    addChild(createLight<SmallLight<RedLight>>(Vec(x1 + 7, y1 + 7.65 * yh), module, Pulse::EOC_LIGHT));
    addChild(createLight<SmallLight<RedLight>>(Vec(x2 + 7, y1 + 7.65 * yh), module, Pulse::GATE_LIGHT));
  }
};

Model *modelPulse = createModel<Pulse, PulseWidget>("Pulse");
