#include <gtest/gtest.h>
#include "../src/Envelope.hpp"
#include <rack.hpp>

using namespace VZemu;
using namespace rack;

// Stub for pluginInstance needed by EnvelopeWidget
namespace VZemu {
    rack::plugin::Plugin* pluginInstance = nullptr;
}

// Test fixture for Envelope tests
class EnvelopeTest : public ::testing::Test {
protected:
    Envelope* module;

    void SetUp() override {
        module = new Envelope();
    }

    void TearDown() override {
        delete module;
    }

    // Helper to process multiple samples
    void processSamples(int numSamples, float sampleRate = 44100.f) {
        for (int i = 0; i < numSamples; i++) {
            module->process(Module::ProcessArgs{sampleRate, 1.f / sampleRate});
        }
    }
};

// Test initial state after construction
TEST_F(EnvelopeTest, InitialState) {
    EXPECT_EQ(module->currentSustainStep, 4); // Default value
    EXPECT_EQ(module->currentStep, -1);
    EXPECT_FLOAT_EQ(module->currentOutput, 0.0f);
    EXPECT_FALSE(module->triggering);
    EXPECT_FALSE(module->anySustainDown);
}

// Test parameter configuration
TEST_F(EnvelopeTest, ParameterConfiguration) {
    // Check that all rate parameters are configured
    for (size_t i = 0; i < Envelope::MAX_STEPS; ++i) {
        EXPECT_NE(module->paramQuantities[Envelope::RATE1_PARAM + i], nullptr);
        EXPECT_TRUE(module->paramQuantities[Envelope::RATE1_PARAM + i]->snapEnabled);
        EXPECT_FLOAT_EQ(module->params[Envelope::RATE1_PARAM + i].getValue(), 99.0f);
    }

    // Check that all level parameters are configured
    for (size_t i = 0; i < Envelope::MAX_STEPS; ++i) {
        EXPECT_NE(module->paramQuantities[Envelope::LEVEL1_PARAM + i], nullptr);
        EXPECT_TRUE(module->paramQuantities[Envelope::LEVEL1_PARAM + i]->snapEnabled);
        EXPECT_FLOAT_EQ(module->params[Envelope::LEVEL1_PARAM + i].getValue(), 99.0f);
    }

    // Check number of steps parameter
    EXPECT_NE(module->paramQuantities[Envelope::NUMSTEPS_PARAM], nullptr);
    EXPECT_TRUE(module->paramQuantities[Envelope::NUMSTEPS_PARAM]->snapEnabled);
}

// Test input/output configuration
TEST_F(EnvelopeTest, IOConfiguration) {
    EXPECT_EQ(module->inputs.size(), Envelope::NUM_INPUTS);
    EXPECT_EQ(module->outputs.size(), Envelope::NUM_OUTPUTS);
    EXPECT_EQ(module->lights.size(), Envelope::NUM_LIGHTS);
}

// Test trigger input detection
TEST_F(EnvelopeTest, TriggerDetection) {
    // Set number of steps
    module->params[Envelope::NUMSTEPS_PARAM].setValue(4);
    
    // Initially no trigger
    module->inputs[Envelope::TRIGGER_INPUT].setVoltage(0.0f);
    module->process(Module::ProcessArgs{44100.f, 1.f / 44100.f});
    EXPECT_EQ(module->currentStep, -1);

    // Apply trigger
    module->inputs[Envelope::TRIGGER_INPUT].setVoltage(5.0f);
    module->process(Module::ProcessArgs{44100.f, 1.f / 44100.f});
    EXPECT_EQ(module->currentStep, 0); // Should start at step 0
}

// Test envelope does not retrigger while held high
TEST_F(EnvelopeTest, NoRetriggerWhileHigh) {
    module->params[Envelope::NUMSTEPS_PARAM].setValue(4);
    module->params[Envelope::RATE1_PARAM].setValue(99.0f);
    
    // Trigger envelope
    module->inputs[Envelope::TRIGGER_INPUT].setVoltage(5.0f);
    module->process(Module::ProcessArgs{44100.f, 1.f / 44100.f});
    EXPECT_EQ(module->currentStep, 0);
    EXPECT_TRUE(module->triggering);

    // Keep trigger high and process more samples
    processSamples(100);
    int currentStepAfterProcessing = module->currentStep;
    
    // Should still be running but not retriggered to step 0
    module->process(Module::ProcessArgs{44100.f, 1.f / 44100.f});
    EXPECT_EQ(module->currentStep, currentStepAfterProcessing);
    EXPECT_TRUE(module->triggering); // Still not loaded
}

