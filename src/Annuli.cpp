#include "Southpole.hpp"
#include "rings/dsp/part.h"
#include "rings/dsp/string_synth_part.h"
#include "rings/dsp/strummer.h"
#include <string.h>

struct Annuli : Module {
  enum ParamIds {
    POLYPHONY_PARAM,
    RESONATOR_PARAM,

    FREQUENCY_PARAM,
    STRUCTURE_PARAM,
    BRIGHTNESS_PARAM,
    DAMPING_PARAM,
    POSITION_PARAM,

    BRIGHTNESS_MOD_PARAM,
    FREQUENCY_MOD_PARAM,
    DAMPING_MOD_PARAM,
    STRUCTURE_MOD_PARAM,
    POSITION_MOD_PARAM,
    NUM_PARAMS
  };
  enum InputIds {
    BRIGHTNESS_MOD_INPUT,
    FREQUENCY_MOD_INPUT,
    DAMPING_MOD_INPUT,
    STRUCTURE_MOD_INPUT,
    POSITION_MOD_INPUT,

    STRUM_INPUT,
    PITCH_INPUT,
    IN_INPUT,
    NUM_INPUTS
  };
  enum OutputIds {
    ODD_OUTPUT,
    EVEN_OUTPUT,
    NUM_OUTPUTS
  };
  enum LightIds {
    POLYPHONY_GREEN_LIGHT,
    POLYPHONY_RED_LIGHT,
    RESONATOR_GREEN_LIGHT,
    RESONATOR_RED_LIGHT,
    NUM_LIGHTS
  };

  dsp::SampleRateConverter<1> inputSrc;
  dsp::SampleRateConverter<2> outputSrc;
  dsp::DoubleRingBuffer<dsp::Frame<1>, 256> inputBuffer;
  dsp::DoubleRingBuffer<dsp::Frame<2>, 256> outputBuffer;

  uint16_t reverb_buffer[32768] = {};
  rings::Part part;
  rings::StringSynthPart string_synth;
  rings::Strummer strummer;
  bool strum = false;
  bool lastStrum = false;

  dsp::SchmittTrigger polyphonyTrigger;
  dsp::SchmittTrigger modelTrigger;
  int polyphonyMode = 0;
  rings::ResonatorModel model = rings::RESONATOR_MODEL_MODAL;
  bool easterEgg = false;

  Annuli() {

    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
    memset(&strummer, 0, sizeof(strummer));
    memset(&part, 0, sizeof(part));
    memset(&string_synth, 0, sizeof(string_synth));

    strummer.Init(0.01, 44100.0 / 24);
    part.Init(reverb_buffer);
    string_synth.Init(reverb_buffer);

    configParam(Annuli::POLYPHONY_PARAM, 0.0, 1.0, 0.0, "");
    configParam(Annuli::RESONATOR_PARAM, 0.0, 1.0, 0.0, "");
    configParam(Annuli::FREQUENCY_PARAM, 0.0, 60.0, 30.0, "");
    configParam(Annuli::STRUCTURE_PARAM, 0.0, 1.0, 0.5, "");
    configParam(Annuli::BRIGHTNESS_PARAM, 0.0, 1.0, 0.5, "");
    configParam(Annuli::DAMPING_PARAM, 0.0, 1.0, 0.5, "");
    configParam(Annuli::POSITION_PARAM, 0.0, 1.0, 0.5, "");
    configParam(Annuli::BRIGHTNESS_MOD_PARAM, -1.0, 1.0, 0.0, "");
    configParam(Annuli::FREQUENCY_MOD_PARAM, -1.0, 1.0, 0.0, "");
    configParam(Annuli::DAMPING_MOD_PARAM, -1.0, 1.0, 0.0, "");
    configParam(Annuli::STRUCTURE_MOD_PARAM, -1.0, 1.0, 0.0, "");
    configParam(Annuli::POSITION_MOD_PARAM, -1.0, 1.0, 0.0, "");
  }
  void process(const ProcessArgs &args) override;

  json_t *dataToJson() override {
    json_t *rootJ = json_object();

    json_object_set_new(rootJ, "polyphony", json_integer(polyphonyMode));
    json_object_set_new(rootJ, "model", json_integer((int)model));
    json_object_set_new(rootJ, "easterEgg", json_boolean(easterEgg));

    return rootJ;
  }

  void dataFromJson(json_t *rootJ) override {
    json_t *polyphonyJ = json_object_get(rootJ, "polyphony");
    if (polyphonyJ) {
      polyphonyMode = json_integer_value(polyphonyJ);
    }

    json_t *modelJ = json_object_get(rootJ, "model");
    if (modelJ) {
      model = (rings::ResonatorModel)json_integer_value(modelJ);
    }

    json_t *easterEggJ = json_object_get(rootJ, "easterEgg");
    if (easterEggJ) {
      easterEgg = json_boolean_value(easterEggJ);
    }
  }

