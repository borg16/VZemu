#pragma once
#include <valarray>

struct Waves
{
private:
    std::valarray<float> wave[6];

public:
    Waves(size_t size);

    float get_Value(size_t wave_idx, float phase) const
    {
        if (wave_idx > 5)
            wave_idx = 5;
        if (phase < 0)
            phase = 0;
        size_t s = wave[wave_idx].size();
        size_t idx = phase * s;
        if (idx >= s)
            idx = s - 1;
        return wave[wave_idx][idx];
    }
};