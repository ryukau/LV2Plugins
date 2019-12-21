// (c) 2019 Takamitsu Endo
//
// This file is part of IterativeSinOvertone.
//
// IterativeSinOvertone is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// IterativeSinOvertone is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with IterativeSinOvertone.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../parameter.hpp"
#include "constants.hpp"
#include "noise.hpp"
#include "oscillator.hpp"
#include "smoother.hpp"

#include "../../lib/vcl/vectorclass.h"

#include <array>
#include <cmath>
#include <memory>

using namespace SomeDSP;

constexpr size_t oscillatorSize = 4;

enum class NoteState { active, release, rest };

#define NOTE_CLASS(INSTRSET)                                                             \
  template<typename Sample> class Note_##INSTRSET {                                      \
  public:                                                                                \
    NoteState state = NoteState::rest;                                                   \
                                                                                         \
    Sample sampleRate = 44100;                                                           \
                                                                                         \
    int32_t id = -1;                                                                     \
    Sample normalizedKey = 0;                                                            \
    Sample velocity = 0;                                                                 \
    Sample gain = 0;                                                                     \
    Sample frequency = 0;                                                                \
                                                                                         \
    QuadOscExpAD<oscillatorSize> oscillator;                                             \
                                                                                         \
    void setup(Sample sampleRate);                                                       \
    void noteOn(                                                                         \
      int32_t noteId,                                                                    \
      Sample normalizedKey,                                                              \
      Sample frequency,                                                                  \
      Sample velocity,                                                                   \
      GlobalParameter &param,                                                            \
      White<float> &rng);                                                                \
    void release();                                                                      \
    void rest();                                                                         \
    Sample process();                                                                    \
  };

NOTE_CLASS(AVX512)
NOTE_CLASS(AVX2)
NOTE_CLASS(SSE41)
NOTE_CLASS(SSE2)

class DSPInterface {
public:
  virtual ~DSPInterface(){};

  static const size_t maxVoice = 32;
  GlobalParameter param;

  virtual void setup(double sampleRate) = 0;
  virtual void reset() = 0;   // Stop sounds.
  virtual void startup() = 0; // Reset phase, random seed etc.
  virtual void setParameters() = 0;
  virtual void process(const size_t length, float *out0, float *out1) = 0;
  virtual void noteOn(int32_t noteId, int16_t pitch, float tuning, float velocity) = 0;
  virtual void noteOff(int32_t noteId) = 0;

  struct MidiNote {
    bool isNoteOn;
    uint32_t frame;
    int32_t id;
    int16_t pitch;
    float tuning;
    float velocity;
  };

  std::vector<MidiNote> midiNotes;

  virtual void pushMidiNote(
    bool isNoteOn,
    uint32_t frame,
    int32_t noteId,
    int16_t pitch,
    float tuning,
    float velocity)
    = 0;
  virtual void processMidiNote(uint32_t frame) = 0;
};

/*
# About transitionBuffer
Transition happens when synth is playing all notes and user send a new note on.
transitionBuffer is used to store a release of a note to reduce pop noise.
*/
#define DSPCORE_CLASS(INSTRSET)                                                          \
  class DSPCore_##INSTRSET final : public DSPInterface {                                 \
  public:                                                                                \
    void setup(double sampleRate) override;                                              \
    void reset() override;                                                               \
    void startup() override;                                                             \
    void setParameters() override;                                                       \
    void process(const size_t length, float *out0, float *out1) override;                \
    void noteOn(int32_t noteId, int16_t pitch, float tuning, float velocity) override;   \
    void noteOff(int32_t noteId) override;                                               \
                                                                                         \
    void pushMidiNote(                                                                   \
      bool isNoteOn,                                                                     \
      uint32_t frame,                                                                    \
      int32_t noteId,                                                                    \
      int16_t pitch,                                                                     \
      float tuning,                                                                      \
      float velocity) override                                                           \
    {                                                                                    \
      MidiNote note;                                                                     \
      note.isNoteOn = isNoteOn;                                                          \
      note.frame = frame;                                                                \
      note.id = noteId;                                                                  \
      note.pitch = pitch;                                                                \
      note.tuning = tuning;                                                              \
      note.velocity = velocity;                                                          \
      midiNotes.push_back(note);                                                         \
    }                                                                                    \
                                                                                         \
    void processMidiNote(uint32_t frame) override                                        \
    {                                                                                    \
      while (true) {                                                                     \
        auto it                                                                          \
          = std::find_if(midiNotes.begin(), midiNotes.end(), [&](const MidiNote &nt) {   \
              return nt.frame == frame;                                                  \
            });                                                                          \
        if (it == std::end(midiNotes)) return;                                           \
        if (it->isNoteOn)                                                                \
          noteOn(it->id, it->pitch, it->tuning, it->velocity);                           \
        else                                                                             \
          noteOff(it->id);                                                               \
        midiNotes.erase(it);                                                             \
      }                                                                                  \
    }                                                                                    \
                                                                                         \
  private:                                                                               \
    float sampleRate = 44100.0f;                                                         \
                                                                                         \
    White<float> rng{0};                                                                 \
                                                                                         \
    size_t nVoice = 32;                                                                  \
    std::array<Note_##INSTRSET<float>, maxVoice> notes;                                  \
    float lastNoteFreq = 1.0f;                                                           \
                                                                                         \
    LinearSmoother<float> interpMasterGain;                                              \
                                                                                         \
    std::vector<float> transitionBuffer{};                                               \
    bool isTransitioning = false;                                                        \
    size_t trIndex = 0;                                                                  \
    size_t trStop = 0;                                                                   \
  };

DSPCORE_CLASS(AVX512)
DSPCORE_CLASS(AVX2)
DSPCORE_CLASS(SSE41)
DSPCORE_CLASS(SSE2)