// Test trigger reload after release
TEST_F(EnvelopeTest, TriggerReload) {
    module->params[Envelope::NUMSTEPS_PARAM].setValue(4);
    
    // Trigger and release
    module->inputs[Envelope::TRIGGER_INPUT].setVoltage(5.0f);
    module->process(Module::ProcessArgs{44100.f, 1.f / 44100.f});
    EXPECT_TRUE(module->triggering);

    // Release trigger
    module->inputs[Envelope::TRIGGER_INPUT].setVoltage(0.0f);
    module->process(Module::ProcessArgs{44100.f, 1.f / 44100.f});
    EXPECT_FALSE(module->triggering); // Should reload for next trigger
}

// Test number of steps parameter
TEST_F(EnvelopeTest, NumberOfSteps) {
    for (int numSteps = 1; numSteps <= 8; ++numSteps) {
        module->params[Envelope::NUMSTEPS_PARAM].setValue(numSteps);
        module->inputs[Envelope::TRIGGER_INPUT].setVoltage(0.0f);
        module->process(Module::ProcessArgs{44100.f, 1.f / 44100.f});
        
        // Check that the correct light is lit for the number of steps
        for (size_t i = 0; i < Envelope::MAX_STEPS; ++i) {
            float expectedBrightness = (numSteps == int(i + 1)) ? 1.0f : 0.0f;
            EXPECT_FLOAT_EQ(
                module->lights[Envelope::END1_LIGHT + 2 * i + 1].getBrightness(),
                expectedBrightness
            ) << "Step " << i << " with numSteps=" << numSteps;
        }
    }
}

// Test sustain step selection
TEST_F(EnvelopeTest, SustainStepSelection) {
    module->params[Envelope::NUMSTEPS_PARAM].setValue(4);
    
    // Initially sustain step is 4
    EXPECT_EQ(module->currentSustainStep, 4);

    // Press sustain button 2 (index 1)
    module->params[Envelope::SUSTAINSTEP1_PARAM + 1].setValue(1.0f);
    module->process(Module::ProcessArgs{44100.f, 1.f / 44100.f});
    EXPECT_EQ(module->currentSustainStep, 2);

    // Release and press same button again (should toggle off)
    module->params[Envelope::SUSTAINSTEP1_PARAM + 1].setValue(0.0f);
    module->process(Module::ProcessArgs{44100.f, 1.f / 44100.f});
    module->params[Envelope::SUSTAINSTEP1_PARAM + 1].setValue(1.0f);
    module->process(Module::ProcessArgs{44100.f, 1.f / 44100.f});
    EXPECT_EQ(module->currentSustainStep, 0); // Toggled off
}

// Test sustain step button debouncing
TEST_F(EnvelopeTest, SustainButtonDebounce) {
    module->params[Envelope::NUMSTEPS_PARAM].setValue(4);
    
    // Press and hold button
    module->params[Envelope::SUSTAINSTEP1_PARAM + 2].setValue(1.0f);
    module->process(Module::ProcessArgs{44100.f, 1.f / 44100.f});
    int firstStep = module->currentSustainStep;
    
    // Keep holding - should not toggle again
    module->process(Module::ProcessArgs{44100.f, 1.f / 44100.f});
    EXPECT_EQ(module->currentSustainStep, firstStep);
}

