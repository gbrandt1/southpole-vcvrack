
#include "Southpole.hpp"

Plugin *pluginInstance;

void init(rack::Plugin *p) {
	pluginInstance = p;

	p->addModel(modelSmoke);
	p->addModel(modelSplash);
}
