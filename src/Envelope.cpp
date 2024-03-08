#include "VZemu.hpp"

#include <rack.hpp>
#include "HorizontalSlider.hpp"

#include <cmath>

namespace VZemu
{
    using namespace rack;

    struct Envelope : Module
    {
        static const size_t MAX_STEPS = 8;

        enum ParamIds
        {
            RATE1_PARAM,
            RATE_END_PARAM = RATE1_PARAM + MAX_STEPS,
            LEVEL1_PARAM = RATE_END_PARAM,
            LEVEL_END_PARAM = LEVEL1_PARAM + MAX_STEPS,
            SUSTAINSTEP1_PARAM = LEVEL_END_PARAM,
            SUSTAINSTEP_END_PARAM = SUSTAINSTEP1_PARAM + MAX_STEPS,
            NUMSTEPS_PARAM = SUSTAINSTEP_END_PARAM,
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
            END_END_LIGHT = END1_LIGHT + MAX_STEPS * 2,
            SUSTAINSTEP1_LIGHT = END_END_LIGHT,
            SUSTAINSTEP_END_LIGHT = SUSTAINSTEP1_LIGHT + MAX_STEPS,
            NUM_LIGHTS = SUSTAINSTEP_END_LIGHT
        };

        int currentSustainStep = 4; // Start with 4 steps

        Envelope()
        {
            config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

            for (size_t i = 0; i < MAX_STEPS; ++i)
            {
                configParam(RATE1_PARAM + i, 0, 99, 99);
                paramQuantities[RATE1_PARAM + i]->snapEnabled = true;
                configParam(LEVEL1_PARAM + i, 0, 99, 99);
                paramQuantities[LEVEL1_PARAM + i]->snapEnabled = true;
                configParam(SUSTAINSTEP1_PARAM + i, 0, 1, 0, "Sustain Step " + i);
                configLight(END1_LIGHT + i, i + " steps");
                configLight(SUSTAINSTEP1_LIGHT + i);
            }
            configParam(NUMSTEPS_PARAM, 1, 8, 1, "Number of Steps");
            paramQuantities[NUMSTEPS_PARAM]->snapEnabled = true;
        }

        bool anySustainDown = false; // do not accept sustain button presses unless a moment of no button down has been noticed
        int currentStep = -1;
        float stepPhase;
        float currentOutput = 0;
        bool loaded = true;

        void process(const ProcessArgs &args) override
        {
            int numSteps = params[NUMSTEPS_PARAM].getValue();
            bool newSustainDown = false;
            for (size_t i = 0; i < MAX_STEPS; ++i)
            {
                lights[END1_LIGHT + 2 * i + 1].setBrightness(numSteps == int(i + 1));

                if (params[SUSTAINSTEP1_PARAM + i].getValue())
                {
                    if (!anySustainDown)
                        currentSustainStep = (currentSustainStep == int(i + 1)) ? 0 : i + 1;
                    newSustainDown = true;
                }
            }
            anySustainDown = newSustainDown;

            float dimSustain = currentSustainStep > numSteps ? .3 : 1;
            for (size_t i = 0; i < MAX_STEPS; ++i)
            {
                lights[SUSTAINSTEP1_LIGHT + i].setBrightness(dimSustain * (currentSustainStep == int(i + 1)));
            }

            if (loaded && inputs[TRIGGER_INPUT].getVoltage() > 0.2)
            {
                lights[END1_LIGHT].setBrightness(1);
                lights[END1_LIGHT + 2 * currentStep].setBrightness(0);
                currentStep = 0;
                stepPhase = 0;
                loaded = false;
            }
            else if (!loaded && inputs[TRIGGER_INPUT].getVoltage() < 0.2)
            {
                loaded = true;
            }
            if (currentStep >= 0)
            {
                stepPhase += args.sampleTime;
                float stepLength = 10. / (params[RATE1_PARAM + currentStep].getValue() + 1);

                if (loaded || currentStep + 1 != currentSustainStep)
                {
                    if (stepPhase >= stepLength)
                    {

                        lights[END1_LIGHT + 2 * currentStep].setBrightness(0);
                        stepPhase = 0;
                        ++currentStep;
                        if (currentStep >= params[NUMSTEPS_PARAM].getValue())
                        {
                            currentStep = -1;
                            currentOutput = 0;
                        }
                        else
                        {
                            lights[END1_LIGHT + 2 * currentStep].setBrightness(1);
                        }
                    }

                    float advance = (params[LEVEL1_PARAM + currentStep].getValue() / 10 - currentOutput) / (stepLength - stepPhase) * args.sampleTime;
                    currentOutput += advance;
                }
            }

            outputs[ENVELOPE_OUTPUT].setChannels(1);
            outputs[ENVELOPE_OUTPUT].setVoltage(currentOutput);
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

            for (size_t i = 0; i < Envelope::MAX_STEPS; ++i)
            {
                addChild(createLightCentered<SmallSimpleLight<GreenRedLight>>(mm2px(Vec(2.2, 21.2 + i * 10.5)), module, Envelope::END1_LIGHT + 2 * i));
                addChild(createLightParamCentered<VCVLightButton<SmallSimpleLight<GreenLight>>>(mm2px(Vec(21.7, 22.6 + i * 10.5)), module, Envelope::SUSTAINSTEP1_PARAM + i, Envelope::SUSTAINSTEP1_LIGHT + i));
                addParam(createParamCentered<Trimpot>(mm2px(Vec(6.7, 25.0 + i * 10.5)), module, Envelope::RATE1_PARAM + i));
                addParam(createParamCentered<Trimpot>(mm2px(Vec(15.0, 25.0 + i * 10.5)), module, Envelope::LEVEL1_PARAM + i));
            }

            addInput(createInputCentered<PJ301MPort>(mm2px(Vec(6.8, 113.7)), module, Envelope::TRIGGER_INPUT));
            addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(18.3, 113.7)), module, Envelope::ENVELOPE_OUTPUT));
        }
    };

    Model *modelEnvelope = createModel<Envelope, EnvelopeWidget>("VZemu-Envelope");
}
