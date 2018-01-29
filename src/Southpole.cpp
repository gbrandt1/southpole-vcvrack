

#include "Southpole.hpp"

Plugin *plugin;

void init(rack::Plugin *p) {
	plugin = p;
	// The "slug" is the unique identifier for your plugin and must never change after release, so choose wisely.
	// It must only contain letters, numbers, and characters "-" and "_". No spaces.
	// To guarantee uniqueness, it is a good idea to prefix the slug by your name, alias, or company name if available, e.g. "MyCompany-MyPlugin".
	// The ZIP package must only contain one folder, with the name equal to the plugin's slug.
	p->slug = "Southpole";
	p->version = TOSTRING(VERSION);
	p->website = "https://github.com/gbrandt1/southpole-vcvrack";
	p->manual = "https://github.com/gbrandt1/southpole-vcvrack/blob/master/README.md";

	// For each module, specify the ModuleWidget subclass, manufacturer slug (for saving in patches), manufacturer human-readable name, module slug, and module name
	//p->addModel(createModel<MyModuleWidget>("Southpole", "MyModule", "My Module", //OSCILLATOR_TAG));

#ifdef PARASITES
    p->addModel(createModel<SmokeWidget>("Southpole", "Smoke", "Smoke", GRANULAR_TAG, REVERB_TAG));
#else
    p->addModel(createModel<AnnuliWidget>("Southpole", "Annuli", "Annuli", UTILITY_TAG));
    p->addModel(createModel<BalaclavaWidget>("Southpole", "Balaclava", "Balaclava", AMPLIFIER_TAG));
    p->addModel(createModel<BandanaWidget>("Southpole", "Bandana", "Bandana", AMPLIFIER_TAG));
    p->addModel(createModel<ButWidget>("Southpole", "But", "But", SWITCH_TAG, UTILITY_TAG));
    p->addModel(createModel<AbrWidget>("Southpole", "Abr", "Abr", SWITCH_TAG, UTILITY_TAG));
    p->addModel(createModel<EtagereWidget>("Southpole", "Etagere", "Etagere", FILTER_TAG));
    p->addModel(createModel<SnsWidget>("Southpole", "SNS", "SNS", SEQUENCER_TAG));
    p->addModel(createModel<PisteWidget>("Southpole", "Piste", "Piste", ENVELOPE_GENERATOR_TAG, EFFECT_TAG, UTILITY_TAG));
    p->addModel(createModel<WriggleWidget>("Southpole", "Wriggle", "Wriggle", LFO_TAG, FUNCTION_GENERATOR_TAG));
    p->addModel(createModel<FuseWidget>("Southpole", "Fuse", "Fuse", SEQUENCER_TAG));
    p->addModel(createModel<CornrowsWidget>("Southpole", "Cornrows", "Cornrows", OSCILLATOR_TAG, WAVESHAPER_TAG));
	p->addModel(createModel<SplashWidget>("Southpole", "Splash", "Splash", LFO_TAG, OSCILLATOR_TAG, WAVESHAPER_TAG, FUNCTION_GENERATOR_TAG));
#endif
}


