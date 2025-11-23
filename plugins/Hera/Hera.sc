// SPDX-License-Identifier: GPL-3.0-or-later
// Hera - Juno 60 emulation synthesizer for SuperCollider
// Based on https://github.com/jpcima/Hera

Hera : UGen {
    *ar { |gate=0, freq=440,
          vcaDepth=0.5, vcaType=0,
          pwmDepth=0.5, pwmMod=0,
          sawLevel=1.0, pulseLevel=0.0, subLevel=0.0, noiseLevel=0.0,
          pitchRange=1, pitchModDepth=0.0,
          vcfCutoff=1000, vcfResonance=0.0,
          vcfEnvModDepth=0.0, vcfLFOModDepth=0.0,
          vcfKeyModDepth=0.0, vcfBendDepth=0.0,
          attack=0.01, decay=0.3, sustain=0.7, release=0.5,
          lfoTrigMode=0, lfoRate=5.0, lfoDelay=0.0,
          hpf=0.0, chorusI=0, chorusII=0|
        
        ^this.multiNew('audio', gate, freq,
            vcaDepth, vcaType,
            pwmDepth, pwmMod,
            sawLevel, pulseLevel, subLevel, noiseLevel,
            pitchRange, pitchModDepth,
            vcfCutoff, vcfResonance,
            vcfEnvModDepth, vcfLFOModDepth,
            vcfKeyModDepth, vcfBendDepth,
            attack, decay, sustain, release,
            lfoTrigMode, lfoRate, lfoDelay,
            hpf, chorusI, chorusII
        );
    }
    
    checkInputs {
        ^this.checkValidInputs;
    }
}

/*
Hera Parameter Documentation:

VCA (Voltage Controlled Amplifier):
- vcaDepth: Overall volume/depth (0.0 - 1.0)
- vcaType: Envelope (0) or Gate (1)

DCO (Digitally Controlled Oscillator):
- pwmDepth: Pulse width modulation depth (0.0 - 1.0)
- pwmMod: PWM modulation source - Manual (0), LFO (1), Envelope (2)
- sawLevel: Sawtooth oscillator level (0.0 - 1.0)
- pulseLevel: Pulse/square wave level (0.0 - 1.0)
- subLevel: Sub-oscillator level (one octave down) (0.0 - 1.0)
- noiseLevel: White noise level (0.0 - 1.0)
- pitchRange: Octave range - 16' (0), 8' (1), 4' (2)
- pitchModDepth: Pitch modulation depth from LFO (0.0 - 1.0)

VCF (Voltage Controlled Filter):
- vcfCutoff: Filter cutoff frequency in Hz (20 - 20000)
- vcfResonance: Filter resonance/Q (0.0 - 1.0)
- vcfEnvModDepth: Envelope modulation of filter cutoff (-1.0 - 1.0)
- vcfLFOModDepth: LFO modulation of filter cutoff (0.0 - 1.0)
- vcfKeyModDepth: Keyboard tracking amount (0.0 - 1.0)
- vcfBendDepth: Pitch bend effect on filter (0.0 - 1.0)

Envelope:
- attack: Attack time in seconds
- decay: Decay time in seconds
- sustain: Sustain level (0.0 - 1.0)
- release: Release time in seconds

LFO (Low Frequency Oscillator):
- lfoTrigMode: Manual (0) or Auto (1)
- lfoRate: LFO frequency in Hz
- lfoDelay: LFO delay time in seconds

Effects:
- hpf: High-pass filter amount (0.0 - 1.0)
- chorusI: Chorus I on/off (0 or 1)
- chorusII: Chorus II on/off (0 or 1)

Example usage:

// Simple sawtooth note
Hera.ar(gate: 1, freq: 440, sawLevel: 1.0);

// Pulse wave with filter sweep
Hera.ar(
    gate: 1,
    freq: 220,
    pulseLevel: 1.0,
    vcfCutoff: 500,
    vcfEnvModDepth: 0.8,
    attack: 0.1,
    decay: 0.5,
    sustain: 0.3,
    release: 0.5
);

// Classic Juno-style pad
Hera.ar(
    gate: 1,
    freq: 440,
    sawLevel: 0.7,
    pulseLevel: 0.3,
    vcfCutoff: 800,
    vcfResonance: 0.4,
    vcfEnvModDepth: 0.5,
    vcfLFOModDepth: 0.2,
    attack: 0.3,
    decay: 0.8,
    sustain: 0.6,
    release: 1.0,
    lfoRate: 0.5
);
*/
