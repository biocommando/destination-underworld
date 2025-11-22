#include "synth.h"
#include "wt_sample_loader.h"
#include "synth_random.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "../command_file/generated/dispatch_synth_params.h"

void init_SynthVoice(SynthVoice *sv, float sample_rate, int key, int channel)
{
    sv->osc1_type = 0;
    sv->osc2_type = 0;
    sv->osc1_mix = 1;
    sv->osc2_mix = 1;
    sv->fm_on = 0;

    sv->volume = 1;
    sv->delay_send = 0;
    sv->allow_note_stealing = 1;

    sv->sample_rate = sample_rate;
    init_MicrotrackerMoog(&sv->filter, sample_rate);
    init_BasicOscillator(&sv->osc1, sample_rate);
    init_BasicOscillator(&sv->osc2, sample_rate);
    sv->key = key;
    sv->channel = channel;

    struct wt_sample *wts = get_wt_sample(0);
    BasicOscillator_setWavetable(&sv->osc1, wts->data, wts->size);
    BasicOscillator_setWaveTableParams(&sv->osc1, 0, 1);
    BasicOscillator_setWavetable(&sv->osc2, wts->data, wts->size);
    BasicOscillator_setWaveTableParams(&sv->osc2, 0, 1);

    init_AdsrEnvelope(&sv->amp_envelope);
    init_AdsrEnvelope(&sv->filter_envelope);

    AdsrEnvelope_trigger(&sv->amp_envelope);
    AdsrEnvelope_trigger(&sv->filter_envelope);
}

void SynthVoice_process(SynthVoice *sv, float *delay_sample, float *left, float *right)
{
    float v = 0;

    if (sv->fm_on)
    {
        BasicOscillator_calculateNext(&sv->osc1);
        BasicOscillator_calculateNext(&sv->osc2);
        const float o2_value = BasicOscillator_getValue(&sv->osc2, (enum OscType)sv->osc2_type) * sv->osc2_mix;
        v = BasicOscillator_getValueFm(&sv->osc1, (enum OscType)sv->osc1_type, o2_value) * sv->osc1_mix;
    }
    else
    {
        if (sv->osc1_mix > 0)
        {
            BasicOscillator_calculateNext(&sv->osc1);
            v += BasicOscillator_getValue(&sv->osc1, (enum OscType)sv->osc1_type) * sv->osc1_mix;
        }
        if (sv->osc2_mix > 0)
        {
            BasicOscillator_calculateNext(&sv->osc2);
            v += BasicOscillator_getValue(&sv->osc2, (enum OscType)sv->osc2_type) * sv->osc2_mix;
        }
    }
    AdsrEnvelope_calculateNext(&sv->filter_envelope);
    float e = AdsrEnvelope_getEnvelope(&sv->filter_envelope);
    if (sv->env_to_pitch)
    {
        BasicOscillator_setFrequency(&sv->osc1, sv->osc1_base_freq * e);
        BasicOscillator_setFrequency(&sv->osc2, sv->osc2_base_freq * e);
    }
    else
    {
        MicrotrackerMoog_setModulation(&sv->filter, e * sv->filter_mod_amount);
    }

    if (sv->noise_amount > 0)
    {
        v += sv->noise_amount * (1 - synth_random() / 2147483648.0f);
    }

    if (sv->distortion > 0)
    {
        v = v * (1 + sv->distortion);
        v = v > 1 ? 1 : (v < -1 ? -1 : v);
    }

    v = MicrotrackerMoog_calculate(&sv->filter, v);

    AdsrEnvelope_calculateNext(&sv->amp_envelope);
    v *= AdsrEnvelope_getEnvelope(&sv->amp_envelope);
    v *= sv->volume;
    *left += v * sv->left_volume;
    *right += v * sv->right_volume;
    *delay_sample += v * sv->delay_send;
}

void SynthVoice_release(SynthVoice *sv)
{
    AdsrEnvelope_release(&sv->amp_envelope);
    AdsrEnvelope_release(&sv->filter_envelope);
}

static inline float note_to_hz(float note)
{
    return pow(2, note / 12) * 16.352;
}

