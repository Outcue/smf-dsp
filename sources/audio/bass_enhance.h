#pragma once
#include <array>
#include <memory>
class BassEnhance_dsp;

class BassEnhance {
public:
    BassEnhance();
    ~BassEnhance();

    void init(float sample_rate);
    void clear();
    void compute(float *ch1, float *ch2, unsigned nframes);

    float get_parameter(size_t index) const;
    void set_parameter(size_t index, float value);

private:
    std::unique_ptr<BassEnhance_dsp> dsp_;
    std::array<float *, 2> parameter_ {};
};
