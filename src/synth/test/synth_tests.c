#include <math.h>
#include <unittests.h>
#include "../adsr_envelope.h"
#include "../basic_delay.h"
#include "../basic_oscillator.h"
#include "../synth_random.h"
#include "../wav_handler.h"
#include "../wt_sample_loader.h"

void adsr_envelope__attack_decay()
{
    AdsrEnvelope e;
    init_AdsrEnvelope(&e);
    AdsrEnvelope_setAttack(&e, 2);
    AdsrEnvelope_setDecay(&e, 2);
    AdsrEnvelope_setRelease(&e, 40);
    AdsrEnvelope_trigger(&e);

    EXPECT_FLOAT_EQ(0, AdsrEnvelope_getRatio(&e, -1));
    EXPECT_FLOAT_EQ(0, AdsrEnvelope_getEnvelope(&e));
    AdsrEnvelope_calculateNext(&e);
    EXPECT_FLOAT_EQ(0.5, AdsrEnvelope_getRatio(&e, -1));
    EXPECT_FLOAT_EQ(0.5, AdsrEnvelope_getEnvelope(&e));
    AdsrEnvelope_calculateNext(&e);
    EXPECT_FLOAT_EQ(1, AdsrEnvelope_getRatio(&e, -1));
    EXPECT_FLOAT_EQ(1, AdsrEnvelope_getEnvelope(&e));
    AdsrEnvelope_calculateNext(&e);
    EXPECT_FLOAT_EQ(0.5, AdsrEnvelope_getRatio(&e, -1));
    EXPECT_FLOAT_EQ(0.5, AdsrEnvelope_getEnvelope(&e));
    for (int i = 0; i < 10; i++)
    {
        TEST_DEBUG_PRINTF("i = %d", i);
        AdsrEnvelope_calculateNext(&e);
        EXPECT_FLOAT_EQ(1, AdsrEnvelope_getRatio(&e, -1));
        EXPECT_FLOAT_EQ(0, AdsrEnvelope_getEnvelope(&e));
        ASSERT(INT_EQ(i > 0, AdsrEnvelope_ended(&e)));
        if (i == 5)
            AdsrEnvelope_release(&e);
    }
    TEST_DEBUG_PRINT_CLEAR();
}

void adsr_envelope__attack_decay_cycling()
{
    AdsrEnvelope e;
    init_AdsrEnvelope(&e);
    AdsrEnvelope_setAttack(&e, 2);
    AdsrEnvelope_setDecay(&e, 4);
    AdsrEnvelope_setCycleOnOff(&e, 1);
    AdsrEnvelope_setRelease(&e, 40);
    AdsrEnvelope_trigger(&e);
    EXPECT_FLOAT_EQ(0, AdsrEnvelope_getEnvelope(&e));
    AdsrEnvelope_calculateNext(&e);
    float expected_env_cycle[6] = {0.5, 1, 0.75, 0.5, 0.25, 0};
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 6; j++)
        {
            TEST_DEBUG_PRINTF("i=%d, j=%d, stage=%d\n", i, j, AdsrEnvelope_getStage(&e));
            EXPECT_FLOAT_EQ(expected_env_cycle[j], AdsrEnvelope_getEnvelope(&e));
            AdsrEnvelope_calculateNext(&e);
        }
    }
    TEST_DEBUG_PRINT_CLEAR();
}