void SynthVoice_set_params(SynthVoice *sv, const SynthParams *params)
{
    int modified_key = sv->key + params->note_offset;
    if (modified_key < 0)
        modified_key = 0;
    if (modified_key > 127)
        modified_key = 127;
    sv->osc1_base_freq = note_to_hz(modified_key + params->osc1_semitones);
    BasicOscillator_setFrequency(&sv->osc1, sv->osc1_base_freq);
    sv->osc1_type = params->osc1_type;
    sv->osc1_mix = params->osc1_mix;
    sv->osc2_base_freq = note_to_hz(modified_key + params->osc2_semitones);
    BasicOscillator_setFrequency(&sv->osc2, sv->osc2_base_freq);
    if (params->randomize_phase)
    {
        BasicOscillator_randomizePhase(&sv->osc1, 1);
        BasicOscillator_randomizePhase(&sv->osc2, 1);
    }
    sv->osc2_type = params->osc2_type;
    sv->osc2_mix = params->osc2_mix;

    sv->fm_on = params->osc2_to_osc1_fm;

    AdsrEnvelope_setAttack(&sv->amp_envelope, sv->sample_rate * 4 * params->amp_attack);
    AdsrEnvelope_setDecay(&sv->amp_envelope, sv->sample_rate * 4 * params->amp_decay);
    AdsrEnvelope_setSustain(&sv->amp_envelope, params->amp_sustain);
    AdsrEnvelope_setRelease(&sv->amp_envelope, sv->sample_rate * 4 * params->amp_release);

    AdsrEnvelope_setAttack(&sv->filter_envelope, sv->sample_rate * 4 * params->filter_attack);
    AdsrEnvelope_setDecay(&sv->filter_envelope, sv->sample_rate * 4 * params->filter_decay);
    AdsrEnvelope_setSustain(&sv->filter_envelope, params->filter_sustain);
    AdsrEnvelope_setRelease(&sv->filter_envelope, sv->sample_rate * 4 * params->filter_release);

    MicrotrackerMoog_setCutoff(&sv->filter, params->filter_cutoff);
    MicrotrackerMoog_setResonance(&sv->filter, params->filter_resonance);
    sv->filter_mod_amount = params->filter_mod_amount;

    sv->distortion = params->distortion * 100;
    sv->env_to_pitch = params->env_to_pitch != 0;
    sv->noise_amount = params->noise_amount;

    sv->left_volume = params->pan < 0.5 ? 1 : 2 - 2 * params->pan;
    sv->right_volume = params->pan > 0.5 ? 1 : 2 * params->pan;

    sv->volume *= params->volume;

    if (params->wt1_slot != 0)
    {
        struct wt_sample *wts = get_wt_sample(params->wt1_slot);
        BasicOscillator_setWavetable(&sv->osc1, wts->data, wts->size);
        BasicOscillator_setWaveTableParams(&sv->osc1, 0, 1);
    }
    if (params->wt2_slot != 0)
    {
        struct wt_sample *wts = get_wt_sample(params->wt2_slot);
        BasicOscillator_setWavetable(&sv->osc2, wts->data, wts->size);
        BasicOscillator_setWaveTableParams(&sv->osc2, 0, 1);
    }
    if (params->wt_oneshot)
    {
        sv->osc1.wt_oneshot = 1;
        sv->osc2.wt_oneshot = 1;
    }
}

int SynthVoice_ended(SynthVoice *sv)
{
    return AdsrEnvelope_ended(&sv->amp_envelope);
}

void init_Synth(Synth *s, float sample_rate)
{
    s->sample_rate = sample_rate;
    init_BasicDelay(&s->send_delay, sample_rate, sample_rate);

    memset(s->instruments, 0, sizeof(s->instruments));
    memset(s->send_delay_amounts, 0, sizeof(s->send_delay_amounts));
    s->voices = linked_list_create();

    s->total_volume = 1;

    s->diagnostics_max_n_voices = 0;
}

void free_Synth(Synth *s)
{
    free_BasicDelay(&s->send_delay);
}