// Test rate parameter affects step length
TEST_F(EnvelopeTest, RateParameterAffectsStepLength) {
    module->params[Envelope::NUMSTEPS_PARAM].setValue(2);
    module->params[Envelope::RATE1_PARAM].setValue(99.0f); // Fast rate
    module->params[Envelope::LEVEL1_PARAM].setValue(50.0f);
    
    // Trigger envelope
    module->inputs[Envelope::TRIGGER_INPUT].setVoltage(5.0f);
    module->process(Module::ProcessArgs{44100.f, 1.f / 44100.f});
    EXPECT_EQ(module->currentStep, 0); // Starts at step 0
    
    // Calculate expected step length: 10 / (rate + 1)
    float expectedStepLength = 10.0f / (99.0f + 1.0f); // = 0.1 seconds
    int samplesForStep = static_cast<int>(expectedStepLength * 44100.f);
    
    // Process samples and verify step advances
    module->inputs[Envelope::TRIGGER_INPUT].setVoltage(0.0f);
    processSamples(samplesForStep - 10); // Just before step change
    EXPECT_EQ(module->currentStep, 0);
    
    processSamples(20); // Should advance to next step
    EXPECT_EQ(module->currentStep, 1);
}

// Test envelope output generation
TEST_F(EnvelopeTest, OutputGeneration) {
    module->params[Envelope::NUMSTEPS_PARAM].setValue(2);
    module->params[Envelope::LEVEL1_PARAM].setValue(50.0f); // Target 5V
    module->params[Envelope::RATE1_PARAM].setValue(9.0f); // Slower rate for testing
    
    // Trigger envelope
    module->inputs[Envelope::TRIGGER_INPUT].setVoltage(5.0f);
    module->process(Module::ProcessArgs{44100.f, 1.f / 44100.f});
    
    // Output should start moving from 0
    float initialOutput = module->outputs[Envelope::ENVELOPE_OUTPUT].getVoltage();
    EXPECT_GE(initialOutput, 0.0f);
    
    // Process more samples - output should increase
    processSamples(100);
    float laterOutput = module->outputs[Envelope::ENVELOPE_OUTPUT].getVoltage();
    EXPECT_GT(laterOutput, initialOutput);
}

// Test envelope completes and resets
TEST_F(EnvelopeTest, EnvelopeCompletion) {
    module->params[Envelope::NUMSTEPS_PARAM].setValue(2);
    module->params[Envelope::RATE1_PARAM].setValue(99.0f); // Fast
    module->params[Envelope::RATE1_PARAM + 1].setValue(99.0f); // Fast
    
    // Trigger envelope
    module->inputs[Envelope::TRIGGER_INPUT].setVoltage(5.0f);
    module->process(Module::ProcessArgs{44100.f, 1.f / 44100.f});
    module->inputs[Envelope::TRIGGER_INPUT].setVoltage(0.0f);
    
    // Process enough samples to complete envelope
    processSamples(50000); // Plenty of time
    
    // Should be complete
    EXPECT_EQ(module->currentStep, -1);
    EXPECT_FLOAT_EQ(module->currentOutput, 0.0f);
}

// Test sustain holds envelope
TEST_F(EnvelopeTest, SustainHoldsEnvelope) {
    module->params[Envelope::NUMSTEPS_PARAM].setValue(4);
    module->currentSustainStep = 2; // Sustain at step 2 (which is index 1)
    module->params[Envelope::RATE1_PARAM].setValue(99.0f);
    module->params[Envelope::RATE1_PARAM + 1].setValue(99.0f);
    module->params[Envelope::LEVEL1_PARAM].setValue(50.0f);
    module->params[Envelope::LEVEL1_PARAM + 1].setValue(50.0f); // Set a target level
    
    // Trigger envelope and HOLD trigger high
    module->inputs[Envelope::TRIGGER_INPUT].setVoltage(5.0f);
    module->process(Module::ProcessArgs{44100.f, 1.f / 44100.f});
    EXPECT_EQ(module->currentStep, 0);
    
    // Advance to step 1 (the sustain step - index 1, which is step 2)
    processSamples(5000);
    EXPECT_EQ(module->currentStep, 1);
    
    // Keep trigger HIGH and wait for target to be reached
    processSamples(5000);
    
    // Should be holding at step 1
    EXPECT_EQ(module->currentStep, 1);
    
    // Process more samples - should still be at step 1
    processSamples(10000);
    EXPECT_EQ(module->currentStep, 1);
}

