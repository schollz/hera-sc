# hera-sc

SuperCollider plugin implementation of the Hera synthesizer, based on [jpcima/Hera](https://github.com/jpcima/Hera).

## Overview

This project provides a self-contained Juno-60 style synthesizer as a SuperCollider UGen. The plugin includes all core synthesis parameters from the original Hera project, excluding MIDI handling, arpeggiator, and preset management (which are better handled by SuperCollider itself).

## Building

```bash
make
```

The Makefile will automatically:
1. Check if SuperCollider source is available
2. If not, clone https://github.com/supercollider/supercollider
3. Checkout Version-3.14.0
4. Initialize submodules recursively
5. Build the Hera plugin

See `plugins/Hera/README.md` for detailed documentation.

## Quick Start

```supercollider
// After installing, test with:
{ Hera.ar(gate: 1, freq: 440, sawLevel: 1.0) }.play;
```

## License

GPL-3.0-or-later