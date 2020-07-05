#include "analyzer_10band.h"
#include <cmath>

void analyzer_10band::init(float sample_rate)
{
    f_lo_band_.init(sample_rate);
    f_lo_smooth_.init(sample_rate);
#if defined(__SSE__)
    for (int i = 0; i < (N - 2) / 4; ++i) {
        f_mid_band_sse_[i].lp.init(sample_rate);
        f_mid_band_sse_[i].hp.init(sample_rate);
        f_mid_smooth_sse_[i].init(sample_rate);
    }
#else
    for (int i = 0; i < N - 2; ++i) {
        f_mid_band_[i].lp.init(sample_rate);
        f_mid_band_[i].hp.init(sample_rate);
        f_mid_smooth_[i].init(sample_rate);
    }
#endif
    f_hi_band_.init(sample_rate);
    f_hi_smooth_.init(sample_rate);
    clear();
}

void analyzer_10band::setup(float M, float ftop, float t60)
{
    float cutoffs[N];
    for (int i = 0; i < N; ++i)
        cutoffs[i] = ftop * std::exp2(float(i - N + 1) / M);

    f_lo_band_.cutoff(cutoffs[0]);
    f_lo_smooth_.tau(t60);
#if defined(__SSE__)
    for (int i = 0; i < (N - 2) / 4; ++i) {
        __m128 fc = _mm_loadu_ps(&cutoffs[1 + 4 * i]);
        f_mid_band_sse_[i].lp.cutoff(fc);
        f_mid_band_sse_[i].hp.cutoff(fc);
        f_mid_smooth_sse_[i].tau(_mm_set1_ps(t60));
    }
#else
    for (int i = 0; i < N - 2; ++i) {
        float fc = cutoffs[i + 1];
        f_mid_band_[i].lp.cutoff(fc);
        f_mid_band_[i].hp.cutoff(fc);
        f_mid_smooth_[i].tau(t60);
    }
#endif
    f_hi_band_.cutoff(cutoffs[N - 1]);
    f_hi_smooth_.tau(t60);
}

void analyzer_10band::clear()
{
    f_lo_band_.clear();
    f_lo_smooth_.clear();
#if defined(__SSE__)
    for (int i = 0; i < (N - 2) / 4; ++i) {
        f_mid_band_sse_[i].lp.clear();
        f_mid_band_sse_[i].hp.clear();
        f_mid_smooth_sse_[i].clear();
    }
#else
    for (int i = 0; i < N - 2; ++i) {
        f_mid_band_[i].lp.clear();
        f_mid_band_[i].hp.clear();
        f_mid_smooth_[i].clear();
    }
#endif
    f_hi_band_.clear();
    f_hi_smooth_.clear();
    for (int i = 0; i < N; ++i)
        outputs_[i] = 0.0f;
}

float *analyzer_10band::compute(const float inputs[], size_t count)
{
    float *outputs = outputs_;
    if (count == 0)
        return outputs;

    float lo;
    float hi;
#if defined(__SSE__)
    __m128 mid[(N - 2) / 4];
#else
    float mid[N - 2];
#endif

    for (size_t frame = 0; frame < count; ++frame) {
        float input = inputs[frame];
        lo = f_lo_smooth_.compute(std::abs(f_lo_band_.compute(input)));
#if defined(__SSE__)
        for (int i = 0; i < (N - 2) / 4; ++i)
            mid[i] = f_mid_smooth_sse_[i].compute(_mm_andnot_ps(_mm_set1_ps(-0.0f),
                f_mid_band_sse_[i].hp.compute(
                f_mid_band_sse_[i].lp.compute(_mm_set1_ps(input)))));
#else
        for (int i = 0; i < N - 2; ++i)
            mid[i] = f_mid_smooth_[i].compute(std::abs(
                f_mid_band_[i].hp.compute(
                f_mid_band_[i].lp.compute(input))));
#endif
        hi = f_hi_smooth_.compute(std::abs(f_hi_band_.compute(input)));
    }

    outputs[0] = lo;
#if defined(__SSE__)
    for (int i = 0; i < (N - 2) / 4; ++i)
        _mm_storeu_ps(&outputs[1 + 4 * i], mid[i]);
#else
    for (int i = 0; i < N - 2; ++i)
        outputs[1 + i] = mid[i];
#endif
    outputs[N - 1] = hi;

    return outputs;
}

float *analyzer_10band::compute_stereo(const float inputs[], size_t count)
{
    size_t index = 0;
    constexpr size_t bufsize = 512;

    while (index < count) {
        size_t cur = count - index;
        cur = (cur < bufsize) ? cur : bufsize;

        float mixdown[bufsize];
        for (size_t i = 0; i < cur; ++i) {
            const float *frame = &inputs[2 * (index + i)];
            mixdown[i] = float(M_SQRT1_2) * (frame[0] + frame[1]);
        }

        compute(mixdown, cur);
        index += cur;
    }

    return outputs_;
}
