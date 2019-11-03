//
// Smoke = Mutable Instruments Clouds Parasites
// copied from Arable Instruments Neil
//

#include "Southpole.hpp"
#include "clouds/dsp/granular_processor.h"
#include <string.h>

struct Smoke : Module {
  enum ParamIds {
    POSITION_PARAM,
    SIZE_PARAM,
    PITCH_PARAM,
    IN_GAIN_PARAM,
    DENSITY_PARAM,
    TEXTURE_PARAM,
    BLEND_PARAM,
    SPREAD_PARAM,
    FEEDBACK_PARAM,
    REVERB_PARAM,
    FREEZE_PARAM,
    REVERSE_PARAM,
    NUM_PARAMS
  };
  enum InputIds {
    FREEZE_INPUT,
    TRIG_INPUT,
    POSITION_INPUT,
    SIZE_INPUT,
    PITCH_INPUT,
    BLEND_INPUT,
    SPREAD_INPUT,
    FEEDBACK_INPUT,
    REVERB_INPUT,
    IN_L_INPUT,
    IN_R_INPUT,
    DENSITY_INPUT,
    TEXTURE_INPUT,
    NUM_INPUTS
  };
  enum OutputIds {
    OUT_L_OUTPUT,
    OUT_R_OUTPUT,
    NUM_OUTPUTS
  };
  enum LightIds {
    FREEZE_LIGHT,
    REVERSE_LIGHT,

    MIX_GREEN_LIGHT,
    MIX_RED_LIGHT,
    PAN_GREEN_LIGHT,
    PAN_RED_LIGHT,
    FEEDBACK_GREEN_LIGHT,
    FEEDBACK_RED_LIGHT,
    REVERB_GREEN_LIGHT,
    REVERB_RED_LIGHT,
    NUM_LIGHTS
  };

  dsp::SampleRateConverter<2> inputSrc;
  dsp::SampleRateConverter<2> outputSrc;
  dsp::DoubleRingBuffer<dsp::Frame<2>, 256> inputBuffer;
  dsp::DoubleRingBuffer<dsp::Frame<2>, 256> outputBuffer;
  dsp::VuMeter2 vuMeter;
  dsp::ClockDivider lightDivider;

  clouds::PlaybackMode playbackmode = clouds::PLAYBACK_MODE_GRANULAR;

  int buffersize = 1;
  int currentbuffersize = 1;
  bool lofi = false;
  bool mono = false;
  uint8_t *block_mem;
  uint8_t *block_ccm;
  clouds::GranularProcessor *processor;

  bool triggered = false;
  float freezeLight = 0.0;
  bool freeze = false;
  bool reverse = false;
  float reverseLight = 0.0;
  dsp::SchmittTrigger reverseTrigger;
  dsp::SchmittTrigger freezeTrigger;

  Smoke() {

    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
    const int memLen = 118784;
    const int ccmLen = 65536 - 128;
    block_mem = new uint8_t[memLen]();
    block_ccm = new uint8_t[ccmLen]();
    processor = new clouds::GranularProcessor();
    memset(processor, 0, sizeof(*processor));
    lightDivider.setDivision(512);

    processor->Init(block_mem, memLen, block_ccm, ccmLen);

    configParam(Smoke::FREEZE_PARAM, 0.0, 1.0, 0.0, "");
    configParam(Smoke::REVERSE_PARAM, 0.0, 1.0, 0.0, "");
    configParam(Smoke::POSITION_PARAM, 0.0, 1.0, 0.5, "");
    configParam(Smoke::SIZE_PARAM, 0.0, 1.0, 0.5, "");
    configParam(Smoke::PITCH_PARAM, -2.0, 2.0, 0.0, "");
    configParam(Smoke::DENSITY_PARAM, 0.0, 1.0, 0.5, "");
    configParam(Smoke::TEXTURE_PARAM, 0.0, 1.0, 0.5, "");
    configParam(Smoke::BLEND_PARAM, 0.0, 1.0, 0.5, "");
    configParam(Smoke::SPREAD_PARAM, 0.0, 1.0, 0.5, "");
    configParam(Smoke::FEEDBACK_PARAM, 0.0, 1.0, 0.5, "");
    configParam(Smoke::REVERB_PARAM, 0.0, 1.0, 0.5, "");
    configParam(Smoke::IN_GAIN_PARAM, 0.0, 1.0, 0.5, "");
  }

