
#include "Southpole.hpp"
#include "dsp/digital.hpp"
#include "VAStateVariableFilter.h"


struct Piste : Module {
	enum ParamIds {
		LP_PARAM,
		HP_PARAM,
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
    VAStateVariableFilter bpFilter;

	float env1 = 0.0;
	float env2 = 0.0;
	SchmittTrigger trigger1;
	SchmittTrigger trigger2;
	SchmittTrigger mute;

	Piste() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {

        params.resize(NUM_PARAMS);
        inputs.resize(NUM_INPUTS);
        outputs.resize(NUM_OUTPUTS);
        lights.resize(NUM_LIGHTS);

		//trigger1.setThresholds(0.0, 2.0);
		//trigger1.setThresholds(0.0, 2.0);

        lpFilter.setFilterType(SVFLowpass);
        hpFilter.setFilterType(SVFHighpass);        
        bpFilter.setFilterType(SVFBandpass);
	}
	void step() override;

    unsigned timer;

};



void Piste::step() {
    
	float drive = clampf(params[DRIVE_PARAM].value, 0, 1.0);

    float lpgain = pow( 20., clampf(params[LP_PARAM].value, -1.0, 1.0));
    float hpgain = pow( 20., clampf(params[HP_PARAM].value, -1.0, 1.0));

	float decay1 = 			clampf(params[DECAY1_PARAM].value + inputs[DECAY1_INPUT].value / 10.0, 0.0, 1.0);
	float decay2 = decay1 * clampf(params[DECAY2_PARAM].value + inputs[DECAY2_INPUT].value / 10.0, 0.0, 1.0);

	float scale1 = 			clampf(params[SCALE1_PARAM].value, 0.0, 1.0);
	float scale2 = scale1 * clampf(params[SCALE2_PARAM].value, 0.0, 1.0);

	// Gate or trigger
	bool muted = inputs[MUTE_INPUT].normalize(0.) >= 1.0;
	
	if (!muted) { 
		if (trigger1.process(inputs[TRIG1_INPUT].value)) {
			env1 = 1.;
		}
		if (trigger2.process(inputs[TRIG2_INPUT].value)) {
			env2 = 1.;
		}
	}
	const float base = 20000.0;
	const float maxTime = 1.0;

	if (decay1 < 1e-4) {
		env1 = 0.;
	}
	else {
		env1 += powf(base, 1. - decay1) / maxTime * ( - env1) / engineGetSampleRate();
	}

	if (decay2 < 1e-4) {
		env2 = 0.;
	}
	else {
		env2 += powf(base, 1. - decay2) / maxTime * ( - env2) / engineGetSampleRate();
	}

	outputs[ENV1_OUTPUT].value = 10.*scale1 * env1;
	outputs[ENV2_OUTPUT].value = 10.*scale2 * env2;

	// VCA
	float v = inputs[IN_INPUT].value;


//        timer++;
//        if (timer > engineGetSampleRate()/2.) {
//            timer = 0;
//            printf("%f %f %f %f\n", lp_cutoff, bp2_cutoff, bp3_cutoff, hp_cutoff);
//        }
	 
	// DRIVE

	v = (1.-drive)*v + drive * 10.*tanhf(10.*drive*v);

    // EQ
	lpFilter.setQ(.5);
    lpFilter.setSampleRate(engineGetSampleRate());
    lpFilter.setCutoffFreq(250.);

    hpFilter.setQ(.5);
    hpFilter.setSampleRate(engineGetSampleRate());
    hpFilter.setCutoffFreq(2000.);
/*
	lp2Filter.setQ(.5);
    lp2Filter.setSampleRate(engineGetSampleRate());
    lp2Filter.setCutoffFreq(4000.);
*/
    bpFilter.setQ(.25);
    bpFilter.setSampleRate(engineGetSampleRate());
    bpFilter.setCutoffFreq(600.);

    float lpout  = lpFilter.processAudioSample( v, 1);
    float hpout  = hpFilter.processAudioSample( v, 1);
    float bpout  = bpFilter.processAudioSample( v, 1);

	//float lp2pout = v; //lp2Filter.processAudioSample( v, 1);
	//float out = hp2Filter.processAudioSample( v, 1);

	//float mids = v - lpout - hpout;

	// VCA
	v = lpout*lpgain + 4.*bpout + hpout*hpgain;
	//if (lpgain >= 1.) {
	//	v = v - lpout + lpout * lpgain;
	//} else {
	//	v = hp2out/lpgain;
	//}
	//v = v * 10.* scale1 * env1 * (1. + 10* scale2 * env2);


	outputs[OUT_OUTPUT].value = v;

	// Lights
	lights[DECAY1_LIGHT].value = env1;
	lights[DECAY2_LIGHT].value = env2;
}


PisteWidget::PisteWidget() {
	Piste *module = new Piste();
	setModule(module);
	box.size = Vec(15*4, 380);

	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/Piste.svg")));
		addChild(panel);
	}

