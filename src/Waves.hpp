#pragma once
#include <valarray>

struct Waves
{
private:
    std::valarray<float> wave;

public:
    Waves(size_t size);

    float get_Value(float phase) const
    {
        if(phase<0) phase = 0;
        size_t s = wave.size();
        size_t idx = phase * s;
        if (idx >= s)
            idx = s - 1;
        return wave[idx];
    }
};