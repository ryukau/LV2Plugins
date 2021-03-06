# Experimental Plugins
Experimental Plugins are not well tuned. They are also unstable in term of development. Expect breaking changes in future.

## Building
Change `build` rule in the top level `Makefile` in this repository as following.

```make
# build: common \
# 	lv2cvport \
# 	CubicPadSynth \
#   ...

build: experimental
```

## CollidingCombSynth
Based on [web version](https://ryukau.github.io/KSCymbal/). The idea is to collide Karplus-Strong string model.

Max 192kHz sample rate is supported.

Note:
- Added decay to exciter.
- Added lowpass cutoff envelope to gate and terminate note.
- Added per note compressor.
- Added naive tri-saw to excitation.
- Changed name from KSCymbal to CollidingCombSynth.
- Changed one-zero lowpass to one-pole lowpass.
- Changed from RCHP to one-pole highpass.
- Changed to 8 voice polyphonic.
- Changed upper bound of HP Cutoff from 4000Hz to 20000Hz.
- Frequency randomization is must for cymbal-ish sound.
- Increasing K-S string makes dense decay. 24 or higher is better.
- Increasing comb filter didn't sound good for this model.
- Oversampling kind of improved sound, but too heavy.

## KuramotoModel
Based on [Kuramoto model](https://en.wikipedia.org/wiki/Kuramoto_model) of synchronization.

Original Kuramoto model is a bit too naive for musical application. Adding decay to Kuramoto model improved the sound.

The sound is similar to FM.

TODO list:
- Implement full pitch bend.
- Improve UI. Especially for oscillator pitch control.
- Tune parameter.

Note:
- Adding overtone is inefficient.
- Oversampling didn't help much to reduce noise.
- Strong coupling tends to converge too fast.
- VCL helps to speed up.
- Wavetable is hard to use with Kuramoto model.
  - Antialiasing isn't working because of phase modulation.
  - Naive wavetable tends to be too noisy.

## BubbleSynth
It sounds like bubble when turning up `Xi` parameter.

I thought this might be good for hand clap synth.

## NoiseTester
Outputs gaussian/uniform noise signal.

## RingDelaySynth
**Caution**: Output may blow up.

16 delays connected as a ring. The output of the ring will be fed into 8 cascaded Karplus-Strong delay. The ring also has a feedback.

This was an attempt to make a bass drum synthesizer, but the result was nowhere near close.
