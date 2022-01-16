#include "VZemu.hpp"
#include "Waves.hpp"

using namespace rack;

// VCV has a limit of 16 channels in a polyphonic cable.
static const int maxPolyphony = engine::PORT_MAX_CHANNELS;

static const Waves waves(1000);

/**
 *  Every synth module must have a Module structure.
 *  This is where all the real-time processing code goes.
 */
struct ModulePair : Module
{
    enum ParamIds
    {
        WAVEFORM1_PARAM,
        VOLUME1_PARAM,
        WAVEFORM2_PARAM,
        VOLUME2_PARAM,
        MIX_PARAM,
        PHASE_PARAM,
        NUM_PARAMS
    };
    enum InputIds
    {
        PITCH1_INPUT,
        VOLUME1_INPUT,
        PITCH2_INPUT,
        VOLUME2_INPUT,
        EXT_PHASE_INPUT,
        NUM_INPUTS
    };
    enum OutputIds
    {
        WAVE_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds
    {
        INTERNAL_PHASE_LIGHT,
        RING_LIGHT,
        VCO_MODE_LIGHT,
        NUM_LIGHTS
    };

    float phase = 0;

    ModulePair()
    {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        configSwitch(WAVEFORM1_PARAM, 1, 8, 1, "Waveform odd", {"Sine", "Saw1", "Saw2", "Saw3", "Saw4", "Saw5", "Noise1", "Noise2"});
        paramQuantities[WAVEFORM1_PARAM]->snapEnabled = true;
        configSwitch(WAVEFORM2_PARAM, 1, 8, 1, "Waveform even", {"Sine", "Saw1", "Saw2", "Saw3", "Saw4", "Saw5", "Noise1", "Noise2"});
        paramQuantities[WAVEFORM2_PARAM]->snapEnabled = true;
        configSwitch(MIX_PARAM, 0, 1, 0, "Mode", {"Mix", "Ring"});
        paramQuantities[MIX_PARAM]->snapEnabled = true;
        configSwitch(PHASE_PARAM, 0, 1, 0, "Off", {"Off", "Phase"});
        configParam(VOLUME1_PARAM, 0, 99, 50);
        paramQuantities[VOLUME1_PARAM]->snapEnabled = true;
        configParam(VOLUME2_PARAM, 0, 99, 50);
        paramQuantities[VOLUME2_PARAM]->snapEnabled = true;

        configInput(PITCH1_INPUT);
        configInput(VOLUME1_INPUT);
        configInput(PITCH2_INPUT);
        configInput(VOLUME2_INPUT);
        configInput(EXT_PHASE_INPUT);

        configOutput(WAVE_OUTPUT);

        configLight(VCO_MODE_LIGHT);
        configLight(RING_LIGHT);
        configLight(INTERNAL_PHASE_LIGHT);
    }

    void process(const ProcessArgs &args) override
    {
        bool intPhase = params[PHASE_PARAM].getValue() > 0.f;
        lights[INTERNAL_PHASE_LIGHT].setBrightness(intPhase);
        bool vcoMode = !intPhase && !inputs[EXT_PHASE_INPUT].isConnected();
        lights[VCO_MODE_LIGHT].setBrightness(vcoMode);
        bool ring = vcoMode && params[MIX_PARAM].getValue() > 0.f;
        lights[RING_LIGHT].setBrightness(ring);

        float cv = inputs[PITCH1_INPUT].getVoltage();

        float pitch = cv + float(std::log2(261.626));
        float freq = std::pow(2.f, pitch);

        const float normalizedFreq = args.sampleTime * freq;
        // phaseAdvance[i] = normalizedFreq;

        phase += normalizedFreq;
        if(phase > 1.f) phase -= 1.f;

        outputs[WAVE_OUTPUT].setChannels(1);
        float value = waves.get_Value(phase);
        outputs[WAVE_OUTPUT].setVoltage(value * 5);
    }
};

struct ModulePairWidget : ModuleWidget
{
    ModulePairWidget(ModulePair *module)
    {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ModulePair_panel.svg")));

        addChild(createWidget<ScrewSilver>(Vec(15, 0)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 30, 0)));
        addChild(createWidget<ScrewSilver>(Vec(15, 365)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 30, 365)));

        Knob *wave1 = createParamCentered<Trimpot>(Vec(20,63), module, ModulePair::WAVEFORM1_PARAM);
        wave1->minAngle = -.7 * M_PI;
        wave1->maxAngle = .7 * M_PI;
        addParam(wave1);

        addParam(createParamCentered<RoundSmallBlackKnob>(Vec(56, 74), module, ModulePair::VOLUME1_PARAM));

        Knob *wave2 = createParamCentered<Trimpot>(Vec(20,172), module, ModulePair::WAVEFORM2_PARAM);
        wave2->minAngle = -.7 * M_PI;
        wave2->maxAngle = .7 * M_PI;
        addParam(wave2);

        addParam(createParamCentered<RoundSmallBlackKnob>(Vec(56, 186), module, ModulePair::VOLUME2_PARAM));

        auto mix = createLightParamCentered<VCVLightButton<MediumSimpleLight<RedLight>>>(Vec(19, 254), module, ModulePair::MIX_PARAM, ModulePair::RING_LIGHT);
        mix->momentary = false;
        addChild(mix);
        auto phase = createLightParamCentered<VCVLightButton<MediumSimpleLight<RedLight>>>(Vec(56, 294), module, ModulePair::PHASE_PARAM, ModulePair::INTERNAL_PHASE_LIGHT);
        phase->momentary = false;
        addParam(phase);

        addInput(createInputCentered<PJ301MPort>(Vec(21, 107), module, ModulePair::PITCH1_INPUT));
        addInput(createInputCentered<PJ301MPort>(Vec(56, 107), module, ModulePair::VOLUME1_INPUT));
        addInput(createInputCentered<PJ301MPort>(Vec(19, 222), module, ModulePair::PITCH2_INPUT));
        addInput(createInputCentered<PJ301MPort>(Vec(56, 222), module, ModulePair::VOLUME2_INPUT));
        addInput(createInputCentered<PJ301MPort>(Vec(18, 293), module, ModulePair::EXT_PHASE_INPUT));

        addOutput(createOutputCentered<PJ301MPort>(Vec(36, 334), module, ModulePair::WAVE_OUTPUT));

        addChild(createLightCentered<TinySimpleLight<GreenLight>>(Vec(13, 199), module, ModulePair::VCO_MODE_LIGHT));
    }
};

Model *modelModulePair = createModel<ModulePair, ModulePairWidget>("VZemu-ModulePair");