	const float x1 = 5.;	
	const float x2 = 33.;

	const float y1 = 47.;
	const float yh = 31.;
	

	addInput(createInput<sp_Port>(Vec(x1, y1), module, Piste::IN_INPUT));
	addParam(createParam<sp_SmallBlackKnob>(Vec(x2, y1), module, Piste::DRIVE_PARAM, 0.0, 1.0, 0.0));
	
	addParam(createParam<sp_SmallBlackKnob>(Vec(x1, y1+1*yh), module, Piste::LP_PARAM, -1.0, 1.0, 0.));
	addParam(createParam<sp_SmallBlackKnob>(Vec(x2, y1+1*yh), module, Piste::HP_PARAM, -1.0, 1.0, 0.));

	addChild(createLight<SmallLight<RedLight>>(Vec(x1, y1+2*yh), module, Piste::DECAY1_LIGHT));
	addChild(createLight<SmallLight<RedLight>>(Vec(x2, y1+2*yh), module, Piste::DECAY2_LIGHT));

	addInput(createInput<sp_Port>(Vec(x1, y1+2.5*yh), module, Piste::TRIG1_INPUT));
	addInput(createInput<sp_Port>(Vec(x2, y1+2.5*yh), module, Piste::TRIG2_INPUT));

	addParam(createParam<sp_SmallBlackKnob>(Vec(x1, y1+3.5*yh), module, Piste::SCALE1_PARAM, 0.0, 1.0, .5));
	addParam(createParam<sp_SmallBlackKnob>(Vec(x2, y1+3.5*yh), module, Piste::SCALE2_PARAM, 0.0, 1.0, 1.));

	addParam(createParam<sp_SmallBlackKnob>(Vec(x1, y1+4.5*yh), module, Piste::DECAY1_PARAM, 0.0, 1.0, 0.5));
	addParam(createParam<sp_SmallBlackKnob>(Vec(x2, y1+4.5*yh), module, Piste::DECAY2_PARAM, 0.0, 1.0, 1.));
	addInput(createInput<sp_Port>(Vec(x1, y1+5.25*yh), module, Piste::DECAY1_INPUT));
	addInput(createInput<sp_Port>(Vec(x2, y1+5.25*yh), module, Piste::DECAY2_INPUT));

	addOutput(createOutput<sp_Port>(Vec(x1, y1+6.5*yh), module, Piste::ENV1_OUTPUT));
	addOutput(createOutput<sp_Port>(Vec(x2, y1+6.5*yh), module, Piste::ENV2_OUTPUT));

	addInput(createInput<sp_Port>(Vec(0.5*(x1+x2), 7.75*yh+y1), module, Piste::MUTE_INPUT));
	addOutput(createOutput<sp_Port>(Vec(0.5*(x1+x2), y1+9*yh), module, Piste::OUT_OUTPUT));

}
