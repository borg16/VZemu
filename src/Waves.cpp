#include "Waves.hpp"

#include <math.h>

Waves::Waves(size_t size)
    : wave(size)
{
    for (size_t i = 0; i < size; ++i)
    {
        wave[i] = sin(2 * M_PI * i / size);
    }
}