  ~Smoke() {
    delete processor;
    delete[] block_mem;
    delete[] block_ccm;
  }

  void process(const ProcessArgs &args) override;

  json_t *dataToJson() override {
    json_t *rootJ = json_object();
    //playbackmode, lofi, mono
    json_object_set_new(rootJ, "playbackmode", json_integer(playbackmode));
    json_object_set_new(rootJ, "lofi", json_integer(lofi));
    json_object_set_new(rootJ, "mono", json_integer(mono));
    json_object_set_new(rootJ, "freeze", json_integer(freeze));
    json_object_set_new(rootJ, "buffersize", json_integer(buffersize));
    json_object_set_new(rootJ, "reverse", json_integer(reverse));
    return rootJ;
  }

  void dataFromJson(json_t *rootJ) override {
    json_t *playbackmodeJ = json_object_get(rootJ, "playbackmode");
    if (playbackmodeJ) {
      playbackmode = (clouds::PlaybackMode)json_integer_value(playbackmodeJ);
    }
    json_t *lofiJ = json_object_get(rootJ, "lofi");
    if (lofiJ) {
      lofi = json_integer_value(lofiJ);
    }
    json_t *monoJ = json_object_get(rootJ, "mono");
    if (monoJ) {
      mono = json_integer_value(monoJ);
    }
    json_t *freezeJ = json_object_get(rootJ, "freeze");
    if (freezeJ) {
      freeze = json_integer_value(freezeJ);
    }
    json_t *buffersizeJ = json_object_get(rootJ, "buffersize");
    if (buffersizeJ) {
      buffersize = json_integer_value(buffersizeJ);
    }
    json_t *reverseJ = json_object_get(rootJ, "reverse");
		if (reverseJ) {
			reverse = json_integer_value(reverseJ);
		}
  }
};