// Test output is zero when envelope is not active
TEST_F(EnvelopeTest, OutputZeroWhenInactive) {
    module->params[Envelope::NUMSTEPS_PARAM].setValue(4);
    module->inputs[Envelope::TRIGGER_INPUT].setVoltage(0.0f);
    
    module->process(Module::ProcessArgs{44100.f, 1.f / 44100.f});
    
    EXPECT_FLOAT_EQ(module->outputs[Envelope::ENVELOPE_OUTPUT].getVoltage(), 0.0f);
}

// Test all lights are properly configured
TEST_F(EnvelopeTest, LightConfiguration) {
    module->params[Envelope::NUMSTEPS_PARAM].setValue(4);
    module->inputs[Envelope::TRIGGER_INPUT].setVoltage(0.0f);
    
    module->process(Module::ProcessArgs{44100.f, 1.f / 44100.f});
    
    // All lights should be accessible and not crash
    for (size_t i = 0; i < Envelope::MAX_STEPS; ++i) {
        EXPECT_NO_THROW(module->lights[Envelope::END1_LIGHT + 2 * i].getBrightness());
        EXPECT_NO_THROW(module->lights[Envelope::SUSTAINSTEP1_LIGHT + i].getBrightness());
    }
}

// Test sustain holds envelope when trigger is HIGH
TEST_F(EnvelopeTest, SustainHoldsWhenTriggerHeld) {
    module->params[Envelope::NUMSTEPS_PARAM].setValue(4);
    module->currentSustainStep = 2; // Sustain at step 2 (index 1)
    module->params[Envelope::RATE1_PARAM].setValue(99.0f);
    module->params[Envelope::RATE1_PARAM + 1].setValue(99.0f);
    module->params[Envelope::LEVEL1_PARAM].setValue(50.0f);
    module->params[Envelope::LEVEL1_PARAM + 1].setValue(50.0f);
    
    // Trigger envelope and HOLD trigger high
    module->inputs[Envelope::TRIGGER_INPUT].setVoltage(5.0f);
    module->process(Module::ProcessArgs{1.f / 44100.f, 44100.f});
    
    // Advance to step 1 (sustain step index)
    processSamples(5000);
    EXPECT_EQ(module->currentStep, 1);
    
    // Wait for target to be reached
    processSamples(5000);
    
    // Keep trigger HIGH - should stay at sustain step
    module->inputs[Envelope::TRIGGER_INPUT].setVoltage(5.0f);
    processSamples(10000);
    EXPECT_EQ(module->currentStep, 1); // Should still be at sustain step
}

// Test sustain releases when trigger goes LOW
TEST_F(EnvelopeTest, SustainReleasesWhenTriggerReleased) {
    module->params[Envelope::NUMSTEPS_PARAM].setValue(4);
    module->currentSustainStep = 2; // Sustain at step 2
    module->params[Envelope::RATE1_PARAM].setValue(99.0f);
    module->params[Envelope::RATE1_PARAM + 1].setValue(99.0f);
    module->params[Envelope::RATE1_PARAM + 2].setValue(99.0f);
    module->params[Envelope::LEVEL1_PARAM].setValue(50.0f);
    module->params[Envelope::LEVEL1_PARAM + 1].setValue(50.0f);
    
    // Trigger and hold
    module->inputs[Envelope::TRIGGER_INPUT].setVoltage(5.0f);
    module->process(Module::ProcessArgs{1.f / 44100.f, 44100.f});
    
    // Advance to sustain step
    processSamples(5000);
    EXPECT_EQ(module->currentStep, 1);
    
    // Wait for target
    processSamples(5000);
    
    // Hold at sustain
    processSamples(5000);
    EXPECT_EQ(module->currentStep, 1);
    
    // Release trigger - should continue envelope
    module->inputs[Envelope::TRIGGER_INPUT].setVoltage(0.0f);
    processSamples(5000);
    EXPECT_EQ(module->currentStep, 3); // Should have advanced
}

