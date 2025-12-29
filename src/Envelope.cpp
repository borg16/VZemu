#include "Envelope.hpp"
#include "VZemu.hpp"

#ifndef ENVELOPE_TEST_BUILD
#include "HorizontalSlider.hpp"
#endif

#include <rack.hpp>
#include <cmath>

namespace VZemu
{
    using namespace rack;

    Envelope::Envelope()
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

    void Envelope::process(const ProcessArgs &args)
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

        if (!triggering && inputs[TRIGGER_INPUT].getVoltage() > 0.2)
        {
            lights[END1_LIGHT].setBrightness(1);
            if (currentStep > 0)
            {
                lights[END1_LIGHT + 2 * currentStep].setBrightness(0);
            }
            currentStep = 0;
            stepPhase = 0;
            triggering = true;
            // Don't process step on the same call where trigger is detected
        }
        else if (triggering && inputs[TRIGGER_INPUT].getVoltage() < 0.2)
        {
            triggering = false;
        }
        else if (currentStep >= 0)
        {
            stepPhase += args.sampleTime;
            float stepLength = 10. / (params[RATE1_PARAM + currentStep].getValue() + 1);
            float targetLevel = params[LEVEL1_PARAM + currentStep].getValue() / 10.0f;

            // Check if we're at the sustain step and trigger is held
            bool atSustainAndHeld = (triggering && currentStep + 1 == currentSustainStep);
            
            // Check if we've reached the target level
            bool reachedTarget = std::abs(currentOutput - targetLevel) < 0.01f;

            // Check if we should advance to next step:
            // - Advance if trigger is released (triggering == false)
            // - OR if not at the sustain step
            // - AND step duration has elapsed
            // - OR if at sustain and target reached
            bool shouldAdvance = false;
            if (atSustainAndHeld && reachedTarget)
            {
                // At sustain step, target reached, and trigger held: don't advance
                shouldAdvance = false;
            }
            else if (!triggering || currentStep + 1 != currentSustainStep)
            {
                // Normal step progression when not at sustain or trigger released
                shouldAdvance = stepPhase >= stepLength;
            }
            
            if (shouldAdvance)
            {
                lights[END1_LIGHT + 2 * currentStep].setBrightness(0);
                stepPhase = 0;
                ++currentStep;
                if (currentStep >= params[NUMSTEPS_PARAM].getValue())
                {
                    currentStep = -1;
                    currentOutput = 0.0f;
                }
                else
                {
                    lights[END1_LIGHT + 2 * currentStep].setBrightness(1);
                }
            }

            // Interpolate output towards target, but only if not holding at sustain
            if (currentStep >= 0)
            {
                if (atSustainAndHeld && reachedTarget)
                {
                    // Hold at target level during sustain
                    currentOutput = targetLevel;
                }
                else
                {
                    // Interpolate towards target using linear interpolation over the step duration
                    // Calculate how far through the step we are (0 to 1)
                    float stepProgress = stepPhase / stepLength;
                    
                    // Get the starting level (previous step's target or 0 if first step)
                    float startLevel = 0.0f;
                    if (currentStep > 0) {
                        startLevel = params[LEVEL1_PARAM + currentStep - 1].getValue() / 10.0f;
                    }
                    
                    // Linear interpolation from start to target based on progress
                    currentOutput = startLevel + (targetLevel - startLevel) * stepProgress;
                }
                
                // Clamp output to valid range [0, 10V]
                currentOutput = clamp(currentOutput, 0.0f, 10.0f);
            }
        }

        outputs[ENVELOPE_OUTPUT].setChannels(1);
        outputs[ENVELOPE_OUTPUT].setVoltage(currentOutput);
    }

#ifndef ENVELOPE_TEST_BUILD
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
#endif
}