  void onReset() override {
    polyphonyMode = 0;
    model = rings::RESONATOR_MODEL_MODAL;
  }

  void onRandomize() override {
    polyphonyMode = random::u32() % 3;
    model = (rings::ResonatorModel)(random::u32() % 3);
  }
};

void Annuli::process(const ProcessArgs &args) {
  // TODO
  // "getNormalVoltaged to a pulse/burst generator that reacts to note changes on the V/OCT input."
  // Get input
  if (!inputBuffer.full()) {
    dsp::Frame<1> f;
    f.samples[0] = inputs[IN_INPUT].getVoltage() / 5.0;
    inputBuffer.push(f);
  }

  if (!strum) {
    strum = inputs[STRUM_INPUT].getVoltage() >= 1.0;
  }

  // Polyphony / model
  if (polyphonyTrigger.process(params[POLYPHONY_PARAM].getValue())) {
    polyphonyMode = (polyphonyMode + 1) % 3;
  }
  lights[POLYPHONY_GREEN_LIGHT].value = (polyphonyMode == 0 || polyphonyMode == 1) ? 1.0 : 0.0;
  lights[POLYPHONY_RED_LIGHT].value = (polyphonyMode == 1 || polyphonyMode == 2) ? 1.0 : 0.0;

  if (modelTrigger.process(params[RESONATOR_PARAM].getValue())) {
    model = (rings::ResonatorModel)((model + 1) % 3);
  }
  int modelColor = model % 3;
  lights[RESONATOR_GREEN_LIGHT].value = (modelColor == 0 || modelColor == 1) ? 1.0 : 0.0;
  lights[RESONATOR_RED_LIGHT].value = (modelColor == 1 || modelColor == 2) ? 1.0 : 0.0;

  // Render frames
  if (outputBuffer.empty()) {
    float in[24] = {};
    // Convert input buffer
    {
      inputSrc.setRates(args.sampleRate, 48000.0);
      int inLen = inputBuffer.size();
      int outLen = 24;
      inputSrc.process(inputBuffer.startData(), &inLen, (dsp::Frame<1> *)in, &outLen);
      inputBuffer.startIncr(inLen);
    }

    // Polyphony
    int polyphony = 1 << polyphonyMode;
    if (part.polyphony() != polyphony)
      part.set_polyphony(polyphony);
    // Model
    if (easterEgg)
      string_synth.set_fx((rings::FxType)model);
    else
      part.set_model(model);

    // Patch
    rings::Patch patch;
    float structure = params[STRUCTURE_PARAM].getValue() + 3.3 * dsp::quadraticBipolar(params[STRUCTURE_MOD_PARAM].getValue()) * inputs[STRUCTURE_MOD_INPUT].getVoltage() / 5.0;
    patch.structure = clamp(structure, 0.0f, 0.9995f);
    patch.brightness = clamp(params[BRIGHTNESS_PARAM].getValue() + 3.3 * dsp::quadraticBipolar(params[BRIGHTNESS_MOD_PARAM].getValue()) * inputs[BRIGHTNESS_MOD_INPUT].getVoltage() / 5.0, 0.0f, 1.0f);
    patch.damping = clamp(params[DAMPING_PARAM].getValue() + 3.3 * dsp::quadraticBipolar(params[DAMPING_MOD_PARAM].getValue()) * inputs[DAMPING_MOD_INPUT].getVoltage() / 5.0, 0.0f, 0.9995f);
    patch.position = clamp(params[POSITION_PARAM].getValue() + 3.3 * dsp::quadraticBipolar(params[POSITION_MOD_PARAM].getValue()) * inputs[POSITION_MOD_INPUT].getVoltage() / 5.0, 0.0f, 0.9995f);

    // Performance
    rings::PerformanceState performance_state;
    performance_state.note = 12.0 * inputs[PITCH_INPUT].getNormalVoltage(1 / 12.0);
    float transpose = params[FREQUENCY_PARAM].getValue();
    // Quantize transpose if pitch input is connected
    if (inputs[PITCH_INPUT].isConnected()) {
      transpose = roundf(transpose);
    }
    performance_state.tonic = 12.0 + clamp(transpose, 0.0f, 60.0f);
    performance_state.fm = clamp(48.0 * 3.3 * dsp::quarticBipolar(params[FREQUENCY_MOD_PARAM].getValue()) * inputs[FREQUENCY_MOD_INPUT].getNormalVoltage(1.0) / 5.0, -48.0f, 48.0f);

    performance_state.internal_exciter = !inputs[IN_INPUT].isConnected();
    performance_state.internal_strum = !inputs[STRUM_INPUT].isConnected();
    performance_state.internal_note = !inputs[PITCH_INPUT].isConnected();

    // TODO
    // "getNormalVoltaged to a step detector on the V/OCT input and a transient detector on the IN input."
    performance_state.strum = strum && !lastStrum;
    lastStrum = strum;
    strum = false;

    performance_state.chord = clamp((int)roundf(structure * (rings::kNumChords - 1)), 0, rings::kNumChords - 1);

    // Process audio
    float out[24];
    float aux[24];
    if (easterEgg) {
      strummer.Process(NULL, 24, &performance_state);
      string_synth.Process(performance_state, patch, in, out, aux, 24);
    } else {
      strummer.Process(in, 24, &performance_state);
      part.Process(performance_state, patch, in, out, aux, 24);
    }

    // Convert output buffer
    {
      dsp::Frame<2> outputFrames[24];
      for (int i = 0; i < 24; i++) {
        outputFrames[i].samples[0] = out[i];
        outputFrames[i].samples[1] = aux[i];
      }

      outputSrc.setRates(48000.0, args.sampleRate);
      int inLen = 24;
      int outLen = outputBuffer.capacity();
      outputSrc.process(outputFrames, &inLen, outputBuffer.endData(), &outLen);
      outputBuffer.endIncr(outLen);
    }
  }

  // Set output
  if (!outputBuffer.empty()) {
    dsp::Frame<2> outputFrame = outputBuffer.shift();
    // "Note that you need to insert a jack into each output to split the signals: when only one jack is inserted, both signals are mixed together."
    if (outputs[ODD_OUTPUT].isConnected() && outputs[EVEN_OUTPUT].isConnected()) {
      outputs[ODD_OUTPUT].setVoltage(clamp(outputFrame.samples[0], -1.0f, 1.0f) * 5.0);
      outputs[EVEN_OUTPUT].setVoltage(clamp(outputFrame.samples[1], -1.0f, 1.0f) * 5.0);
    } else {
      float v = clamp(outputFrame.samples[0] + outputFrame.samples[1], -1.0f, 1.0f) * 5.0;
      outputs[ODD_OUTPUT].setVoltage(v);
      outputs[EVEN_OUTPUT].setVoltage(v);
    }
  }
}

