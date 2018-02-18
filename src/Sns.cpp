
#include <array>

#include "Southpole.hpp"
#include "dsp/digital.hpp"

#include "Bjorklund.hpp"

struct Sns : Module {
	enum ParamIds {
		K_PARAM,
		L_PARAM,
		R_PARAM,
		S_PARAM,
		P_PARAM,
		A_PARAM,
		CLK_PARAM,
		TRIG_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		K_INPUT,
		L_INPUT,
		R_INPUT,
		S_INPUT,
		A_INPUT,
		P_INPUT,
		CLK_INPUT,
		RESET_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		GATE_OUTPUT,
		ACCENT_OUTPUT,
		CLK_OUTPUT,
		RESET_OUTPUT,
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

	#define MAXLEN 64

	std::array<bool, MAXLEN> sequence;
	std::array<bool, MAXLEN> accents;

	bool calculate;
	bool from_reset;

	unsigned int  par_k=1; // fill
	unsigned int  par_l=1; // pattern length		
	unsigned int  par_r=1; // rotation
	unsigned int  par_p=1; // padding
	unsigned int  par_s=1; // shift
	unsigned int  par_a=1; // accent
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
	euclid.pad(par_p);
	euclid.rotater(par_r);
	//  euclid.print();
    
	euclid2.reset();	
	if (par_a>0) {
		euclid2.init(par_k, par_a);
		euclid2.iter();
		euclid2.rotater(par_r+par_s);
		//euclid2.print();
	}

//	std::vector<bool>::iterator it = euclid2.sequence.begin();
	unsigned int j = 0;
	for (unsigned int i = 0; i != euclid.sequence.size(); i++) 
	{
    	sequence[i] = euclid.sequence.at(i);
    	if (par_a && euclid.sequence.at(i)) 
		{
			accents[i] = euclid2.sequence.at( j % euclid2.sequence.size() ); //*it;
			//std::advance(it,1); 
			j++;
			//if (j == ) j=
		} else {
			accents[i] = 0;
		}
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
			currentStep = par_l + par_p;
		}		
	    outputs[RESET_OUTPUT].value = inputs[RESET_INPUT].value; 
	}	

	if (inputs[CLK_INPUT].active) {
		if (clockTrigger.process(inputs[CLK_INPUT].value)) {
			nextStep = true;
		}
	    outputs[CLK_OUTPUT].value = inputs[CLK_INPUT].value; 
	}  

	if (nextStep) {
		
		currentStep++;
		if (currentStep >= par_l + par_p) {
		   currentStep = 0;
		}
		
	    if (sequence[currentStep] ) {
		  	gatePulse.trigger(1e-3);
		}
	  	if (par_a && accents.at( currentStep )) {
			accentPulse.trigger(1e-3);
	  	}					
	} 

	bool gpulse = gatePulse.process(1.0 / engineGetSampleRate());
	bool apulse = accentPulse.process(1.0 / engineGetSampleRate());

	outputs[GATE_OUTPUT].value   = gpulse ? 10.0 : 0.0;
	outputs[ACCENT_OUTPUT].value = apulse ? 10.0 : 0.0;

	par_l = (unsigned int) ( 1. +       15.  * clampf( params[L_PARAM].value + inputs[L_INPUT].normalize(0.) / 9., 0.0f, 1.0f));
	par_p = (unsigned int) (32. - par_l) * clampf( params[P_PARAM].value + inputs[P_INPUT].normalize(0.) / 9., 0.0f, 1.0f);

	par_r = (unsigned int) (par_l + par_p - 1.) * clampf( params[R_PARAM].value + inputs[R_INPUT].normalize(0.) / 9., 0.0f, 1.0f);
	par_k = (unsigned int) ( 1. + (par_l-1.) * clampf( params[K_PARAM].value + inputs[K_INPUT].normalize(0.) / 9., 0.0f, 1.0f));

