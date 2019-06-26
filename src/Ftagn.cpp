#include "Southpole.hpp"

struct Ftagn : Module {
	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		IN1_INPUT,
		IN2_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		OUT1_OUTPUT,
		OUT2_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		// no lights
		NUM_LIGHTS
	};

	Ftagn() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);}

	void process(const ProcessArgs &args) override;
};


void Ftagn::process(const ProcessArgs &args) {

	outputs[OUT1_OUTPUT].value = 0.0;
	outputs[OUT2_OUTPUT].value = 0.0;
}

struct FtagnWidget : ModuleWidget { 
	
	FtagnWidget(Ftagn *module)  {
		setModule(module);
 
		box.size = Vec(15*4, 380);

		{
			SVGPanel *panel = new SVGPanel();
			panel->setBackground(APP->window->loadSvg(assetPlugin(pluginInstance, "res/Ftagn.svg")));
			panel->box.size = box.size;
			addChild(panel);	
		}

		addInput(createPort<sp_Port>(Vec( 6.,  380./2.-30.), PortWidget::INPUT, module, Ftagn::IN1_INPUT));
		addInput(createPort<sp_Port>(Vec( 6.,  380./2.), PortWidget::INPUT, module, Ftagn::IN2_INPUT));

		addOutput(createPort<sp_Port>(Vec(35.,  380./2.-30.), PortWidget::OUTPUT, module, Ftagn::OUT1_OUTPUT));
		addOutput(createPort<sp_Port>(Vec(35.,  380./2.), PortWidget::OUTPUT, module, Ftagn::OUT2_OUTPUT));
	}
};

Model *modelFtagn 	= createModel<Ftagn,FtagnWidget>("Ftagn");
