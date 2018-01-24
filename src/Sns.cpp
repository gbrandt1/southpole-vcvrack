#include <string.h>
#include "Southpole.hpp"
#include "dsp/digital.hpp"

#include "Bjorklund.hpp"

#define BUFFER_SIZE 512

struct Sns : Module {
	enum ParamIds {
		K_PARAM,
		N_PARAM,
		L_PARAM,
		S_PARAM,
		CLK_PARAM,
		TRIG_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		CLK_INPUT,
		TRIG_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		GATE_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	Sns() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
		reset();
	}

	void step() override;
	void reset() override;
	void onSampleRateChange() override;

	json_t *toJson() override {
		json_t *rootJ = json_object();
		return rootJ;
	}

	void fromJson(json_t *rootJ) override {
	}
};

	Bjorklund euclid;

	int par_k; // fill
	int par_n; // burst length
	int par_l; // pattern length		
	int par_s; // shift

	int last;

	bool running = true;
  	SchmittTrigger clockTrigger;  // for external clock
  	SchmittTrigger resetTrigger;  // reset button

 	double ltime;
  	double dTime;
  	int bpm;
  	double timerLength;
	double timerTime;

void Sns::reset() {
	ltime = 0.0;
	dTime = 1.0 / static_cast<double>(engineGetSampleRate());
	bpm = 120;
	timerLength = 1.0 / (static_cast<double>(bpm) / 60.0);
	timerTime = timerLength;
}

void Sns::onSampleRateChange() {
  dTime = 1.0 / static_cast<double>(engineGetSampleRate());
}

void Sns::step() {

  	bool nextStep = false;

  	// clocking
  	if (running) {
		ltime += dTime;
		if (inputs[CLK_INPUT].active) {
			if (clockTrigger.process(inputs[CLK_INPUT].value)) {
				nextStep = true;
			}
		}  else {
			timerLength = 1.0 / (static_cast<double>(bpm) / 60.0);
			timerTime -= dTime;
			if (dTime > timerTime) {
				timerTime = 0.0;
			}
			if (timerTime <= 0.0) {
				nextStep = true;
				timerTime = timerLength;
			}
		}
    }

	// act on clk pulse
	//if ( params[CLK_PARAM].value ) {
	//}

	// Reset the Schmitt trigger so we don't trigger immediately if the input is high
	//clkTrigger.reset();

	// Must go below 0.1V to trigger
	//resetTrigger.setThresholds(params[TRIG_PARAM].value - 0.1, params[TRIG_PARAM].value);
	//float gate = external ? inputs[TRIG_INPUT].value : inputs[X_INPUT].value;

	// Reset if triggered
	//float holdTime = 0.1;
	//if (resetTrigger.process(gate) || (frameIndex >= engineGetSampleRate() * holdTime)) {
	//		bufferIndex = 0; frameIndex = 0; return;
	//}

	// Reset if we've waited too long
	//	if (frameIndex >= engineGetSampleRate() * holdTime) {
	//bufferIndex = 0; frameIndex = 0; return;
	//}
	//}
//}
//void Sns::onChange(EventChange &e) override {

	par_l = int(1.+15.*params[Sns::L_PARAM].value);		
	par_s = int((par_l-1.)*params[Sns::S_PARAM].value);
	par_n = int(1.+(par_l-1.)*params[Sns::N_PARAM].value);
	par_k = int(1.+(par_n-1.)*params[Sns::K_PARAM].value);
	
	// new sequence in case of change to parameters
	if (par_l+par_s+par_n+par_k != last) {
       printf("Pattern for %d %d %d %d\n",par_k,par_n,par_l,par_s);
	   last = par_l+par_s+par_n+par_k;

	   euclid.reset();	
	   euclid.init(par_n,par_k);
	   euclid.iter();
	   euclid.print();
	}
	
}


struct SnsDisplay : TransparentWidget {
	Sns *module;
	int frame = 0;
	std::shared_ptr<Font> font;

	SnsDisplay() {
	  //font = Font::load(assetPlugin(plugin, "res/fonts/Sudo.ttf"));
	  font = Font::load(assetPlugin(plugin, "res/hdad-segment14-1.002/Segment14.ttf"));
	}

