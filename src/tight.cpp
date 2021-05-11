#include "daisy_seed.h"
#include "daisysp.h"

using namespace daisy;
using namespace daisysp;

DaisySeed hw;

Svf hp_filter;
Svf peak_filter;
Overdrive drive;
Svf lp_filter;

float map(float x, float out_min, float out_max, float in_min = 0, float in_max = 1) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void AudioCallback(float **in, float **out, size_t size) {
    // Read knob positions
    float drive_pos = hw.adc.GetFloat(0);
    float hp_pos = hw.adc.GetFloat(1);
    // Map knobs to parameters
    drive.SetDrive(map(drive_pos, 0.25, 0.5));
    hp_filter.SetFreq(map(hp_pos, 20, 150));

    // Scale volume to compensate gain
    float volume_coeff = 1.0 / map(drive_pos, 1, 12);

    for (size_t i = 0; i < size; i++) {
        float sig = in[0][i];

        // Apply high pass
        hp_filter.Process(sig);
        sig = hp_filter.High();
        // Apply mid bump
        peak_filter.Process(sig);
        sig = peak_filter.Peak();
        // Apply overdrive
        sig = drive.Process(sig);
        // Apply low pass
        lp_filter.Process(sig);
        sig = lp_filter.Low();

        out[0][i] = sig * volume_coeff;
    }
}
int main(void) {
    hw.Configure();
    hw.Init();
    // hw.StartL og(true);
    float sample_rate = hw.AudioSampleRate();

    // Initialize potentiometers
    AdcChannelConfig adc[2];
    adc[0].InitSingle(hw.GetPin(15));
    adc[1].InitSingle(hw.GetPin(16));
    hw.adc.Init(adc, 2);
    hw.adc.Start();

    // high pass at 100 Hz
    hp_filter.Init(sample_rate);
    hp_filter.SetFreq(20);
    hp_filter.SetRes(0.25);
    hp_filter.SetDrive(0.75);

    // Mid bump at 720 Hz
    peak_filter.Init(sample_rate);
    peak_filter.SetFreq(720);
    peak_filter.SetRes(0.25);
    peak_filter.SetDrive(0.8);

    // Low pass at 12 kHz
    lp_filter.Init(sample_rate);
    lp_filter.SetFreq(12000);
    lp_filter.SetRes(0.1);
    lp_filter.SetDrive(0.75);

    // Initialize overdrive
    drive.Init();
    drive.SetDrive(0.5);

    hw.StartAudio(AudioCallback);
    
    while (true) {}
}
