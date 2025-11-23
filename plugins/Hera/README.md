# Hera SuperCollider Plugin

A SuperCollider implementation of the Hera synthesizer, based on [jpcima/Hera](https://github.com/jpcima/Hera), which is a Juno-60 emulation.

## Features

This plugin provides a self-contained polyphonic synthesizer (up to 6 voices) with all the core parameters from the original Hera:

### Oscillator Section (DCO)
- Sawtooth, pulse, sub-oscillator, and noise generators
- Pulse width modulation with multiple modulation sources
- Pitch modulation via LFO
- Octave range selection (16', 8', 4')

### Filter Section (VCF)
- Resonant lowpass filter
- Envelope modulation
- LFO modulation
- Keyboard tracking
- Pitch bend routing

### Envelope Generator
- ADSR envelope for amplitude and filter modulation
- Separate gate mode for organ-style sounds

### LFO
- Sine wave modulator
- Manual or auto-trigger modes
- Rate and delay controls

### Effects Section
- VCA (Voltage Controlled Amplifier) with depth control
- High-pass filter
- Chorus I and II (placeholder in current implementation)

## Building

### Prerequisites

The build system will automatically download the SuperCollider source if not present. You can also provide your own:

```bash
# Option 1: Let the Makefile download SC automatically
make

# Option 2: Provide your own SC source
make SC_PATH=/path/to/supercollider/source
```

### Build Steps

```bash
# From the repository root:
make

# Clean build files:
make clean

# Remove everything including downloaded SC source:
make distclean
```

### Manual Build

If you prefer to build manually:

```bash
cd plugins/Hera
mkdir build && cd build
cmake -DSC_PATH=/path/to/supercollider/source ..
make
```

## Installation

After building, copy or symlink the plugin to your SuperCollider extensions directory:

```bash
# Find your extensions directory in SuperCollider:
# Platform.userExtensionDir

# Copy the built plugin
cp -r plugins/Hera ~/.local/share/SuperCollider/Extensions/
```

Make sure to copy both the `.scx` (or `.so` on Linux) file and the `.sc` class file.

## Usage

```supercollider
// Simple test
{ Hera.ar(gate: 1, freq: 440, sawLevel: 1.0) }.play;

// Classic synth pad
(
{
    var gate = LFPulse.kr(0.5);
    var freq = [220, 220.5];  // Slight detune for width
    
    Hera.ar(
        gate: gate,
        freq: freq,
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
    )
}.play;
)

// Bass sequence
(
{
    var gate = Impulse.kr(4);
    var freq = Dseq([110, 110, 165, 220], inf).demand(gate);
    
    Hera.ar(
        gate: gate,
        freq: freq,
        sawLevel: 0.8,
        subLevel: 0.5,
        vcfCutoff: 400,
        vcfResonance: 0.6,
        vcfEnvModDepth: 0.7,
        attack: 0.01,
        decay: 0.2,
        sustain: 0.3,
        release: 0.2
    )
}.play;
)
```

## Parameters

All parameters are listed in the Hera.sc file with detailed documentation. Key parameters:

- **gate**: Trigger note on/off (0 or 1)
- **freq**: Frequency in Hz
- **vcaDepth**: Overall output level (0.0 - 1.0)
- **sawLevel, pulseLevel, subLevel, noiseLevel**: Oscillator mix (0.0 - 1.0 each)
- **vcfCutoff**: Filter cutoff frequency in Hz
- **vcfResonance**: Filter resonance (0.0 - 1.0)
- **attack, decay, sustain, release**: Envelope parameters
- **lfoRate**: LFO speed in Hz

## Limitations

This is a simplified implementation compared to the original Hera JUCE plugin:

- No MIDI input handling (use SuperCollider's MIDI classes)
- No preset management (use SuperCollider patterns and buses)
- Simplified filter model (basic one-pole instead of authentic Juno filter)
- Chorus effects are placeholders
- No arpeggiator (use SuperCollider patterns)
- Maximum 6-voice polyphony (hard-coded)

## License

GPL-3.0-or-later (same as the original Hera project)

## Credits

Based on the Hera synthesizer by Jean Pierre Cimalando (jpcima)
Original work inspired by pendragon-andyh's Juno60 and junox projects