void adsr_envelope__attack_decay_release()
{
    AdsrEnvelope e;
    init_AdsrEnvelope(&e);
    AdsrEnvelope_setAttack(&e, 2);
    AdsrEnvelope_setDecay(&e, 4);
    AdsrEnvelope_setRelease(&e, 40);
    AdsrEnvelope_trigger(&e);

    EXPECT_FLOAT_EQ(0, AdsrEnvelope_getRatio(&e, -1));
    EXPECT_FLOAT_EQ(0, AdsrEnvelope_getEnvelope(&e));
    AdsrEnvelope_calculateNext(&e);
    EXPECT_FLOAT_EQ(0.5, AdsrEnvelope_getRatio(&e, -1));
    EXPECT_FLOAT_EQ(0.5, AdsrEnvelope_getEnvelope(&e));
    AdsrEnvelope_calculateNext(&e);
    EXPECT_FLOAT_EQ(1, AdsrEnvelope_getRatio(&e, -1));
    EXPECT_FLOAT_EQ(1, AdsrEnvelope_getEnvelope(&e));
    AdsrEnvelope_calculateNext(&e);
    EXPECT_FLOAT_EQ(0.25, AdsrEnvelope_getRatio(&e, -1));
    EXPECT_FLOAT_EQ(0.75, AdsrEnvelope_getEnvelope(&e));
    AdsrEnvelope_release(&e);

    for (int i = 0; i < 40; i++)
    {
        TEST_DEBUG_PRINTF("i = %d", i);
        EXPECT_FLOAT_EQ(i / 40.0f, AdsrEnvelope_getRatio(&e, -1));
        EXPECT_FLOAT_EQ(0.75 - 0.75 * i / 40.0f, AdsrEnvelope_getEnvelope(&e));
        ASSERT(INT_EQ(0, AdsrEnvelope_ended(&e)));
        AdsrEnvelope_calculateNext(&e);
    }
    TEST_DEBUG_PRINT_CLEAR();
    AdsrEnvelope_calculateNext(&e);
    ASSERT(INT_EQ(1, AdsrEnvelope_ended(&e)));
}

void adsr_envelope__attack_release()
{
    AdsrEnvelope e;
    init_AdsrEnvelope(&e);
    AdsrEnvelope_setAttack(&e, 4);
    AdsrEnvelope_setDecay(&e, 2);
    AdsrEnvelope_setRelease(&e, 40);
    AdsrEnvelope_trigger(&e);

    EXPECT_FLOAT_EQ(0, AdsrEnvelope_getRatio(&e, -1));
    EXPECT_FLOAT_EQ(0, AdsrEnvelope_getEnvelope(&e));
    AdsrEnvelope_calculateNext(&e);
    EXPECT_FLOAT_EQ(0.25, AdsrEnvelope_getRatio(&e, -1));
    EXPECT_FLOAT_EQ(0.25, AdsrEnvelope_getEnvelope(&e));
    AdsrEnvelope_release(&e);

    for (int i = 0; i < 40; i++)
    {
        TEST_DEBUG_PRINTF("i = %d", i);
        EXPECT_FLOAT_EQ(i / 40.0f, AdsrEnvelope_getRatio(&e, -1));
        EXPECT_FLOAT_EQ(0.25 - 0.25 * i / 40.0f, AdsrEnvelope_getEnvelope(&e));
        ASSERT(INT_EQ(0, AdsrEnvelope_ended(&e)));
        AdsrEnvelope_calculateNext(&e);
    }
    TEST_DEBUG_PRINT_CLEAR();
    AdsrEnvelope_calculateNext(&e);
    ASSERT(INT_EQ(1, AdsrEnvelope_ended(&e)));
}

