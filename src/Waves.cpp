#include "Waves.hpp"

#include <cmath>

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
        float phase = 2 * M_PI * i / size;
        wave[0][i] = sin(phase);
        wave[1][i] = sin(phase) / (phase + 1) / (2 * M_PI + 1 - phase) / .34 * 5;
        wave[2][i] = sin(phase) / (phase + .5) / (2 * M_PI + .5 - phase) / .48 * 5;
        wave[3][i] = sin(phase) / (phase + .1) / (2 * M_PI + .1 - phase) / .71 * 5;
        wave[4][i] = sin(phase) / (phase + .05) / (2 * M_PI + .05 - phase) /.76 * 5;
        wave[5][i] = sin(phase) / (phase + .01) / (2 * M_PI + .01 - phase) / .8 * 5;
    }
}