#pragma once

#include <rack.hpp>

namespace VZemu
{
    using namespace rack;

    // Forward declaration to avoid widget dependencies in tests
    extern rack::plugin::Plugin* pluginInstance;

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

        int currentSustainStep = 4; // Initialize to default value
        bool anySustainDown = false; // do not accept sustain button presses unless a moment of no button down has been noticed
        int currentStep = -1;
        float stepPhase = 0;
        float currentOutput = 0.0f;
        bool triggering = false;

        Envelope();
        void process(const ProcessArgs &args) override;

    private:
        void updateSustainControls(int numSteps);
        void updateStepIndicatorLights(int numSteps);
        bool handleTrigger();
        void processEnvelopeStep(const ProcessArgs &args, int numSteps);
        void advanceToNextStep(int numSteps);
        void updateOutput(bool holdingSustain, float targetLevel, float stepLength);
    };
}
