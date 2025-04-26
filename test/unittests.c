#include <test-du.h>

int logging_enabled = 0;
TEST_GLOBAL_STATE;

int main(int argc, char **argv)
{
    INIT_TESTS;
    TEST_SUITE(synth)
    {
        TEST_SUITE(adsr_envelope)
        {
            RUN_TEST(adsr_envelope__attack_decay);
            RUN_TEST(adsr_envelope__attack_decay_release);
            RUN_TEST(adsr_envelope__attack_release);
            RUN_TEST(adsr_envelope__attack_decay_sustain_release);
            RUN_TEST(adsr_envelope__attack_decay_cycling);
        }
        TEST_SUITE(basic_delay)
        {
            RUN_TEST(basic_delay__works);
        }
        TEST_SUITE(basic_oscillator)
        {
            RUN_TEST(basic_oscillator__sin_waveform);
            RUN_TEST(basic_oscillator__sqr_waveform);
            RUN_TEST(basic_oscillator__saw_waveform);
            RUN_TEST(basic_oscillator__tri_waveform);
            RUN_TEST(basic_oscillator__wt_waveform);
            RUN_TEST(basic_oscillator__fm_output);
            RUN_TEST(basic_oscillator__randomize_phase);
        }
        TEST_SUITE(wt_sample_loader)
        {
            RUN_TEST(wt_sample_loader__works);
        }
    }
    TEST_SUITE(record_file)
    {
        RUN_TEST(record_file__read_write_and_switching_files);
        RUN_TEST(record_file__format_functions);
        RUN_TEST(record_file__limits);
    }
    END_TESTS;
}