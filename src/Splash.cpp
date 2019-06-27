
#include <string.h>

#include "Southpole.hpp"

#include "tides/generator.h"

struct Splash : Module {
  enum ParamIds {
    MODE_PARAM,
    RANGE_PARAM,

    FREQUENCY_PARAM,
    FM_PARAM,

    SHAPE_PARAM,
    SLOPE_PARAM,
    SMOOTHNESS_PARAM,
    NUM_PARAMS
  };
  enum InputIds {
    SHAPE_INPUT,
    SLOPE_INPUT,
    SMOOTHNESS_INPUT,

    TRIG_INPUT,
    FREEZE_INPUT,
    PITCH_INPUT,
    FM_INPUT,
    LEVEL_INPUT,

    CLOCK_INPUT,
    NUM_INPUTS
  };
  enum OutputIds {
    HIGH_OUTPUT,
    LOW_OUTPUT,
    UNI_OUTPUT,
    BI_OUTPUT,
    NUM_OUTPUTS
  };
  enum LightIds {
    MODE_GREEN_LIGHT,
    MODE_RED_LIGHT,
    PHASE_GREEN_LIGHT,
    PHASE_RED_LIGHT,
    RANGE_GREEN_LIGHT,
    RANGE_RED_LIGHT,
    NUM_LIGHTS
  };

  bool sheep;
  tides::Generator generator;
  int frame = 0;
  uint8_t lastGate;
  dsp::SchmittTrigger modeTrigger;
  dsp::SchmittTrigger rangeTrigger;

  Splash() {

    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
    memset(&generator, 0, sizeof(generator));
    generator.Init();
    generator.set_sync(false);
    reset();

    configParam(Splash::MODE_PARAM, 0.0, 1.0, 0.0, "");
    configParam(Splash::RANGE_PARAM, 0.0, 1.0, 0.0, "");
    configParam(Splash::FREQUENCY_PARAM, -48.0, 48.0, 0.0, "");
    configParam(Splash::SHAPE_PARAM, -1.0, 1.0, 0.0, "");
    configParam(Splash::SLOPE_PARAM, -1.0, 1.0, 0.0, "");
    configParam(Splash::SMOOTHNESS_PARAM, -1.0, 1.0, 0.0, "");
    configParam(Splash::FM_PARAM, -12.0, 12.0, 0.0, "");
  }
  void process(const ProcessArgs &args) override;

  void reset() {
    generator.set_range(tides::GENERATOR_RANGE_MEDIUM);
    generator.set_mode(tides::GENERATOR_MODE_LOOPING);
    sheep = false;
  }

  void randomize() {
    generator.set_range((tides::GeneratorRange)(random::u32() % 3));
    generator.set_mode((tides::GeneratorMode)(random::u32() % 3));
  }

  json_t *dataToJson() override {
    json_t *rootJ = json_object();

    json_object_set_new(rootJ, "mode", json_integer((int)generator.mode()));
    json_object_set_new(rootJ, "range", json_integer((int)generator.range()));
    json_object_set_new(rootJ, "sheep", json_boolean(sheep));

    return rootJ;
  }

  void dataFromJson(json_t *rootJ) override {
    json_t *modeJ = json_object_get(rootJ, "mode");
    if (modeJ) {
      generator.set_mode((tides::GeneratorMode)json_integer_value(modeJ));
    }

    json_t *rangeJ = json_object_get(rootJ, "range");
    if (rangeJ) {
      generator.set_range((tides::GeneratorRange)json_integer_value(rangeJ));
    }

    json_t *sheepJ = json_object_get(rootJ, "sheep");
    if (sheepJ) {
      sheep = json_boolean_value(sheepJ);
    }
  }
};

