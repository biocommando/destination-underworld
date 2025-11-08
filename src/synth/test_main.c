#include <stdio.h>
#include <string.h>
#include "midi_player.h"
#include "wav_handler.h"
#include "wt_sample_loader.h"

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        puts("Following paths expected arguments:");
        puts("midi file");
        puts("data directory");
        return -1;
    }
    int wt_sample_count = wt_sample_read_all(argv[2]);
    printf("Wavetable samples loaded: %d\n", wt_sample_count);
    if (wt_sample_count == 0)
    {
        printf("Couldn't load wavetable samples. Running from correct directory?");
        return 1;
    }
    MidiPlayer mp;
    MidiFile mf;
    init_midi_player(&mp, 44100);

    char metadata_file[1024];
    sprintf(metadata_file, "%s_meta.ini", argv[1]);
    Synth_read_instruments(&mp.synth, metadata_file);

    init_midi_file(&mf);
    read_midi_file(argv[1], &mf);

    midi_player_set_midi_file(&mp, &mf);

    printf("Total time: %lf s\n", mf.file_length_samples / 44100.0f);
    printf("Tempos - player: %f, file: %f\n", mp.tempo, mf.tempo);
    printf("Delay params: time:%lu, feed:%lf\n", mp.synth.send_delay.delaySamples, mp.synth.send_delay.feed);

    struct wav_file wf;
    create_wav_file(&wf, mf.file_length_samples + 44100 * 5, 2, 16, 44100);

    int show_progress_counter = 44100 * 20;
    for (unsigned i = 0; i < wf.num_frames; i += 2048)
    {
        float buf[2 * 2048];
        midi_player_process_buffer(&mp, buf, 2048);
        for (int j = 0; j < 2048; j++)
        {
            wav_set_normalized(&wf, i + j, &buf[j * 2]);
        }
        show_progress_counter -= 2048;
        if (show_progress_counter <= 0)
        {
            show_progress_counter = 44100 * 20;
            printf("%.0lf secs done\n", i / 44100.0f);
        }
    }

    printf("Max voices in use simultaneously: %d\n", mp.synth.diagnostics_max_n_voices);

    char output_file[1024];
    sprintf(output_file, "%s.wav", argv[1]);
    write_wav_file(output_file, &wf);
    free_wav_file(&wf);

    free_midi_file(&mf);
    free_midi_player(&mp);

    wt_sample_free();
}