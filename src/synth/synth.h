#pragma once

#include "moog_filter.h"
#include "adsr_envelope.h"
#include "basic_oscillator.h"
#include "basic_delay.h"
#include "../linked_list.h"

// Ported from C++ code

// Sound editing parameters for the synth. One combination of these parameters is called an instrument.
// See synth architecture from Synth.
typedef struct
{
    // Oscillator 1 type as OscType cast to int
    int osc1_type;
    // Oscillator 1 tuning in semitones
    float osc1_semitones;
    // Oscillator 1 volume
    float osc1_mix;
    // Oscillator 2 type as OscType cast to int
    int osc2_type;
    // Oscillator 2 tuning in semitones
    float osc2_semitones;
    // Oscillator 2 volume
    float osc2_mix;
    // Amplitude ADSR length / sustain level parameters.
    // Lengths given as ratio where 1 = 4 seconds.
    float amp_attack, amp_decay, amp_sustain, amp_release;
    // Filter cutoff ADSR length / sustain level parameters.
    // Lengths given as ratio where 1 = 4 seconds.
    float filter_attack, filter_decay, filter_sustain, filter_release;
    // Filter parameters (range 0-1)
    float filter_cutoff, filter_resonance;
    // 1 = randomize both osc1 and osc2 phase on voice trigger
    int randomize_phase;
    // Distortion gain multiplier: 1 = 101, 0 = 1
    float distortion;
    // Use filter envelope to control osc1 and osc2 pitch instead
    // of filter cutoff
    int env_to_pitch;
    // Amplitude of white noise (range 0-1)
    float noise_amount;
    // Envelope to filter cutoff modulation amount (same parameter used for
    // pitch modulation if env_to_pitch = 1).
    float filter_mod_amount;
    // Total voice volume multiplier
    float volume;
    // Panning, 0 = 100 % left, 0.5 = middle, 1 = 100 % right
    float pan;
    // Use oscillator 2 output to modulate oscillator 1 phase instead of
    // mixing the oscillator output to output signal
    int osc2_to_osc1_fm;
    // Wavetable slot index (see wt_sample_loader.h) for oscillator 1 when
    // OSC_WT waveform is selected.
    int wt1_slot;
    // Wavetable slot index (see wt_sample_loader.h) for oscillator 2 when
    // OSC_WT waveform is selected.
    int wt2_slot;
    // Should the BasicOscillator.wt_oneshot be set to 1?
    int wt_oneshot;
    // Semitone offset. The complete tuning will be the sum of
    // oscX_semitones + note_offset. This parameter allows transponing the
    // instrument without modifying the note data (workflow optimization when
    // creating the midi files).
    int note_offset;
} SynthParams;

/*
 * A single synth voice. See voice architecture from Synth.
 */
typedef struct
{
    // "private"
    MicrotrackerMoog filter;
    BasicOscillator osc1;
    BasicOscillator osc2;
    float osc1_mix; // = 1;
    float osc2_mix; // = 1;
    int osc1_type;  // = 0;
    int osc2_type;  // = 0;
    int fm_on;
    AdsrEnvelope amp_envelope;
    AdsrEnvelope filter_envelope;
    float sample_rate;
    float distortion;
    int env_to_pitch;
    float osc1_base_freq;
    float osc2_base_freq;
    float noise_amount;
    float filter_mod_amount;
    float left_volume;
    float right_volume;
    // "public"

    // Midi note number
    int key;
    // Midi channel (0 - 15)
    int channel;
    // Total voice volume
    float volume;
    // Delay send amount (delay is global for the whole Synth instance)
    float delay_send;
    // Not implemented
    int allow_note_stealing;
    // A flag for checking if the voice is active or not. The synth uses this to
    // keep track of inactive voices so that it doesn't need to deduct it from
    // the active_voices array before triggering a new note.
    // This is used instead of the actual voice status for two reasons:
    // - Synth voice is considered "inactive" if it's ended which means that the status
    //   is hidden within the amp envelope release stage
    // - There's no convenient method of setting the amp envelope ended without doing
    //   some ugly hack (without actually triggering the voice)
    int active;
} SynthVoice;

/*
 * Initialize a synth voice. This function should be called before all notes as the
 * synth voice state is modified during the note playback.
 */
void init_SynthVoice(SynthVoice *sv, float sample_rate, int key, int channel);

/*
 * Progress synth voice state by one sample. Panned output is set to provided left and right
 * pointers. The delay_sample pointer will contain the sample sent to delay (as delay is mono)
 * which is the synth sample before panning.
 */
void SynthVoice_process(SynthVoice *sv, float *delay_sample, float *left, float *right);

/*
 * Trigger the release stage for synth voice envelopes.
 */
void SynthVoice_release(SynthVoice *sv);

/*
 * Set synth voice parameters from the SynthParams instance
 */
void SynthVoice_set_params(SynthVoice *sv, const SynthParams *params);

/*
 * Returns 1 if amplitude envelope has ended.
 */
int SynthVoice_ended(SynthVoice *sv);