	par_a = (unsigned int) ( par_k ) * clampf( params[A_PARAM].value + inputs[A_INPUT].normalize(0.) / 9., 0.0f, 1.0f);
	if (par_a == 0) par_s = 0;
	else
	par_s = (unsigned int) ( par_a-1. ) * clampf( params[S_PARAM].value + inputs[S_INPUT].normalize(0.) / 9., 0.0f, 1.0f);

	// new sequence in case of change to parameters
	if (par_l+par_r+par_a+par_k+par_p+par_s != par_last) {
	
//		printf("%d %d",par_k,par_a);	
	   par_last = par_l+par_r+par_a+par_k+par_p+par_s;
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
		float cy = 0.5*b.size.y-12;
		const float r1 = .45*b.size.x;
		const float r2 = .35*b.size.x;

		// Circles
		nvgBeginPath(vg);
		nvgStrokeColor(vg, nvgRGBA(0x7f, 0x00, 0x00, 0xff));
		nvgFillColor(vg, nvgRGBA(0xff, 0x00, 0x00, 0xff));		
		nvgStrokeWidth(vg, 1.);
	    nvgCircle(vg, cx, cy, r1);
	    nvgCircle(vg, cx, cy, r2);
		nvgStroke(vg);

		unsigned len = module->par_l + module->par_p;

		// Step Rings
		for (unsigned i = 0; i < len; i++) {

			float r = module->accents[i] ? r1 : r2;
			float x = cx + r * cosf(2.*M_PI*i/len-.5*M_PI);
			float y = cy + r * sinf(2.*M_PI*i/len-.5*M_PI);

			nvgBeginPath(vg);
			nvgStrokeColor(vg, nvgRGBA(0x7f, 0x00, 0x00, 0xff));
			nvgFillColor(vg, nvgRGBA(0x30, 0x10, 0x10, 0xff));
			nvgStrokeWidth(vg, 1.);
			nvgCircle(vg, x, y, 3.);
			nvgFill(vg);
			nvgStroke(vg);
			}

		nvgStrokeColor(vg, nvgRGBA(0xff, 0x00, 0x00, 0xff));
		nvgBeginPath(vg);
		bool first = true;

		for (unsigned int  i = 0; i < len; i++) {

			float a = i/float(len);
			float r = module->accents[i] ? r1 : r2;
			float x = cx + r * cosf(2.*M_PI*a-.5*M_PI);
			float y = cy + r * sinf(2.*M_PI*a-.5*M_PI);

			if ( module->sequence[i] ) {
				Vec p(x,y);
				if (module->par_k == 1) nvgCircle(vg, x, y, 3.);				
				if (first) {
					nvgMoveTo(vg, p.x, p.y);
				 	first = false;
				} else 
					nvgLineTo(vg, p.x, p.y);
			}
		}

		nvgClosePath(vg);
		nvgStrokeWidth(vg, 1.);
		nvgStroke(vg);

		unsigned int i = module->currentStep;
		float r = module->accents[i] ? r1 : r2;
		float x = cx + r * cosf(2.*M_PI*i/len-.5*M_PI);
		float y = cy + r * sinf(2.*M_PI*i/len-.5*M_PI);
		nvgBeginPath(vg);
		nvgStrokeColor(vg, 	nvgRGBA(0xff, 0x00, 0x00, 0xff));
		if ( module->sequence[i] ) {
			nvgFillColor(vg, 	nvgRGBA(0xff, 0x00, 0x00, 0xff));
		} else {
			nvgFillColor(vg, 	nvgRGBA(0x30, 0x10, 0x10, 0xff));
		}
		nvgCircle(vg, x, y, 3.);
		nvgStrokeWidth(vg, 1.5);
		nvgFill(vg);
		nvgStroke(vg);
	}

