#include "Waves.hpp"

#include <cmath>

float saw(float phase, float p, float u, float v)
{
    return phase <= p
               ? u / p * phase
           : phase <= v
               ? (1 - u / p * (1 - v) - u) / (v - p) * (phase - p) + u
               : u / p * (phase - 1) + 1;
}

Waves::Waves(size_t size)
    : wave{std::valarray<float>(size),
           std::valarray<float>(size),
           std::valarray<float>(size),
           std::valarray<float>(size),
           std::valarray<float>(size),
           std::valarray<float>(size)}
{
    for (size_t i = 0; i < size; ++i)
    {
        float phase = float(i) / size;

        wave[0][i] = sin(2 * M_PI * phase);
        wave[1][i] = sin(2 * M_PI * saw(phase, 0.201063, 0.298472, 0.861249));
        wave[2][i] = sin(2 * M_PI * saw(phase, 0.122448, 0.299202, 0.915584));
        wave[3][i] = sin(2 * M_PI * saw(phase, 0.0692947, 0.300676, 0.952308));
        wave[4][i] = sin(2 * M_PI * saw(phase, 0.0371464, 0.300532, 0.974066));
        wave[5][i] = sin(2 * M_PI * saw(phase, 0.0199944, 0.303999, 0.986284));
    }
}