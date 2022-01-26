#include "VZemu.hpp"
#include "Waves.hpp"

#include <cmath>

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

    float phase1[maxPolyphony] = {0};
    float phase2[maxPolyphony] = {0};

    ModulePair()
    {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        configSwitch(WAVEFORM1_PARAM, 0, 7, 0, "Waveform odd", {"Sine", "Saw1", "Saw2", "Saw3", "Saw4", "Saw5", "Noise1", "Noise2"});
        paramQuantities[WAVEFORM1_PARAM]->snapEnabled = true;
        configSwitch(WAVEFORM2_PARAM, 0, 7, 0, "Waveform even", {"Sine", "Saw1", "Saw2", "Saw3", "Saw4", "Saw5", "Noise1", "Noise2"});
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
        bool ringLight = vcoMode && params[MIX_PARAM].getValue() > 0.f;
        lights[RING_LIGHT].setBrightness(ringLight);

        int currentPolyphony = std::max(1, std::max(inputs[PITCH1_INPUT].getChannels(), inputs[PITCH2_INPUT].getChannels()));
        outputs[WAVE_OUTPUT].setChannels(currentPolyphony);

        float ampParameter1 = params[VOLUME1_PARAM].getValue();
        float ampParameter2 = params[VOLUME2_PARAM].getValue();

        int waveform1 = params[WAVEFORM1_PARAM].getValue();
        int waveform2 = params[WAVEFORM2_PARAM].getValue();

        bool phaseMod = params[PHASE_PARAM].getValue() != 0;
        bool ring = params[MIX_PARAM].getValue() != 0;

        for (int i = 0; i < currentPolyphony; ++i)
        {
            float cv1 = inputs[PITCH1_INPUT].getChannels() > i ? inputs[PITCH1_INPUT].getVoltage(i) : 0;

            float amp1 = ampParameter1 / 100.f * inputs[VOLUME1_INPUT].getVoltage(i) / 10.f;
            float amp2 = ampParameter2 / 100.f * inputs[VOLUME2_INPUT].getVoltage(i) / 10.f;

            phase1[i] += args.sampleTime * std::pow(2.f, cv1 + float(std::log2(261.626)));
            if (phase1[i] > 1.f)
                phase1[i] -= 1.f;
            float w1 = waves.get_Value(waveform1, phase1[i]);
            float value;
            if (phaseMod)
            {
                float wave1 = 1.f * w1 * (amp1 + .5f / 2 / M_PI);
                float phase = wave1 - std::floor(wave1);
                float w2 = waves.get_Value(waveform2, phase);
                value = w2 * amp2 * 5.f;
            }
            else
            {
                float cv2 = inputs[PITCH2_INPUT].getVoltage(i);

                phase2[i] += args.sampleTime * std::pow(2.f, cv2 + float(std::log2(261.626)));
                if (phase2[i] > 1.f)
                    phase2[i] -= 1.f;

                float w2 = waves.get_Value(waveform2, phase2[i]);
                value = ring ? w1 * amp1 * (1 + w2 * amp2 ) *5.f
                             : (w1 * amp1 + w2 * amp2) *5.f;
            }
            outputs[WAVE_OUTPUT].setVoltage(value, i);
        }
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

        Knob *wave1 = createParamCentered<Trimpot>(Vec(20, 63), module, ModulePair::WAVEFORM1_PARAM);
        wave1->minAngle = -.7 * M_PI;
        wave1->maxAngle = .7 * M_PI;
        addParam(wave1);

        addParam(createParamCentered<RoundSmallBlackKnob>(Vec(56, 74), module, ModulePair::VOLUME1_PARAM));

        Knob *wave2 = createParamCentered<Trimpot>(Vec(20, 172), module, ModulePair::WAVEFORM2_PARAM);
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