/*
 * A substractive synthesizer with roughly the following kind of architecture.
 * Signal sources and destinations marked by `*NUMBER`.
```
[MIDI input] -> tune *1 -> osc 1 *5 ---|
.            |       white noise    -->+---> *4
.            |                         |
.            |->tune *1 -> osc 2 *5 ---|

ADSR envelope -> envelope routing -> *1
                        |
                        |-> *2

ADSR envelope -> *3

*4 -> distort --> Moog filter *2 -> Total volume *3 -> pan ->+--> output
.                                         |                  |
.                                         |-------> delay ---|
```

 * Note about modulation *5: FM modulation osc 2 -> osc 1 possible but it disconnects osc 2 from
 * the signal path.

 * Voice allocation is done using the active_voices array which contains the indices of currently active voices.
 * When a new voice needs to be triggered, the first voice in voices array having active = 0 is selected.
 * When a voice ends the corresponding value in active_voices is set to -1. Always after modifying the active_voices
 * array it's sorted so it's always in descending order.
 */
typedef struct
{
    // All active synth voices
    LinkedList voices;

    // Synth sample rate in Hz
    float sample_rate;

    // Synth parameter sets. Selected parameters correspond to
    // the midi channel.
    SynthParams instruments[16];
    // Global send delay with maximum of 1 second delay time
    BasicDelay send_delay;
    // A list of send delay amounts for each channel
    float send_delay_amounts[16];

    // Debug data; maximum number of active voices seen during processing
    int diagnostics_max_n_voices;
    // Total volume multiplier that is applied after all other processing (including
    // "master" soft clipping).
    float total_volume;
} Synth;

/*
 * Initialize synth instance with sample rate (Hz).
 * Needs to be called only once.
 */
void init_Synth(Synth *s, float sample_rate);

/*
 * Free all memory reserved in init_Synth.
 */
void free_Synth(Synth *s);

/*
 * Handle a midi event. Note on / off supported. The instrument selected for the channel
 * depends on the midi channel for the note event. Note off event will release all
 * notes triggered with the same channel and midi note number information.
 *
 * Flags can be used to prevent note stealing which is, however, not supported.
 * Note stealing is not implemented so if the voice pool is exhausted, no more voices
 * can be played.
 */
void Synth_handle_midi_event(Synth *s, unsigned char *event_data, unsigned flags);

/*
 * Sets synth parameters for instrument at index channel. Send delay parameter set as a separate
 * parameter.
 */
void Synth_add_instrument(Synth *s, int channel, SynthParams params, float send_delay_amount);

/*
 * Set feedback and delay time parameters. Feedback should be in range 0-1 and delay given as milliseconds.
 * Maximum delay time is 1000 milliseconds.
 */
void Synth_set_send_delay_params(Synth *s, float feedback, float delay_ms);

/*
 * Fill a stereo buffer of provided length (all data is interleaved in buffer_left; TODO: remove buffer_right.
 * Buffer size contains the total number of elements in buffer_left. This is kinda messed up API, TBH.)
 * with synthesized signal.
 */
void Synth_process(Synth *s, float *buffer_left, float *buffer_right, int buffer_size);

/*
 * Kills all voices by setting them inactive (not just releasing them). Does not cut off delay tail.
 */
void Synth_kill_all_voices(Synth *s);

/*
 * Kills voices on the selected channel same way Synth_kill_all_voices does.
 */
void Synth_kill_voices(Synth *s, int channel);

/*
 * Reads all instruments in the file.
 * The file uses a syntax where each instrument instance is separated by a line containing `SYNTH_DATA_END`.
 * The parameter data is listed as key-value pairs in format `key=value` where the key is a predefined string
 * and value is a float with range 0-1 that is then mapped to the correct range. The following keys are "global",
 * meaning that the last defined key-value pair determines the global status of the parameter: deltm, delfb, glob_gain_adj_db.
 *
 * All parameter keys and their corresponding SynthParams fields is listed below. The actual parameter range is marked in braces
 * after the parameter and the range data type after if. Syntax is: `key, field [range min, range max] datatype`.
 *
 * o1type osc1_type [0, 4] int
 * o2type osc2_type [0, 4] int
 * o1tune osc1_semitones [-24, 24] float
 * o2tune osc2_semitones [-24, 24] float
 * o1mix osc1_mix
 * o2mix osc2_mix
 * o2o1_fm osc2_to_osc1_fm [0, 1] int
 * o1_wt wt1_slot [0, MAX_WT_SAMPLE_SLOTS - 1] int
 * o2_wt wt2_slot [0, MAX_WT_SAMPLE_SLOTS - 1] int
 * v_a amp_attack
 * v_d amp_decay
 * v_s amp_sustain
 * v_r amp_release
 * f_a filter_attack
 * f_d filter_decay
 * f_s filter_sustain
 * f_r filter_release
 * cut filter_cutoff
 * env2f filter_mod_amount
 * res filter_resonance
 * phrand randomize_phase [0, 1] int
 * dist distortion
 * env2p env_to_pitch [0, 1] int
 * noise noise_amount
 * volume volume [0, volume modified by glob_gain_adj_db] float
 * pan pan
 * delsnd N/A (delay send)
 * wt1shot wt_oneshot [0, 1] int
 * keyoffs note_offset [-64, 64] int
 * deltm N/A (delay time in ms) [0, 1000] float
 * delfb N/A (delay feedback)
 * glob_gain_adj_db N/A (this sets the Synth.total_volume and is intended for normalizing the tracks)
 *
 * Note that instrument number 9 is skipped because of the MIDI export workflow. The MIDI files are exported from
 * FLStudio that skips the channel 10 (first channel is 1) that traditionally contains drum data. So using this
 * workflow sacrifices one instrument instance. The MIDI files are prepared using a custom VST compatible implementation
 * of this synth that saves its parameter values in this format as a binary blob and the binary blobs are parsed as is from
 * the FLStudio project file.
 */
void Synth_read_instruments(Synth *s, const char *file);