#include "Southpole.hpp"

#define NBUF 6

struct Rakes : Module {
  enum ParamIds {
    DECAY_PARAM,
    QUANT_PARAM,
    MIX_PARAM,
    TUNE1_PARAM,
    TUNE2_PARAM,
    TUNE3_PARAM,
    TUNE4_PARAM,
    TUNE5_PARAM,
    TUNE6_PARAM,
    FINE1_PARAM,
    FINE2_PARAM,
    FINE3_PARAM,
    FINE4_PARAM,
    FINE5_PARAM,
    FINE6_PARAM,
    GAIN1_PARAM,
    GAIN2_PARAM,
    GAIN3_PARAM,
    GAIN4_PARAM,
    GAIN5_PARAM,
    GAIN6_PARAM,
    NUM_PARAMS
  };
  enum InputIds {
    INL_INPUT,
    INR_INPUT,
    DECAY_INPUT,
    MIX_INPUT,
    TUNE1_INPUT,
    TUNE2_INPUT,
    TUNE3_INPUT,
    TUNE4_INPUT,
    TUNE5_INPUT,
    TUNE6_INPUT,
    //GAIN1_INPUT,GAIN2_INPUT,
    //GAIN3_INPUT,GAIN4_INPUT,
    //GAIN5_INPUT,GAIN6_INPUT,
    NUM_INPUTS
  };
  enum OutputIds {
    OUTL_OUTPUT,
    OUTR_OUTPUT,
    NUM_OUTPUTS
  };
  enum LightIds {
    NUM_LIGHTS
  };

  //dsp::SchmittTrigger clock;

  float *bufl[NBUF];
  float *bufr[NBUF];
  int maxsize;

  int headl[NBUF];
  int headr[NBUF];

  int sizel[NBUF];
  int sizer[NBUF];

  int lastsizel[NBUF];
  int lastsizer[NBUF];

  Rakes() {
    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

    maxsize = APP->engine->getSampleRate();

    for (int j = 0; j < NBUF; j++) {
      bufl[j] = new float[maxsize];
      bufr[j] = new float[maxsize];
      for (int i = 0; i < maxsize; i++) {
        bufl[j][i] = 0;
        bufr[j][i] = 0;
      }
      headl[j] = 0;
      headr[j] = 0;
    }

    configParam(Rakes::DECAY_PARAM, 0.0, 1.0, 0.0, "");
    for (int j = 0; j < NBUF; j++) {
      configParam(Rakes::TUNE1_PARAM + j, -5.0, 5.5, 0.0, "");
      configParam(Rakes::FINE1_PARAM + j, -1.0, 1.0, 0.0, "");
      configParam(Rakes::GAIN1_PARAM + j, 0.0, 1.0, 0.0, "");
    }
    configParam(Rakes::QUANT_PARAM, 0.0, 1.0, 0.0, "");
    configParam(Rakes::MIX_PARAM, 0.0, 1.0, 0.5, "");
  }

  float xm1 = 0;
  float ym1 = 0;

  float dcblock(float x) {
    float y = x - xm1 + 0.995 * ym1;
    xm1 = x;
    ym1 = y;
    return y;
  }

  void process(const ProcessArgs &args) override;
};

