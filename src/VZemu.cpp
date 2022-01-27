#include "VZemu.hpp"

#include <rack.hpp>

namespace VZemu
{
	rack::plugin::Plugin *pluginInstance;
}

void init(rack::plugin::Plugin *p)
{
	VZemu::pluginInstance = p;

	// Add modules here
	p->addModel(VZemu::modelModulePair);
	p->addModel(VZemu::modelEnvelope);

	// Any other plugin initialization may go here.
	// As an alternative, consider lazy-loading assets and lookup tables when your module is created to reduce startup times of Rack.
}