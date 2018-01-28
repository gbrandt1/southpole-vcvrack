#include "Southpole.hpp"
#include "dsp/digital.hpp"


struct Piste : Module {
	enum ParamIds {
		GAIN_PARAM,
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

	float env1 = 0.0;
	float env2 = 0.0;
	SchmittTrigger trigger1;
	SchmittTrigger trigger2;
	SchmittTrigger mute;

	Piste() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
		trigger1.setThresholds(0.0, 1.0);
	}
	void step() override;

};



void Piste::step() {
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
	float v = inputs[IN_INPUT].value * 10.*(scale1 * env1 + scale2 * env2);
	//if (lin.active)
	//	v *= clampf(lin.value / 10.0, 0.0, 1.0);
	//const float expBase = 50.0;



	outputs[OUT_OUTPUT].value = v;

	// Lights
	lights[DECAY1_LIGHT].value = (env1 > 0.) ? 1.0 : 0.0;
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

	const float x1 = 4.;	
	const float x2 = 30.;

	const float y1 = 40.;
	const float yh = 31.;
	
	addParam(createParam<sp_SmallBlackKnob>(Vec(x1, y1+1*yh), module, Piste::DECAY1_PARAM, 0.0, 1.0, 0.5));
	addParam(createParam<sp_SmallBlackKnob>(Vec(x2, y1+1*yh), module, Piste::DECAY2_PARAM, 0.0, 1.0, 1.));

	addParam(createParam<sp_SmallBlackKnob>(Vec(x1, y1+2*yh), module, Piste::SCALE1_PARAM, 0.0, 1.0, .5));
	addParam(createParam<sp_SmallBlackKnob>(Vec(x2, y1+2*yh), module, Piste::SCALE2_PARAM, 0.0, 1.0, 1.));

	addInput(createInput<sp_Port>(Vec(x1, y1+3*yh), module, Piste::DECAY1_INPUT));
	addInput(createInput<sp_Port>(Vec(x2, y1+3*yh), module, Piste::DECAY2_INPUT));

	addInput(createInput<sp_Port>(Vec(x1, y1+4*yh), module, Piste::TRIG1_INPUT));
	addInput(createInput<sp_Port>(Vec(x2, y1+4*yh), module, Piste::TRIG2_INPUT));
	addInput(createInput<sp_Port>(Vec(x1, y1+5*yh), module, Piste::MUTE_INPUT));

	addOutput(createOutput<sp_Port>(Vec(x1, y1+6*yh), module, Piste::ENV1_OUTPUT));
	addOutput(createOutput<sp_Port>(Vec(x2, y1+6*yh), module, Piste::ENV2_OUTPUT));

	addInput(createInput<sp_Port>(Vec(x1, y1+7*yh), module, Piste::IN_INPUT));
	addOutput(createOutput<sp_Port>(Vec(x2, y1+7*yh), module, Piste::OUT_OUTPUT));

	addChild(createLight<SmallLight<RedLight>>(Vec(x1, y1+8*yh), module, Piste::DECAY1_LIGHT));
}