// Test envelope bypasses sustain when no sustain point set
TEST_F(EnvelopeTest, NoSustainBypassesHold) {
    module->params[Envelope::NUMSTEPS_PARAM].setValue(3);
    module->currentSustainStep = 0; // No sustain
    module->params[Envelope::RATE1_PARAM].setValue(99.0f);
    module->params[Envelope::RATE1_PARAM + 1].setValue(99.0f);
    module->params[Envelope::RATE1_PARAM + 2].setValue(99.0f);
    
    // Trigger and hold
    module->inputs[Envelope::TRIGGER_INPUT].setVoltage(5.0f);
    module->process(Module::ProcessArgs{1.f / 44100.f, 44100.f});
    
    // Should progress through all steps even with trigger held
    processSamples(5000);
    EXPECT_EQ(module->currentStep, 1);
    
    processSamples(5000);
    EXPECT_EQ(module->currentStep, 2);
    
    processSamples(5000);
    EXPECT_EQ(module->currentStep, -1); // Complete
}

// Test sustain at first step
TEST_F(EnvelopeTest, SustainAtFirstStep) {
    module->params[Envelope::NUMSTEPS_PARAM].setValue(4);
    module->currentSustainStep = 1; // Sustain at first step
    module->params[Envelope::RATE1_PARAM].setValue(99.0f);
    module->params[Envelope::LEVEL1_PARAM].setValue(50.0f);
    
    // Trigger and hold
    module->inputs[Envelope::TRIGGER_INPUT].setVoltage(5.0f);
    module->process(Module::ProcessArgs{1.f / 44100.f, 44100.f});
    EXPECT_EQ(module->currentStep, 0);
    
    // Wait for target to be reached
    processSamples(5000);
    
    // Should stay at first step while trigger held
    processSamples(10000);
    EXPECT_EQ(module->currentStep, 0);
}

// Test sustain at last step
TEST_F(EnvelopeTest, SustainAtLastStep) {
    module->params[Envelope::NUMSTEPS_PARAM].setValue(3);
    module->currentSustainStep = 3; // Sustain at last step
    module->params[Envelope::RATE1_PARAM].setValue(99.0f);
    module->params[Envelope::RATE1_PARAM + 1].setValue(99.0f);
    module->params[Envelope::RATE1_PARAM + 2].setValue(99.0f);
    module->params[Envelope::LEVEL1_PARAM].setValue(50.0f);
    module->params[Envelope::LEVEL1_PARAM + 1].setValue(50.0f);
    module->params[Envelope::LEVEL1_PARAM + 2].setValue(50.0f);
    
    // Trigger and hold
    module->inputs[Envelope::TRIGGER_INPUT].setVoltage(5.0f);
    module->process(Module::ProcessArgs{1.f / 44100.f, 44100.f});
    
    // Advance to last step
    processSamples(5000);
    EXPECT_EQ(module->currentStep, 1);
    
    processSamples(5000);
    EXPECT_EQ(module->currentStep, 2);
    
    // Wait for target
    processSamples(5000);
    
    // Should stay at last step while trigger held
    processSamples(10000);
    EXPECT_EQ(module->currentStep, 2);
}

// Test output continues to interpolate during sustain
TEST_F(EnvelopeTest, OutputInterpolatesDuringSustain) {
    module->params[Envelope::NUMSTEPS_PARAM].setValue(3);
    module->currentSustainStep = 2;
    module->params[Envelope::LEVEL1_PARAM].setValue(30.0f);
    module->params[Envelope::LEVEL1_PARAM + 1].setValue(70.0f);
    module->params[Envelope::RATE1_PARAM].setValue(50.0f);
    module->params[Envelope::RATE1_PARAM + 1].setValue(10.0f); // Slower rate
    
    // Trigger and hold
    module->inputs[Envelope::TRIGGER_INPUT].setVoltage(5.0f);
    module->process(Module::ProcessArgs{1.f / 44100.f, 44100.f});
    
    // Advance to sustain step
    processSamples(10000);
    EXPECT_EQ(module->currentStep, 1);
    
    float outputAtSustainStart = module->currentOutput;
    float targetLevel = 7.0f; // 70 / 10
    
    // During sustain approach, output should still be changing toward target
    if (std::abs(outputAtSustainStart - targetLevel) > 0.02f) {
        processSamples(5000);
        float outputDuringSustain = module->currentOutput;
        // Should be different (moving toward target) or equal if already at target
        EXPECT_TRUE(std::abs(outputDuringSustain - outputAtSustainStart) > 0.001f || 
                    std::abs(outputDuringSustain - targetLevel) < 0.02f);
    }
}