void Splash::process(const ProcessArgs &args) {

  tides::GeneratorMode mode = generator.mode();
  if (modeTrigger.process(params[MODE_PARAM].getValue())) {
    mode = (tides::GeneratorMode)(((int)mode - 1 + 3) % 3);
    generator.set_mode(mode);
  }
  lights[MODE_GREEN_LIGHT].value = (mode == 2) ? 1.0 : 0.0;
  lights[MODE_GREEN_LIGHT].value = (mode == 0) ? 0.0 : 1.0;

  lights[MODE_RED_LIGHT].value = (mode == 0) ? 1.0 : 0.0;
  lights[MODE_RED_LIGHT].value = (mode == 2) ? 0.0 : 1.0;

  tides::GeneratorRange range = generator.range();
  if (rangeTrigger.process(params[RANGE_PARAM].getValue())) {
    range = (tides::GeneratorRange)(((int)range - 1 + 3) % 3);
    generator.set_range(range);
  }
  lights[RANGE_GREEN_LIGHT].value = (range == 2) ? 1.0 : 0.0;
  lights[RANGE_GREEN_LIGHT].value = (range == 0) ? 0.0 : 1.0;

  lights[RANGE_RED_LIGHT].value = (range == 0) ? 1.0 : 0.0;
  lights[RANGE_RED_LIGHT].value = (range == 2) ? 0.0 : 1.0;

  // Buffer loop
  if (++frame >= 16) {
    frame = 0;

    // Pitch
    float pitch = params[FREQUENCY_PARAM].getValue();
    pitch += 12.0 * inputs[PITCH_INPUT].getVoltage();
    pitch += params[FM_PARAM].getValue() * inputs[FM_INPUT].getNormalVoltage(0.1) / 5.0;
    pitch += 60.0;
    // Scale to the global sample rate
    pitch += log2f(48000.0 / args.sampleRate) * 12.0;
    generator.set_pitch(clamp(int(pitch) * 0x80, -0x8000, 0x7fff));

    // Slope, smoothness, pitch
    int16_t shape = clamp(params[SHAPE_PARAM].getValue() + inputs[SHAPE_INPUT].getVoltage() / 5.0, -1.0f, 1.0f) * 0x7fff;
    int16_t slope = clamp(params[SLOPE_PARAM].getValue() + inputs[SLOPE_INPUT].getVoltage() / 5.0, -1.0f, 1.0f) * 0x7fff;
    int16_t smoothness = clamp(params[SMOOTHNESS_PARAM].getValue() + inputs[SMOOTHNESS_INPUT].getVoltage() / 5.0, -1.0f, 1.0f) * 0x7fff;
    generator.set_shape(shape);
    generator.set_slope(slope);
    generator.set_smoothness(smoothness);

    // Sync
    // Slight deviation from spec here.
    // Instead of toggling sync by holding the range button, just enable it if the clock port is plugged in.
    generator.set_sync(inputs[CLOCK_INPUT].isConnected());

    // Generator
    generator.Process(sheep);
  }

  // Level
  uint16_t level = clamp(inputs[LEVEL_INPUT].getNormalVoltage(8.0) / 8.0, 0.0f, 1.0f) * 0xffff;
  if (level < 32)
    level = 0;

  uint8_t gate = 0;
  if (inputs[FREEZE_INPUT].getVoltage() >= 0.7)
    gate |= tides::CONTROL_FREEZE;
  if (inputs[TRIG_INPUT].getVoltage() >= 0.7)
    gate |= tides::CONTROL_GATE;
  if (inputs[CLOCK_INPUT].getVoltage() >= 0.7)
    gate |= tides::CONTROL_CLOCK;
  if (!(lastGate & tides::CONTROL_CLOCK) && (gate & tides::CONTROL_CLOCK))
    gate |= tides::CONTROL_GATE_RISING;
  if (!(lastGate & tides::CONTROL_GATE) && (gate & tides::CONTROL_GATE))
    gate |= tides::CONTROL_GATE_RISING;
  if ((lastGate & tides::CONTROL_GATE) && !(gate & tides::CONTROL_GATE))
    gate |= tides::CONTROL_GATE_FALLING;
  lastGate = gate;

  const tides::GeneratorSample &sample = generator.Process(gate);

  uint32_t uni = sample.unipolar;
  int32_t bi = sample.bipolar;

  uni = uni * level >> 16;
  bi = -bi * level >> 16;
  float unif = (float)uni / 0xffff;
  float bif = (float)bi / 0x8000;

  outputs[HIGH_OUTPUT].setVoltage(sample.flags & tides::FLAG_END_OF_ATTACK ? 0.0 : 5.0);
  outputs[LOW_OUTPUT].setVoltage(sample.flags & tides::FLAG_END_OF_RELEASE ? 0.0 : 5.0);
  outputs[UNI_OUTPUT].setVoltage(unif * 8.0);
  outputs[BI_OUTPUT].setVoltage(bif * 5.0);

  if (sample.flags & tides::FLAG_END_OF_ATTACK)
    unif *= -1.0;
  lights[PHASE_GREEN_LIGHT].setSmoothBrightness(fmaxf(0.0, unif), args.sampleTime);
  lights[PHASE_RED_LIGHT].setSmoothBrightness(fmaxf(0.0, -unif), args.sampleTime);
}

struct SplashWidget : ModuleWidget {
  SvgPanel *tidesPanel;
  SvgPanel *sheepPanel;

