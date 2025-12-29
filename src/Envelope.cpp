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
        
        // Update sustain button states and lights
        updateSustainControls(numSteps);
        
        // Update number of steps indicator lights
        updateStepIndicatorLights(numSteps);
        
        // Handle trigger input
        if (handleTrigger())
        {
            return; // Skip step processing on the same frame as trigger detection
        }
        
        // Process envelope step if active
        if (currentStep >= 0)
        {
            processEnvelopeStep(args, numSteps);
        }

        // Output the current envelope value
        outputs[ENVELOPE_OUTPUT].setChannels(1);
        outputs[ENVELOPE_OUTPUT].setVoltage(currentOutput);
    }

    void Envelope::updateSustainControls(int numSteps)
    {
        bool newSustainDown = false;
        for (size_t i = 0; i < MAX_STEPS; ++i)
        {
            if (params[SUSTAINSTEP1_PARAM + i].getValue())
            {
                if (!anySustainDown)
                    currentSustainStep = (currentSustainStep == int(i + 1)) ? 0 : i + 1;
                newSustainDown = true;
            }
            
            // Update sustain lights
            float dimSustain = currentSustainStep > numSteps ? 0.3f : 1.0f;
            lights[SUSTAINSTEP1_LIGHT + i].setBrightness(dimSustain * (currentSustainStep == int(i + 1)));
        }
        anySustainDown = newSustainDown;
    }

    void Envelope::updateStepIndicatorLights(int numSteps)
    {
        for (size_t i = 0; i < MAX_STEPS; ++i)
        {
            lights[END1_LIGHT + 2 * i + 1].setBrightness(numSteps == int(i + 1));
        }
    }

    bool Envelope::handleTrigger()
    {
        bool triggerHigh = inputs[TRIGGER_INPUT].getVoltage() > 0.2f;
        
        if (!triggering && triggerHigh)
        {
            // Trigger detected - start new envelope
            lights[END1_LIGHT].setBrightness(1);
            if (currentStep > 0)
            {
                lights[END1_LIGHT + 2 * currentStep].setBrightness(0);
            }
            currentStep = 0;
            stepPhase = 0;
            triggering = true;
            return true; // Signal to skip step processing this frame
        }
        else if (triggering && !triggerHigh)
        {
            // Trigger released
            triggering = false;
        }
        
        return false;
    }

    void Envelope::processEnvelopeStep(const ProcessArgs &args, int numSteps)
    {
        stepPhase += args.sampleTime;
        float stepLength = 10.0f / (params[RATE1_PARAM + currentStep].getValue() + 1.0f);
        float targetLevel = params[LEVEL1_PARAM + currentStep].getValue() / 10.0f;

        // Check if we're holding at sustain point with target reached
        bool atSustainStep = (currentStep + 1 == currentSustainStep);
        bool reachedTarget = std::abs(currentOutput - targetLevel) < 0.01f;
        bool holdingSustain = (atSustainStep && triggering && reachedTarget);

        // Advance to next step when time elapsed, unless holding at sustain
        bool shouldAdvance = (stepPhase >= stepLength) && !holdingSustain;
        
        if (shouldAdvance)
        {
            advanceToNextStep(numSteps);
        }

        // Update output voltage
        if (currentStep >= 0)
        {
            updateOutput(holdingSustain, targetLevel, stepLength);
        }
    }

    void Envelope::advanceToNextStep(int numSteps)
    {
        lights[END1_LIGHT + 2 * currentStep].setBrightness(0);
        stepPhase = 0;
        ++currentStep;
        
        if (currentStep >= numSteps)
        {
            // Envelope complete
            currentStep = -1;
            currentOutput = 0.0f;
        }
        else
        {
            // Light up next step indicator
            lights[END1_LIGHT + 2 * currentStep].setBrightness(1);
        }
    }

    void Envelope::updateOutput(bool holdingSustain, float targetLevel, float stepLength)
    {
        if (holdingSustain)
        {
            // Hold at target level during sustain
            currentOutput = targetLevel;
        }
        else
        {
            // Linear interpolation from start level to target level
            float stepProgress = stepPhase / stepLength;
            
            // Get starting level (previous step's target or 0 if first step)
            float startLevel = 0.0f;
            if (currentStep > 0)
            {
                startLevel = params[LEVEL1_PARAM + currentStep - 1].getValue() / 10.0f;
            }
            
            currentOutput = startLevel + (targetLevel - startLevel) * stepProgress;
        }
        
        // Clamp output to valid range
        currentOutput = clamp(currentOutput, 0.0f, 10.0f);
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
