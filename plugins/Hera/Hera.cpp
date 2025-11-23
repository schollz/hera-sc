// SPDX-License-Identifier: GPL-3.0-or-later
// Hera - Juno 60 emulation synthesizer for SuperCollider
// Based on https://github.com/jpcima/Hera

#include "SC_PlugIn.h"
#include <cmath>
#include <cstring>

static InterfaceTable *ft;

// Maximum number of simultaneously playing notes
const int kMaxVoices = 6;

// Envelope generator
struct Envelope {
    enum Stage { Idle, Attack, Decay, Sustain, Release };
    
    Stage stage;
    float level;
    float attackRate;
    float decayRate;
    float sustainLevel;
    float releaseRate;
    
    void init() {
        stage = Idle;
        level = 0.0f;
        attackRate = 0.001f;
        decayRate = 0.001f;
        sustainLevel = 0.7f;
        releaseRate = 0.001f;
    }
    
    void noteOn() {
        stage = Attack;
    }
    
    void noteOff() {
        if (stage != Idle) {
            stage = Release;
        }
    }
    
    float next() {
        switch (stage) {
            case Idle:
                return 0.0f;
                
            case Attack:
                level += attackRate;
                if (level >= 1.0f) {
                    level = 1.0f;
                    stage = Decay;
                }
                return level;
                
            case Decay:
                level -= decayRate;
                if (level <= sustainLevel) {
                    level = sustainLevel;
                    stage = Sustain;
                }
                return level;
                
            case Sustain:
                return level;
                
            case Release:
                level -= releaseRate;
                if (level <= 0.0f) {
                    level = 0.0f;
                    stage = Idle;
                }
                return level;
        }
        return 0.0f;
    }
};

// Simple LFO
struct LFO {
    float phase;
    float rate;
    
    void init() {
        phase = 0.0f;
        rate = 0.0f;
    }
    
    float next(float sampleRate) {
        phase += rate / sampleRate;
        if (phase >= 1.0f) phase -= 1.0f;
        // Sine wave LFO
        return sinf(phase * 2.0f * M_PI);
    }
};

// Simple DCO (Digitally Controlled Oscillator)
struct DCO {
    float phase;
    float frequency;
    float sawLevel;
    float pulseLevel;
    float subLevel;
    float noiseLevel;
    float pwmDepth;
    unsigned int noiseState;
    
    void init() {
        phase = 0.0f;
        frequency = 440.0f;
        sawLevel = 1.0f;
        pulseLevel = 0.0f;
        subLevel = 0.0f;
        noiseLevel = 0.0f;
        pwmDepth = 0.5f;
        noiseState = 1;
    }
    
    float nextSaw(float sampleRate) {
        phase += frequency / sampleRate;
        if (phase >= 1.0f) phase -= 1.0f;
        return (2.0f * phase - 1.0f);
    }
    
    float nextPulse(float sampleRate, float pulseWidth) {
        return (phase < pulseWidth) ? 1.0f : -1.0f;
    }
    
    float nextSub(float sampleRate) {
        // Sub oscillator is one octave down (square wave)
        float subPhase = fmodf(phase * 0.5f, 1.0f);
        return (subPhase < 0.5f) ? 1.0f : -1.0f;
    }
    
    float nextNoise() {
        // Simple white noise generator
        noiseState = noiseState * 1103515245 + 12345;
        return ((noiseState >> 16) & 0x7FFF) / 16384.0f - 1.0f;
    }
    
    float next(float sampleRate) {
        float output = 0.0f;
        
        if (sawLevel > 0.0f) {
            output += nextSaw(sampleRate) * sawLevel;
        }
        
        if (pulseLevel > 0.0f) {
            output += nextPulse(sampleRate, pwmDepth) * pulseLevel;
        }
        
        if (subLevel > 0.0f) {
            output += nextSub(sampleRate) * subLevel;
        }
        
        if (noiseLevel > 0.0f) {
            output += nextNoise() * noiseLevel;
        }
        
        return output * 0.25f; // Scale down to avoid clipping
    }
};

// Simple one-pole lowpass filter (VCF)
struct VCF {
    float z1;
    float cutoff;
    float resonance;
    
    void init() {
        z1 = 0.0f;
        cutoff = 1000.0f;
        resonance = 0.0f;
    }
    
    float process(float input, float sampleRate) {
        float fc = cutoff / sampleRate;
        fc = fc < 0.5f ? fc : 0.5f;
        float omega = 2.0f * M_PI * fc;
        float cosOmega = cosf(omega);
        float alpha = 1.0f - cosOmega;
        
        // Add resonance feedback
        float feedback = resonance * 4.0f;
        float inputWithFeedback = input + z1 * feedback;
        
        z1 = z1 + alpha * (inputWithFeedback - z1);
        return z1;
    }
};

