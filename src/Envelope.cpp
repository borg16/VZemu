#include "VZemu.hpp"

#include <rack.hpp>

namespace VZemu
{
    using namespace rack;

    struct Envelope : Module
    {
        enum ParamIds
        {
            RATE1_PARAM,
            RATE2_PARAM,
            LEVEL1_PARAM,
            LEVEL2_PARAM,
            NUM_PARAMS
        };
        enum InputIds
        {
            NUM_INPUTS
        };
        enum OutputIds
        {
            NUM_OUTPUTS
        };
        enum LightIds
        {
            NUM_LIGHTS
        };

        Envelope()
        {
            config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
            configParam(RATE1_PARAM, 0, 99, 99);
            configParam(LEVEL1_PARAM, 0, 99, 99);
            configParam(RATE2_PARAM, 0, 99, 99);
            configParam(LEVEL2_PARAM, 0, 99, 99);
        }
    };

    struct EnvelopeWidget : ModuleWidget
    {
        EnvelopeWidget(Envelope *module)
        {
            setModule(module);
            setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Envelope_panel.svg")));

            addChild(createWidget<ScrewSilver>(Vec(15, 0)));
            addChild(createWidget<ScrewSilver>(Vec(box.size.x - 30, 0)));
            addChild(createWidget<ScrewSilver>(Vec(15, 365)));
            addChild(createWidget<ScrewSilver>(Vec(box.size.x - 30, 365)));

            addParam(createParamCentered<Trimpot>(Vec(20, 63), module, Envelope::RATE1_PARAM));
            addParam(createParamCentered<Trimpot>(Vec(50, 63), module, Envelope::LEVEL1_PARAM));
            addParam(createParamCentered<Trimpot>(Vec(20, 93), module, Envelope::RATE2_PARAM));
            addParam(createParamCentered<Trimpot>(Vec(50, 93), module, Envelope::LEVEL2_PARAM));
        }
    };

    Model *modelEnvelope = createModel<Envelope, EnvelopeWidget>("VZemu-Envelope");
}