struct AnnuliWidget : ModuleWidget {
  SvgPanel *panel;
  SvgPanel *panel2;

  AnnuliWidget(Annuli *module) {
    setModule(module);

    box.size = Vec(15 * 6, 380);
    {
      panel = new SvgPanel();
      panel->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Annuli.svg")));
      panel->box.size = box.size;
      panel->visible = true;
      addChild(panel);
    }
    {
      panel2 = new SvgPanel();
      panel2->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/DisastrousPeace.svg")));
      panel2->box.size = box.size;
      panel2->visible = false;
      addChild(panel2);
    }
    const float x1 = 5;
    const float x2 = 35;
    const float x3 = 65;

    const float y1 = 20.0f;
    const float yh = 33.0f;

    addParam(createParam<TL1105>(Vec(x3, y1 + .25 * yh), module, Annuli::POLYPHONY_PARAM));
    addParam(createParam<TL1105>(Vec(x3, y1 + yh), module, Annuli::RESONATOR_PARAM));

    addParam(createParam<sp_SmallBlackKnob>(Vec(x3, y1 + 2.12 * yh), module, Annuli::FREQUENCY_PARAM));
    addParam(createParam<sp_SmallBlackKnob>(Vec(x3, y1 + 3.12 * yh), module, Annuli::STRUCTURE_PARAM));
    addParam(createParam<sp_SmallBlackKnob>(Vec(x3, y1 + 4.12 * yh), module, Annuli::BRIGHTNESS_PARAM));
    addParam(createParam<sp_SmallBlackKnob>(Vec(x3, y1 + 5.12 * yh), module, Annuli::DAMPING_PARAM));
    addParam(createParam<sp_SmallBlackKnob>(Vec(x3, y1 + 6.12 * yh), module, Annuli::POSITION_PARAM));

    addParam(createParam<sp_Trimpot>(Vec(x2, y1 + 2 * yh + 6), module, Annuli::BRIGHTNESS_MOD_PARAM));
    addParam(createParam<sp_Trimpot>(Vec(x2, y1 + 3 * yh + 6), module, Annuli::FREQUENCY_MOD_PARAM));
    addParam(createParam<sp_Trimpot>(Vec(x2, y1 + 4 * yh + 6), module, Annuli::DAMPING_MOD_PARAM));
    addParam(createParam<sp_Trimpot>(Vec(x2, y1 + 5 * yh + 6), module, Annuli::STRUCTURE_MOD_PARAM));
    addParam(createParam<sp_Trimpot>(Vec(x2, y1 + 6 * yh + 6), module, Annuli::POSITION_MOD_PARAM));

    addInput(createInput<sp_Port>(Vec(x1, y1 + 2.12 * yh), module, Annuli::BRIGHTNESS_MOD_INPUT));
    addInput(createInput<sp_Port>(Vec(x1, y1 + 3.12 * yh), module, Annuli::FREQUENCY_MOD_INPUT));
    addInput(createInput<sp_Port>(Vec(x1, y1 + 4.12 * yh), module, Annuli::DAMPING_MOD_INPUT));
    addInput(createInput<sp_Port>(Vec(x1, y1 + 5.12 * yh), module, Annuli::STRUCTURE_MOD_INPUT));
    addInput(createInput<sp_Port>(Vec(x1, y1 + 6.12 * yh), module, Annuli::POSITION_MOD_INPUT));

    addInput(createInput<sp_Port>(Vec(x1 + 10, y1 + 7 * yh), module, Annuli::STRUM_INPUT));
    addInput(createInput<sp_Port>(Vec(x3 - 10, y1 + 7 * yh), module, Annuli::PITCH_INPUT));

    addInput(createInput<sp_Port>(Vec(x1, y1 + 8.875 * yh), module, Annuli::IN_INPUT));
    addOutput(createOutput<sp_Port>(Vec(x3, y1 + 8.25 * yh), module, Annuli::ODD_OUTPUT));
    addOutput(createOutput<sp_Port>(Vec(x3, y1 + 9.125 * yh), module, Annuli::EVEN_OUTPUT));

    addChild(createLight<MediumLight<GreenRedLight>>(Vec(x2 + 18, y1 + .25 * yh + 3), module, Annuli::POLYPHONY_GREEN_LIGHT));
    addChild(createLight<MediumLight<GreenRedLight>>(Vec(x2 + 18, y1 + yh + 3), module, Annuli::RESONATOR_GREEN_LIGHT));
  }

