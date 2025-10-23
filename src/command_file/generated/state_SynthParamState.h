#pragma once

#include "../../synth/synth.h"

typedef struct {
    Synth *s;
    SynthParams currentParams;
    
    float delay_send, delay_time, delay_feed;
    int instrument_count;
    float gain_adjust;
} SynthParamState;
