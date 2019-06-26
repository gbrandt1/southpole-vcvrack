
#include "Southpole.hpp"
#include "VAStateVariableFilter.h"


struct Piste : Module {
	enum ParamIds {
		FREQ_PARAM,
		RESO_PARAM,
		DECAY1_PARAM,
		DECAY2_PARAM,
		SCALE1_PARAM,
		SCALE2_PARAM,
		DRIVE_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		IN_INPUT,
		DECAY1_INPUT,
		DECAY2_INPUT,
		TRIG1_INPUT,
		TRIG2_INPUT,
		SCALE1_INPUT,
		SCALE2_INPUT,
		MUTE_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		ENV1_OUTPUT,
		ENV2_OUTPUT,
		OUT_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		DECAY1_LIGHT,
		DECAY2_LIGHT,
		NUM_LIGHTS
	};

    VAStateVariableFilter lpFilter;
    VAStateVariableFilter hpFilter;

	float env1 = 0.0;
	float env2 = 0.0;
	dsp::SchmittTrigger trigger1;
	dsp::SchmittTrigger trigger2;
	dsp::SchmittTrigger mute;

  Piste() {
    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

    params.resize(NUM_PARAMS);
    inputs.resize(NUM_INPUTS);
    outputs.resize(NUM_OUTPUTS);
    lights.resize(NUM_LIGHTS);

    //trigger1.setThresholds(0.0, 2.0);
    //trigger1.setThresholds(0.0, 2.0);

    lpFilter.setFilterType(SVFLowpass);
    hpFilter.setFilterType(SVFHighpass);        

    configParam(Piste::DRIVE_PARAM, 0.0, 1.0, 0.0, "");
    configParam(Piste::FREQ_PARAM, -1.0, 1.0, 0., "");
    configParam(Piste::RESO_PARAM, .0, 1.0, 0., "");
    configParam(Piste::SCALE1_PARAM, 0.0, 1.0, .5, "");
    configParam(Piste::SCALE2_PARAM, 0.0, 1.0, 1., "");
    configParam(Piste::DECAY1_PARAM, 0.0, 1.0, 0.5, "");
    configParam(Piste::DECAY2_PARAM, 0.0, 1.0, 1., "");
  }

	void process(const ProcessArgs &args) override;

    unsigned timer;

};



void Piste::process(const ProcessArgs &args) {
    
	float drive = clamp(params[DRIVE_PARAM].getValue(), 0.0f, 1.0f);

	float freq = clamp(params[FREQ_PARAM].getValue(), -1.0f, 1.0f);
    float reso = clamp(params[RESO_PARAM].getValue(), 0.0f, 1.0f);

	float decay1 = 			clamp(params[DECAY1_PARAM].getValue() + inputs[DECAY1_INPUT].getVoltage() / 10.0f, 0.0f, 1.0f);
	float decay2 = decay1 * clamp(params[DECAY2_PARAM].getValue() + inputs[DECAY2_INPUT].getVoltage() / 10.0f, 0.0f, 1.0f);

	float scale1 = 			clamp(params[SCALE1_PARAM].getValue(), 0.0, 1.0);
	float scale2 = scale1 * clamp(params[SCALE2_PARAM].getValue(), 0.0, 1.0);

	bool muted = inputs[MUTE_INPUT].normalize(0.) >= 1.0;
	
	if (!muted) { 
		if (trigger1.process(inputs[TRIG1_INPUT].getVoltage())) {
			env1 = 1.;
		}
		if (trigger2.process(inputs[TRIG2_INPUT].getVoltage())) {
			env2 = 1.;
		}
	}
	const float base = 20000.0;
	const float maxTime = 1.0;

	if (decay1 < 1e-4) { env1 = 0.;
	} else {
		env1 += powf(base, 1. - decay1) / maxTime * ( - env1) / args.sampleRate;
	}

	if (decay2 < 1e-4) { env2 = 0.;
	} else {
		env2 += powf(base, 1. - decay2) / maxTime * ( - env2) / args.sampleRate;
	}

	outputs[ENV1_OUTPUT].setVoltage(10.*scale1 * env1);
	outputs[ENV2_OUTPUT].setVoltage(10.*scale2 * env2);

	float v = inputs[IN_INPUT].getVoltage();
	 
	// DRIVE
	v = (1.-drive)*v + drive * 10.*tanhf(10.*drive*v);

    const float f0 = 261.626;		
    const float rmax = 0.9995; // Qmax = 1000

	float fout = v;

    // FILTER	
	if (freq < 0.) {

	    float lp_cutoff  = f0 * powf(2.f, 8.*(freq+1.)-4.);
		lpFilter.setResonance(reso*rmax);
    	lpFilter.setSampleRate(args.sampleRate);
    	lpFilter.setCutoffFreq(lp_cutoff);
    	fout  = lpFilter.processAudioSample( v, 1);

	} else if ( freq > 0.) {

	    float hp_cutoff  = f0 * powf(2.f, 8.*freq-3.);
    	hpFilter.setResonance(reso*rmax);
    	hpFilter.setSampleRate(args.sampleRate);
    	hpFilter.setCutoffFreq(hp_cutoff);
    	fout  = hpFilter.processAudioSample( v, 1);
	}

	// VCA	
	v = fout * 10.*scale1 * env1 * (1. + 10* scale2 * env2);

	outputs[OUT_OUTPUT].setVoltage(v);

	// Lights
	lights[DECAY1_LIGHT].value = env1;
	lights[DECAY2_LIGHT].value = env2;
}

