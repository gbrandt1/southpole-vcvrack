#include "Southpole.hpp"
#include "dsp/digital.hpp"

#define NPORTS 12

float *SnakeWidget::cable;
unsigned int* SnakeWidget::lockid;
unsigned int SnakeWidget::counter;

struct Snake : Module {
	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		IN_INPUT,
		NUM_INPUTS = IN_INPUT + NPORTS
	};
	enum OutputIds {
		OUT_OUTPUT,
		NUM_OUTPUTS = OUT_OUTPUT + NPORTS
	};
	enum LightIds {
		LOCK_LIGHT,
		NUM_LIGHTS = LOCK_LIGHT + 2*NPORTS 
	};

	unsigned int id = 0;

	Snake() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
		id = SnakeWidget::counter;
	}

	void step() override;
};


void Snake::step() {

	for (unsigned int i=0; i < NPORTS; i++) {

		if ( inputs[IN_INPUT+i].active) {
			if ( SnakeWidget::lockid[i] == 0 ) {
				SnakeWidget::lockid[i] = id;
			}

			if ( SnakeWidget::lockid[i] == id ) {
				SnakeWidget::cable[i] = inputs[IN_INPUT+i].value;
			}
		} else {
			if ( SnakeWidget::lockid[i] == id ) {
				SnakeWidget::lockid[i] = 0;
			}
		}	
	}		
	for (unsigned int i=0; i < NPORTS; i++) {
	
		if ( SnakeWidget::lockid[i] == 0 ) {
			lights[LOCK_LIGHT+2*i].setBrightness(0);
			lights[LOCK_LIGHT+2*i+1].setBrightness(0);
			continue;
		} 

		if ( SnakeWidget::lockid[i] == id ) {
			lights[LOCK_LIGHT+2*i  ].setBrightness(1.0);
			lights[LOCK_LIGHT+2*i+1].setBrightness(0.);
		} else {
			lights[LOCK_LIGHT+2*i  ].setBrightness(0.);			
			lights[LOCK_LIGHT+2*i+1].setBrightness(1.);			
		}
	}		
	for (unsigned int i=0; i < NPORTS; i++) {
		outputs[OUT_OUTPUT+i].value = SnakeWidget::cable[i];
	}
}


SnakeWidget::SnakeWidget() {

	Snake *module = new Snake();
	setModule(module);

	if (counter == 0) {

		lockid = new unsigned int [NPORTS];
		cable = new float [NPORTS];
		for (unsigned int i=0; i< NPORTS; i++)
		{
			lockid[i] = 0;
			cable[i] = 0.;
		}
	}

	counter++;
	module->id = counter;

	box.size = Vec(15*4, 380);

	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/Snake.svg")));
		addChild(panel);
	}

	float y1 = 45;	
	float yh = 26;

	for (unsigned int i=0; i< NPORTS; i++)
	{
		addInput(createInput<sp_Port>(Vec(  5, y1+i*yh), module, Snake::IN_INPUT + i));
		addOutput(createOutput<sp_Port>(Vec(34, y1+i*yh), module, Snake::OUT_OUTPUT + i));
		addChild(createLight<SmallLight<GreenRedLight>>(Vec(26, y1+i*yh-2), module, Snake::LOCK_LIGHT + 2*i));
	}
}
