

#include "Southpole.hpp"

#include "dsp/digital.hpp"

struct Fuse : Module {
	enum ParamIds {
		SWITCH1_PARAM,
		SWITCH2_PARAM,
		SWITCH3_PARAM,
		SWITCH4_PARAM,
	 	NUM_PARAMS
	};

	enum InputIds {
		ARM1_INPUT,
		ARM2_INPUT,
		ARM3_INPUT,
		ARM4_INPUT,
		CLK_INPUT,
		RESET_INPUT,
		NUM_INPUTS
	};

	enum OutputIds {
		OUT1_OUTPUT,
		OUT2_OUTPUT,
		OUT3_OUTPUT,
		OUT4_OUTPUT,
	 	NUM_OUTPUTS
	};
	
	enum LightIds {
		ARM1_LIGHT,
		ARM2_LIGHT,
		ARM3_LIGHT,
		ARM4_LIGHT,
		NUM_LIGHTS
	};

	Fuse() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {

        params.resize(NUM_PARAMS);
        inputs.resize(NUM_INPUTS);
        outputs.resize(NUM_OUTPUTS);
        lights.resize(NUM_LIGHTS);
	}

	void step() override;

  	SchmittTrigger clockTrigger;
  	SchmittTrigger resetTrigger;

	const unsigned maxsteps = 16;
	unsigned curstep = 0;
};

void Fuse::step() {

  	bool nextStep = false;

	if (inputs[RESET_INPUT].active) {
		if (resetTrigger.process(inputs[RESET_INPUT].value)) {
			curstep = maxsteps;
		}		
	}	

	if (inputs[CLK_INPUT].active) {
		if (clockTrigger.process(inputs[CLK_INPUT].value)) {
			nextStep = true;
		}
	} 

	if ( nextStep ) {
		curstep++;
		if ( curstep >= maxsteps ) curstep = 0;
	}

	bool arm1;
	if ( params[SWITCH1_PARAM].value > 0. ) arm1 = true;
	else arm1 =false;

	lights[ARM1_LIGHT].setBrightness( arm1 ? 1.0 : 0.0 );
};

struct FuseDisplay : TransparentWidget {

	Fuse *module;

	FuseDisplay() {}

	void draw(NVGcontext *vg) override {

		nvgStrokeColor(vg, nvgRGBA(0xff, 0xff, 0x00, 0xff));
		nvgFillColor(vg, nvgRGBA(0xff, 0xff, 0x00, 0xff));		

		for ( unsigned y_ = 0; y_ < 16; y_++ ) {
			unsigned y = 15 - y_;
			nvgBeginPath(vg);
	    	nvgRect(vg, 0, y*box.size.y/16.+6.*floor(y/4.), box.size.x, box.size.y/16.-28.);
			if (y_ <= module->curstep) nvgFill(vg);
			nvgStroke(vg);
		}

	}	
};

FuseWidget::FuseWidget() {

	Fuse *module = new Fuse();
	setModule(module);

	float y1 = 28;
	float yh = 20;

	float x1 = 5;	
	float x2 = 50;

	box.size = Vec(8 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/Fuse.svg")));
		addChild(panel);
	}

	{
		FuseDisplay *display = new FuseDisplay();
		display->module = module;
		display->box.pos = Vec( y1, 30.);
		display->box.size = Vec( 20., box.size.y-80. );
		addChild(display);
	}


	//INPUTS
	addInput(createInput<sp_Port>(Vec(x1, y1+15*yh), module, Fuse::CLK_INPUT));
	addInput(createInput<sp_Port>(Vec(x2, y1+15*yh), module, Fuse::RESET_INPUT));
	
    for(int i = 0; i < 4; i++)
    {
        addInput(createInput<sp_Port>  (Vec(x1, y1+(4*i+2)*yh), module, Fuse::ARM1_INPUT + i));
        addParam(createParam<CKD6>(Vec(x2, y1+(4*i)*yh), module, Fuse::SWITCH1_PARAM + i, 0.0, 1.0, 0.0));
		addChild(createLight<SmallLight<RedLight>>(Vec(x2, y1+(4*i)*yh), module, Fuse::ARM1_LIGHT+i));
        addOutput(createOutput<sp_Port>(Vec(x2, y1+(4*i+2)*yh), module, Fuse::OUT1_OUTPUT + i));
    }

	//OUTPUTS
	//addOutput(createOutput<sp_Port>(Vec(63,327), module, Fuse::OUT));

	//for ( int y = 0; y < 4; y++ )
	//	addOutput(createOutput<sp_Port>(Vec(x2,y1+y*yh), module, Fuse::OUT+1));

}