void Smoke::process(const ProcessArgs &args) {

  dsp::Frame<2> inputFrame;
  // Get input
  if (!inputBuffer.full()) {
    //dsp::Frame<2> inputFrame;
    inputFrame.samples[0] = inputs[IN_L_INPUT].getVoltage() * params[IN_GAIN_PARAM].getValue() / 5.0;
    inputFrame.samples[1] = inputs[IN_R_INPUT].isConnected() ? inputs[IN_R_INPUT].getVoltage() * params[IN_GAIN_PARAM].getValue() / 5.0 : inputFrame.samples[0];
    inputBuffer.push(inputFrame);
  }

  // Trigger
  if (inputs[TRIG_INPUT].getVoltage() >= 1.0) {
    triggered = true;
  }

  // Render frames
  if (outputBuffer.empty()) {
    clouds::ShortFrame input[32] = {};
    // Convert input buffer
    {
      inputSrc.setRates(args.sampleRate, 32000);
      dsp::Frame<2> inputFrames[32];
      int inLen = inputBuffer.size();
      int outLen = 32;
      inputSrc.process(inputBuffer.startData(), &inLen, inputFrames, &outLen);
      inputBuffer.startIncr(inLen);

      // We might not fill all of the input buffer if there is a deficiency, but this cannot be avoided due to imprecisions between the input and output SRC.
      for (int i = 0; i < outLen; i++) {
        input[i].l = clamp(inputFrames[i].samples[0] * 32767.0, -32768, 32767);
        input[i].r = clamp(inputFrames[i].samples[1] * 32767.0, -32768, 32767);
      }
    }
    if (currentbuffersize != buffersize) {
      //re-init processor with new size
      delete processor;
      delete[] block_mem;
      int memLen = 118784 * buffersize;
      const int ccmLen = 65536 - 128;
      block_mem = new uint8_t[memLen]();
      processor = new clouds::GranularProcessor();
      memset(processor, 0, sizeof(*processor));
      processor->Init(block_mem, memLen, block_ccm, ccmLen);
      currentbuffersize = buffersize;
    }

    // Set up processor
    processor->set_num_channels(mono ? 1 : 2);
    processor->set_low_fidelity(lofi);
    // TODO Support the other modes
    processor->set_playback_mode(playbackmode);
    processor->Prepare();

    if (freezeTrigger.process(params[FREEZE_PARAM].getValue())) {
      freeze = !freeze;
    }

    clouds::Parameters *p = processor->mutable_parameters();
    p->trigger = triggered;
    p->gate = triggered;
    p->freeze = (inputs[FREEZE_INPUT].getVoltage() >= 1.0 || freeze);
    p->position = clamp(params[POSITION_PARAM].getValue() + inputs[POSITION_INPUT].getVoltage() / 5.0, 0.0f, 1.0f);
    p->size = clamp(params[SIZE_PARAM].getValue() + inputs[SIZE_INPUT].getVoltage() / 5.0, 0.0f, 1.0f);
    p->pitch = clamp((params[PITCH_PARAM].getValue() + inputs[PITCH_INPUT].getVoltage()) * 12.0, -48.0f, 48.0f);
    p->density = clamp(params[DENSITY_PARAM].getValue() + inputs[DENSITY_INPUT].getVoltage() / 5.0, 0.0f, 1.0f);
    p->texture = clamp(params[TEXTURE_PARAM].getValue() + inputs[TEXTURE_INPUT].getVoltage() / 5.0, 0.0f, 1.0f);
    float blend = clamp(params[BLEND_PARAM].getValue() + inputs[BLEND_INPUT].getVoltage() / 5.0, 0.0f, 1.0f);
    p->dry_wet = blend;
    p->stereo_spread = clamp(params[SPREAD_PARAM].getValue() + inputs[SPREAD_INPUT].getVoltage() / 5.0, 0.0f, 1.0f);
    p->feedback = clamp(params[FEEDBACK_PARAM].getValue() + inputs[FEEDBACK_INPUT].getVoltage() / 5.0, 0.0f, 1.0f);
    p->reverb = clamp(params[REVERB_PARAM].getValue() + inputs[REVERB_INPUT].getVoltage() / 5.0, 0.0f, 1.0f);
    if (reverseTrigger.process(params[REVERSE_PARAM].getValue())) {
       reverse = !reverse;
    }
    p->granular.reverse = reverse;
    lights[REVERSE_LIGHT].setBrightness(p->granular.reverse ? 1.0 : 0.0);

    clouds::ShortFrame output[32];
    processor->Process(input, output, 32);

    lights[FREEZE_LIGHT].setBrightness(p->freeze ? 1.0 : 0.0);

    // Convert output buffer
    {
      dsp::Frame<2> outputFrames[32];
      for (int i = 0; i < 32; i++) {
        outputFrames[i].samples[0] = output[i].l / 32768.0;
        outputFrames[i].samples[1] = output[i].r / 32768.0;
      }

      outputSrc.setRates(32000, args.sampleRate);
      int inLen = 32;
      int outLen = outputBuffer.capacity();
      outputSrc.process(outputFrames, &inLen, outputBuffer.endData(), &outLen);
      outputBuffer.endIncr(outLen);
    }

    triggered = false;
  }

  // Set output
  dsp::Frame<2> outputFrame;
  if (!outputBuffer.empty()) {
    outputFrame = outputBuffer.shift();
    outputs[OUT_L_OUTPUT].setVoltage(5.0 * outputFrame.samples[0]);
    outputs[OUT_R_OUTPUT].setVoltage(5.0 * outputFrame.samples[1]);
  }

  // Lights

  clouds::Parameters *p = processor->mutable_parameters();
  dsp::Frame<2> lightFrame = p->freeze ? outputFrame : inputFrame;
  vuMeter.process(args.sampleTime, fmaxf(fabsf(lightFrame.samples[0]), fabsf(lightFrame.samples[1])));
  lights[FREEZE_LIGHT].setBrightness(p->freeze ? 0.75 : 0.0);
  if (lightDivider.process()) { // Expensive, so call this infrequently
    lights[MIX_GREEN_LIGHT].setBrightness(vuMeter.getBrightness(-24.f, -18.f));
    lights[PAN_GREEN_LIGHT].setBrightness(vuMeter.getBrightness(-18.f, -12.f));
    lights[FEEDBACK_GREEN_LIGHT].setBrightness(vuMeter.getBrightness(-12.f, -6.f));
    lights[REVERB_GREEN_LIGHT].setBrightness(0.0);
    lights[MIX_RED_LIGHT].setBrightness(0.0);
    lights[PAN_RED_LIGHT].setBrightness(0.0);
    lights[FEEDBACK_RED_LIGHT].setBrightness(vuMeter.getBrightness(-12.f, -6.f));
    lights[REVERB_RED_LIGHT].setBrightness(vuMeter.getBrightness(-6.f, 0.f));
  }
}