void adsr_envelope__attack_decay_sustain_release()
{
    AdsrEnvelope e;
    init_AdsrEnvelope(&e);
    AdsrEnvelope_setAttack(&e, 2);
    AdsrEnvelope_setDecay(&e, 2);
    AdsrEnvelope_setSustain(&e, 0.5);
    AdsrEnvelope_setRelease(&e, 40);
    AdsrEnvelope_trigger(&e);

    EXPECT_FLOAT_EQ(0, AdsrEnvelope_getRatio(&e, -1));
    EXPECT_FLOAT_EQ(0, AdsrEnvelope_getEnvelope(&e));
    AdsrEnvelope_calculateNext(&e);
    EXPECT_FLOAT_EQ(0.5, AdsrEnvelope_getRatio(&e, -1));
    EXPECT_FLOAT_EQ(0.5, AdsrEnvelope_getEnvelope(&e));
    AdsrEnvelope_calculateNext(&e);
    EXPECT_FLOAT_EQ(1, AdsrEnvelope_getRatio(&e, -1));
    EXPECT_FLOAT_EQ(1, AdsrEnvelope_getEnvelope(&e));
    AdsrEnvelope_calculateNext(&e);
    EXPECT_FLOAT_EQ(0.5, AdsrEnvelope_getRatio(&e, -1));
    EXPECT_FLOAT_EQ(0.75, AdsrEnvelope_getEnvelope(&e));
    // Test that sustain works (level is sustained)
    for (int i = 0; i < 50; i++)
    {
        AdsrEnvelope_calculateNext(&e);
        EXPECT_FLOAT_EQ(1, AdsrEnvelope_getRatio(&e, -1));
        EXPECT_FLOAT_EQ(0.5, AdsrEnvelope_getEnvelope(&e));
    }
    AdsrEnvelope_release(&e);

    for (int i = 0; i < 40; i++)
    {
        TEST_DEBUG_PRINTF("i = %d", i);
        EXPECT_FLOAT_EQ(i / 40.0f, AdsrEnvelope_getRatio(&e, -1));
        EXPECT_FLOAT_EQ(0.5 - 0.5 * i / 40.0f, AdsrEnvelope_getEnvelope(&e));
        ASSERT(INT_EQ(0, AdsrEnvelope_ended(&e)));
        AdsrEnvelope_calculateNext(&e);
    }
    TEST_DEBUG_PRINT_CLEAR();
    AdsrEnvelope_calculateNext(&e);
    ASSERT(INT_EQ(1, AdsrEnvelope_ended(&e)));
}

void basic_delay__works()
{
    BasicDelay bd;
    init_BasicDelay(&bd, 10, 10);

    BasicDelay_setTime(&bd, 500);
    BasicDelay_setFeedback(&bd, 0.5);
    EXPECT_FLOAT_EQ(0, BasicDelay_process(&bd, 1));
    EXPECT_FLOAT_EQ(0, BasicDelay_process(&bd, 0));
    EXPECT_FLOAT_EQ(0, BasicDelay_process(&bd, 0.5));
    EXPECT_FLOAT_EQ(0, BasicDelay_process(&bd, 0));
    EXPECT_FLOAT_EQ(0, BasicDelay_process(&bd, 0));

    EXPECT_FLOAT_EQ(1, BasicDelay_process(&bd, 0.3)); // 0.5 * 1 + 0.3 = 0.8
    EXPECT_FLOAT_EQ(0, BasicDelay_process(&bd, 0));
    EXPECT_FLOAT_EQ(0.5, BasicDelay_process(&bd, 0));
    EXPECT_FLOAT_EQ(0, BasicDelay_process(&bd, 0));
    EXPECT_FLOAT_EQ(0, BasicDelay_process(&bd, 0));

    EXPECT_FLOAT_EQ(0.8, BasicDelay_process(&bd, 0));
    EXPECT_FLOAT_EQ(0, BasicDelay_process(&bd, 0));
    EXPECT_FLOAT_EQ(0.25, BasicDelay_process(&bd, 0));
    EXPECT_FLOAT_EQ(0, BasicDelay_process(&bd, 0));
    EXPECT_FLOAT_EQ(0, BasicDelay_process(&bd, 0));

    EXPECT_FLOAT_EQ(0.4, BasicDelay_process(&bd, 0));
    EXPECT_FLOAT_EQ(0, BasicDelay_process(&bd, 0));
    EXPECT_FLOAT_EQ(0.125, BasicDelay_process(&bd, 0));
    EXPECT_FLOAT_EQ(0, BasicDelay_process(&bd, 0));
    EXPECT_FLOAT_EQ(0, BasicDelay_process(&bd, 0));

    // Setting delay to a _longer_ value clears anything that was in the
    // buffer already
    BasicDelay_setTime(&bd, 100); // 10 / (1000 / 100) = 1, so the first sample in the buffer will be retained
    BasicDelay_setTime(&bd, 500);
    EXPECT_FLOAT_EQ(0.2, BasicDelay_process(&bd, 0));
    EXPECT_FLOAT_EQ(0, BasicDelay_process(&bd, 0));
    EXPECT_FLOAT_EQ(0, BasicDelay_process(&bd, 0));
    EXPECT_FLOAT_EQ(0, BasicDelay_process(&bd, 0));
    EXPECT_FLOAT_EQ(0, BasicDelay_process(&bd, 0));

    free_BasicDelay(&bd);
}

