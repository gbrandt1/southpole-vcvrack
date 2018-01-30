//#include <string.h>
#include <array>

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
		K_INPUT,
		L_INPUT,
		S_INPUT,
		ACCENT_INPUT,
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

	Bjorklund euclid;
	Bjorklund euclid2;

	std::array<bool,32> sequence;
	std::array<bool,32> accents;

	bool calculate;
	bool from_reset;

	unsigned int  par_k = 1; // fill
	unsigned int  par_l = 1; // pattern length		
	unsigned int  par_s = 1; // shift
	unsigned int  par_a = 1; // accent
	unsigned int  par_last;

  	SchmittTrigger clockTrigger;
  	SchmittTrigger resetTrigger;
	PulseGenerator gatePulse;
	PulseGenerator accentPulse;

	unsigned int  currentStep = 0;
};

void Sns::reset() {

	euclid.reset();	
	euclid.init(par_l,par_k);
	euclid.iter();
	euclid.rotater(par_s);
	//  euclid.print();
    
	euclid2.reset();	
	if (par_a>0) {
		euclid2.init(par_k,par_a);
		euclid2.iter();
		//euclid2.print();
	}

	std::vector<bool>::iterator it = euclid2.sequence.begin();
	for (unsigned int i = 0; i != euclid.sequence.size(); i++) {
    	sequence[i] = euclid.sequence.at(i);
    	if (par_a && euclid.sequence.at(i)) {
			accents[i] = *it;
			std::advance(it,1); 
		} else accents[i] = 0;
	}
/*
    for (unsigned int i = 0; i != accents.size(); i++) {
        std::cout << sequence[i];
    }
    std::cout << '\n';
    for (unsigned int i = 0; i != accents.size(); i++) {
        std::cout << accents[i];
    }
    std::cout << '\n';
*/

   	calculate = false;
	from_reset = true;
}

void Sns::step() {

  	bool nextStep = false;

	// reset sequence
	if (inputs[RESET_INPUT].active) {
		if (resetTrigger.process(inputs[RESET_INPUT].value)) {
			currentStep = par_l;
		}		
	}	

	if (inputs[CLK_INPUT].active) {
		if (clockTrigger.process(inputs[CLK_INPUT].value)) {
			nextStep = true;
		}
	}  

	if (nextStep) {
		
		currentStep++;
		if (currentStep >= par_l) {
		   currentStep = 0;
		}
		
		// no processing during recalculation
		//if ( !calculate)
	    if (sequence[currentStep] ) {
		  	gatePulse.trigger(1e-3);
		} 
	  	if (par_a && accents.at( currentStep )) {
			accentPulse.trigger(1e-3);
	  	}					
	} 


	bool gpulse = gatePulse.process(1.0 / engineGetSampleRate());
	bool apulse = accentPulse.process(1.0 / engineGetSampleRate());

	outputs[GATE_OUTPUT].value = gpulse ? 10.0 : 0.0;
	outputs[ACCENT_OUTPUT].value = apulse ? 10.0 : 0.0;

	par_l = (unsigned int) ( 1. +       15.  * ( clampf( params[L_PARAM].value + inputs[L_INPUT].normalize(0.) / 9., 0.0f, 1.0f)));
	par_s = (unsigned int) (      (par_l-1.) * ( clampf( params[S_PARAM].value + inputs[S_INPUT].normalize(0.) / 9., 0.0f, 1.0f)));
	par_k = (unsigned int) ( 1. + (par_l-1.) * ( clampf( params[K_PARAM].value + inputs[K_INPUT].normalize(0.) / 9., 0.0f, 1.0f)));
	par_a = float( par_k ) * ( clampf( params[ACCENT_PARAM].value + inputs[ACCENT_INPUT].normalize(0.) / 9., 0.0f, 1.0f));

	//par_l = unsigned int (1.+15.*params[Sns::L_PARAM].value);		
	//par_s = int((par_l-1.)*params[Sns::S_PARAM].value);
	//par_k = int(1.+(par_l-1.)*params[Sns::K_PARAM].value);
	//par_a = float(par_k)*params[Sns::ACCENT_PARAM].value;
	
	// new sequence in case of change to parameters
	if (par_l+par_s+par_a+par_k != par_last) {
	
//		printf("%d %d",par_k,par_a);	
	   par_last = par_l+par_s+par_a+par_k;
	   calculate = true;
	}	
}