	void draw(NVGcontext *vg) override {

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

		Vec textPos = Vec(15, 105);
		NVGcolor textColor = nvgRGB(0xff, 0x00, 0x00);
		//nvgFillColor(vg, nvgTransRGBA(textColor, 16));
		//nvgText(vg, textPos.x, textPos.y, "~~~~", NULL);
		nvgFillColor(vg, textColor);
		char str[20];
		snprintf(str,sizeof(str),"%2d %2d %2d",int(module->par_k),int(module->par_l),int(module->par_r));
		nvgText(vg, textPos.x, textPos.y-11, str, NULL);

		snprintf(str,sizeof(str),"%2d %2d %2d",int(module->par_p),int(module->par_a),int(module->par_s));
		nvgText(vg, textPos.x, textPos.y, str, NULL);
	}
};


SnsWidget::SnsWidget() {
	Sns *module = new Sns();
	setModule(module);
	box.size = Vec(15*6, 380);

	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/Sns.svg")));
		addChild(panel);
	}

	{
		SnsDisplay *display = new SnsDisplay(180,30);
		display->module = module;
		display->box.pos = Vec( 3., 30);
		display->box.size = Vec(box.size.x-6., 110. );
		addChild(display);
	}

	const float y1 = 160;
	const float yh = 30;

	float x1 = 4.;
	float x2 = 4.+30;
	float x3 = 4.+60;

	addParam(createParam<sp_SmallBlackKnob>(Vec(x1, y1   ), module, Sns::K_PARAM, 0., 1., .25));
	addParam(createParam<sp_SmallBlackKnob>(Vec(x2, y1   ), module, Sns::L_PARAM, 0., 1., .5));
	addParam(createParam<sp_SmallBlackKnob>(Vec(x3, y1   ), module, Sns::R_PARAM, 0., 1., 0.));
	addInput(createInput<sp_Port>(Vec(x1, y1+1*yh), module, Sns::K_INPUT));
	addInput(createInput<sp_Port>(Vec(x2, y1+1*yh), module, Sns::L_INPUT));
	addInput(createInput<sp_Port>(Vec(x3, y1+1*yh), module, Sns::R_INPUT));

	addParam(createParam<sp_SmallBlackKnob>(Vec(x1, y1+2.5*yh), module, Sns::P_PARAM, 0., 1., 0.));
	addParam(createParam<sp_SmallBlackKnob>(Vec(x2, y1+2.5*yh), module, Sns::A_PARAM, 0., 1., 0.));
	addParam(createParam<sp_SmallBlackKnob>(Vec(x3, y1+2.5*yh), module, Sns::S_PARAM, 0., 1., 0.));

	addInput(createInput<sp_Port>(Vec(x1, y1+3.5*yh), module, Sns::P_INPUT));
	addInput(createInput<sp_Port>(Vec(x2, y1+3.5*yh), module, Sns::A_INPUT));
	addInput(createInput<sp_Port>(Vec(x3, y1+3.5*yh), module, Sns::S_INPUT));

	addInput(createInput<sp_Port>(Vec(x1, y1+4.65*yh), module, Sns::CLK_INPUT));
	addInput(createInput<sp_Port>(Vec(x1, y1+5.4*yh), module, Sns::RESET_INPUT));
	addOutput(createOutput<sp_Port>(Vec(x3, y1+4.65*yh), module, Sns::CLK_OUTPUT));
	addOutput(createOutput<sp_Port>(Vec(x3, y1+5.4*yh), module, Sns::RESET_OUTPUT));

	addOutput(createOutput<sp_Port>(Vec(x2, y1+4.65*yh), module, Sns::GATE_OUTPUT));
	addOutput(createOutput<sp_Port>(Vec(x2, y1+5.4*yh), module, Sns::ACCENT_OUTPUT));


	//addChild(createLight<SmallLight<RedLight>>(Vec(4, 281), module, Sns::CLK_LIGHT));
	//addChild(createLight<SmallLight<RedLight>>(Vec(4+25, 281), module, Sns::GATE_LIGHT));
	//addChild(createLight<SmallLight<RedLight>>(Vec(4+50, 281), module, Sns::ACCENT_LIGHT));

}
