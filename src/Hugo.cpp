
#include <array>

#include "Southpole.hpp"
#include "dsp/digital.hpp"

#include "Bjorklund.hpp"

const float radius = 35.;

#define MAXPARTS 7
#define TNROWS 12
// circle of fifths
const int cof[12] = { 0,7,2,9,4,11,6,1,8,3,10,5 };

//chroma vectors
const int cn[5][12] =
	{ { 1,1,1,1,1,1,1,1,1,1,1,1 }, 	// Chromatic
	  { 1,0,1,0,1,1,0,1,0,1,0,1 }, 	// Major scale
	  { 1,0,1,1,0,1,0,1,1,0,0,1 }, 	// minor scale
	  { 1,0,0,0,1,0,0,1,0,0,0,0 }, 	// M
	  { 1,0,0,1,0,0,0,1,0,0,0,0 }, 	// m
	};

enum chroma_vector_names {
	CHROMATIC_CHROMA,
	MAJOR_CHROMA,
	MINOR_CHROMA,
	M3_CHROMA,
	m3_CHROMA,
	NUM_CHROMAS
};

enum chord_type_names {
	MAJ_CHORD,
	MIN_CHORD,
	AUG_CHORD,
	DIM_CHORD,
	SUS_CHORD
};

struct Note {

	// pitch class;
	int pc;
	float active;

	// projected position
	float x;
	float y;
};

struct Hugo : Module {
	enum ParamIds {		
		NUM_PARAMS
	};
	enum InputIds {
		X_INPUT,
		Y_INPUT,
		TR_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		N0_OUTPUT,
		N1_OUTPUT,
		N2_OUTPUT,
		N3_OUTPUT,
		N4_OUTPUT,
		N5_OUTPUT,
		N6_OUTPUT,
		N7_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {		
		NUM_LIGHTS
	};

	Hugo() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
		reset();

	}

	void step() override;
	void reset() override;

	// Tonnetz torus aligned to major thirds 
	// -> three rows are enough, fourth is for display, twelve completes the map
	Note notes[TNROWS][12];
	Note *chord[MAXPARTS];
	int octave[MAXPARTS];
	int parts;
	int transposition;
	int scale;
	chord_type_names type;	

	float freq[12];

	// playhead logical position
	int nx;
	int ny;

	// playhead geometrical
	float x0;
	float y0;
};

void Hugo::reset() {

	float ratio = pow(2., 1./12.);
	for (int i=0; i<12; i++) {
		freq[i] = 220. * pow(ratio, i+3);
		printf("%f\n", freq[i]);
	}

	for (int j=0; j<12; j++) {
		for (int i=0; i<12; i++) {
			notes[j][i].pc = cof[(i+10 + j*8)%12];

			float x0 = i;
			float y0 = j;
			float phi = -(x0 - y0/2.)*2.*M_PI / 12.;
			float   r = radius + 25.*y0;

			float x = r*sin(phi);
			float y = r*cos(phi);
			//float x = 30*(x0-6);
			//float y = 30*(y0-1);

			notes[j][i].x  = x;
			notes[j][i].y  = y;
		}
	}

	for ( int i=0; i< MAXPARTS; i++ ) {
		chord[i] = &notes[0][i];
		octave[i] = 1;
	}

	scale = 0;
	parts = 7;
}