void Rakes::process(const ProcessArgs &args) {

  //float mix  = clamp(params[MIX_PARAM].getValue() + inputs[MIX_INPUT].getNormalVoltage(0.) / 10.0, 0.0, 1.0);
  float mix = params[MIX_PARAM].getValue();
  float rate = clamp(params[DECAY_PARAM].getValue() + inputs[DECAY_INPUT].getNormalVoltage(0.) / 10.0, 0.0f, .99f);

  const float f0 = 261.626;
  float inl = inputs[INL_INPUT].getNormalVoltage(0.);
  float inr = inputs[INR_INPUT].getNormalVoltage(inl);

  float sumoutl = 0;
  float sumoutr = 0;
  float sumgain = 1.;

  for (int j = 0; j < NBUF; j++) {
    //float gain = clamp(params[GAIN1_PARAM + j].getValue() + inputs[GAIN1_INPUT + j].getNormalVoltage(0.) / 10.0, 0.0, 1.0);
    float gain = params[GAIN1_PARAM + j].getValue();
    if (gain < 1e-3)
      continue;
    sumgain += gain;

    float tune = clamp(params[TUNE1_PARAM + j].getValue() + inputs[TUNE1_INPUT + j].getNormalVoltage(0.), -5.0, 5.5);
    float fine = clamp(params[FINE1_PARAM + j].getValue(), -1.0, 1.0);

    if (params[QUANT_PARAM].getValue() > 0.5) {
      tune = round(12. * tune) / 12.;
    }

    float freql = f0 * powf(2., tune + fine / 12.);
    float freqr = f0 * powf(2., tune - fine / 12.);

    // key follow
    //float fb = crossfade(f0, freq, follow);

    // full follow decay rate is T60 time
    float fbl = pow(10, -3. / freql / fabs(5. * rate));
    float fbr = pow(10, -3. / freqr / fabs(5. * rate));

    //fb = fb * ((0. < rate) - (rate < 0.));

    //printf("%f %f %f\n",lfreq,rate,fb);

    sizel[j] = maxsize / freqr;
    sizer[j] = maxsize / freql;

    if (sizel[j] > lastsizel[j]) {
      for (int i = sizel[j]; i < lastsizel[j]; i++)
        bufl[i] = 0;
    }
    if (sizel[j] > lastsizer[j]) {
      for (int i = sizer[j]; i < lastsizer[j]; i++)
        bufr[i] = 0;
    }

    lastsizel[j] = maxsize / freqr;
    lastsizer[j] = maxsize / freql;

    float outl = bufl[j][headl[j]];
    float outr = bufr[j][headr[j]];

    bufl[j][headl[j]] = inl + fbl * outl;
    bufr[j][headr[j]] = inr + fbr * outr;
    headl[j]++;
    headr[j]++;
    if (headl[j] > sizel[j]) {
      headl[j] = 0;
    }
    if (headr[j] > sizer[j]) {
      headr[j] = 0;
    }

    sumoutl += gain * outl;
    sumoutr += gain * outr;
  }

  sumoutl = clamp(dcblock(sumoutl) / sumgain, -10., 10.); //in + gain*out;
  sumoutr = clamp(dcblock(sumoutr) / sumgain, -10., 10.); //in + gain*out;

  outputs[OUTL_OUTPUT].setVoltage(crossfade(inl, sumoutl, mix));
  outputs[OUTR_OUTPUT].setVoltage(crossfade(inr, sumoutr, mix));
}

struct RakesWidget : ModuleWidget {

  RakesWidget(Rakes *module) {
    setModule(module);

    box.size = Vec(15 * 8, 380);

    setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Rakes.svg")));

    const float x1 = 5.;
    const float x2 = 35.;
    const float x3 = 65.;
    const float x4 = 95.;
    const float y1 = 40.;
    const float yh = 32.;

    addInput(createInput<sp_Port>(Vec(x2, y1 + 0 * yh), module, Rakes::DECAY_INPUT));
    addParam(createParam<sp_SmallBlackKnob>(Vec(x3, y1 + 0 * yh), module, Rakes::DECAY_PARAM));
    //addParam(createParam<sp_SmallBlackKnob>	(Vec(x3, y1+0*yh), module, Rakes::FOLLOW_PARAM));

    for (int j = 0; j < NBUF; j++) {
      addInput(createInput<sp_Port>(Vec(x1, y1 + (j + 1.5) * yh), module, Rakes::TUNE1_INPUT + j));
      addParam(createParam<sp_SmallBlackKnob>(Vec(x2, y1 + (j + 1.5) * yh), module, Rakes::TUNE1_PARAM + j));
      addParam(createParam<sp_SmallBlackKnob>(Vec(x3, y1 + (j + 1.5) * yh), module, Rakes::FINE1_PARAM + j));
      addParam(createParam<sp_SmallBlackKnob>(Vec(x4, y1 + (j + 1.5) * yh), module, Rakes::GAIN1_PARAM + j));
    }

    addInput(createInput<sp_Port>(Vec(x1, y1 + 8 * yh), module, Rakes::INL_INPUT));
    addInput(createInput<sp_Port>(Vec(x1, y1 + 9 * yh), module, Rakes::INR_INPUT));

    addParam(createParam<CKSS>(Vec(x2, y1 + 7.5 * yh), module, Rakes::QUANT_PARAM));
    addParam(createParam<sp_SmallBlackKnob>(Vec((x2 + x3) / 2., y1 + 8.5 * yh), module, Rakes::MIX_PARAM));

    addOutput(createOutput<sp_Port>(Vec(x4, y1 + 8 * yh), module, Rakes::OUTL_OUTPUT));
    addOutput(createOutput<sp_Port>(Vec(x4, y1 + 9 * yh), module, Rakes::OUTR_OUTPUT));
  }
};

Model *modelRakes = createModel<Rakes, RakesWidget>("Rakes");