  SplashWidget(Splash *module) {
    setModule(module);

    box.size = Vec(15 * 8, 380);

    {
      tidesPanel = new SvgPanel();
      tidesPanel->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Splash.svg")));
      tidesPanel->box.size = box.size;
      addChild(tidesPanel);
    }
    {
      sheepPanel = new SvgPanel();
      sheepPanel->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Lambs.svg")));
      sheepPanel->box.size = box.size;
      addChild(sheepPanel);
    }
    const float x1 = 0.5 * RACK_GRID_WIDTH;
    const float x2 = 3.25 * RACK_GRID_WIDTH;
    const float x3 = 5.75 * RACK_GRID_WIDTH;
    const float y1 = 40.0f;
    const float y2 = 25.0f;
    const float yh = 38.0f;

    addParam(createParam<CKD6>(Vec(x3 - 3, y1 - 3), module, Splash::MODE_PARAM));
    addChild(createLight<MediumLight<GreenRedLight>>(Vec(x3 + 7, y1 + 7), module, Splash::MODE_GREEN_LIGHT));

    addParam(createParam<CKD6>(Vec(x3 - 3, y1 + 1.45 * yh), module, Splash::RANGE_PARAM));
    addChild(createLight<MediumLight<GreenRedLight>>(Vec(x3 + 7, y1 + 2 * yh - 10), module, Splash::RANGE_GREEN_LIGHT));

    addChild(createLight<MediumLight<GreenRedLight>>(Vec(x2 - 20, y2 + 2 * yh), module, Splash::PHASE_GREEN_LIGHT));
    addParam(createParam<sp_BlackKnob>(Vec(x2 - 7, y2 + 1.75 * yh), module, Splash::FREQUENCY_PARAM));

    addParam(createParam<sp_SmallBlackKnob>(Vec(x3, y2 + 4 * yh), module, Splash::SHAPE_PARAM));
    addParam(createParam<sp_SmallBlackKnob>(Vec(x3, y2 + 4.75 * yh), module, Splash::SLOPE_PARAM));
    addParam(createParam<sp_SmallBlackKnob>(Vec(x3, y2 + 5.5 * yh), module, Splash::SMOOTHNESS_PARAM));

    addInput(createInput<sp_Port>(Vec(x1, y1), module, Splash::TRIG_INPUT));
    addInput(createInput<sp_Port>(Vec(x2, y1), module, Splash::FREEZE_INPUT));

    addInput(createInput<sp_Port>(Vec(x1, y2 + 2 * yh), module, Splash::PITCH_INPUT));
    addInput(createInput<sp_Port>(Vec(x1, y2 + 3.25 * yh), module, Splash::FM_INPUT));
    addParam(createParam<sp_Trimpot>(Vec(x2, y2 + 3.25 * yh), module, Splash::FM_PARAM));

    addInput(createInput<sp_Port>(Vec(x1, y2 + 4 * yh), module, Splash::SHAPE_INPUT));
    addInput(createInput<sp_Port>(Vec(x1, y2 + 4.75 * yh), module, Splash::SLOPE_INPUT));
    addInput(createInput<sp_Port>(Vec(x1, y2 + 5.5 * yh), module, Splash::SMOOTHNESS_INPUT));

    addInput(createInput<sp_Port>(Vec(x3, y1 + 5.9 * yh), module, Splash::LEVEL_INPUT));
    addInput(createInput<sp_Port>(Vec(x1, y1 + 5.9 * yh), module, Splash::CLOCK_INPUT));

    addOutput(createOutput<sp_Port>(Vec(x1, y1 + 7.125 * yh), module, Splash::HIGH_OUTPUT));
    addOutput(createOutput<sp_Port>(Vec(x1 + 1 * 28., y1 + 7.125 * yh), module, Splash::LOW_OUTPUT));
    addOutput(createOutput<sp_Port>(Vec(x1 + 2 * 28., y1 + 7.125 * yh), module, Splash::UNI_OUTPUT));
    addOutput(createOutput<sp_Port>(Vec(x1 + 3 * 28., y1 + 7.125 * yh), module, Splash::BI_OUTPUT));
  }

  void appendContextMenu(Menu *menu) override {
    Splash *tides = dynamic_cast<Splash *>(module);
    assert(tides);

    struct SplashSheepItem : MenuItem {
      Splash *tides;
      void onAction(const event::Action &e) override {
        tides->sheep ^= true;
      }
      void step() override {
        rightText = (tides->sheep) ? "✔" : "";
        MenuItem::step();
      }
    };

    menu->addChild(construct<MenuLabel>());
    menu->addChild(construct<SplashSheepItem>(&MenuItem::text, "Lambs", &SplashSheepItem::tides, tides));
  }

  void step() override {
    Splash *tides = dynamic_cast<Splash *>(module);

    if (tides) {
      tidesPanel->visible = !tides->sheep;
      sheepPanel->visible = tides->sheep;
    }

    ModuleWidget::step();
  }
};

Model *modelSplash = createModel<Splash, SplashWidget>("Splash");
