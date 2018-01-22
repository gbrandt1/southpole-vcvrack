#include "rack.hpp"


using namespace rack;


extern Plugin *plugin;

////////////////////
// module widgets
////////////////////

//struct MyModuleWidget : ModuleWidget {
//	MyModuleWidget();
//};

#ifdef PARASITES

struct CornrowsWidget : ModuleWidget {
	CornrowsWidget();
	Menu *createContextMenu() override;
};

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

struct EtagereWidget : ModuleWidget {
	EtagereWidget();
};

#else

struct SplashWidget : ModuleWidget {
	SVGPanel *tidesPanel;
	SVGPanel *sheepPanel;
	SplashWidget();
	void step() override;
	Menu *createContextMenu() override;
};

#endif

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
		box.size = Vec(24,24);
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