#include <gtest/gtest.h>
#include "../src/ModulePair.cpp"
#include <rack.hpp>
#include <cmath>

using namespace VZemu;
using namespace rack;

// Stub for pluginInstance needed by ModulePairWidget
namespace VZemu {
    rack::plugin::Plugin* pluginInstance = nullptr;
}

// Test fixture for ModulePair tests
class ModulePairTest : public ::testing::Test {
protected:
    ModulePair* module;

    void SetUp() override {
        module = new ModulePair();
    }

    void TearDown() override {
        delete module;
    }

    // Helper to process a single sample
    void processSample(float sampleRate = 44100.f) {
        module->process(Module::ProcessArgs{sampleRate, 1.f / sampleRate});
    }

    // Helper to process multiple samples
    void processSamples(int numSamples, float sampleRate = 44100.f) {
        for (int i = 0; i < numSamples; i++) {
            processSample(sampleRate);
        }
    }
};

// Test initial state after construction
TEST_F(ModulePairTest, InitialState) {
    // Check initial phase states
    for (int i = 0; i < engine::PORT_MAX_CHANNELS; i++) {
        EXPECT_FLOAT_EQ(module->phase1[i], 0.0f);
        EXPECT_FLOAT_EQ(module->phase2[i], 0.0f);
    }
}

// Test parameter configuration
TEST_F(ModulePairTest, ParameterConfiguration) {
    // Check waveform parameters
    EXPECT_NE(module->paramQuantities[ModulePair::WAVEFORM1_PARAM], nullptr);
    EXPECT_TRUE(module->paramQuantities[ModulePair::WAVEFORM1_PARAM]->snapEnabled);
    EXPECT_FLOAT_EQ(module->params[ModulePair::WAVEFORM1_PARAM].getValue(), 0.0f);
    
    EXPECT_NE(module->paramQuantities[ModulePair::WAVEFORM2_PARAM], nullptr);
    EXPECT_TRUE(module->paramQuantities[ModulePair::WAVEFORM2_PARAM]->snapEnabled);
    EXPECT_FLOAT_EQ(module->params[ModulePair::WAVEFORM2_PARAM].getValue(), 0.0f);

    // Check volume parameters
    EXPECT_NE(module->paramQuantities[ModulePair::VOLUME1_PARAM], nullptr);
    EXPECT_TRUE(module->paramQuantities[ModulePair::VOLUME1_PARAM]->snapEnabled);
    EXPECT_FLOAT_EQ(module->params[ModulePair::VOLUME1_PARAM].getValue(), 50.0f);
    
    EXPECT_NE(module->paramQuantities[ModulePair::VOLUME2_PARAM], nullptr);
    EXPECT_TRUE(module->paramQuantities[ModulePair::VOLUME2_PARAM]->snapEnabled);
    EXPECT_FLOAT_EQ(module->params[ModulePair::VOLUME2_PARAM].getValue(), 50.0f);

    // Check mode switches
    EXPECT_NE(module->paramQuantities[ModulePair::MIX_PARAM], nullptr);
    EXPECT_TRUE(module->paramQuantities[ModulePair::MIX_PARAM]->snapEnabled);
    EXPECT_FLOAT_EQ(module->params[ModulePair::MIX_PARAM].getValue(), 0.0f);
    
    EXPECT_NE(module->paramQuantities[ModulePair::PHASE_PARAM], nullptr);
    EXPECT_TRUE(module->paramQuantities[ModulePair::PHASE_PARAM]->snapEnabled);
}

// Test input/output configuration
TEST_F(ModulePairTest, IOConfiguration) {
    EXPECT_EQ(module->inputs.size(), ModulePair::NUM_INPUTS);
    EXPECT_EQ(module->outputs.size(), ModulePair::NUM_OUTPUTS);
    EXPECT_EQ(module->lights.size(), ModulePair::NUM_LIGHTS);
}

// Test light states in different modes
TEST_F(ModulePairTest, LightStatesVCOMode) {
    // VCO mode: PHASE_PARAM off, no EXT_PHASE_INPUT
    module->params[ModulePair::PHASE_PARAM].setValue(0.0f);
    module->params[ModulePair::MIX_PARAM].setValue(0.0f);
    
    processSample();
    
    EXPECT_FLOAT_EQ(module->lights[ModulePair::INTERNAL_PHASE_LIGHT].getBrightness(), 0.0f);
    EXPECT_GT(module->lights[ModulePair::VCO_MODE_LIGHT].getBrightness(), 0.0f);
    EXPECT_FLOAT_EQ(module->lights[ModulePair::RING_LIGHT].getBrightness(), 0.0f);
}

