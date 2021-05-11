#include "daisy_seed.h"
#include "daisysp.h"

using namespace daisy;
using namespace daisysp;

DaisySeed hw;

Svf hp_filter;
Svf peak_filter;
Overdrive drive;
Svf lp_filter;

// Re-maps a number from one range to another. That is, a value of in_min would get mapped to out_min,
// a value of in_max to out_max, and values in-between to values in-between.
// Does not constrain values to within the range, because out-of-range values are sometimes intended and useful.
// Based on the equivalent function in the Arduino core library.
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

// Two pots on pins 15 and 16 (ADC 0 and 1)
void init_potentiometers() {
    AdcChannelConfig adc[2];
    adc[0].InitSingle(hw.GetPin(15));
    adc[1].InitSingle(hw.GetPin(16));
    hw.adc.Init(adc, 2);
    hw.adc.Start();
}

// Initialize all the DSP components that we're going to use
void init_dsp_blocks() {
    float sample_rate = hw.AudioSampleRate();

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
}

int main(void) {
    // Internal hardware setup (GPIOs, timers etc)
    hw.Configure();
    hw.Init();

    // External hardware setup (pots, buttons etc)
    init_potentiometers();

    // DSP component setup (filters, overdrive etc)
    init_dsp_blocks();

    // Start processing audio
    hw.StartAudio(AudioCallback);
    while (true) {
    }
}
