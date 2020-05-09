//          Copyright Jean Pierre Cimalando 2019.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE.md or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include "keystate.h"
#include "adev/adev.h"
#include <ring_buffer.h>
#include <gsl.hpp>
#include <vector>
#include <memory>
#include <atomic>
#include <cstdint>
class Synth_Host;
class RtMidiOut;

struct Midi_Output {
    std::string id;
    std::string name;
};

enum Midi_Message_Flag {
    Midi_Message_Is_First = 1,
};

///
class Midi_Instrument {
public:
    Midi_Instrument();
    virtual ~Midi_Instrument() {}
    void send_message(const uint8_t *data, unsigned len, double ts, uint8_t flags);
    void initialize();
    void all_sound_off();

    const Keyboard_State &keyboard_state() const noexcept { return kbs_; }

    virtual void flush_events() {}

    virtual void open_midi_output(gsl::cstring_span id) = 0;
    virtual void close_midi_output() = 0;

protected:
    virtual void handle_send_message(const uint8_t *data, unsigned len, double ts, uint8_t flags) = 0;

private:
    Keyboard_State kbs_;
};

///
class Dummy_Instrument : public Midi_Instrument {
public:
    void open_midi_output(gsl::cstring_span id) override {}
    void close_midi_output() override {}

protected:
    void handle_send_message(const uint8_t *data, unsigned len, double ts, uint8_t flags) override;
};

///
class Midi_Port_Instrument : public Midi_Instrument {
public:
    Midi_Port_Instrument();
    ~Midi_Port_Instrument();

    static std::vector<Midi_Output> get_midi_outputs();
    void open_midi_output(gsl::cstring_span id) override;
    void close_midi_output() override;

protected:
    void handle_send_message(const uint8_t *data, unsigned len, double ts, uint8_t flags) override;

private:
    void handle_midi_error(int type, const std::string &text);

private:
    std::unique_ptr<RtMidiOut> out_;
    int midi_error_status_ = 0;
};

///
class Midi_Synth_Instrument : public Midi_Instrument {
public:
    Midi_Synth_Instrument();
    ~Midi_Synth_Instrument();

    void flush_events() override;

    void open_midi_output(gsl::cstring_span id) override;
    void close_midi_output() override;

protected:
    void handle_send_message(const uint8_t *data, unsigned len, double ts, uint8_t flags) override;

private:
    static void audio_callback(float *output, unsigned nframes, void *user_data);
    void process_midi(double time_incr);

    bool extract_next_message();

private:
    std::unique_ptr<Synth_Host> host_;
    std::unique_ptr<Ring_Buffer> midibuf_;
    std::unique_ptr<Audio_Device> audio_;
    double audio_rate_ = 0;
    double audio_latency_ = 0;
    double time_delta_ = 0;

    struct Message_Header {
        unsigned len;
        double timestamp;
        uint8_t flags;
    };

private:
    static constexpr unsigned midi_buffer_size = 8192;
    static constexpr unsigned midi_message_max = 256;
    static constexpr unsigned midi_interval_max = 64;

private:
    bool have_next_message_ = false;
    Message_Header next_header_;
    uint8_t next_message_[midi_message_max];
    std::atomic_uint message_count_ = {0};
};