  void appendContextMenu(Menu *menu) override {
    Annuli *rings = dynamic_cast<Annuli *>(module);
    assert(rings);

    struct AnnuliModelItem : MenuItem {
      Annuli *rings;
      rings::ResonatorModel model;

      void onAction(const event::Action &e) override {
        rings->model = model;
      }
      void step() override {
        rightText = (rings->model == model) ? "✔" : "";
        MenuItem::step();
      }
    };

    struct AnnuliEasterEggItem : MenuItem {
      Annuli *rings;
      void onAction(const event::Action &e) override {
        rings->easterEgg = !rings->easterEgg;
      }
      void step() override {
        rightText = (rings->easterEgg) ? "✔" : "";
        MenuItem::step();
      }
    };

    menu->addChild(construct<MenuLabel>());
    menu->addChild(construct<MenuLabel>(&MenuLabel::text, "Resonator"));
    menu->addChild(construct<AnnuliModelItem>(&MenuItem::text, "Modal resonator", &AnnuliModelItem::rings, rings, &AnnuliModelItem::model, rings::RESONATOR_MODEL_MODAL));
    menu->addChild(construct<AnnuliModelItem>(&MenuItem::text, "Sympathetic strings", &AnnuliModelItem::rings, rings, &AnnuliModelItem::model, rings::RESONATOR_MODEL_SYMPATHETIC_STRING));
    menu->addChild(construct<AnnuliModelItem>(&MenuItem::text, "Modulated/inharmonic string", &AnnuliModelItem::rings, rings, &AnnuliModelItem::model, rings::RESONATOR_MODEL_STRING));
    menu->addChild(construct<AnnuliModelItem>(&MenuItem::text, "FM voice", &AnnuliModelItem::rings, rings, &AnnuliModelItem::model, rings::RESONATOR_MODEL_FM_VOICE));
    menu->addChild(construct<AnnuliModelItem>(&MenuItem::text, "Quantized sympathetic strings", &AnnuliModelItem::rings, rings, &AnnuliModelItem::model, rings::RESONATOR_MODEL_SYMPATHETIC_STRING_QUANTIZED));
    menu->addChild(construct<AnnuliModelItem>(&MenuItem::text, "Reverb string", &AnnuliModelItem::rings, rings, &AnnuliModelItem::model, rings::RESONATOR_MODEL_STRING_AND_REVERB));

    menu->addChild(construct<MenuLabel>());
    menu->addChild(construct<AnnuliEasterEggItem>(&MenuItem::text, "Disastrous Peace", &AnnuliEasterEggItem::rings, rings));
  }

  void step() override {
    Annuli *annuli = dynamic_cast<Annuli *>(module);

    if (annuli) {
      panel->visible = !annuli->easterEgg;
      panel2->visible = annuli->easterEgg;
    }

    ModuleWidget::step();
  }
};

Model *modelAnnuli = createModel<Annuli, AnnuliWidget>("Annuli");