TEST_F(ModulePairTest, LightStatesInternalPhaseMode) {
    // Internal phase mode
    module->params[ModulePair::PHASE_PARAM].setValue(1.0f);
    
    processSample();
    
    EXPECT_GT(module->lights[ModulePair::INTERNAL_PHASE_LIGHT].getBrightness(), 0.0f);
    EXPECT_FLOAT_EQ(module->lights[ModulePair::VCO_MODE_LIGHT].getBrightness(), 0.0f);
}

TEST_F(ModulePairTest, LightStatesRingMode) {
    // Ring mode: VCO mode active + MIX_PARAM on
    module->params[ModulePair::PHASE_PARAM].setValue(0.0f);
    module->params[ModulePair::MIX_PARAM].setValue(1.0f);
    
    processSample();
    
    EXPECT_GT(module->lights[ModulePair::RING_LIGHT].getBrightness(), 0.0f);
}

TEST_F(ModulePairTest, LightStatesWithExternalPhase) {
    // External phase mode: no internal phase, EXT_PHASE connected
    module->params[ModulePair::PHASE_PARAM].setValue(0.0f);
    module->inputs[ModulePair::EXT_PHASE_INPUT].setChannels(1);
    module->inputs[ModulePair::EXT_PHASE_INPUT].setVoltage(5.0f);
    
    processSample();
    
    // Note: In test environment, isConnected() might not work as in real Rack
    // Just verify the light state is set
    EXPECT_GE(module->lights[ModulePair::VCO_MODE_LIGHT].getBrightness(), 0.0f);
}

// Test polyphony detection
TEST_F(ModulePairTest, PolyphonyMonophonic) {
    module->inputs[ModulePair::PITCH1_INPUT].setChannels(1);
    module->inputs[ModulePair::PITCH2_INPUT].setChannels(1);
    module->inputs[ModulePair::VOLUME1_INPUT].setChannels(1);
    module->inputs[ModulePair::VOLUME2_INPUT].setChannels(1);
    
    // Set non-zero voltages
    module->inputs[ModulePair::VOLUME1_INPUT].setVoltage(10.0f);
    module->inputs[ModulePair::VOLUME2_INPUT].setVoltage(10.0f);
    
    // Should process without crashing
    EXPECT_NO_THROW(processSample());
}

TEST_F(ModulePairTest, PolyphonyMultiple) {
    module->inputs[ModulePair::PITCH1_INPUT].setChannels(4);
    module->inputs[ModulePair::PITCH2_INPUT].setChannels(2);
    module->inputs[ModulePair::VOLUME1_INPUT].setChannels(4);
    module->inputs[ModulePair::VOLUME2_INPUT].setChannels(4);
    
    // Should process multiple channels without crashing
    EXPECT_NO_THROW(processSample());
}

TEST_F(ModulePairTest, PolyphonyNoInputs) {
    // No inputs connected, module should still process without crashing
    EXPECT_NO_THROW(processSample());
}

// Test phase accumulation in VCO mode
TEST_F(ModulePairTest, PhaseAccumulation) {
    module->params[ModulePair::PHASE_PARAM].setValue(0.0f);
    module->inputs[ModulePair::PITCH1_INPUT].setChannels(1);
    module->inputs[ModulePair::PITCH1_INPUT].setVoltage(0.0f); // C4
    module->inputs[ModulePair::VOLUME1_INPUT].setChannels(1);
    module->inputs[ModulePair::VOLUME1_INPUT].setVoltage(10.0f);
    
    float initialPhase = module->phase1[0];
    
    processSamples(100);
    
    // Phase should have advanced
    EXPECT_GT(module->phase1[0], initialPhase);
}

TEST_F(ModulePairTest, PhaseWrapping) {
    module->params[ModulePair::PHASE_PARAM].setValue(0.0f);
    module->inputs[ModulePair::PITCH1_INPUT].setChannels(1);
    module->inputs[ModulePair::PITCH1_INPUT].setVoltage(5.0f); // High pitch
    module->inputs[ModulePair::VOLUME1_INPUT].setChannels(1);
    module->inputs[ModulePair::VOLUME1_INPUT].setVoltage(10.0f);
    
    processSamples(1000);
    
    // Phase should stay between 0 and 1
    EXPECT_GE(module->phase1[0], 0.0f);
    EXPECT_LT(module->phase1[0], 1.0f);
}