struct PisteWidget : ModuleWidget {	
	
	PisteWidget(Module *module)  {
		setModule(module);

		box.size = Vec(15*4, 380);

		{
			SVGPanel *panel = new SVGPanel();
			panel->box.size = box.size;
			panel->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Piste.svg")));
			addChild(panel);
		}

		const float x1 = 5.;	
		const float x2 = 36.;

		const float y1 = 47.;
		const float yh = 31.;

		addInput(createInput<sp_Port>(Vec(x1, y1), module, Piste::IN_INPUT));
		addParam(createParam<sp_SmallBlackKnob>(Vec(x2, y1), module, Piste::DRIVE_PARAM));
		
		addParam(createParam<sp_SmallBlackKnob>(Vec(x1, y1+1*yh), module, Piste::FREQ_PARAM));
		addParam(createParam<sp_SmallBlackKnob>(Vec(x2, y1+1*yh), module, Piste::RESO_PARAM));

		addChild(createLight<SmallLight<RedLight>>(Vec(x1+6, y1+2*yh+5), module, Piste::DECAY1_LIGHT));
		addChild(createLight<SmallLight<RedLight>>(Vec(x2+6, y1+2*yh+5), module, Piste::DECAY2_LIGHT));

		addInput(createInput<sp_Port>(Vec(x1, y1+2.5*yh), module, Piste::TRIG1_INPUT));
		addInput(createInput<sp_Port>(Vec(x2, y1+2.5*yh), module, Piste::TRIG2_INPUT));

		addParam(createParam<sp_SmallBlackKnob>(Vec(x1, y1+3.5*yh), module, Piste::SCALE1_PARAM));
		addParam(createParam<sp_SmallBlackKnob>(Vec(x2, y1+3.5*yh), module, Piste::SCALE2_PARAM));

		addParam(createParam<sp_SmallBlackKnob>(Vec(x1, y1+4.5*yh), module, Piste::DECAY1_PARAM));
		addParam(createParam<sp_SmallBlackKnob>(Vec(x2, y1+4.5*yh), module, Piste::DECAY2_PARAM));
		addInput(createInput<sp_Port>(Vec(x1, y1+5.25*yh), module, Piste::DECAY1_INPUT));
		addInput(createInput<sp_Port>(Vec(x2, y1+5.25*yh), module, Piste::DECAY2_INPUT));

		addOutput(createOutput<sp_Port>(Vec(x1, y1+6.5*yh), module, Piste::ENV1_OUTPUT));
		addOutput(createOutput<sp_Port>(Vec(x2, y1+6.5*yh), module, Piste::ENV2_OUTPUT));

		addInput(createInput<sp_Port>(Vec(0.5*(x1+x2), 7.75*yh+y1), module, Piste::MUTE_INPUT));
		addOutput(createOutput<sp_Port>(Vec(0.5*(x1+x2), y1+9*yh), module, Piste::OUT_OUTPUT));

	}
};

Model *modelPiste 	= createModel<Piste,PisteWidget>("Piste");