// Voice structure
struct Voice {
    bool active;
    int note;
    DCO dco;
    VCF vcf;
    Envelope envelope;
    
    void init() {
        active = false;
        note = -1;
        dco.init();
        vcf.init();
        envelope.init();
    }
    
    void noteOn(int midiNote, float velocity) {
        active = true;
        note = midiNote;
        dco.frequency = 440.0f * powf(2.0f, (midiNote - 69) / 12.0f);
        dco.phase = 0.0f;
        envelope.noteOn();
    }
    
    void noteOff() {
        envelope.noteOff();
    }
};

// Main Hera UGen structure
struct Hera : public Unit {
    // Voices
    Voice voices[kMaxVoices];
    
    // Global LFO
    LFO lfo;
    
    // Parameters (mapped to control inputs)
    float vcaDepth;
    int vcaType;
    float pwmDepth;
    int pwmMod;
    float sawLevel;
    float pulseLevel;
    float subLevel;
    float noiseLevel;
    int pitchRange;
    float pitchModDepth;
    float vcfCutoff;
    float vcfResonance;
    float vcfEnvModDepth;
    float vcfLFOModDepth;
    float vcfKeyModDepth;
    float vcfBendDepth;
    float attack;
    float decay;
    float sustain;
    float release;
    int lfoTrigMode;
    float lfoRate;
    float lfoDelay;
    float hpf;
    bool chorusI;
    bool chorusII;
    
    // Gate state for note on/off detection
    float lastGate;
};

// Forward declarations
static void Hera_next(Hera *unit, int inNumSamples);
static void Hera_Ctor(Hera *unit);

// Constructor
void Hera_Ctor(Hera *unit) {
    // Initialize voices
    for (int i = 0; i < kMaxVoices; i++) {
        unit->voices[i].init();
    }
    
    // Initialize LFO
    unit->lfo.init();
    
    // Initialize parameters to defaults
    unit->vcaDepth = 0.5f;
    unit->vcaType = 0;
    unit->pwmDepth = 0.5f;
    unit->pwmMod = 0;
    unit->sawLevel = 1.0f;
    unit->pulseLevel = 0.0f;
    unit->subLevel = 0.0f;
    unit->noiseLevel = 0.0f;
    unit->pitchRange = 1;
    unit->pitchModDepth = 0.0f;
    unit->vcfCutoff = 1000.0f;
    unit->vcfResonance = 0.0f;
    unit->vcfEnvModDepth = 0.0f;
    unit->vcfLFOModDepth = 0.0f;
    unit->vcfKeyModDepth = 0.0f;
    unit->vcfBendDepth = 0.0f;
    unit->attack = 0.01f;
    unit->decay = 0.3f;
    unit->sustain = 0.7f;
    unit->release = 0.5f;
    unit->lfoTrigMode = 0;
    unit->lfoRate = 5.0f;
    unit->lfoDelay = 0.0f;
    unit->hpf = 0.0f;
    unit->chorusI = false;
    unit->chorusII = false;
    unit->lastGate = 0.0f;
    
    SETCALC(Hera_next);
    Hera_next(unit, 1);
}

// Parameter update helper
static void Hera_updateParams(Hera *unit) {
    // Read all control parameters
    // Input 0: gate (trigger)
    // Input 1: freq (pitch in Hz)
    // Input 2-26: all Hera parameters
    
    int paramIdx = 2;
    unit->vcaDepth = IN0(paramIdx++);
    unit->vcaType = (int)IN0(paramIdx++);
    unit->pwmDepth = IN0(paramIdx++);
    unit->pwmMod = (int)IN0(paramIdx++);
    unit->sawLevel = IN0(paramIdx++);
    unit->pulseLevel = IN0(paramIdx++);
    unit->subLevel = IN0(paramIdx++);
    unit->noiseLevel = IN0(paramIdx++);
    unit->pitchRange = (int)IN0(paramIdx++);
    unit->pitchModDepth = IN0(paramIdx++);
    unit->vcfCutoff = IN0(paramIdx++);
    unit->vcfResonance = IN0(paramIdx++);
    unit->vcfEnvModDepth = IN0(paramIdx++);
    unit->vcfLFOModDepth = IN0(paramIdx++);
    unit->vcfKeyModDepth = IN0(paramIdx++);
    unit->vcfBendDepth = IN0(paramIdx++);
    unit->attack = IN0(paramIdx++);
    unit->decay = IN0(paramIdx++);
    unit->sustain = IN0(paramIdx++);
    unit->release = IN0(paramIdx++);
    unit->lfoTrigMode = (int)IN0(paramIdx++);
    unit->lfoRate = IN0(paramIdx++);
    unit->lfoDelay = IN0(paramIdx++);
    unit->hpf = IN0(paramIdx++);
    unit->chorusI = IN0(paramIdx++) > 0.5f;
    unit->chorusII = IN0(paramIdx++) > 0.5f;
    
    // Update LFO
    unit->lfo.rate = unit->lfoRate;
    
    // Update envelope parameters for all voices
    float sampleRate = SAMPLERATE;
    for (int i = 0; i < kMaxVoices; i++) {
        Voice &voice = unit->voices[i];
        voice.envelope.attackRate = 1.0f / (unit->attack * sampleRate + 1.0f);
        voice.envelope.decayRate = 1.0f / (unit->decay * sampleRate + 1.0f);
        voice.envelope.sustainLevel = unit->sustain;
        voice.envelope.releaseRate = 1.0f / (unit->release * sampleRate + 1.0f);
        
        // Update oscillator levels
        voice.dco.sawLevel = unit->sawLevel;
        voice.dco.pulseLevel = unit->pulseLevel;
        voice.dco.subLevel = unit->subLevel;
        voice.dco.noiseLevel = unit->noiseLevel;
        voice.dco.pwmDepth = unit->pwmDepth;
    }
}