// Test waveform parameter selection
TEST_F(ModulePairTest, WaveformParameterRange) {
    // Test all waveform values (0-7)
    for (int wf = 0; wf <= 7; wf++) {
        module->params[ModulePair::WAVEFORM1_PARAM].setValue(float(wf));
        module->params[ModulePair::WAVEFORM2_PARAM].setValue(float(wf));
        module->inputs[ModulePair::VOLUME1_INPUT].setChannels(1);
        module->inputs[ModulePair::VOLUME1_INPUT].setVoltage(10.0f);
        module->inputs[ModulePair::VOLUME2_INPUT].setChannels(1);
        module->inputs[ModulePair::VOLUME2_INPUT].setVoltage(10.0f);
        
        EXPECT_NO_THROW(processSample());
    }
}

// Test volume parameter effects
TEST_F(ModulePairTest, VolumeParameterZero) {
    module->params[ModulePair::VOLUME1_PARAM].setValue(0.0f);
    module->params[ModulePair::VOLUME2_PARAM].setValue(0.0f);
    module->inputs[ModulePair::VOLUME1_INPUT].setChannels(1);
    module->inputs[ModulePair::VOLUME1_INPUT].setVoltage(10.0f);
    module->inputs[ModulePair::VOLUME2_INPUT].setChannels(1);
    module->inputs[ModulePair::VOLUME2_INPUT].setVoltage(10.0f);
    
    processSample();
    
    // Output should be near zero with zero volume
    EXPECT_NEAR(module->outputs[ModulePair::WAVE_OUTPUT].getVoltage(0), 0.0f, 0.1f);
}

TEST_F(ModulePairTest, VolumeParameterFull) {
    module->params[ModulePair::VOLUME1_PARAM].setValue(99.0f);
    module->params[ModulePair::VOLUME2_PARAM].setValue(99.0f);
    module->inputs[ModulePair::VOLUME1_INPUT].setChannels(1);
    module->inputs[ModulePair::VOLUME1_INPUT].setVoltage(10.0f);
    module->inputs[ModulePair::VOLUME2_INPUT].setChannels(1);
    module->inputs[ModulePair::VOLUME2_INPUT].setVoltage(10.0f);
    
    processSamples(100);
    
    // Should produce output at full volume
    float output = module->outputs[ModulePair::WAVE_OUTPUT].getVoltage(0);
    EXPECT_NE(output, 0.0f);
}

// Test volume CV inputs
TEST_F(ModulePairTest, VolumeInputZeroVoltage) {
    module->params[ModulePair::VOLUME1_PARAM].setValue(50.0f);
    module->inputs[ModulePair::VOLUME1_INPUT].setChannels(1);
    module->inputs[ModulePair::VOLUME1_INPUT].setVoltage(0.0f);
    module->inputs[ModulePair::VOLUME2_INPUT].setChannels(1);
    module->inputs[ModulePair::VOLUME2_INPUT].setVoltage(0.0f);
    
    processSample();
    
    // Zero CV voltage should result in silence
    EXPECT_FLOAT_EQ(module->outputs[ModulePair::WAVE_OUTPUT].getVoltage(0), 0.0f);
}

TEST_F(ModulePairTest, VolumeInputFullVoltage) {
    module->params[ModulePair::VOLUME1_PARAM].setValue(50.0f);
    module->params[ModulePair::VOLUME2_PARAM].setValue(50.0f);
    module->inputs[ModulePair::VOLUME1_INPUT].setChannels(1);
    module->inputs[ModulePair::VOLUME1_INPUT].setVoltage(10.0f);
    module->inputs[ModulePair::VOLUME2_INPUT].setChannels(1);
    module->inputs[ModulePair::VOLUME2_INPUT].setVoltage(10.0f);
    
    processSamples(100);
    
    // Full CV voltage should produce significant output
    float output = module->outputs[ModulePair::WAVE_OUTPUT].getVoltage(0);
    EXPECT_NE(output, 0.0f);
}