struct SnsDisplay : TransparentWidget {

	Sns *module;
	int frame = 0;
	std::shared_ptr<Font> font;

	float y1;
	float yh;

	SnsDisplay( float y1_, float yh_ ) {
	  
	  y1 = y1_;
	  yh = yh_;
	  //font = Font::load(assetPlugin(plugin, "res/fonts/Sudo.ttf"));
	  font = Font::load(assetPlugin(plugin, "res/hdad-segment14-1.002/Segment14.ttf"));
	}

	void drawPolygon(NVGcontext *vg) {
		
		Rect b = Rect(Vec(2, 2), box.size.minus(Vec(2, 2)));

		float cx = 0.5*b.size.x+1;
		float cy = 0.5*b.size.y-6;
		const float r1 = .45*b.size.x;
		const float r2 = .3*b.size.x;

		nvgBeginPath(vg);
		nvgStrokeColor(vg, nvgRGBA(0x7f, 0x00, 0x00, 0xff));
		nvgFillColor(vg, nvgRGBA(0xff, 0x00, 0x00, 0xff));		
		nvgStrokeWidth(vg, 1.);
	    nvgCircle(vg, cx, cy, r1);
	    nvgCircle(vg, cx, cy, r2);
		nvgStroke(vg);

		unsigned len = module->par_l;

		for (unsigned i = 0; i < len; i++) {

			float r = module->accents[i] ? r1 : r2;
			float x = cx + r * cosf(2.*M_PI*i/len-.5*M_PI);
			float y = cy + r * sinf(2.*M_PI*i/len-.5*M_PI);

			nvgBeginPath(vg);
			nvgStrokeColor(vg, nvgRGBA(0x7f, 0x00, 0x00, 0xff));
			nvgStrokeWidth(vg, 1.);
			nvgCircle(vg, x, y, 3.);
			if ( i == module->currentStep  ) {
				nvgCircle(vg, x, y, 3.);
				nvgStrokeWidth(vg, 1.5);
				if ( module->sequence[i] ) {
					nvgStrokeColor(vg, nvgRGBA(0xff, 0x00, 0x00, 0xff));
					nvgFill(vg);
				}
				nvgLineTo(vg, cx, cy);
			}	
			nvgStroke(vg);
		}

		nvgStrokeColor(vg, nvgRGBA(0xff, 0x00, 0x00, 0xff));
		nvgBeginPath(vg);
		bool first = true;
		//std::vector<bool>::iterator it = euclid.sequence.begin();
		for (unsigned int  i = 0; i < module->par_l; i++) {
			float a = i/float(module->par_l);
			float r = module->accents[i] ? r1 : r2;
			float x = cx + r * cosf(2.*M_PI*a-.5*M_PI);
			float y = cy + r * sinf(2.*M_PI*a-.5*M_PI);
			if ( module->sequence[i] ) {
				//if (accents[i]) {
				//	x = cx + r1 * cosf(2.*M_PI*a-.5*M_PI);
				//	y = cy + r1 * sinf(2.*M_PI*a-.5*M_PI);
				//}
				Vec p(x,y);
				if (module->par_k == 1) nvgCircle(vg, x, y, 3.);				
				if (first) {
					nvgMoveTo(vg, p.x, p.y);
				 	first = false;
				} else 
					nvgLineTo(vg, p.x, p.y);
			}
			//std::advance(it,1);
		}
		nvgClosePath(vg);
		nvgStrokeWidth(vg, 1.5);
		nvgStroke(vg);
	}