// Calculation function
void Hera_next(Hera *unit, int inNumSamples) {
    float *out = OUT(0);
    float *gate = IN(0);
    float *freq = IN(1);
    
    float sampleRate = SAMPLERATE;
    
    // Update parameters
    Hera_updateParams(unit);
    
    // Process each sample
    for (int i = 0; i < inNumSamples; i++) {
        // Handle gate/trigger for voice allocation
        float gateVal = gate[i];
        float freqVal = freq[i];
        
        // Simple voice allocation on positive gate
        if (gateVal > 0.5f && unit->lastGate <= 0.5f) {
            // Note on - find free voice
            int voiceIdx = -1;
            for (int v = 0; v < kMaxVoices; v++) {
                if (!unit->voices[v].active || unit->voices[v].envelope.stage == Envelope::Idle) {
                    voiceIdx = v;
                    break;
                }
            }
            if (voiceIdx >= 0) {
                int midiNote = 69 + 12.0f * log2f(freqVal / 440.0f);
                unit->voices[voiceIdx].noteOn(midiNote, 1.0f);
            }
        } else if (gateVal <= 0.5f && unit->lastGate > 0.5f) {
            // Note off - release all active voices
            for (int v = 0; v < kMaxVoices; v++) {
                if (unit->voices[v].active) {
                    unit->voices[v].noteOff();
                }
            }
        }
        unit->lastGate = gateVal;
        
        // Update LFO
        float lfoValue = unit->lfo.next(sampleRate);
        
        // Mix all voices
        float sample = 0.0f;
        int activeVoices = 0;
        
        for (int v = 0; v < kMaxVoices; v++) {
            Voice &voice = unit->voices[v];
            
            if (!voice.active && voice.envelope.stage == Envelope::Idle) {
                continue;
            }
            
            // Generate oscillator output
            float dcoOut = voice.dco.next(sampleRate);
            
            // Get envelope value
            float envValue = voice.envelope.next();
            
            // Calculate filter cutoff with modulation
            float baseCutoff = unit->vcfCutoff;
            float envMod = unit->vcfEnvModDepth * envValue * 5000.0f;
            float lfoMod = unit->vcfLFOModDepth * lfoValue * 2000.0f;
            float finalCutoff = baseCutoff + envMod + lfoMod;
            finalCutoff = finalCutoff < 20.0f ? 20.0f : (finalCutoff > 20000.0f ? 20000.0f : finalCutoff);
            
            voice.vcf.cutoff = finalCutoff;
            voice.vcf.resonance = unit->vcfResonance;
            
            // Apply filter
            float vcfOut = voice.vcf.process(dcoOut, sampleRate);
            
            // Apply VCA (envelope)
            float vcaOut = vcfOut * envValue * unit->vcaDepth;
            
            sample += vcaOut;
            activeVoices++;
            
            // Mark voice as inactive if envelope is done
            if (voice.envelope.stage == Envelope::Idle) {
                voice.active = false;
            }
        }
        
        // Normalize by number of voices to avoid clipping
        if (activeVoices > 0) {
            sample /= sqrtf((float)activeVoices);
        }
        
        out[i] = sample;
    }
}

// Plugin entry point
PluginLoad(Hera) {
    ft = inTable;
    DefineSimpleUnit(Hera);
}