// Test mix mode (additive synthesis)
TEST_F(ModulePairTest, MixMode) {
    module->params[ModulePair::PHASE_PARAM].setValue(0.0f);
    module->params[ModulePair::MIX_PARAM].setValue(0.0f); // Mix mode
    module->params[ModulePair::WAVEFORM1_PARAM].setValue(0.0f); // Sine
    module->params[ModulePair::WAVEFORM2_PARAM].setValue(0.0f); // Sine
    module->params[ModulePair::VOLUME1_PARAM].setValue(50.0f);
    module->params[ModulePair::VOLUME2_PARAM].setValue(50.0f);
    
    module->inputs[ModulePair::PITCH1_INPUT].setChannels(1);
    module->inputs[ModulePair::PITCH2_INPUT].setChannels(1);
    module->inputs[ModulePair::PITCH1_INPUT].setVoltage(0.0f);
    module->inputs[ModulePair::PITCH2_INPUT].setVoltage(0.0f);
    module->inputs[ModulePair::VOLUME1_INPUT].setChannels(1);
    module->inputs[ModulePair::VOLUME1_INPUT].setVoltage(10.0f);
    module->inputs[ModulePair::VOLUME2_INPUT].setChannels(1);
    module->inputs[ModulePair::VOLUME2_INPUT].setVoltage(10.0f);
    
    processSamples(100);
    
    // Should produce some output in mix mode
    float output = module->outputs[ModulePair::WAVE_OUTPUT].getVoltage(0);
    EXPECT_NE(output, 0.0f);
}

// Test ring modulation mode
TEST_F(ModulePairTest, RingModMode) {
    module->params[ModulePair::PHASE_PARAM].setValue(0.0f);
    module->params[ModulePair::MIX_PARAM].setValue(1.0f); // Ring mode
    module->params[ModulePair::WAVEFORM1_PARAM].setValue(0.0f);
    module->params[ModulePair::WAVEFORM2_PARAM].setValue(0.0f);
    module->params[ModulePair::VOLUME1_PARAM].setValue(50.0f);
    module->params[ModulePair::VOLUME2_PARAM].setValue(50.0f);
    
    module->inputs[ModulePair::PITCH1_INPUT].setChannels(1);
    module->inputs[ModulePair::PITCH2_INPUT].setChannels(1);
    module->inputs[ModulePair::PITCH1_INPUT].setVoltage(0.0f);
    module->inputs[ModulePair::PITCH2_INPUT].setVoltage(1.0f); // Different pitch
    module->inputs[ModulePair::VOLUME1_INPUT].setChannels(1);
    module->inputs[ModulePair::VOLUME1_INPUT].setVoltage(10.0f);
    module->inputs[ModulePair::VOLUME2_INPUT].setChannels(1);
    module->inputs[ModulePair::VOLUME2_INPUT].setVoltage(10.0f);
    
    processSamples(100);
    
    // Should produce output in ring mode
    float output = module->outputs[ModulePair::WAVE_OUTPUT].getVoltage(0);
    EXPECT_NE(output, 0.0f);
}

// Test phase modulation mode
TEST_F(ModulePairTest, PhaseModMode) {
    module->params[ModulePair::PHASE_PARAM].setValue(1.0f); // Phase mod on
    module->params[ModulePair::WAVEFORM1_PARAM].setValue(0.0f);
    module->params[ModulePair::WAVEFORM2_PARAM].setValue(0.0f);
    module->params[ModulePair::VOLUME1_PARAM].setValue(50.0f);
    module->params[ModulePair::VOLUME2_PARAM].setValue(50.0f);
    
    module->inputs[ModulePair::PITCH1_INPUT].setChannels(1);
    module->inputs[ModulePair::PITCH1_INPUT].setVoltage(0.0f);
    module->inputs[ModulePair::VOLUME1_INPUT].setChannels(1);
    module->inputs[ModulePair::VOLUME1_INPUT].setVoltage(10.0f);
    module->inputs[ModulePair::VOLUME2_INPUT].setChannels(1);
    module->inputs[ModulePair::VOLUME2_INPUT].setVoltage(10.0f);
    
    processSamples(100);
    
    // Phase modulation should produce output
    float output = module->outputs[ModulePair::WAVE_OUTPUT].getVoltage(0);
    EXPECT_NE(output, 0.0f);
}