void Synth_handle_midi_event(Synth *s, unsigned char *event_data, unsigned flags)
{
    if ((event_data[0] & 0xF0) == 0b10000000)
    {
        int channel = event_data[0] & 0xF;

        SynthVoice *v;
        LINKED_LIST_FOR_EACH(&s->voices, SynthVoice, v, 0)
        {
            if (v->channel == channel && v->key == event_data[1])
                SynthVoice_release(v);
        }
    }
    else if ((event_data[0] & 0xF0) == 0b10010000)
    {
        int channel = event_data[0] & 0xF;

        SynthVoice *new_voice = LINKED_LIST_ADD(&s->voices, SynthVoice);
        if (s->voices.count > (size_t)s->diagnostics_max_n_voices)
            s->diagnostics_max_n_voices = s->voices.count;
        init_SynthVoice(new_voice, s->sample_rate, event_data[1], channel);
        new_voice->volume = event_data[2] / 127.0f;
        new_voice->delay_send = s->send_delay_amounts[channel];
        SynthVoice_set_params(new_voice, &s->instruments[channel]);
        // Note: note stealing not implemented
        if (flags & 1)
            new_voice->allow_note_stealing = 0;

        new_voice->active = 1;
    }
}

void Synth_add_instrument(Synth *s, int channel, SynthParams params, float send_delay_amount)
{
    if (channel < 0 || channel >= 16)
        return;
    s->instruments[channel] = params;
    s->send_delay_amounts[channel] = send_delay_amount;
}

void Synth_set_send_delay_params(Synth *s, float feedback, float delay_ms)
{
    BasicDelay_setFeedback(&s->send_delay, feedback);
    BasicDelay_setTime(&s->send_delay, delay_ms);
}

static inline float soft_clip(const float f)
{
    const float f2 = f * f;
    return f * (27 + f2) / (27 + 9 * f2);
}

void Synth_process(Synth *s, float *buffer_left, [[maybe_unused]] float *buffer_right, int buffer_size)
{
    for (int i = 0; i < buffer_size; i++)
    {
        float sample_left = 0;
        float sample_right = 0;
        float delay_send_sample = 0;

        SynthVoice *voice;
        LINKED_LIST_FOR_EACH(&s->voices, SynthVoice, voice, SynthVoice_ended(voice))
        {
            SynthVoice_process(voice, &delay_send_sample, &sample_left, &sample_right);
        }
        const double delay_output = BasicDelay_process(&s->send_delay, delay_send_sample);
        buffer_left[i] = soft_clip(sample_left + delay_output) * s->total_volume;
        i++;
        buffer_left[i] = soft_clip(sample_right + delay_output) * s->total_volume;
    }
}

void Synth_kill_all_voices(Synth *s)
{
    linked_list_clear(&s->voices);
    // Let's also clear the delay buffer
    memset(s->send_delay.buffer, 0, sizeof(double) * s->send_delay.bufferLength);
}

void Synth_kill_voices(Synth *s, int channel)
{
    SynthVoice *voice;
    LINKED_LIST_FOR_EACH(&s->voices, SynthVoice, voice, voice->channel == channel);
}

