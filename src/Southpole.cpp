
#include "Southpole.hpp"

Plugin *plugin;

void init(rack::Plugin *p) {
	plugin = p;
	// The "slug" is the unique identifier for your plugin and must never change after release, so choose wisely.
	// It must only contain letters, numbers, and characters "-" and "_". No spaces.
	// To guarantee uniqueness, it is a good idea to prefix the slug by your name, alias, or company name if available, e.g. "MyCompany-MyPlugin".
	// The ZIP package must only contain one folder, with the name equal to the plugin's slug.
	#ifdef PARASITES
		p->slug = "Southpole-parasites";    
	#else
		p->slug = "Southpole";
	#endif
	p->version = TOSTRING(VERSION);
	p->website = "https://github.com/gbrandt1/southpole-vcvrack";
	p->manual = "https://github.com/gbrandt1/southpole-vcvrack/blob/master/README.md";

	#ifdef PARASITES
		Model *modelSmoke  			= Model::create<Smoke,SmokeWidget>(	"Southpole-parasites", "Smoke", "Smoke Parasite", GRANULAR_TAG, REVERB_TAG);
		Model *modelSplashParasite 	= Model::create<SplashParasite,SplashParasiteWidget>("Southpole-parasites", "Splash Parasite", "Splash Parasites", LFO_TAG, OSCILLATOR_TAG, WAVESHAPER_TAG, FUNCTION_GENERATOR_TAG);
	#endif
	//#endif

		p->addModel(modelAbr 	);
		p->addModel(modelAnnuli );	
		p->addModel(modelAux 	);
		p->addModel(modelBalaclava );
		p->addModel(modelBandana );
		p->addModel(modelBlank1HP );	
		p->addModel(modelBlank2HP );	
		p->addModel(modelBlank4HP );	
		p->addModel(modelBlank8HP );	
		p->addModel(modelBlank16HP );	
		p->addModel(modelBlank42HP );	
		p->addModel(modelBut 	);
		p->addModel(modelCornrowsX );
		p->addModel(modelDeuxEtageres); 
		p->addModel(modelEtagere );
		p->addModel(modelFalls 	);
		p->addModel(modelFtagn 	);
		p->addModel(modelFuse 	);
		p->addModel(modelGnome 	);
		p->addModel(modelPiste 	);
		p->addModel(modelPulse	);
		p->addModel(modelRakes 	);
		p->addModel(modelRiemann); 
		p->addModel(modelSmoke); 	
		p->addModel(modelSnake); 	
		p->addModel(modelSns);
		p->addModel(modelSplash); 	
		p->addModel(modelSssh); 	
		p->addModel(modelWriggle);
}
