#include <iostream>
#include <stdlib.h>

#include "Southpole.hpp"
#include "dsp/digital.hpp"
#include "VAStateVariableFilter.h"

struct Etagere : Module {
    enum ParamIds {
        FREQ1_PARAM,
        FREQ2_PARAM,
        FREQ3_PARAM,
        FREQ4_PARAM,
        GAIN1_PARAM,
        GAIN2_PARAM,
        GAIN3_PARAM,
        GAIN4_PARAM,
        Q2_PARAM,
        Q3_PARAM,
        NUM_PARAMS
    };
    enum InputIds {
        FREQ1_INPUT,
        FREQ2_INPUT,
        FREQ3_INPUT,
        FREQ4_INPUT,
        FREQ5_INPUT,
		GAIN1_INPUT,
		GAIN2_INPUT,
		GAIN3_INPUT,
		GAIN4_INPUT,
		GAIN5_INPUT,
		Q2_INPUT,
		Q3_INPUT,
		IN_INPUT,
        NUM_INPUTS
    };
        //HP2_OUTPUT,
        //HP3_OUTPUT,
        //BP2_OUTPUT,
        //BP3_OUTPUT,
        //LP2_OUTPUT,
        //LP3_OUTPUT,
    enum OutputIds {
		OUT_OUTPUT,
        NUM_OUTPUTS
    };

    VAStateVariableFilter lpFilter;
    VAStateVariableFilter bp2Filter;
    VAStateVariableFilter bp3Filter;
    VAStateVariableFilter hpFilter;

    Etagere() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS) {
        reset();
    }
    void step() override;

    void reset() override {
        //for (int i = 0; i < NUM_CHANNELS; i++) {
        //    state[i] = true;
        //}
    }
    void randomize() override {
        //for (int i = 0; i < NUM_CHANNELS; i++) {
        //    state[i] = (randomf() < 0.5);
        //}
    }

    json_t *toJson() override {
        json_t *rootJ = json_object();
        // states
        //json_t *statesJ = json_array();
        //for (int i = 0; i < NUM_CHANNELS; i++) {
        //    json_t *stateJ = json_boolean(state[i]);
        //    json_array_append_new(statesJ, stateJ);
        //}
        //json_object_set_new(rootJ, "states", statesJ);
        return rootJ;
    }
    void fromJson(json_t *rootJ) override {
        // states
        //json_t *statesJ = json_object_get(rootJ, "states");
        //if (statesJ) {
        //    for (int i = 0; i < NUM_CHANNELS; i++) {
        //        json_t *stateJ = json_array_get(statesJ, i);
        //        if (stateJ)
        //            state[i] = json_boolean_value(stateJ);
        //    }
        //}
    }
};

void Etagere::step() {

        lpFilter.setFilterType(SVFLowpass);
        hpFilter.setFilterType(SVFHighpass);
        
		bp2Filter.setFilterType(SVFBandpass);
        bp3Filter.setFilterType(SVFBandpass);

        lpFilter.setResonance(.5);
        hpFilter.setResonance(.5);

        lpFilter.setSampleRate(engineGetSampleRate());
        hpFilter.setSampleRate(engineGetSampleRate());
        bp2Filter.setSampleRate(engineGetSampleRate());
        bp3Filter.setSampleRate(engineGetSampleRate());

        float dry = inputs[IN_INPUT].value;

		const float fmax = 20000.;
		const float fmin = 30.;

		float g_cutoff = inputs[FREQ5_INPUT].value; 
		float g_gain   = inputs[GAIN5_INPUT].value; 
//		float g_cutoff = clampf( inputs[FREQ5_INPUT].value, -4., 4.); 
//		float g_gain   = clampf( inputs[GAIN5_INPUT].value / 10.0, 0., 1.); 

        const float f0 = 261.626;

        float lp_cutoff = clampf(f0 * powf(2.f, params[FREQ1_PARAM].value + g_cutoff),fmin,fmax);
        lpFilter.setCutoffFreq(lp_cutoff);
        lpFilter.setResonance( params[Q2_PARAM].value );
        float lpout = lpFilter.processAudioSample(dry, 1);

        float bp2_cutoff = clampf(f0 * powf(2.f, params[FREQ2_PARAM].value + g_cutoff),fmin,fmax);
        bp2Filter.setCutoffFreq(bp2_cutoff);
        bp2Filter.setResonance( params[Q2_PARAM].value );
        float bp2out = bp2Filter.processAudioSample(dry, 1);

        float bp3_cutoff = clampf(f0 * powf(2.f, params[FREQ3_PARAM].value + g_cutoff),fmin,fmax);
        bp3Filter.setCutoffFreq(bp3_cutoff);
        bp3Filter.setResonance( params[Q3_PARAM].value );
        float bp3out = bp3Filter.processAudioSample(dry, 1);

        float hp_cutoff = clampf(f0 * powf(2.f, params[FREQ4_PARAM].value + g_cutoff),fmin,fmax);
        hpFilter.setCutoffFreq(hp_cutoff);
        hpFilter.setResonance( params[Q3_PARAM].value );
        float hpout = hpFilter.processAudioSample(dry, 1);

        const float gmax = 5.62;
        //const float gmin = 0.177;

		float lpgain  = clampf(params[GAIN1_PARAM].value + g_gain, 0., gmax);
		float bp2gain = clampf(params[GAIN2_PARAM].value + g_gain, 0., gmax);
		float bp3gain = clampf(params[GAIN3_PARAM].value + g_gain, 0., gmax);
		float hpgain  = clampf(params[GAIN4_PARAM].value + g_gain, 0., gmax);
		
        outputs[OUT_OUTPUT].value = lpout*lpgain + hpout*hpgain +  bp2out*bp2gain + bp3out*bp3gain;

}