void Hugo::step() {

	x0 = inputs[X_INPUT].normalize(0.)+6;
	y0 = inputs[Y_INPUT].normalize(0.)+1;

	while (x0 <   0.) { x0 += 12.; }
	while (x0 >= 12.) { x0 -= 12.; }

	while (y0 < 0. ) { y0 += 3.; }
	while (y0 >= 3.) { y0 -= 3.; }

	// determine active triad
	nx = floor(x0);
	ny = floor(y0);

	//if (nx<0) { nx+=12; }
	//if (ny<0) { ny+=3; }

	transposition = floor(inputs[TR_INPUT].normalize(0.));
	while ( transposition < 0.) { transposition += 12.; }

/*
    if ( y0-ny > x0-nx  ) {
		// Major (0 4 7 e 2 6 9)
		chord[0] = &notes[(ny+1)%TNROWS][ nx	  ];
		chord[1] = &notes[ ny	       ][ nx	  ];
		chord[2] = &notes[(ny+1)%TNROWS][(nx+1)%12];
		chord[3] = &notes[ ny	       ][(nx+1)%12];
		chord[4] = &notes[(ny+1)%TNROWS][(nx+2)%12];
		chord[5] = &notes[ ny          ][(nx+2)%12];
		chord[6] = &notes[(ny+1)%TNROWS][(nx+3)%12];
		type = MAJ_CHORD;
	} else {
		// minor (0 3 7 t 2 5 9)
		chord[0] = &notes[ ny     	   ][ nx  	  ];
		chord[1] = &notes[(ny+1)%TNROWS][(nx+1)%12];
		chord[2] = &notes[ ny     	   ][(nx+1)%12];
		chord[3] = &notes[(ny+1)%TNROWS][(nx+2)%12];
		chord[4] = &notes[ ny     	   ][(nx+2)%12];
		chord[5] = &notes[(ny+1)%TNROWS][(nx+3)%12];
		chord[6] = &notes[ ny          ][(nx+3)%12];
		type = MIN_CHORD;
	}
*/

	// augmented (wrap??)
	for (int i=0; i<MAXPARTS; i++) { chord[i] = &notes[(12+ny-i)%3][ nx ]; }
	type = AUG_CHORD;
/*
	// diminished
	//for (int i=0; i<MAXPARTS; i++) { chord[i] = &notes[(ny+i)%3][(nx+i)%12]; }
	type = DIM_CHORD;

	// suspended (sus4)
	for (int i=0; i<MAXPARTS; i++) { chord[i] = &notes[ ny ][(nx+i)%12]; }
	type = SUS_CHORD;
*/
	//count octaves
	int o = 0.;
	octave[0] = o;
	for (int i=1; i<MAXPARTS; i++) { 
		if ( chord[i]->pc < chord[i-1]->pc ) o++;
		octave[i] = o;
	}


	outputs[N0_OUTPUT].value = octave[0]+(transposition 			)   /12.;
	for (int i=1; i<MAXPARTS; i++) { 
		outputs[N0_OUTPUT+i].value = octave[1]+(transposition+chord[i-1]->pc)%12/12.;
	}	
}


struct HugoDisplay : TransparentWidget {

	Hugo *module;
	int frame = 0;
	std::shared_ptr<Font> font;

	HugoDisplay() {	  
	  font = Font::load(assetPlugin(plugin, "res/DejaVuSansMono.ttf"));
	}