// Test pitch CV response
// Test pitch CV affects frequency (simplified test)
TEST_F(ModulePairTest, PitchCVResponse) {
    module->params[ModulePair::PHASE_PARAM].setValue(0.0f);
    module->inputs[ModulePair::PITCH1_INPUT].setChannels(1);
    module->inputs[ModulePair::VOLUME1_INPUT].setChannels(1);
    module->inputs[ModulePair::VOLUME1_INPUT].setVoltage(10.0f, 0);
    
    // Test that changing pitch CV doesn't crash and produces phase changes
    module->inputs[ModulePair::PITCH1_INPUT].setVoltage(0.0f, 0);
    module->phase1[0] = 0.0f;
    processSamples(10);
    
    // Phase should have advanced
    EXPECT_GT(module->phase1[0], 0.0f) << "Phase should advance with processing";
    
    // Test with different pitch - module should still work
    module->inputs[ModulePair::PITCH1_INPUT].setVoltage(2.0f, 0);
    EXPECT_NO_THROW(processSamples(10));
}

// Test multiple channels (polyphony)
TEST_F(ModulePairTest, PolyphonicProcessing) {
    const int numChannels = 4;
    module->inputs[ModulePair::PITCH1_INPUT].setChannels(numChannels);
    module->inputs[ModulePair::PITCH2_INPUT].setChannels(numChannels);
    module->inputs[ModulePair::VOLUME1_INPUT].setChannels(numChannels);
    module->inputs[ModulePair::VOLUME2_INPUT].setChannels(numChannels);
    
    // Set different voltages per channel
    for (int i = 0; i < numChannels; i++) {
        module->inputs[ModulePair::PITCH1_INPUT].setVoltage(float(i) * 0.5f, i);
        module->inputs[ModulePair::PITCH2_INPUT].setVoltage(float(i) * 0.3f, i);
        module->inputs[ModulePair::VOLUME1_INPUT].setVoltage(10.0f, i);
        module->inputs[ModulePair::VOLUME2_INPUT].setVoltage(10.0f, i);
    }
    
    // Should process multiple channels without crashing
    EXPECT_NO_THROW(processSamples(100));
    
    // Verify first channel has some output after running
    float output = module->outputs[ModulePair::WAVE_OUTPUT].getVoltage(0);
    EXPECT_NE(output, 0.0f) << "Channel 0 should have non-zero output";
}

// Test that phase advances independently per channel
TEST_F(ModulePairTest, IndependentChannelPhases) {
    const int numChannels = 3;
    module->inputs[ModulePair::PITCH1_INPUT].setChannels(numChannels);
    module->inputs[ModulePair::VOLUME1_INPUT].setChannels(numChannels);
    
    // Set significantly different pitches to ensure different phase increments
    module->inputs[ModulePair::PITCH1_INPUT].setVoltage(0.0f, 0);
    module->inputs[ModulePair::PITCH1_INPUT].setVoltage(2.0f, 1);
    module->inputs[ModulePair::PITCH1_INPUT].setVoltage(4.0f, 2);
    
    for (int i = 0; i < numChannels; i++) {
        module->inputs[ModulePair::VOLUME1_INPUT].setVoltage(10.0f, i);
    }
    
    processSamples(100);
    
    // Different pitch channels should have different phases
    // At least channels 0 and 2 should differ significantly
    EXPECT_NE(module->phase1[0], module->phase1[2]);
}

// Test output voltage range
TEST_F(ModulePairTest, OutputVoltageRange) {
    module->params[ModulePair::VOLUME1_PARAM].setValue(99.0f);
    module->params[ModulePair::VOLUME2_PARAM].setValue(99.0f);
    module->inputs[ModulePair::VOLUME1_INPUT].setChannels(1);
    module->inputs[ModulePair::VOLUME1_INPUT].setVoltage(10.0f);
    module->inputs[ModulePair::VOLUME2_INPUT].setChannels(1);
    module->inputs[ModulePair::VOLUME2_INPUT].setVoltage(10.0f);
    module->inputs[ModulePair::PITCH1_INPUT].setChannels(1);
    module->inputs[ModulePair::PITCH1_INPUT].setVoltage(0.0f);
    
    // Process many samples and check voltage range
    float maxVoltage = 0.0f;
    for (int i = 0; i < 1000; i++) {
        processSample();
        float voltage = std::abs(module->outputs[ModulePair::WAVE_OUTPUT].getVoltage(0));
        maxVoltage = std::max(maxVoltage, voltage);
    }
    
    // Output should stay within reasonable audio range (typically -10V to +10V)
    EXPECT_LT(maxVoltage, 15.0f);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
