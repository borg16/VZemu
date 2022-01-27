#pragma once

namespace rack
{
    namespace plugin
    {
        struct Plugin;
        struct Model;
    }
}

namespace VZemu
{
    extern rack::plugin::Plugin *pluginInstance;

    extern rack::plugin::Model *modelModulePair;

    extern rack::plugin::Model *modelEnvelope;
}