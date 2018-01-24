#include <string.h>
#include "Southpole.hpp"
#include "dsp/digital.hpp"

#include "Bjorklund.hpp"

struct Sns : Module {
	enum ParamIds {
		K_PARAM,
		L_PARAM,
		S_PARAM,
		ACCENT_PARAM,
		CLK_PARAM,
		TRIG_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		CLK_INPUT,
		RESET_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		GATE_OUTPUT,
		ACCENT_OUTPUT,
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
	Bjorklund accents;
	bool calculate;
	bool from_reset;

	int par_k = 1; // fill
	int par_l = 1; // pattern length		
	int par_s = 1; // shift
	int par_a = 1; // accent
	int par_last;

  	SchmittTrigger clockTrigger;  // external clock
  	SchmittTrigger resetTrigger;  // reset button
	PulseGenerator gatePulse;
	PulseGenerator accentPulse;

	int currentStep = 0;
	int currentIndex;
	int accentIndex;

void Sns::reset() {

	euclid.reset();	
	euclid.init(par_l,par_k);
	euclid.iter();
   	euclid.print();
   	calculate = false;
    
	if (par_a>0) {
		accents.reset();	
		accents.init(par_k,par_a);
		accents.iter();
		accents.print();
	}
	from_reset = true;
}

void Sns::onSampleRateChange() {
}

void Sns::step() {

	// no processing during recalculation
	if (calculate) return;

  	bool nextStep = false;

	if (inputs[CLK_INPUT].active)
	{
		if (clockTrigger.process(inputs[CLK_INPUT].value)) {
			nextStep = true;
		}
	}  

	// reset sequence
	if (from_reset) { 
		currentStep = par_l;
		//nextStep = true;
		from_reset = false;
	}
	if (inputs[RESET_INPUT].active) {
		if (resetTrigger.process(inputs[RESET_INPUT].value)) {
			currentStep = par_l;
			//nextStep = true;
		}		
	}	

	if (nextStep) {
		
		currentStep++;
		if (currentStep >= par_l) {
		   currentStep = 0;
		   accentIndex = 0;
		}
		
		currentIndex = ( currentStep + par_s ) % par_l;
	    if (euclid.sequence.at( currentIndex )) {
		  	gatePulse.trigger(1e-3);
		  	if (par_a) {
		      if (accents.sequence.at( accentIndex )) {
			    accentPulse.trigger(1e-3);
			  }					
			  accentIndex ++;
			} 
		} 
	} 


	bool gpulse = gatePulse.process(1.0 / engineGetSampleRate());
	bool apulse = accentPulse.process(1.0 / engineGetSampleRate());

	outputs[GATE_OUTPUT].value = gpulse ? 10.0 : 0.0;
	outputs[ACCENT_OUTPUT].value = apulse ? 10.0 : 0.0;

	par_l = int(1.+15.*params[Sns::L_PARAM].value);		
	par_s = par_l-int((par_l-1.)*params[Sns::S_PARAM].value);
	par_k = int(1.+(par_l-1.)*params[Sns::K_PARAM].value);
	par_a = float(par_k)*params[Sns::ACCENT_PARAM].value;
	
	// new sequence in case of change to parameters
	if (par_l+par_s+par_a+par_k != par_last) {
	
//		printf("%d %d",par_k,par_a);	
	   par_last = par_l+par_s+par_a+par_k;
	   calculate = true;
	   reset();		// to do: don't call in step ...
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

	void drawPolygon(NVGcontext *vg) {
		
		//nvgSave(vg);
		Rect b = Rect(Vec(2, 2), box.size.minus(Vec(2, 2)));
		//nvgScissor(vg, b.pos.x, b.pos.y, b.size.x, b.size.y);

		float cx = 0.5*b.size.x;
		float cy = 0.5*b.size.y;
		float  r = 40.;

		// empty circles for l total steps
		nvgStrokeColor(vg, nvgRGBA(0xff, 0x00, 0x00, 0x7f));
		nvgBeginPath(vg);
	    nvgCircle(vg, cx, cy, r);
	    nvgCircle(vg, cx, cy, 0.5*r);
		for (int i = 0; i < par_l; i++) {
			float x = cx + r * cosf(2.*M_PI*i/par_l-.5*M_PI);
			float y = cy + r * sinf(2.*M_PI*i/par_l-.5*M_PI);
    	    nvgCircle(vg, x, y, 3.);
		}
		nvgStroke(vg);

		// circle for current step, filled if active step

		int acc = 0;
		float x = cx + r * cosf(2.*M_PI*currentStep/par_l-.5*M_PI);
		float y = cy + r * sinf(2.*M_PI*currentStep/par_l-.5*M_PI);
		nvgStrokeWidth(vg, 1.5);
		nvgFillColor(vg, nvgRGBA(0xff, 0x00, 0x00, 0xff));
		nvgBeginPath(vg);
		nvgCircle(vg, x, y, 3.);				
		if ( euclid.sequence.at( currentIndex )) nvgFill(vg);
		nvgStroke(vg);


		nvgStrokeColor(vg, nvgRGBA(0xff, 0x00, 0x00, 0xff));
		nvgBeginPath(vg);
		bool first = true;
		int acc = 0;
		for (int i = 0; i < par_l; i++) {
			float a = i/float(par_l);
			float x = cx + .5*r * cosf(2.*M_PI*a-.5*M_PI);
			float y = cy + .5*r * sinf(2.*M_PI*a-.5*M_PI);
			if ( euclid.sequence.at( (i+par_s) % par_l )) {
				if (par_a) {
					if (accents.sequence.at( acc++ )) {
						x = cx + r * cosf(2.*M_PI*a-.5*M_PI);
						y = cy + r * sinf(2.*M_PI*a-.5*M_PI);
					}	
				}	
				Vec p(x,y);
				if (par_k == 1) nvgCircle(vg, x, y, 3.);				
				if (first) {
					nvgMoveTo(vg, p.x, p.y);
				 	first = false;
				} else 
					nvgLineTo(vg, p.x, p.y);
			}
		}
		nvgClosePath(vg);
		nvgStrokeWidth(vg, 1.5);
		nvgStroke(vg);
	}

	void draw(NVGcontext *vg) override {

		// wait until in defined state
		if (calculate) return;

		drawPolygon(vg); //, par_k, par_n, par_l, par_s);

		nvgFontSize(vg, 11);
		nvgFontFaceId(vg, font->handle);
		nvgTextLetterSpacing(vg, 2.);

		Vec textPos = Vec(8, 146);
		NVGcolor textColor = nvgRGB(0xff, 0x00, 0x00);
		nvgFillColor(vg, nvgTransRGBA(textColor, 16));
		//nvgText(vg, textPos.x, textPos.y, "~~~~", NULL);
		nvgFillColor(vg, textColor);
		char str[20];
		snprintf(str,sizeof(str),"%d %d %d %d",int(par_k),int(par_l),int(par_l-par_s),int(par_a));
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

	float yh = 30;

	addParam(createParam<sp_SmallBlackKnob>(Vec(4   , 209), module, Sns::K_PARAM, 0., 1., .25));
	addParam(createParam<sp_SmallBlackKnob>(Vec(4+25, 209), module, Sns::L_PARAM, 0., 1., 1.));
	addParam(createParam<sp_SmallBlackKnob>(Vec(4+50, 209), module, Sns::S_PARAM, 0., 1., 0.));
	addParam(createParam<sp_SmallBlackKnob>(Vec(4   , 209+yh), module, Sns::ACCENT_PARAM, 0., 1., 0.));

	addInput(createInput<sp_Port>(Vec(17, 319-30), module, Sns::CLK_INPUT));
	addInput(createInput<sp_Port>(Vec(17+25, 319-30), module, Sns::RESET_INPUT));

	addOutput(createOutput<sp_Port>(Vec(17+25, 319), module, Sns::GATE_OUTPUT));
	addOutput(createOutput<sp_Port>(Vec(17+50, 319), module, Sns::ACCENT_OUTPUT));

	//addChild(createLight<SmallLight<RedLight>>(Vec(4, 281), module, Sns::CLK_LIGHT));
	//addChild(createLight<SmallLight<RedLight>>(Vec(4+25, 281), module, Sns::GATE_LIGHT));
	//addChild(createLight<SmallLight<RedLight>>(Vec(4+50, 281), module, Sns::ACCENT_LIGHT));

}
