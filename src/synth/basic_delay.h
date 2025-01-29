#pragma once

// Ported from C++ code

/*
 * A mono delay effect with fixed buffer length. Doesn't do any additional scaling to
 * wet / dry signal, just multiplies the existing buffer with the feed property
 * before adding it to the output.
 */
typedef struct
{
    // Delay length in samples
    unsigned long delaySamples;
    // Buffer total length
    unsigned long bufferLength;
    // Current playback index
    unsigned long index;
    // Sample rate in Hz
    int sampleRate;
    // The delay buffer
    double *buffer;
    // Feedback amount. Should be less than 1.0 for stable functionality.
    double feed;
} BasicDelay;

/*
 * Initializes the structure with buffer length and sample rate.
 */
void init_BasicDelay(BasicDelay *bd, unsigned long bufferLength, int sampleRate);
/*
 * Frees all memory reserved using the init function.
 */
void free_BasicDelay(BasicDelay *bd);
/*
 * Proceeds the delay effect state by one sample. The input signal should be
 * fed to this function sample by sample and the output signal is returned from
 * this function sample by sample.
 */
double BasicDelay_process(BasicDelay *bd, double input);
/*
 * Set the delay length in milliseconds. If the delay length exceeds the buffer length,
 * the maximum buffer length is used. Clears any pre-existing data in the delay buffer when
 * the new delay time is longer than before.
 */
void BasicDelay_setTime(BasicDelay *bd, double milliseconds);
/*
 * Sets the feed property with limit checking (0-0.99). If limits are exceeded, sets the limit
 * value to the property.
 */
void BasicDelay_setFeedback(BasicDelay *bd, double feedback);