	void draw(NVGcontext *vg) override {

		// wait until in defined state
		if (module->calculate) {
		   module->reset();		
		}

		// Background
		NVGcolor backgroundColor = nvgRGB(0x30, 0x10, 0x10);
		NVGcolor borderColor = nvgRGB(0xd0, 0xd0, 0xd0);
		nvgBeginPath(vg);
		nvgRoundedRect(vg, 0.0, 0.0, box.size.x, box.size.y, 5.0);
		nvgFillColor(vg, backgroundColor);
		nvgFill(vg);
		nvgStrokeWidth(vg, 1.5);
		nvgStrokeColor(vg, borderColor);
		nvgStroke(vg);


		drawPolygon(vg);

		nvgFontSize(vg, 8);
		nvgFontFaceId(vg, font->handle);
//		nvgTextLetterSpacing(vg, 2.);

		Vec textPos = Vec(2, 95);
		NVGcolor textColor = nvgRGB(0xff, 0x00, 0x00);
		nvgFillColor(vg, nvgTransRGBA(textColor, 16));
		//nvgText(vg, textPos.x, textPos.y, "~~~~", NULL);
		nvgFillColor(vg, textColor);
		char str[20];
		snprintf(str,sizeof(str),"%2d %2d",int(module->par_k),int(module->par_l));
		nvgText(vg, textPos.x, textPos.y, str, NULL);

		snprintf(str,sizeof(str),"%2d %2d",int(module->par_s),int(module->par_a));
		nvgText(vg, textPos.x+36, textPos.y, str, NULL);
	}
};


SnsWidget::SnsWidget() {
	Sns *module = new Sns();
	setModule(module);
	box.size = Vec(15*6, 380);

	const float y1 = 180;
	const float yh = 30;

	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/Sns.svg")));
		addChild(panel);
	}

	{
		SnsDisplay *display = new SnsDisplay(y1,yh);
		display->module = module;
		display->box.pos = Vec( 3., 30);
		display->box.size = Vec(box.size.x-6., 100. );
		addChild(display);
	}

	addParam(createParam<sp_SmallBlackKnob>(Vec(4   , y1   ), module, Sns::K_PARAM, 0., 1., .25));
	addParam(createParam<sp_SmallBlackKnob>(Vec(4+25, y1   ), module, Sns::L_PARAM, 0., 1., 1.));
	addParam(createParam<sp_SmallBlackKnob>(Vec(4   , y1+2*yh), module, Sns::S_PARAM, 0., 1., 0.));
	addParam(createParam<sp_SmallBlackKnob>(Vec(4+25, y1+2*yh), module, Sns::ACCENT_PARAM, 0., 1., 0.));

	addInput(createInput<sp_Port>(Vec(4,    y1+1*yh), module, Sns::K_INPUT));
	addInput(createInput<sp_Port>(Vec(4+25, y1+1*yh), module, Sns::L_INPUT));
	addInput(createInput<sp_Port>(Vec(4   , y1+3*yh), module, Sns::S_INPUT));
	addInput(createInput<sp_Port>(Vec(4+25, y1+3*yh), module, Sns::ACCENT_INPUT));

	addInput(createInput<sp_Port>(Vec(4,    y1+4*yh), module, Sns::CLK_INPUT));
	addInput(createInput<sp_Port>(Vec(4+25, y1+4*yh), module, Sns::RESET_INPUT));

	addOutput(createOutput<sp_Port>(Vec(4   , y1+5*yh), module, Sns::GATE_OUTPUT));
	addOutput(createOutput<sp_Port>(Vec(4+25, y1+5*yh), module, Sns::ACCENT_OUTPUT));

	//addChild(createLight<SmallLight<RedLight>>(Vec(4, 281), module, Sns::CLK_LIGHT));
	//addChild(createLight<SmallLight<RedLight>>(Vec(4+25, 281), module, Sns::GATE_LIGHT));
	//addChild(createLight<SmallLight<RedLight>>(Vec(4+50, 281), module, Sns::ACCENT_LIGHT));

}