	void drawPolygon(NVGcontext *vg, float k, float n, float l, float s) {


		//nvgSave(vg);
		Rect b = Rect(Vec(2, 2), box.size.minus(Vec(2, 2)));
		//nvgScissor(vg, b.pos.x, b.pos.y, b.size.x, b.size.y);

		float cx = 0.5*b.size.x;
		float cy = 0.5*b.size.y;
		float  r = 40.;

		nvgStrokeColor(vg, nvgRGBA(0xff, 0x00, 0x00, 0x7f));
		nvgBeginPath(vg);
	    nvgCircle(vg, cx, cy, r);
		for (int i = 0; i < l; i++) {
			float x = cx + r * cosf(2.*M_PI*i/l-.5*M_PI);
			float y = cy + r * sinf(2.*M_PI*i/l-.5*M_PI);
    	    nvgCircle(vg, x, y, 3.);
		}
		nvgStroke(vg);

		nvgStrokeColor(vg, nvgRGBA(0xff, 0x00, 0x00, 0xff));
		nvgBeginPath(vg);
		for (int i = 0; i < n; i++) {
			float a = (i+s)/l;
			float x = cx + r * cosf(2.*M_PI*a-.5*M_PI);
			float y = cy + r * sinf(2.*M_PI*a-.5*M_PI);
			Vec p(x,y);
			if (i==0) nvgMoveTo(vg, p.x, p.y);
			else 
			if ( euclid.sequence.at(i) ) {
				nvgLineTo(vg, p.x, p.y);
			}
		}
		nvgClosePath(vg);

		//nvgLineCap(vg, NVG_ROUND);
		//nvgMiterLimit(vg, 2.0);
		nvgStrokeWidth(vg, 1.5);
		//nvgGlobalCompositeOperation(vg, NVG_LIGHTER);
		nvgStroke(vg);
		//nvgResetScissor(vg);
		//nvgRestore(vg);
		
	}

	void draw(NVGcontext *vg) override {

		drawPolygon(vg, par_k, par_n, par_l, par_s);

		nvgFontSize(vg, 11);
		nvgFontFaceId(vg, font->handle);
		nvgTextLetterSpacing(vg, 2.);

		Vec textPos = Vec(8, 8);
		NVGcolor textColor = nvgRGB(0xff, 0x00, 0x00);
		nvgFillColor(vg, nvgTransRGBA(textColor, 16));
		nvgText(vg, textPos.x, textPos.y, "~~~~", NULL);
		nvgFillColor(vg, textColor);
		char str[20];
		snprintf(str,sizeof(str),"%d %d %d %d",int(par_k),int(par_n),int(par_l),int(par_s));
		nvgText(vg, textPos.x, textPos.y, str, NULL);

	}
};


SnsWidget::SnsWidget() {
	Sns *module = new Sns();
	setModule(module);
	box.size = Vec(15*8, 380);

	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/Sns.svg")));
		addChild(panel);
	}

	{
		SnsDisplay *display = new SnsDisplay();
		display->module = module;
		display->box.pos = Vec(4, 30);
		display->box.size = Vec(14*8, 14*8);
		addChild(display);
	}

	addParam(createParam<sp_SmallBlackKnob>(Vec(15   , 209), module, Sns::K_PARAM, 0., 1., .5));
	addParam(createParam<sp_SmallBlackKnob>(Vec(15+25, 209), module, Sns::N_PARAM, 0., 1., .5));
	addParam(createParam<sp_SmallBlackKnob>(Vec(15+50, 209), module, Sns::L_PARAM, 0., 1., 1.));
	addParam(createParam<sp_SmallBlackKnob>(Vec(15+70, 209), module, Sns::S_PARAM, 0., 1., 0.));

//	addInput(createInput<PJ301MPort>(Vec(17, 319), module, Sns::X_INPUT));

	addChild(createLight<SmallLight<GreenLight>>(Vec(104, 251), module, Sns::CLK_LIGHT));

}
