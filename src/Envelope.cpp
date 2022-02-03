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
            SUSTAINSTEP2_PARAM,
            SUSTAINSTEP3_PARAM,
            SUSTAINSTEP4_PARAM,
            SUSTAINSTEP5_PARAM,
            SUSTAINSTEP6_PARAM,
            SUSTAINSTEP7_PARAM,
            SUSTAINSTEP8_PARAM,
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
            END2_LIGHT,
            END3_LIGHT,
            END4_LIGHT,
            END5_LIGHT,
            END6_LIGHT,
            END7_LIGHT,
            END8_LIGHT,
            SUSTAINSTEP1_LIGHT,
            SUSTAINSTEP2_LIGHT,
            SUSTAINSTEP3_LIGHT,
            SUSTAINSTEP4_LIGHT,
            SUSTAINSTEP5_LIGHT,
            SUSTAINSTEP6_LIGHT,
            SUSTAINSTEP7_LIGHT,
            SUSTAINSTEP8_LIGHT,
            NUM_LIGHTS
        };

        int currentSustainStep = 4;

        Envelope()
        {
            config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
            configParam(RATE1_PARAM, 0, 99, 99);
            paramQuantities[RATE1_PARAM]->snapEnabled = true;
            configParam(LEVEL1_PARAM, 0, 99, 99);
            paramQuantities[LEVEL1_PARAM]->snapEnabled = true;
            configParam(NUMSTEPS_PARAM, 1, 8, 1, "Number of Steps");
            paramQuantities[NUMSTEPS_PARAM]->snapEnabled = true;
            configParam(SUSTAINSTEP1_PARAM, 0, 1, 0, "Sustain Step 1");
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
            configLight(END1_LIGHT, "One Step");
            configLight(END2_LIGHT, "Two Steps");
            configLight(END3_LIGHT, "Three Steps");
            configLight(END4_LIGHT, "Four Steps");
            configLight(END5_LIGHT, "Five Steps");
            configLight(END6_LIGHT, "Six Steps");
            configLight(END7_LIGHT, "Seven Steps");
            configLight(END8_LIGHT, "Eight Steps");
            configLight(SUSTAINSTEP1_LIGHT);
            configLight(SUSTAINSTEP2_LIGHT);
            configLight(SUSTAINSTEP3_LIGHT);
            configLight(SUSTAINSTEP4_LIGHT);
            configLight(SUSTAINSTEP5_LIGHT);
            configLight(SUSTAINSTEP6_LIGHT);
            configLight(SUSTAINSTEP7_LIGHT);
            configLight(SUSTAINSTEP8_LIGHT);
        }

        void process(const ProcessArgs &args) override
        {
            int numSteps = params[NUMSTEPS_PARAM].getValue();
            lights[END1_LIGHT].setBrightness(numSteps == 1);
            lights[END2_LIGHT].setBrightness(numSteps == 2);
            lights[END3_LIGHT].setBrightness(numSteps == 3);
            lights[END4_LIGHT].setBrightness(numSteps == 4);
            lights[END5_LIGHT].setBrightness(numSteps == 5);
            lights[END6_LIGHT].setBrightness(numSteps == 6);
            lights[END7_LIGHT].setBrightness(numSteps == 7);
            lights[END8_LIGHT].setBrightness(numSteps == 8);

            if (params[SUSTAINSTEP1_PARAM].getValue())
                currentSustainStep = 1;
            else if (params[SUSTAINSTEP2_PARAM].getValue())
                currentSustainStep = 2;
            else if (params[SUSTAINSTEP3_PARAM].getValue())
                currentSustainStep = 3;
            else if (params[SUSTAINSTEP4_PARAM].getValue())
                currentSustainStep = 4;
            else if (params[SUSTAINSTEP5_PARAM].getValue())
                currentSustainStep = 5;
            else if (params[SUSTAINSTEP6_PARAM].getValue())
                currentSustainStep = 6;
            else if (params[SUSTAINSTEP7_PARAM].getValue())
                currentSustainStep = 7;
            else if (params[SUSTAINSTEP8_PARAM].getValue())
                currentSustainStep = 8;

            float dimSustain = currentSustainStep > numSteps ? .3 : 1;

            lights[SUSTAINSTEP1_LIGHT].setBrightness(dimSustain*(currentSustainStep == 1));
            lights[SUSTAINSTEP2_LIGHT].setBrightness(dimSustain*(currentSustainStep == 2));
            lights[SUSTAINSTEP3_LIGHT].setBrightness(dimSustain*(currentSustainStep == 3));
            lights[SUSTAINSTEP4_LIGHT].setBrightness(dimSustain*(currentSustainStep == 4));
            lights[SUSTAINSTEP5_LIGHT].setBrightness(dimSustain*(currentSustainStep == 5));
            lights[SUSTAINSTEP6_LIGHT].setBrightness(dimSustain*(currentSustainStep == 6));
            lights[SUSTAINSTEP7_LIGHT].setBrightness(dimSustain*(currentSustainStep == 7));
            lights[SUSTAINSTEP8_LIGHT].setBrightness(dimSustain*(currentSustainStep == 8));

            outputs[ENVELOPE_OUTPUT].setChannels(1);
            outputs[ENVELOPE_OUTPUT].setVoltage(.23);
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

            addChild(createLightCentered<SmallSimpleLight<RedLight>>(mm2px(Vec(2.2, 21.2)), module, Envelope::END1_LIGHT));
            addChild(createLightParamCentered<VCVLightButton<SmallSimpleLight<GreenLight>>>(mm2px(Vec(21.7, 22.6)), module, Envelope::SUSTAINSTEP1_PARAM, Envelope::SUSTAINSTEP1_LIGHT));
            addParam(createParamCentered<Trimpot>(mm2px(Vec(6.7, 25.0)), module, Envelope::RATE1_PARAM));
            addParam(createParamCentered<Trimpot>(mm2px(Vec(15.0, 25.0)), module, Envelope::LEVEL1_PARAM));
            addChild(createLightCentered<SmallSimpleLight<RedLight>>(mm2px(Vec(2.2, 31.7)), module, Envelope::END2_LIGHT));
            addChild(createLightParamCentered<VCVLightButton<SmallSimpleLight<GreenLight>>>(mm2px(Vec(21.7, 33.1)), module, Envelope::SUSTAINSTEP2_PARAM, Envelope::SUSTAINSTEP2_LIGHT));
            addParam(createParamCentered<Trimpot>(mm2px(Vec(6.7, 35.5)), module, Envelope::RATE2_PARAM));
            addParam(createParamCentered<Trimpot>(mm2px(Vec(15.0, 35.5)), module, Envelope::LEVEL2_PARAM));
            addChild(createLightCentered<SmallSimpleLight<RedLight>>(mm2px(Vec(2.2, 42.2)), module, Envelope::END3_LIGHT));
            addChild(createLightParamCentered<VCVLightButton<SmallSimpleLight<GreenLight>>>(mm2px(Vec(21.7, 43.6)), module, Envelope::SUSTAINSTEP3_PARAM, Envelope::SUSTAINSTEP3_LIGHT));
            addParam(createParamCentered<Trimpot>(mm2px(Vec(6.7, 46.0)), module, Envelope::RATE3_PARAM));
            addParam(createParamCentered<Trimpot>(mm2px(Vec(15.0, 46.0)), module, Envelope::LEVEL3_PARAM));
            addChild(createLightCentered<SmallSimpleLight<RedLight>>(mm2px(Vec(2.2, 52.7)), module, Envelope::END4_LIGHT));
            addChild(createLightParamCentered<VCVLightButton<SmallSimpleLight<GreenLight>>>(mm2px(Vec(21.7, 54.1)), module, Envelope::SUSTAINSTEP4_PARAM, Envelope::SUSTAINSTEP4_LIGHT));
            addParam(createParamCentered<Trimpot>(mm2px(Vec(6.7, 56.5)), module, Envelope::RATE4_PARAM));
            addParam(createParamCentered<Trimpot>(mm2px(Vec(15.0, 56.5)), module, Envelope::LEVEL4_PARAM));
            addChild(createLightCentered<SmallSimpleLight<RedLight>>(mm2px(Vec(2.2, 63.2)), module, Envelope::END5_LIGHT));
            addChild(createLightParamCentered<VCVLightButton<SmallSimpleLight<GreenLight>>>(mm2px(Vec(21.7, 64.6)), module, Envelope::SUSTAINSTEP5_PARAM, Envelope::SUSTAINSTEP5_LIGHT));
            addParam(createParamCentered<Trimpot>(mm2px(Vec(6.7, 67.0)), module, Envelope::RATE5_PARAM));
            addParam(createParamCentered<Trimpot>(mm2px(Vec(15.0, 67.0)), module, Envelope::LEVEL5_PARAM));
            addChild(createLightCentered<SmallSimpleLight<RedLight>>(mm2px(Vec(2.2, 73.7)), module, Envelope::END6_LIGHT));
            addChild(createLightParamCentered<VCVLightButton<SmallSimpleLight<GreenLight>>>(mm2px(Vec(21.7, 75.1)), module, Envelope::SUSTAINSTEP6_PARAM, Envelope::SUSTAINSTEP6_LIGHT));
            addParam(createParamCentered<Trimpot>(mm2px(Vec(6.7, 77.5)), module, Envelope::RATE6_PARAM));
            addParam(createParamCentered<Trimpot>(mm2px(Vec(15.0, 77.5)), module, Envelope::LEVEL6_PARAM));
            addChild(createLightCentered<SmallSimpleLight<RedLight>>(mm2px(Vec(2.2, 84.2)), module, Envelope::END7_LIGHT));
            addChild(createLightParamCentered<VCVLightButton<SmallSimpleLight<GreenLight>>>(mm2px(Vec(21.7, 85.6)), module, Envelope::SUSTAINSTEP7_PARAM, Envelope::SUSTAINSTEP7_LIGHT));
            addParam(createParamCentered<Trimpot>(mm2px(Vec(6.7, 88.0)), module, Envelope::RATE7_PARAM));
            addParam(createParamCentered<Trimpot>(mm2px(Vec(15.0, 88.0)), module, Envelope::LEVEL7_PARAM));
            addChild(createLightCentered<SmallSimpleLight<RedLight>>(mm2px(Vec(2.2, 94.7)), module, Envelope::END8_LIGHT));
            addChild(createLightParamCentered<VCVLightButton<SmallSimpleLight<GreenLight>>>(mm2px(Vec(21.7, 96.1)), module, Envelope::SUSTAINSTEP8_PARAM, Envelope::SUSTAINSTEP8_LIGHT));
            addParam(createParamCentered<Trimpot>(mm2px(Vec(6.7, 98.5)), module, Envelope::RATE8_PARAM));
            addParam(createParamCentered<Trimpot>(mm2px(Vec(15.0, 98.5)), module, Envelope::LEVEL8_PARAM));

            addInput(createInputCentered<PJ301MPort>(mm2px(Vec(6.8, 113.7)), module, Envelope::TRIGGER_INPUT));
            addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(18.3, 113.7)), module, Envelope::ENVELOPE_OUTPUT));
        }
    };

    Model *modelEnvelope = createModel<Envelope, EnvelopeWidget>("VZemu-Envelope");
}