/*
template <typename BASE>
struct MuteLight : BASE {
    MuteLight() {
        this->box.size = (Vec(6.0, 6.0));
    }
};
*/

EtagereWidget::EtagereWidget() {

    Etagere *module = new Etagere();
    setModule(module);
	box.size = Vec(15*8, 380);

    //setPanel(SVG::load(assetPlugin(plugin, "res/Etagere.svg")));
	{
		SVGPanel *panel = new SVGPanel();
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/Etagere.svg")));
		panel->box.size = box.size;
		addChild(panel);	
	}

    const float x1 = 8;
	const float x2 = 63;
	const float x3 = 90;

    const float y1 = 0;
    const float yh = 25;

    const float vfmin = -3.5;
    const float vfmax = 3.7;

    const float gmax = 5.62;
    const float gmin = 0.177;

    // 880, 5000

    addParam(createParam<sp_SmallBlackKnob>(Vec(x2, y1+ 1*yh), module, Etagere::FREQ1_PARAM, vfmin, vfmax, 0.));
    addParam(createParam<sp_SmallBlackKnob>(Vec(x2, y1+ 2*yh), module, Etagere::GAIN1_PARAM,  gmin,  gmax, 1.0));

    addParam(createParam<sp_SmallBlackKnob>(Vec(x2, y1+ 3*yh), module, Etagere::FREQ2_PARAM, vfmin, vfmax, 0.));
    addParam(createParam<sp_SmallBlackKnob>(Vec(x2, y1+ 4*yh), module, Etagere::GAIN2_PARAM,  gmin,  gmax, 1.0));
    addParam(createParam<sp_SmallBlackKnob>(Vec(x2, y1+ 5*yh), module, Etagere::Q2_PARAM,      0.0,   1.0, 0.5));
    
	addParam(createParam<sp_SmallBlackKnob>(Vec(x2, y1+ 6*yh), module, Etagere::FREQ3_PARAM, vfmin, vfmax, 0.));
    addParam(createParam<sp_SmallBlackKnob>(Vec(x2, y1+ 7*yh), module, Etagere::GAIN3_PARAM,  gmin,  gmax, 1.0));
    addParam(createParam<sp_SmallBlackKnob>(Vec(x2, y1+ 8*yh), module, Etagere::Q3_PARAM,      0.0,   1.0, 0.5));
    
	addParam(createParam<sp_SmallBlackKnob>(Vec(x2, y1+ 9*yh), module, Etagere::FREQ4_PARAM, vfmin, vfmax, 0.));
    addParam(createParam<sp_SmallBlackKnob>(Vec(x2, y1+10*yh), module, Etagere::GAIN4_PARAM,  gmin,  gmax, 1.0));

    addInput(createInput<sp_Port>(Vec(x1, y1+ 1* yh), module, Etagere::FREQ1_INPUT));
    addInput(createInput<sp_Port>(Vec(x1, y1+ 2* yh), module, Etagere::GAIN1_INPUT));

    addInput(createInput<sp_Port>(Vec(x1, y1+ 3* yh), module, Etagere::FREQ2_INPUT));
    addInput(createInput<sp_Port>(Vec(x1, y1+ 4* yh), module, Etagere::GAIN2_INPUT));
    addInput(createInput<sp_Port>(Vec(x1, y1+ 5* yh), module, Etagere::Q2_INPUT));
    
    addInput(createInput<sp_Port>(Vec(x1, y1+ 6* yh), module, Etagere::FREQ3_INPUT));
    addInput(createInput<sp_Port>(Vec(x1, y1+ 7* yh), module, Etagere::GAIN3_INPUT));
    addInput(createInput<sp_Port>(Vec(x1, y1+ 8* yh), module, Etagere::Q3_INPUT));
    
    addInput(createInput<sp_Port>(Vec(x1, y1+ 9* yh), module, Etagere::FREQ4_INPUT));
    addInput(createInput<sp_Port>(Vec(x1, y1+10* yh), module, Etagere::GAIN4_INPUT));    
	
    addInput(createInput<sp_Port>(Vec(x1, y1+11.5* yh), module, Etagere::FREQ5_INPUT));
    addInput(createInput<sp_Port>(Vec(x2, y1+11.5* yh), module, Etagere::GAIN5_INPUT));    
	
    addInput(createInput<sp_Port>(Vec(x1, y1+13* yh), module, Etagere::IN_INPUT));

/*
    addOutput(createOutput<sp_Port>(Vec(x3, y1+3*yh), module, Etagere::LP2_OUTPUT + 0));
    addOutput(createOutput<sp_Port>(Vec(x3, y1+4*yh), module, Etagere::BP2_OUTPUT + 0));
    addOutput(createOutput<sp_Port>(Vec(x3, y1+5*yh), module, Etagere::HP2_OUTPUT + 0));
    addOutput(createOutput<sp_Port>(Vec(x3, y1+6*yh), module, Etagere::LP3_OUTPUT + 0));
    addOutput(createOutput<sp_Port>(Vec(x3, y1+7*yh), module, Etagere::BP3_OUTPUT + 0));
    addOutput(createOutput<sp_Port>(Vec(x3, y1+8*yh), module, Etagere::HP3_OUTPUT + 0));
*/
    addOutput(createOutput<sp_Port>(Vec(x3, y1+13*yh), module, Etagere::OUT_OUTPUT));
}