	void drawTonnetz(NVGcontext *vg) {

		float cx = box.size.x*0.5;
		float cy = box.size.y*0.5;
				
		// draw active chord - triangles
		if (module->parts >= 3)
		for (int i=0; i< module->parts-2; i++)	{
			nvgBeginPath(vg);
			nvgFillColor(vg, nvgRGBA(0x7f, 0x10, 0x10, 0xff));
			nvgMoveTo(vg, module->chord[i  ]->x + cx, module->chord[i  ]->y + cy);
			nvgLineTo(vg, module->chord[i+1]->x + cx, module->chord[i+1]->y + cy);
			nvgLineTo(vg, module->chord[i+2]->x + cx, module->chord[i+2]->y + cy);
			nvgClosePath(vg);
			nvgFill(vg);
		}

		// draw active chord - lines
		for (int i=0; i<module->parts-1; i++)	{
			nvgBeginPath(vg);
			nvgStrokeWidth(vg, 2.);			
			nvgStrokeColor(vg, nvgRGBA(0xff, 0x10, 0x10, 0xff));
			float y0 = module->chord[i  ]->y + cy;
			float y1 = module->chord[i+1]->y + cy;
			nvgMoveTo(vg, module->chord[i  ]->x + cx, y0);
			nvgLineTo(vg, module->chord[i+1]->x + cx, y1);
			nvgStroke(vg);
		}

		nvgStrokeWidth(vg, .7);				
		nvgStrokeColor(vg, nvgRGBA(0xff, 0x00, 0x00, 0xff));

		// draw triad grid
		for ( int ny = 0; ny < 4; ny++) {
			for ( int nx = 0; nx < 12; nx++) {

				float x = module->notes[ny][nx].x + cx;
				float y = module->notes[ny][nx].y + cy;

				nvgBeginPath(vg);
				nvgMoveTo(vg, x, y);
				float x1 = module->notes[ny][(nx+1)%12].x + cx;
				float y1 = module->notes[ny][(nx+1)%12].y + cy;
				nvgLineTo(vg, x1, y1);
				if ( ny < 3 ) {
					nvgMoveTo(vg, x, y);
					x1 = module->notes[ny+1][(nx)%12].x + cx;
					y1 = module->notes[ny+1][(nx)%12].y + cy;
					nvgLineTo(vg, x1, y1);
					nvgMoveTo(vg, x, y);
					x1 = module->notes[ny+1][(nx+1)%12].x + cx;
					y1 = module->notes[ny+1][(nx+1)%12].y + cy;
					nvgLineTo(vg, x1, y1);
				}
				nvgStroke(vg);	
			}
		}

		// draw notes
		
		const char *note_names[12] = { 
			//"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" 
			//"C", "Db", "D", "Eb", "E", "F", "Gb", "G", "Ab", "A", "Bb", "B" 
			"0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "t", "e" 
		};
		
		for ( int ny = 0; ny < 4; ny++) {
			for ( int nx = 0; nx < 12; nx++) {

				float x = module->notes[ny][nx].x + cx;
				float y = module->notes[ny][nx].y + cy;

				// circle with note name
				nvgBeginPath(vg);
				nvgFillColor(vg, nvgRGBA(0x30, 0x10, 0x10, 0xff));				
				//if ( module->notes[ny][nx].active > 0. ) {
				//	nvgFillColor(vg, nvgRGBA(0xff, 0x10, 0x10, 0xff));
				nvgCircle(vg, x, y, 6.);
				nvgFill(vg);
				//}									

				//nvgStroke(vg);

				nvgFontSize(vg, 12);
				nvgFontFaceId(vg, font->handle);
				Vec textPos = Vec( x-3, y+3);
				NVGcolor textColor = nvgRGB(0xff, 0x00, 0x00);
				nvgFillColor(vg, textColor);

				int pc = (module->notes[ny][nx].pc + module->transposition) %12;

				if ( cn[module->scale][pc] ) {
					nvgText(vg, textPos.x, textPos.y, note_names[ pc ], NULL);
				}
			}
		}
		
		// note values

		nvgFontSize(vg, 10);
		nvgFontFaceId(vg, font->handle);
		Vec textPos = Vec( 3, box.size.y-25);
		NVGcolor textColor = nvgRGB(0xff, 0x00, 0x00);
		nvgFillColor(vg, textColor);
		for (int i=0; i<module->parts; i++) {
			nvgText(vg, textPos.x+i*15, textPos.y   , note_names[module->chord[i]->pc], NULL);
			nvgText(vg, textPos.x+i*15, textPos.y+15, note_names[module->octave[i]], NULL);
		}
		//nvgStroke(vg);

		// circle for root note		
		nvgBeginPath(vg);
		//	nvgFillColor(vg, nvgRGBA(0x30, 0x10, 0x10, 0xff));				
		nvgFillColor(vg, nvgRGBA(0xff, 0x10, 0x10, 0xff));
		nvgCircle(vg, module->chord[0]->x + cx, module->chord[0]->y + cy, 6.);
		nvgFill(vg);
		nvgStroke(vg);

  	}

	void draw(NVGcontext *vg) {
		
		//Rect b = Rect(Vec(2, 2), box.size.minus(Vec(2, 2)));

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

		drawTonnetz(vg);

		// draw playhead	

		float phi = -(module->x0 - module->y0/2.)*2.*M_PI / 12.;
		float   r = radius + 25.*module->y0;

		float x = r*sin(phi) + box.size.x*0.5;
		float y = r*cos(phi) + box.size.y*0.5;

		nvgBeginPath(vg);
		nvgCircle(vg, x, y, 3.);
		nvgFill(vg);
		nvgStrokeWidth(vg, 1.5);
		nvgStroke(vg);
		
	}
};


HugoWidget::HugoWidget() {

	Hugo *module = new Hugo();
	setModule(module);
	box.size = Vec(15*16, 380);

	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/sp-Blank16HP.svg")));
		addChild(panel);
	}

	{
		HugoDisplay *display = new HugoDisplay();
		display->module = module;
		display->box.pos = Vec( 5., 30);
		display->box.size = Vec(box.size.x-10.,box.size.x-10. );
		addChild(display);
	}

	const float y1 = 16;
	const float yh = 22;

	float x1 = 4.;
	//float x2 = 4.+30;
	//float x3 = 4.+60;

	//addParam(createParam<sp_SmallBlackKnob>(Vec(x1, y1   ), module, Hugo::K_PARAM, 0., 1., .25));
	addInput(createInput<sp_Port>(Vec(x1, y1+13*yh), module, Hugo::X_INPUT));
	addInput(createInput<sp_Port>(Vec(x1+26, y1+13*yh), module, Hugo::Y_INPUT));
	addInput(createInput<sp_Port>(Vec(x1+26*2, y1+13*yh), module, Hugo::TR_INPUT));

	for (int i=0; i<8; i++)	{
		addOutput(createOutput<sp_Port>(Vec(x1+26*i,  y1+14*yh), module, Hugo::N0_OUTPUT + i));
	}
}
