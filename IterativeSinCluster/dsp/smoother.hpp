// (c) 2019 Takamitsu Endo
//
// This file is part of IterativeSinCluster.
//
// IterativeSinCluster is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// IterativeSinCluster is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with IterativeSinCluster.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "somemath.hpp"

#include <array>

namespace SomeDSP {

template<typename Sample> class LinearSmoother {
public:
  static void setSampleRate(Sample _sampleRate, Sample time = 0.04)
  {
    sampleRate = _sampleRate;
    setTime(time);
  }

  static void setTime(Sample seconds) { timeInSamples = seconds * sampleRate; }
  static void setBufferSize(Sample _bufferSize) { bufferSize = _bufferSize; }

  virtual void push(Sample newTarget)
  {
    target = newTarget;
    if (timeInSamples < bufferSize)
      value = target;
    else
      ramp = (target - value) / timeInSamples;
  }

  inline Sample getValue() { return value; }

  virtual Sample process()
  {
    if (value == target) return value;
    value += ramp;

    auto diff = value - target;
    if (somefabs<Sample>(diff) < 1e-5) value = target;
    return value;
  }

protected:
  static Sample sampleRate;
  static Sample timeInSamples;
  static Sample bufferSize;
  Sample value = 1.0;
  Sample target = 1.0;
  Sample ramp = 0.0;
};

template<typename Sample> Sample LinearSmoother<Sample>::sampleRate = 44100.0;
template<typename Sample> Sample LinearSmoother<Sample>::timeInSamples = 0.0;
template<typename Sample> Sample LinearSmoother<Sample>::bufferSize = 44100.0;

// PID controller without I and D.
template<typename Sample> class PController {
public:
  // p in [0, 1].
  void setup(Sample sampleRate, Sample p)
  {
    this->sampleRate = sampleRate;
    kp = p;
  };

  void reset() { value = 0; }

  Sample process(Sample input)
  {
    value += kp * (input - value);
    return value;
  }

private:
  Sample sampleRate = 44100;
  Sample kp;
  Sample value = 0;
};

} // namespace SomeDSP
