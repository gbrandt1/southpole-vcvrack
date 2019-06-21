#include "Southpole.hpp"

Plugin *pluginInstance;

void init(rack::Plugin *p) {
	pluginInstance = p;

	#ifdef PARASITES
		Model *modelSmoke  			= createModel<Smoke,SmokeWidget>(	"Southpole-parasites", "Smoke", "Smoke Parasite", GRANULAR_TAG, REVERB_TAG);
		Model *modelSplashParasite 	= createModel<SplashParasite,SplashParasiteWidget>("Southpole-parasites", "Splash Parasite", "Splash Parasites", LFO_TAG, OSCILLATOR_TAG, WAVESHAPER_TAG, FUNCTION_GENERATOR_TAG);
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
