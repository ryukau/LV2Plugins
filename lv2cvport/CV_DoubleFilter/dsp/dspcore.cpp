// (c) 2019-2020 Takamitsu Endo
//
// This file is part of CV_DoubleFilter.
//
// CV_DoubleFilter is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// CV_DoubleFilter is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with CV_DoubleFilter.  If not, see <https://www.gnu.org/licenses/>.

#include "dspcore.hpp"

void DSPCore::setup(double sampleRate)
{
  this->sampleRate = sampleRate;

  SmootherCommon<float>::setSampleRate(sampleRate);
  SmootherCommon<float>::setTime(0.01f);

  reset();
}

void DSPCore::reset() { filter.reset(); }

void DSPCore::startup() {}

void DSPCore::setParameters()
{
  interpGain.push(param.value[ParameterID::gain]->getFloat());
  interpCutoff.push(param.value[ParameterID::cutoff]->getFloat());
  interpResonance.push(param.value[ParameterID::resonance]->getFloat());
}

void DSPCore::process(
  const size_t length,
  const float *in0,
  const float *inCutoff,
  const float *inResonance,
  float *out0)
{
  SmootherCommon<float>::setBufferSize(length);

  const bool uniformGain = param.value[ParameterID::uniformGain]->getInt();
  const bool highpass = param.value[ParameterID::highpass]->getInt();
  for (size_t i = 0; i < length; ++i) {
    SmootherCommon<float>::setBufferIndex(i);

    const float cutoff = interpCutoff.process();
    const float resonance = interpResonance.process();

    // 440 * pow(2, (111.07623199229747 - 69) / 12) ~= 5000 Hz.
    filter.set(
      sampleRate,
      std::clamp<float>(
        cutoff
          + 440.0f
            * powf(2.0f, (fabsf(inCutoff[i]) * 111.07623199229747f - 69.0f) / 12.0f),
        0.0f, 5000.0f),
      std::clamp<float>(resonance + inResonance[i], 0.0f, 1.0f), uniformGain);
    out0[i] = interpGain.process() * filter.process(in0[i], highpass);
  }
}