void basic_oscillator__sin_waveform()
{
    BasicOscillator bo;
    init_BasicOscillator(&bo, 10);
    BasicOscillator_setFrequency(&bo, 1);

    for (int i = 0; i < 20; i++)
    {
        float expected = sin(6.283185307179586476925286766559 / 10 * i);
        EXPECT_FLOAT_EQ_DELTA(expected, BasicOscillator_getValue(&bo, OSC_SINE), 0.1);
        BasicOscillator_calculateNext(&bo);
    }
}

void basic_oscillator__sqr_waveform()
{
    BasicOscillator bo;
    init_BasicOscillator(&bo, 10);
    BasicOscillator_setFrequency(&bo, 1);

    for (int i = 0; i < 20; i++)
    {
        float expected = i % 10 < 5 ? 1 : -1;
        EXPECT_FLOAT_EQ_DELTA(expected, BasicOscillator_getValue(&bo, OSC_SQUARE), 0.1);
        BasicOscillator_calculateNext(&bo);
    }
}

void basic_oscillator__saw_waveform()
{
    BasicOscillator bo;
    init_BasicOscillator(&bo, 10);
    BasicOscillator_setFrequency(&bo, 1);

    for (int i = 0; i < 20; i++)
    {
        float expected = (i % 10) / 5.0 - 1;
        EXPECT_FLOAT_EQ_DELTA(expected, BasicOscillator_getValue(&bo, OSC_SAW), 0.1);
        BasicOscillator_calculateNext(&bo);
    }
}

void basic_oscillator__tri_waveform()
{
    BasicOscillator bo;
    init_BasicOscillator(&bo, 32);
    BasicOscillator_setFrequency(&bo, 2);

    float tbl[16] = {
        -1,
        -0.75,
        -0.5,
        -0.25,
        0,
        0.25,
        0.5,
        0.75,
        1,
        0.75,
        0.5,
        0.25,
        0,
        -0.25,
        -0.5,
        -0.75,
    };

    for (int i = 0; i < 32; i++)
    {
        float expected = tbl[i % 16];
        EXPECT_FLOAT_EQ_DELTA(expected, BasicOscillator_getValue(&bo, OSC_TRIANGLE), 0.1);
        BasicOscillator_calculateNext(&bo);
    }
}

void basic_oscillator__wt_waveform()
{
    BasicOscillator bo;
    init_BasicOscillator(&bo, 10);
    BasicOscillator_setFrequency(&bo, 1);

    float tbl[] = {
        0.1, 0.5, 0.66, -1, 1, -1, 0.5, 1, -0.5, 0};

    BasicOscillator_setWavetable(&bo, tbl, 10);
    BasicOscillator_setWaveTableParams(&bo, 0, 1);

    TEST_DEBUG_PRINTF("params (0, 1)");
    for (int i = 0; i < 20; i++)
    {
        float expected = tbl[i % 10];
        EXPECT_FLOAT_EQ_DELTA(expected, BasicOscillator_getValue(&bo, OSC_WT), 0.01);
        BasicOscillator_calculateNext(&bo);
    }

    // The window calculation has some safety offsets so the parameters may be a bit confusing
    // for this small wavetables
    BasicOscillator_setWaveTableParams(&bo, 0.4, 0.25);

    TEST_DEBUG_PRINTF("params (0.4, 0.25)");
    for (int i = 0; i < 20; i++)
    {
        float expected = tbl[(int)((i % 10) / 10.0 * 4 + 2)];
        EXPECT_FLOAT_EQ_DELTA(expected, BasicOscillator_getValue(&bo, OSC_WT), 0.02);
        BasicOscillator_calculateNext(&bo);
    }

    TEST_DEBUG_PRINTF("oneshot = 1");
    bo.wt_oneshot = 1;
    for (int i = 0; i < 20; i++)
    {
        float expected = i < 10 ? tbl[(int)(i / 10.0 * 4 + 2)] : 0;
        EXPECT_FLOAT_EQ_DELTA(expected, BasicOscillator_getValue(&bo, OSC_WT), 0.02);
        BasicOscillator_calculateNext(&bo);
    }
}

