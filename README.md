# VZemu
[VCV Rack](https://vcvrack.com/) synthesizer modules emulating aspects of the Casio VZ-1 synthesizer

**Draft**: Work in progress - basic modules implemented.

## Overview
VZemu brings key features of the Casio VZ-1, an operator-based digital synthesizer from the 1980s, to VCV Rack. The VZ-1 architecture uses up to 8 operators per voice, organized in pairs. Each operator pair can work in two modes:
- **Parallel mode**: Both operators sound independently
- **Phase modulation mode**: One operator modulates the phase of the other

This plugin recreates these capabilities in a modular environment, allowing you to build VZ-1-inspired patches using VCV Rack's flexible routing.

## Modules

### Module Pair
Emulates a VZ-1 operator pair with two VCOs that can be connected in different configurations:
- **Independent operation**: Two separate oscillators running in parallel
- **Phase modulation**: Operator 1 modulates the phase of Operator 2, creating complex harmonic timbres
- **Wave mapping mode**: Operator 2 can be disabled to serve as a simple wave mapper for Operator 1

This recreates the fundamental building block of VZ-1 sound synthesis.

### Envelope
8-step envelope generator emulating the VZ-1's envelope system:
- Controls both amplitude (DCA) and pitch (DCO)
- Flexible multi-stage envelope design
- Authentic VZ-1 envelope behavior

## Disclaimer
VZemu is an independent, open-source project and is not affiliated with, endorsed by, or sponsored by Casio Computer Co., Ltd. The Casio VZ-1 and all related trademarks are property of their respective owners. This project aims to recreate the synthesis techniques and workflow of the VZ-1 synthesizer for educational and creative purposes within the VCV Rack environment.