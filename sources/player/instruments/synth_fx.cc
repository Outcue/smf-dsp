#include "synth_fx.h"
#include "audio/eq_5band.h"
#include "audio/reverb.h"

static const Fx_Parameter fx_parameters[] = {
    {"EQ", 0, Fx_Parameter::Boolean},
    {"EQ Low", 50, Fx_Parameter::Integer},
    {"EQ Mid-Low", 50, Fx_Parameter::Integer},
    {"EQ Mid", 50, Fx_Parameter::Integer},
    {"EQ Mid-High", 50, Fx_Parameter::Integer},
    {"EQ High", 50, Fx_Parameter::Integer},
    {"Reverb", 0, Fx_Parameter::Boolean, Fx_Parameter::HasSeparator},
    {"Reverb amount", 50, Fx_Parameter::Integer},
    {"Reverb size", 10, Fx_Parameter::Integer},
};

static_assert(
    sizeof(fx_parameters) / sizeof(fx_parameters[0]) == Synth_Fx::Parameter_Count,
    "The parameter count does not match");

Synth_Fx::Synth_Fx()
    : eq_(new Eq_5band), rev_(new Reverb)
{
    gsl::span<const Fx_Parameter> params(fx_parameters);

    for (size_t p = 0; p < params.size(); ++p)
        set_parameter(p, params[p].default_value);
}

Synth_Fx::~Synth_Fx()
{
}

void Synth_Fx::init(float sample_rate)
{
    eq_->init(sample_rate);
    rev_->init(sample_rate);
}

void Synth_Fx::clear()
{
    eq_->clear();
    rev_->clear();
}

void Synth_Fx::compute(float data[], unsigned nframes)
{
    Eq_5band &eq = *eq_;
    Reverb &rev = *rev_;
    unsigned index = 0;

    while (index < nframes) {
        constexpr unsigned maxframes = 512;

        unsigned cur = nframes - index;
        cur = (cur < maxframes) ? cur : maxframes;

        float ch1[maxframes];
        float ch2[maxframes];

        for (unsigned i = 0; i < cur; ++i) {
            const float *frame = data + 2 * (index + i);
            ch1[i] = frame[0];
            ch2[i] = frame[1];
        }

        if (eq_enable_)
            eq.compute(ch1, ch2, cur);

        if (rev_enable_)
            rev.compute(ch1, ch2, cur);

        for (unsigned i = 0; i < cur; ++i) {
            float *frame = data + 2 * (index + i);
            frame[0] = ch1[i];
            frame[1] = ch2[i];
        }

        index += cur;
    }
}

int Synth_Fx::get_parameter(size_t index) const
{
    const Eq_5band &eq = *eq_;
    const Reverb &rev = *rev_;

    switch (index) {
    case P_Eq_Enable:
        return eq_enable_ ? 100 : 0;
    case P_Eq_Low:
        return static_cast<int>(eq.get_parameter(0));
    case P_Eq_Mid_Low:
        return static_cast<int>(eq.get_parameter(1));
    case P_Eq_Mid:
        return static_cast<int>(eq.get_parameter(2));
    case P_Eq_Mid_High:
        return static_cast<int>(eq.get_parameter(3));
    case P_Eq_High:
        return static_cast<int>(eq.get_parameter(4));
    case P_Reverb_Enable:
        return rev_enable_ ? 100 : 0;
    case P_Reverb_Amount:
        return static_cast<int>(rev.get_parameter(1));
    case P_Reverb_Size:
        return static_cast<int>(rev.get_parameter(0));
    }
    return 0;
}

void Synth_Fx::set_parameter(size_t index, int value)
{
    Eq_5band &eq = *eq_;
    Reverb &rev = *rev_;

    switch (index) {
    case P_Eq_Enable: {
        bool en = value > 0;
        if (eq_enable_ != en) {
            if (!en)
                eq.clear();
            eq_enable_ = en;
        }
        break;
    }
    case P_Eq_Low:
        eq.set_parameter(0, value);
        break;
    case P_Eq_Mid_Low:
        eq.set_parameter(1, value);
        break;
    case P_Eq_Mid:
        eq.set_parameter(2, value);
        break;
    case P_Eq_Mid_High:
        eq.set_parameter(3, value);
        break;
    case P_Eq_High:
        eq.set_parameter(4, value);
        break;
    case P_Reverb_Enable: {
        bool en = value > 0;
        if (rev_enable_ != en) {
            if (!en)
                rev.clear();
            rev_enable_ = en;
        }
        break;
    }
    case P_Reverb_Amount:
        rev.set_parameter(1, value);
        break;
    case P_Reverb_Size:
        rev.set_parameter(0, value);
        break;
    }
}

gsl::span<const Fx_Parameter> Synth_Fx::parameters()
{
    return fx_parameters;
}
