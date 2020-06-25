//          Copyright Jean Pierre Cimalando 2019.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE.md or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include "player/instrument.h"

class Dummy_Instrument : public Midi_Instrument {
public:
    void open_midi_output(gsl::cstring_span id) override {}
    void close_midi_output() override {}

protected:
    void handle_send_message(const uint8_t *data, unsigned len, double ts, uint8_t flags) override;
};
