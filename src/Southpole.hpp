
#include "rack.hpp"

using namespace rack;

extern Plugin *plugin;

////////////////////
// module widgets
////////////////////

#ifndef PARASITES

struct SmokeWidget : ModuleWidget {
	SmokeWidget();
	Menu *createContextMenu() override;
};

struct AnnuliWidget : ModuleWidget {
	AnnuliWidget();
	Menu *createContextMenu() override;
};

struct BalaclavaWidget : ModuleWidget {
	BalaclavaWidget();
};

struct BandanaWidget : ModuleWidget {
	BandanaWidget();
};

struct ButWidget : ModuleWidget {
	ButWidget();
};

struct AbrWidget : ModuleWidget {
	AbrWidget();
};

struct EtagereWidget : ModuleWidget {
	EtagereWidget();
};

struct SnsWidget : ModuleWidget {
	SnsWidget();
};

struct WriggleWidget : ModuleWidget {
	WriggleWidget();
};

struct PisteWidget : ModuleWidget {
	PisteWidget();
};

struct FuseWidget : ModuleWidget {
	FuseWidget();
};

#else

struct CornrowsWidget : ModuleWidget {
	CornrowsWidget();
	Menu *createContextMenu() override;
};

struct SplashWidget : ModuleWidget {
	SVGPanel *tidesPanel;
	SVGPanel *sheepPanel;
	SplashWidget();
	void step() override;
	Menu *createContextMenu() override;
};

#endif

// GUI COMPONENTS

struct sp_Port : SVGPort {
	sp_Port() {
		background->svg = SVG::load(assetPlugin(plugin, "res/sp-Port18.svg"));
		background->wrap();
		box.size = background->box.size;
	}
};

struct sp_Switch : SVGSwitch, ToggleSwitch {
	sp_Switch() {
		addFrame(SVG::load(assetPlugin(plugin,"res/sp-switchv_0.svg")));
		addFrame(SVG::load(assetPlugin(plugin,"res/sp-switchv_1.svg")));
	}
};

struct sp_Encoder : SVGKnob {
	sp_Encoder() {
        minAngle = -0.83 * M_PI;
		maxAngle = 0.83 * M_PI;
		sw->svg = SVG::load(assetPlugin(plugin, "res/sp-encoder.svg"));
		sw->wrap();
		box.size = sw->box.size;
	}
};

struct sp_BlackKnob : SVGKnob {
	sp_BlackKnob() {
        minAngle = -0.83 * M_PI;
		maxAngle = 0.83 * M_PI;
		sw->svg = SVG::load(assetPlugin(plugin, "res/sp-knobBlack.svg"));
		sw->wrap();
		box.size = Vec(32,32);
	}
};

struct sp_SmallBlackKnob : SVGKnob {
	sp_SmallBlackKnob() {
        minAngle = -0.83 * M_PI;
		maxAngle = 0.83 * M_PI;
		sw->svg = SVG::load(assetPlugin(plugin, "res/sp-knobBlack.svg"));
		sw->wrap();
		box.size = Vec(20,20);
	}
};

struct sp_Trimpot : SVGKnob {
	sp_Trimpot() {
        minAngle = -0.83 * M_PI;
		maxAngle = 0.83 * M_PI;
		sw->svg = SVG::load(assetPlugin(plugin, "res/sp-trimpot.svg"));
		sw->wrap();
		box.size = Vec(18,18);
	}
};