// Test sustain step beyond number of steps is ignored
TEST_F(EnvelopeTest, SustainBeyondNumStepsIgnored) {
    module->params[Envelope::NUMSTEPS_PARAM].setValue(3);
    module->currentSustainStep = 5; // Beyond number of steps
    module->params[Envelope::RATE1_PARAM].setValue(99.0f);
    module->params[Envelope::RATE1_PARAM + 1].setValue(99.0f);
    module->params[Envelope::RATE1_PARAM + 2].setValue(99.0f);
    
    // Trigger and hold
    module->inputs[Envelope::TRIGGER_INPUT].setVoltage(5.0f);
    module->process(Module::ProcessArgs{1.f / 44100.f, 44100.f});
    
    // Should progress through all steps without sustaining
    processSamples(5000);
    processSamples(5000);
    processSamples(5000);
    EXPECT_EQ(module->currentStep, -1); // Should complete
}

// Test trigger threshold detection
TEST_F(EnvelopeTest, TriggerThreshold) {
    module->params[Envelope::NUMSTEPS_PARAM].setValue(2);
    
    // Below threshold - should not trigger
    module->inputs[Envelope::TRIGGER_INPUT].setVoltage(0.1f);
    module->process(Module::ProcessArgs{44100.f, 1.f / 44100.f});
    EXPECT_EQ(module->currentStep, -1);
    
    // Make sure loaded is true for next test
    module->triggering = false;
    
    // Above threshold (0.2V) - should trigger
    module->inputs[Envelope::TRIGGER_INPUT].setVoltage(0.3f);
    module->process(Module::ProcessArgs{44100.f, 1.f / 44100.f});
    EXPECT_EQ(module->currentStep, 0);
}

// Test changing sustain step during envelope playback
TEST_F(EnvelopeTest, ChangeSustainDuringPlayback) {
    module->params[Envelope::NUMSTEPS_PARAM].setValue(4);
    module->currentSustainStep = 3; // Start with sustain at step 3
    module->params[Envelope::RATE1_PARAM].setValue(99.0f);
    module->params[Envelope::RATE1_PARAM + 1].setValue(99.0f);
    module->params[Envelope::LEVEL1_PARAM].setValue(50.0f);
    module->params[Envelope::LEVEL1_PARAM + 1].setValue(50.0f);
    
    // Trigger and hold
    module->inputs[Envelope::TRIGGER_INPUT].setVoltage(5.0f);
    module->process(Module::ProcessArgs{1.f / 44100.f, 44100.f});
    
    // Advance to step 1
    processSamples(5000);
    EXPECT_EQ(module->currentStep, 1);
    
    // Change sustain point to step 2 during playback
    module->currentSustainStep = 2;
    
    // Wait for target to be reached at current step
    processSamples(5000);
    
    // Should now sustain at step 1 (which is step 2 - 1)
    processSamples(5000);
    EXPECT_EQ(module->currentStep, 1);
}

// Test rapid trigger pulses
TEST_F(EnvelopeTest, RapidTriggerPulses) {
    module->params[Envelope::NUMSTEPS_PARAM].setValue(2);
    module->params[Envelope::RATE1_PARAM].setValue(99.0f);
    
    for (int i = 0; i < 5; ++i) {
        // Trigger
        module->inputs[Envelope::TRIGGER_INPUT].setVoltage(5.0f);
        module->process(Module::ProcessArgs{44100.f, 1.f / 44100.f});
        EXPECT_EQ(module->currentStep, 0);
        
        // Release
        module->inputs[Envelope::TRIGGER_INPUT].setVoltage(0.0f);
        module->process(Module::ProcessArgs{44100.f, 1.f / 44100.f});
        EXPECT_FALSE(module->triggering);
    }
}

