#include "Southpole.hpp"

struct Blank1HPWidget : ModuleWidget {
	Blank1HPWidget(Module *module)  {
		setModule(module);
	
		box.size = Vec(1 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
		{
			auto *panel = new SVGPanel();
			panel->box.size = box.size;
			panel->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/sp-Blank2HP.svg")));
			addChild(panel);
		}
	}
};
struct Blank2HPWidget : ModuleWidget {
	Blank2HPWidget(Module *module)  {
		setModule(module);
	
		box.size = Vec(2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
		{
			auto *panel = new SVGPanel();
			panel->box.size = box.size;
			panel->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/sp-Blank2HP.svg")));
			addChild(panel);
		}
	}
};
struct Blank4HPWidget : ModuleWidget {
	Blank4HPWidget(Module *module)  {
		setModule(module);
	
		box.size = Vec(4 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
		{
			auto *panel = new SVGPanel();
			panel->box.size = box.size;
			panel->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/sp-Blank4HP.svg")));
			addChild(panel);
		}
	}
};
struct Blank8HPWidget : ModuleWidget {
	Blank8HPWidget(Module *module)  {
		setModule(module);
	
		box.size = Vec(8 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
		{
			auto *panel = new SVGPanel();
			panel->box.size = box.size;
			panel->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/sp-Blank8HP.svg")));
			addChild(panel);
		}
	}
};
struct Blank16HPWidget : ModuleWidget {
	Blank16HPWidget(Module *module)  {
		setModule(module);
	
		box.size = Vec(16 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
		{
			auto *panel = new SVGPanel();
			panel->box.size = box.size;
			panel->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/sp-Blank16HP.svg")));
			addChild(panel);
		}
	}
};
struct Blank42HPWidget : ModuleWidget {	
	Blank42HPWidget(Module *module)  {
		setModule(module);
	
		box.size = Vec(42 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
		{
			auto *panel = new SVGPanel();
			panel->box.size = box.size;
			panel->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/sp-Blank42HP.svg")));
			addChild(panel);
		}
	}
};

Model *modelBlank16HP 	= createModel<Module,Blank16HPWidget>("Blank16HP");
Model *modelBlank1HP 	= createModel<Module,Blank1HPWidget >("Blank1HP");
Model *modelBlank2HP 	= createModel<Module,Blank2HPWidget >("Blank2HP");
Model *modelBlank42HP 	= createModel<Module,Blank42HPWidget>("Blank42HP");
Model *modelBlank4HP 	= createModel<Module,Blank4HPWidget >("Blank4HP");
Model *modelBlank8HP 	= createModel<Module,Blank8HPWidget >("Blank8HP");