struct SmokeWidget : ModuleWidget {
  SvgPanel *panel1;
  SvgPanel *panel2;
  SvgPanel *panel3;
  SvgPanel *panel4;
  SvgPanel *panel5;
  SvgPanel *panel6;

  SmokeWidget(Smoke *module) {
    setModule(module);

    box.size = Vec(6 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

    {
      panel1 = new SvgPanel();
      panel1->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Smoke-parasite.svg")));
      panel1->box.size = box.size;
      panel1->visible = true;
      addChild(panel1);
    }
    {
      panel2 = new SvgPanel();
      panel2->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Espectro-parasite.svg")));
      panel2->box.size = box.size;
      panel2->visible = false;
      addChild(panel2);
    }
    {
      panel3 = new SvgPanel();
      panel3->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Ritardo-parasite.svg")));
      panel3->box.size = box.size;
      panel3->visible = false;
      addChild(panel3);
    }
    {
      panel4 = new SvgPanel();
      panel4->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Camilla-parasite.svg")));
      panel4->box.size = box.size;
      panel4->visible = false;
      addChild(panel4);
    }
    {
      panel5 = new SvgPanel();
      panel5->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Oliverb.svg")));
      panel5->box.size = box.size;
      panel5->visible = false;
      addChild(panel5);
    }
    {
      panel6 = new SvgPanel();
      panel6->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Resonestor.svg")));
      panel6->box.size = box.size;
      panel6->visible = false;
      addChild(panel6);
    }

    const float x1 = 5;
    const float x2 = 35;
    const float x3 = 65;
    const float y1 = 25.0f;
    const float yh = 29.0f;

    struct FreezeLight : GreenLight {
      FreezeLight() {
        box.size = Vec(28 - 16, 28 - 16);
        bgColor = componentlibrary::SCHEME_BLACK_TRANSPARENT;
      }
    };

    addInput(createInput<sp_Port>(Vec(x1, .25 * yh + y1), module, Smoke::TRIG_INPUT));

    addInput(createInput<sp_Port>(Vec(x1, 1.25 * yh + y1), module, Smoke::FREEZE_INPUT));
    addParam(createParam<LEDButton>(Vec(x2, 1.35 * yh + y1), module, Smoke::FREEZE_PARAM));
    addChild(createLight<FreezeLight>(Vec(x2 + 3, 1.35 * yh + y1 + 3), module, Smoke::FREEZE_LIGHT));
    addParam(createParam<LEDButton>(Vec(x3, 1.35 * yh + y1), module, Smoke::REVERSE_PARAM));
    addChild(createLight<FreezeLight>(Vec(x3 + 3, 1.35 * yh + y1 + 3), module, Smoke::REVERSE_LIGHT));
    addParam(createParam<sp_SmallBlackKnob>(Vec(x1, 2.5 * yh + y1), module, Smoke::POSITION_PARAM));
    addParam(createParam<sp_SmallBlackKnob>(Vec(x2, 2.5 * yh + y1), module, Smoke::SIZE_PARAM));
    addParam(createParam<sp_SmallBlackKnob>(Vec(x3, 2.5 * yh + y1), module, Smoke::PITCH_PARAM));

    addParam(createParam<sp_SmallBlackKnob>(Vec(x1, 5.0 * yh + y1), module, Smoke::DENSITY_PARAM));
    addParam(createParam<sp_SmallBlackKnob>(Vec(x2, 5.0 * yh + y1), module, Smoke::TEXTURE_PARAM));
    addParam(createParam<sp_SmallBlackKnob>(Vec(x3, 5.0 * yh + y1), module, Smoke::BLEND_PARAM));

    addParam(createParam<sp_SmallBlackKnob>(Vec(x1, 7.5 * yh + y1), module, Smoke::SPREAD_PARAM));
    addParam(createParam<sp_SmallBlackKnob>(Vec(x2, 7.5 * yh + y1), module, Smoke::FEEDBACK_PARAM));
    addParam(createParam<sp_SmallBlackKnob>(Vec(x3, 7.5 * yh + y1), module, Smoke::REVERB_PARAM));

    addInput(createInput<sp_Port>(Vec(x1, 3.25 * yh + y1), module, Smoke::POSITION_INPUT));
    addInput(createInput<sp_Port>(Vec(x2, 3.25 * yh + y1), module, Smoke::SIZE_INPUT));
    addInput(createInput<sp_Port>(Vec(x3, 3.25 * yh + y1), module, Smoke::PITCH_INPUT));

    addInput(createInput<sp_Port>(Vec(x1, 5.75 * yh + y1), module, Smoke::DENSITY_INPUT));
    addInput(createInput<sp_Port>(Vec(x2, 5.75 * yh + y1), module, Smoke::TEXTURE_INPUT));
    addInput(createInput<sp_Port>(Vec(x3, 5.75 * yh + y1), module, Smoke::BLEND_INPUT));

    addInput(createInput<sp_Port>(Vec(x1, 8.25 * yh + y1), module, Smoke::SPREAD_INPUT));
    addInput(createInput<sp_Port>(Vec(x2, 8.25 * yh + y1), module, Smoke::FEEDBACK_INPUT));
    addInput(createInput<sp_Port>(Vec(x3, 8.25 * yh + y1), module, Smoke::REVERB_INPUT));

    addParam(createParam<sp_SmallBlackKnob>(Vec(x2, 10 * yh + y1), module, Smoke::IN_GAIN_PARAM));

    addInput(createInput<sp_Port>(Vec(x1, 9.5 * yh + y1), module, Smoke::IN_L_INPUT));
    addInput(createInput<sp_Port>(Vec(x1, 10.5 * yh + y1), module, Smoke::IN_R_INPUT));
    addOutput(createOutput<sp_Port>(Vec(x3, 9.5 * yh + y1), module, Smoke::OUT_L_OUTPUT));
    addOutput(createOutput<sp_Port>(Vec(x3, 10.5 * yh + y1), module, Smoke::OUT_R_OUTPUT));

    addChild(createLight<MediumLight<GreenRedLight>>(Vec(x3 + 10, 40), module, Smoke::MIX_GREEN_LIGHT));
    addChild(createLight<MediumLight<GreenRedLight>>(Vec(x3 + 10, 30), module, Smoke::PAN_GREEN_LIGHT));
    addChild(createLight<MediumLight<GreenRedLight>>(Vec(x3 + 10, 20), module, Smoke::FEEDBACK_GREEN_LIGHT));
    addChild(createLight<MediumLight<GreenRedLight>>(Vec(x3 + 10, 10), module, Smoke::REVERB_GREEN_LIGHT));
  }

  void appendContextMenu(Menu *menu) override {
    Smoke *clouds = dynamic_cast<Smoke *>(module);
    assert(clouds);

    struct CloudsModeItem : MenuItem {
      Smoke *clouds;
      clouds::PlaybackMode mode;

      void onAction(const event::Action &e) override {
        clouds->playbackmode = mode;
      }
      void step() override {
        rightText = (clouds->playbackmode == mode) ? "✔" : "";
        MenuItem::step();
      }
    };

    struct CloudsMonoItem : MenuItem {
      Smoke *clouds;
      bool setting;

      void onAction(const event::Action &e) override {
        clouds->mono = setting;
      }
      void step() override {
        rightText = (clouds->mono == setting) ? "✔" : "";
        MenuItem::step();
      }
    };

    struct CloudsLofiItem : MenuItem {
      Smoke *clouds;
      bool setting;

      void onAction(const event::Action &e) override {
        clouds->lofi = setting;
      }
      void step() override {
        rightText = (clouds->lofi == setting) ? "✔" : "";
        MenuItem::step();
      }
    };

    struct CloudsBufferItem : MenuItem {
      Smoke *clouds;
      int setting;

      void onAction(const event::Action &e) override {
        clouds->buffersize = setting;
      }
      void step() override {
        rightText = (clouds->buffersize == setting) ? "✔" : "";
        MenuItem::step();
      }
    };

    menu->addChild(construct<MenuLabel>());

    menu->addChild(construct<MenuLabel>(&MenuLabel::text, "MODE"));
    menu->addChild(construct<CloudsModeItem>(&MenuItem::text, "GRANULAR", &CloudsModeItem::clouds, clouds, &CloudsModeItem::mode, clouds::PLAYBACK_MODE_GRANULAR));
    menu->addChild(construct<CloudsModeItem>(&MenuItem::text, "SPECTRAL", &CloudsModeItem::clouds, clouds, &CloudsModeItem::mode, clouds::PLAYBACK_MODE_SPECTRAL));
    menu->addChild(construct<CloudsModeItem>(&MenuItem::text, "LOOPING_DELAY", &CloudsModeItem::clouds, clouds, &CloudsModeItem::mode, clouds::PLAYBACK_MODE_LOOPING_DELAY));
    menu->addChild(construct<CloudsModeItem>(&MenuItem::text, "STRETCH", &CloudsModeItem::clouds, clouds, &CloudsModeItem::mode, clouds::PLAYBACK_MODE_STRETCH));
    menu->addChild(construct<CloudsModeItem>(&MenuItem::text, "OLIVERB", &CloudsModeItem::clouds, clouds, &CloudsModeItem::mode, clouds::PLAYBACK_MODE_OLIVERB));
    menu->addChild(construct<CloudsModeItem>(&MenuItem::text, "RESONESTOR", &CloudsModeItem::clouds, clouds, &CloudsModeItem::mode, clouds::PLAYBACK_MODE_RESONESTOR));
    menu->addChild(construct<MenuLabel>(&MenuLabel::text, "STEREO/MONO"));
    menu->addChild(construct<CloudsMonoItem>(&MenuItem::text, "STEREO", &CloudsMonoItem::clouds, clouds, &CloudsMonoItem::setting, false));
    menu->addChild(construct<CloudsMonoItem>(&MenuItem::text, "MONO", &CloudsMonoItem::clouds, clouds, &CloudsMonoItem::setting, true));

    menu->addChild(construct<MenuLabel>(&MenuLabel::text, "HIFI/LOFI"));
    menu->addChild(construct<CloudsLofiItem>(&MenuItem::text, "HIFI", &CloudsLofiItem::clouds, clouds, &CloudsLofiItem::setting, false));
    menu->addChild(construct<CloudsLofiItem>(&MenuItem::text, "LOFI", &CloudsLofiItem::clouds, clouds, &CloudsLofiItem::setting, true));

#ifdef BUFFERRESIZING
    // disable by default as it seems to make alternative modes unstable
    menu->addChild(construct<MenuLabel>(&MenuLabel::text, "BUFFER SIZE (EXPERIMENTAL)"));
    menu->addChild(construct<CloudsBufferItem>(&MenuItem::text, "ORIGINAL", &CloudsBufferItem::clouds, clouds, &CloudsBufferItem::setting, 1));
    menu->addChild(construct<CloudsBufferItem>(&MenuItem::text, "2X", &CloudsBufferItem::clouds, clouds, &CloudsBufferItem::setting, 2));
    menu->addChild(construct<CloudsBufferItem>(&MenuItem::text, "4X", &CloudsBufferItem::clouds, clouds, &CloudsBufferItem::setting, 4));
    menu->addChild(construct<CloudsBufferItem>(&MenuItem::text, "8X", &CloudsBufferItem::clouds, clouds, &CloudsBufferItem::setting, 8));
#endif
  }

  void step() override {
    Smoke *smoke = dynamic_cast<Smoke *>(module);

    if (smoke) {
      panel1->visible = true;
      panel2->visible = false;
      panel3->visible = false;
      panel4->visible = false;
      panel5->visible = false;
      panel6->visible = false;
      if (smoke->playbackmode == clouds::PLAYBACK_MODE_SPECTRAL) {
        panel1->visible = false;
        panel2->visible = true;
      }
      if (smoke->playbackmode == clouds::PLAYBACK_MODE_LOOPING_DELAY) {
        panel1->visible = false;
        panel3->visible = true;
      }
      if (smoke->playbackmode == clouds::PLAYBACK_MODE_STRETCH) {
        panel1->visible = false;
        panel4->visible = true;
      }
      if (smoke->playbackmode == clouds::PLAYBACK_MODE_OLIVERB) {
        panel1->visible = false;
        panel5->visible = true;
      }
      if (smoke->playbackmode == clouds::PLAYBACK_MODE_RESONESTOR) {
        panel1->visible = false;
        panel6->visible = true;
      }
    }

    ModuleWidget::step();
  }
};

Model *modelSmoke = createModel<Smoke, SmokeWidget>("Smoke");
