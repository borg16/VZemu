#include "VZemu.hpp"

#include <rack.hpp>
#include "HorizontalSlider.hpp"

namespace VZemu
{
    using namespace rack;

    struct Envelope : Module
    {
        enum ParamIds
        {
            RATE1_PARAM,
            RATE2_PARAM,
            RATE3_PARAM,
            RATE4_PARAM,
            RATE5_PARAM,
            RATE6_PARAM,
            RATE7_PARAM,
            RATE8_PARAM,
            LEVEL1_PARAM,
            LEVEL2_PARAM,
            LEVEL3_PARAM,
            LEVEL4_PARAM,
            LEVEL5_PARAM,
            LEVEL6_PARAM,
            LEVEL7_PARAM,
            LEVEL8_PARAM,
            SUSTAINSTEP1_PARAM,
            NUMSTEPS_PARAM,
            NUM_PARAMS
        };
        enum InputIds
        {
            TRIGGER_INPUT,
            NUM_INPUTS
        };
        enum OutputIds
        {
            ENVELOPE_OUTPUT,
            NUM_OUTPUTS
        };
        enum LightIds
        {
            END1_LIGHT,
            SUSTAINSTEP1_LIGHT,
            NUM_LIGHTS
        };

        Envelope()
        {
            config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
            configParam(RATE1_PARAM, 0, 99, 99);
            paramQuantities[RATE1_PARAM]->snapEnabled = true;
            configParam(LEVEL1_PARAM, 0, 99, 99);
            paramQuantities[LEVEL1_PARAM]->snapEnabled = true;
            configParam(NUMSTEPS_PARAM, 1, 8, 0);
            paramQuantities[NUMSTEPS_PARAM]->snapEnabled = true;
            configParam(SUSTAINSTEP1_PARAM, 0, 1, 0);
            paramQuantities[SUSTAINSTEP1_PARAM]->snapEnabled = true;
            configParam(RATE2_PARAM, 0, 99, 99);
            configParam(LEVEL2_PARAM, 0, 99, 99);
            configParam(RATE3_PARAM, 0, 99, 99);
            configParam(LEVEL3_PARAM, 0, 99, 99);
            configParam(RATE4_PARAM, 0, 99, 99);
            configParam(LEVEL4_PARAM, 0, 99, 99);
            configParam(RATE5_PARAM, 0, 99, 99);
            configParam(LEVEL5_PARAM, 0, 99, 99);
            configParam(RATE6_PARAM, 0, 99, 99);
            configParam(LEVEL6_PARAM, 0, 99, 99);
            configParam(RATE7_PARAM, 0, 99, 99);
            configParam(LEVEL7_PARAM, 0, 99, 99);
            configParam(RATE8_PARAM, 0, 99, 99);
            configParam(LEVEL8_PARAM, 0, 99, 99);
            configLight(END1_LIGHT);
            configLight(SUSTAINSTEP1_LIGHT);
        }
    };

    struct EnvelopeWidget : ModuleWidget
    {
        EnvelopeWidget(Envelope *module)
        {
            setModule(module);
            setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Envelope_panel.svg")));

            addChild(createWidget<ScrewBlack>(Vec(15, 0)));
            addChild(createWidget<ScrewBlack>(Vec(box.size.x - 30, 0)));
            addChild(createWidget<ScrewBlack>(Vec(15, 365)));
            addChild(createWidget<ScrewBlack>(Vec(box.size.x - 30, 365)));

            addParam(createParamCentered<HorizontalSlider>(mm2px(Vec(12., 11.)), module, Envelope::NUMSTEPS_PARAM));

            addChild(createLightCentered<MediumSimpleLight<RedLight>>(mm2px(Vec(2.2, 21.2)), module, Envelope::END1_LIGHT));
            addChild(createLightParamCentered<VCVLightButton<MediumSimpleLight<GreenLight>>>(mm2px(Vec(21.7, 22.6)), module, Envelope::SUSTAINSTEP1_PARAM, Envelope::SUSTAINSTEP1_LIGHT));
            addParam(createParamCentered<Trimpot>(mm2px(Vec(6.7, 25.0)), module, Envelope::RATE1_PARAM));
            addParam(createParamCentered<Trimpot>(mm2px(Vec(15.0, 25.0)), module, Envelope::LEVEL1_PARAM));
            addParam(createParamCentered<Trimpot>(mm2px(Vec(6.7, 35.5)), module, Envelope::RATE2_PARAM));
            addParam(createParamCentered<Trimpot>(mm2px(Vec(15.0, 35.5)), module, Envelope::LEVEL2_PARAM));
            addParam(createParamCentered<Trimpot>(mm2px(Vec(6.7, 46.0)), module, Envelope::RATE3_PARAM));
            addParam(createParamCentered<Trimpot>(mm2px(Vec(15.0, 46.0)), module, Envelope::LEVEL3_PARAM));
            addParam(createParamCentered<Trimpot>(mm2px(Vec(6.7, 56.5)), module, Envelope::RATE4_PARAM));
            addParam(createParamCentered<Trimpot>(mm2px(Vec(15.0, 56.5)), module, Envelope::LEVEL4_PARAM));
            addParam(createParamCentered<Trimpot>(mm2px(Vec(6.7, 67.0)), module, Envelope::RATE5_PARAM));
            addParam(createParamCentered<Trimpot>(mm2px(Vec(15.0, 67.0)), module, Envelope::LEVEL5_PARAM));
            addParam(createParamCentered<Trimpot>(mm2px(Vec(6.7, 77.5)), module, Envelope::RATE6_PARAM));
            addParam(createParamCentered<Trimpot>(mm2px(Vec(15.0, 77.5)), module, Envelope::LEVEL6_PARAM));
            addParam(createParamCentered<Trimpot>(mm2px(Vec(6.7, 88.0)), module, Envelope::RATE7_PARAM));
            addParam(createParamCentered<Trimpot>(mm2px(Vec(15.0, 88.0)), module, Envelope::LEVEL7_PARAM));
            addParam(createParamCentered<Trimpot>(mm2px(Vec(6.7, 98.5)), module, Envelope::RATE8_PARAM));
            addParam(createParamCentered<Trimpot>(mm2px(Vec(15.0, 98.5)), module, Envelope::LEVEL8_PARAM));
        }
    };

    Model *modelEnvelope = createModel<Envelope, EnvelopeWidget>("VZemu-Envelope");
}