void basic_oscillator__fm_output()
{
    BasicOscillator bo;
    init_BasicOscillator(&bo, 10);
    BasicOscillator_setFrequency(&bo, 1);

    for (int i = 0; i < 20; i++)
    {
        float expected = (i + 5) % 10 < 5 ? 1 : -1;
        EXPECT_FLOAT_EQ_DELTA(expected, BasicOscillator_getValueFm(&bo, OSC_SQUARE, i % 2 ? 0.5 : -0.5), 0.1);
        BasicOscillator_calculateNext(&bo);
    }
}

void basic_oscillator__randomize_phase()
{
    BasicOscillator bo;
    init_BasicOscillator(&bo, 10);
    synth_random_reset();
    float max = 0, avg = 0;
    for (int i = 0; i < 100; i++)
    {
        BasicOscillator_randomizePhase(&bo, 1);
        max = bo.phase > max ? max : bo.phase;
        avg += bo.phase / 100;
    }
    ASSERT(FLOAT_GREATER(max, 1));
    EXPECT_FLOAT_EQ_DELTA(0.5, avg, 0.1);
    max = 0, avg = 0;
    for (int i = 0; i < 100; i++)
    {
        BasicOscillator_randomizePhase(&bo, 0.5);
        max = bo.phase > max ? max : bo.phase;
        avg += bo.phase / 100;
    }
    ASSERT(FLOAT_GREATER(max, 0.5));
    EXPECT_FLOAT_EQ_DELTA(0.25, avg, 0.1);
    synth_random_reset();
}

void wt_sample_loader__works()
{
    struct wav_file wf;
    create_wav_file(&wf, 10, 1, 32, 44100);
    for (int i = 0; i < 10; i++)
    {
        float f[1] = {i / 10.0};
        wav_set_normalized(&wf, i, f);
    }
    write_wav_file("wt_sample_slot_0.wav", &wf);
    free_wav_file(&wf);

    int read_wts = wt_sample_read_all("./");
    ASSERT(INT_EQ(1, read_wts));

    // More than max -> null
    ASSERT(get_wt_sample(MAX_WT_SAMPLE_SLOTS) == NULL);

    struct wt_sample *wts;
    for (int i = 1; i < MAX_WT_SAMPLE_SLOTS; i++)
    {
        wts = get_wt_sample(i);
        ASSERT(INT_EQ(0, wts->size));
        ASSERT(INT_EQ(0, *wts->data));
    }

    wts = get_wt_sample(0);
    ASSERT(INT_EQ(10, wts->size));
    for (int i = 0; i < 10; i++)
    {
        EXPECT_FLOAT_EQ(i / 10.0, wts->data[i]);
    }

    wt_sample_free();

    remove("wt_sample_slot_0.wav");
}

void test_suite__adsr_envelope()
{
    RUN_TEST(adsr_envelope__attack_decay);
    RUN_TEST(adsr_envelope__attack_decay_release);
    RUN_TEST(adsr_envelope__attack_release);
    RUN_TEST(adsr_envelope__attack_decay_sustain_release);
    RUN_TEST(adsr_envelope__attack_decay_cycling);
}

void test_suite__basic_delay()
{
    RUN_TEST(basic_delay__works);
}

void test_suite__basic_oscillator()
{
    RUN_TEST(basic_oscillator__sin_waveform);
    RUN_TEST(basic_oscillator__sqr_waveform);
    RUN_TEST(basic_oscillator__saw_waveform);
    RUN_TEST(basic_oscillator__tri_waveform);
    RUN_TEST(basic_oscillator__wt_waveform);
    RUN_TEST(basic_oscillator__fm_output);
    RUN_TEST(basic_oscillator__randomize_phase);
}

void test_suite__wt_sample_loader()
{
    RUN_TEST(wt_sample_loader__works);
}

void test_suite__synth()
{
    RUN_TEST_SUITE(test_suite__adsr_envelope)
    RUN_TEST_SUITE(test_suite__basic_delay)
    RUN_TEST_SUITE(test_suite__basic_oscillator)
    RUN_TEST_SUITE(test_suite__wt_sample_loader)
}