// Test output reaches target level
TEST_F(EnvelopeTest, OutputReachesTargetLevel) {
    module->params[Envelope::NUMSTEPS_PARAM].setValue(1);
    module->params[Envelope::LEVEL1_PARAM].setValue(75.0f); // 7.5V
    module->params[Envelope::RATE1_PARAM].setValue(99.0f);
    
    module->inputs[Envelope::TRIGGER_INPUT].setVoltage(5.0f);
    module->currentSustainStep = 1;

    module->process(Module::ProcessArgs{44100.f, 1.f / 44100.f});
    
    // Process through entire step
    processSamples(5000);
    
    // Should be close to target (7.5V)
    EXPECT_NEAR(module->currentOutput, 7.5f, 0.2f);
}

// Test sustain light brightness dimming
TEST_F(EnvelopeTest, SustainLightDimming) {
    module->params[Envelope::NUMSTEPS_PARAM].setValue(3);
    
    // Set sustain beyond num steps
    module->currentSustainStep = 5;
    module->process(Module::ProcessArgs{44100.f, 1.f / 44100.f});
    
    // Sustain light should be dimmed
    float brightness = module->lights[Envelope::SUSTAINSTEP1_LIGHT + 4].getBrightness();
    EXPECT_FLOAT_EQ(brightness, 0.3f);
    
    // Set sustain within range
    module->currentSustainStep = 2;
    module->process(Module::ProcessArgs{44100.f, 1.f / 44100.f});
    
    brightness = module->lights[Envelope::SUSTAINSTEP1_LIGHT + 1].getBrightness();
    EXPECT_FLOAT_EQ(brightness, 1.0f);
}

// Test step phase resets on step change
TEST_F(EnvelopeTest, StepPhaseResetsOnChange) {
    module->params[Envelope::NUMSTEPS_PARAM].setValue(2);
    module->params[Envelope::RATE1_PARAM].setValue(99.0f);
    module->params[Envelope::RATE1_PARAM + 1].setValue(99.0f);
    
    module->inputs[Envelope::TRIGGER_INPUT].setVoltage(5.0f);
    module->process(Module::ProcessArgs{44100.f, 1.f / 44100.f});
    module->inputs[Envelope::TRIGGER_INPUT].setVoltage(0.0f);
    
    // Advance partway through step
    processSamples(2000);
    float phaseAfterPartialStep = module->stepPhase;
    EXPECT_GT(phaseAfterPartialStep, 0.0f);
    
    // Advance to next step
    processSamples(3000);
    EXPECT_EQ(module->currentStep, 1);
    
    // Phase should have reset
    EXPECT_LT(module->stepPhase, phaseAfterPartialStep);
}

// Test envelope with all steps at minimum rate
TEST_F(EnvelopeTest, MinimumRateSteps) {
    module->params[Envelope::NUMSTEPS_PARAM].setValue(2);
    module->params[Envelope::RATE1_PARAM].setValue(0.0f); // Slowest
    module->params[Envelope::RATE1_PARAM + 1].setValue(0.0f);
    
    module->inputs[Envelope::TRIGGER_INPUT].setVoltage(5.0f);
    module->process(Module::ProcessArgs{44100.f, 1.f / 44100.f});
    module->inputs[Envelope::TRIGGER_INPUT].setVoltage(0.0f);
    
    // At rate 0, step length = 10 seconds
    processSamples(44100); // 1 second
    EXPECT_EQ(module->currentStep, 0); // Should still be on first step
}

// Test output zero after completion
TEST_F(EnvelopeTest, OutputZeroAfterCompletion) {
    module->params[Envelope::NUMSTEPS_PARAM].setValue(1);
    module->params[Envelope::RATE1_PARAM].setValue(99.0f);
    module->params[Envelope::LEVEL1_PARAM].setValue(80.0f);
    
    module->inputs[Envelope::TRIGGER_INPUT].setVoltage(5.0f);
    module->process(Module::ProcessArgs{1.f / 44100.f, 44100.f});
    module->inputs[Envelope::TRIGGER_INPUT].setVoltage(0.0f);
    
    processSamples(10000);
    EXPECT_EQ(module->currentStep, -1);
    EXPECT_FLOAT_EQ(module->currentOutput, 0.0f);
    EXPECT_FLOAT_EQ(module->outputs[Envelope::ENVELOPE_OUTPUT].getVoltage(), 0.0f);
}

// Main function for running tests
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