void dispatch__handle_synth_params_default(struct synth_params_default_DispatchDto *dto)
{

    if (dto->state->instrument_count >= 16)
        return;
    char name[256];
    float value;
    if (sscanf(dto->command, "%[^=]=%f", name, &value) != 2)
        return;

    if (!strcmp(name, "o1type"))
        dto->state->currentParams.osc1_type = (int)(5 * value * 0.99);
    if (!strcmp(name, "o2type"))
        dto->state->currentParams.osc2_type = (int)(5 * value * 0.99);

    if (!strcmp(name, "o1tune"))
        dto->state->currentParams.osc1_semitones = -24 + 48 * value;
    if (!strcmp(name, "o2tune"))
        dto->state->currentParams.osc2_semitones = -24 + 48 * value;

    if (!strcmp(name, "o1mix"))
        dto->state->currentParams.osc1_mix = value;
    if (!strcmp(name, "o2mix"))
        dto->state->currentParams.osc2_mix = value;

    if (!strcmp(name, "o2o1_fm"))
        dto->state->currentParams.osc2_to_osc1_fm = value > 0.5 ? 1 : 0;

    if (!strcmp(name, "o1_wt"))
        dto->state->currentParams.wt1_slot = (int)(MAX_WT_SAMPLE_SLOTS * value * 0.99);
    if (!strcmp(name, "o2_wt"))
        dto->state->currentParams.wt2_slot = (int)(MAX_WT_SAMPLE_SLOTS * value * 0.99);

    if (!strcmp(name, "v_a"))
        dto->state->currentParams.amp_attack = value;
    if (!strcmp(name, "v_d"))
        dto->state->currentParams.amp_decay = value;
    if (!strcmp(name, "v_s"))
        dto->state->currentParams.amp_sustain = value;
    if (!strcmp(name, "v_r"))
        dto->state->currentParams.amp_release = value;

    if (!strcmp(name, "f_a"))
        dto->state->currentParams.filter_attack = value;
    if (!strcmp(name, "f_d"))
        dto->state->currentParams.filter_decay = value;
    if (!strcmp(name, "f_s"))
        dto->state->currentParams.filter_sustain = value;
    if (!strcmp(name, "f_r"))
        dto->state->currentParams.filter_release = value;

    if (!strcmp(name, "cut"))
        dto->state->currentParams.filter_cutoff = value;
    if (!strcmp(name, "env2f"))
        dto->state->currentParams.filter_mod_amount = value;
    if (!strcmp(name, "res"))
        dto->state->currentParams.filter_resonance = value;

    if (!strcmp(name, "phrand"))
        dto->state->currentParams.randomize_phase = value > 0.5 ? 1 : 0;

    if (!strcmp(name, "dist"))
        dto->state->currentParams.distortion = value;

    if (!strcmp(name, "env2p"))
        dto->state->currentParams.env_to_pitch = value > 0.5 ? 1 : 0;

    if (!strcmp(name, "noise"))
        dto->state->currentParams.noise_amount = value;

    if (!strcmp(name, "volume"))
        dto->state->currentParams.volume = value * dto->state->gain_adjust;
    if (!strcmp(name, "pan"))
        dto->state->currentParams.pan = value;

    if (!strcmp(name, "delsnd"))
        dto->state->delay_send = value;

    if (!strcmp(name, "wt1shot"))
        dto->state->currentParams.wt_oneshot = value > 0.5 ? 1 : 0;

    if (!strcmp(name, "keyoffs"))
        dto->state->currentParams.note_offset = 128 * value - 64;

    if (!strcmp(name, "deltm"))
    {
        dto->state->delay_time = 1000 * value;
        Synth_set_send_delay_params(dto->state->s, dto->state->delay_feed, dto->state->delay_time);
    }
    if (!strcmp(name, "delfb"))
    {
        dto->state->delay_feed = value;
        Synth_set_send_delay_params(dto->state->s, dto->state->delay_feed, dto->state->delay_time);
    }
    // Global gain adjust in dB.
    // Modify each instrument gain by this value
    if (!strcmp(name, "glob_gain_adj_db"))
        dto->state->gain_adjust = powf(10, value / 20);
}

void dispatch__handle_synth_params_SYNTH_DATA_END(struct synth_params_SYNTH_DATA_END_DispatchDto *dto)
{
    Synth_add_instrument(dto->state->s, dto->state->instrument_count, dto->state->currentParams, dto->state->delay_send);
    memset(&dto->state->currentParams, 0, sizeof(SynthParams));
    dto->state->instrument_count++;
    // Skip drum track in GM (ch num 10 = idx 9)
    if (dto->state->instrument_count == 9)
        dto->state->instrument_count++;
}

void Synth_read_instruments(Synth *s, const char *file)
{
    SynthParamState state;
    memset(&state, 0, sizeof(state));
    state.delay_time = 500;
    state.delay_feed = 0.5;
    state.gain_adjust = 1;
    state.s = s;

    read_command_file(file, dispatch__synth_params, &state);
}