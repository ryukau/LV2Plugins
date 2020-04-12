// (c) 2019-2020 Takamitsu Endo
//
// This file is part of ModuloShaper.
//
// ModuloShaper is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// ModuloShaper is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with ModuloShaper.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <memory>
#include <string>
#include <vector>

#include <iostream>

#include "../common/dsp/constants.hpp"
#include "../common/dsp/scale.hpp"
#include "../common/value.hpp"

#ifdef TEST_BUILD
static const uint32_t kParameterIsAutomable = 0x01;
static const uint32_t kParameterIsBoolean = 0x02;
static const uint32_t kParameterIsInteger = 0x04;
static const uint32_t kParameterIsLogarithmic = 0x08;
#endif

constexpr int32_t nOvertone = 360;
constexpr int32_t nLFOWavetable = 64;
constexpr int32_t tableSize = 262144;
constexpr int32_t lfoTableSize = 1023;
constexpr int32_t spectrumSize = tableSize / 2 + 1;

namespace ParameterID {
enum ID {
  bypass,

  inputGain,
  clipGain,
  outputGain,
  add,
  mul,
  type,
  hardclip,

  lowpass,
  lowpassCutoff,

  smoothness,

  ID_ENUM_LENGTH,
};
} // namespace ParameterID

struct Scales {
  static SomeDSP::IntScale<double> boolScale;
  static SomeDSP::LinearScale<double> defaultScale;

  static SomeDSP::LogScale<double> inputGain;
  static SomeDSP::LinearScale<double> add;
  static SomeDSP::LinearScale<double> mul;
  static SomeDSP::LogScale<double> outputGain;
  static SomeDSP::IntScale<double> type;

  static SomeDSP::LogScale<double> lowpassCutoff;

  static SomeDSP::LogScale<double> smoothness;
};

struct GlobalParameter {
  std::vector<std::unique_ptr<ValueInterface>> value;

  GlobalParameter()
  {
    value.resize(ParameterID::ID_ENUM_LENGTH);

    using ID = ParameterID::ID;
    using LinearValue = FloatValue<SomeDSP::LinearScale<double>>;
    using LogValue = FloatValue<SomeDSP::LogScale<double>>;
    // using SPolyValue = FloatValue<SomeDSP::SPolyScale<double>>;
    // using DecibelValue = FloatValue<SomeDSP::DecibelScale<double>>;

    value[ID::bypass] = std::make_unique<IntValue>(
      0, Scales::boolScale, "bypass", kParameterIsAutomable | kParameterIsBoolean);

    value[ID::inputGain] = std::make_unique<LogValue>(
      0.5, Scales::inputGain, "inputGain", kParameterIsAutomable);
    value[ID::add]
      = std::make_unique<LinearValue>(0.5, Scales::add, "add", kParameterIsAutomable);
    value[ID::mul]
      = std::make_unique<LinearValue>(0.5, Scales::mul, "mul", kParameterIsAutomable);
    value[ID::clipGain] = std::make_unique<LogValue>(
      1.0, Scales::outputGain, "clipGain", kParameterIsAutomable);
    value[ID::outputGain] = std::make_unique<LogValue>(
      0.5, Scales::outputGain, "outputGain", kParameterIsAutomable);
    value[ID::type]
      = std::make_unique<IntValue>(3, Scales::type, "type", kParameterIsAutomable);
    value[ID::hardclip] = std::make_unique<IntValue>(
      true, Scales::boolScale, "hardclip", kParameterIsAutomable | kParameterIsBoolean);

    value[ID::lowpass] = std::make_unique<IntValue>(
      true, Scales::boolScale, "lowpass", kParameterIsAutomable | kParameterIsBoolean);
    value[ID::lowpassCutoff] = std::make_unique<LogValue>(
      1.0, Scales::lowpassCutoff, "lowpassCutoff", kParameterIsAutomable);

    value[ID::smoothness] = std::make_unique<LogValue>(
      0.1, Scales::smoothness, "smoothness", kParameterIsAutomable);
  }

#ifndef TEST_BUILD
  void initParameter(uint32_t index, Parameter &parameter)
  {
    if (index >= value.size()) return;
    value[index]->setParameterRange(parameter);
  }
#endif

  void resetParameter()
  {
    for (auto &val : value) val->setFromNormalized(val->getDefaultNormalized());
  }

  double getParameterValue(uint32_t index) const
  {
    if (index >= value.size()) return 0.0;
    return value[index]->getFloat();
  }

  void setParameterValue(uint32_t index, float raw)
  {
    if (index >= value.size()) return;
    value[index]->setFromFloat(raw);
  }

  double parameterChanged(uint32_t index, float raw)
  {
    if (index >= value.size()) return 0.0;
    value[index]->setFromFloat(raw);
    return value[index]->getNormalized();
  }

  double updateValue(uint32_t index, float normalized)
  {
    if (index >= value.size()) return 0.0;
    value[index]->setFromNormalized(normalized);
    return value[index]->getFloat();
  }

  enum Preset {
    presetDefault,

    Preset_ENUM_LENGTH,
  };

  std::array<const char *, 1> programName{
    "Default",
  };

#ifndef TEST_BUILD
  void initProgramName(uint32_t index, String &programName)
  {
    programName = this->programName[index];
  }

  void loadProgram(uint32_t index);
#endif

  void validate()
  {
    for (size_t i = 0; i < value.size(); ++i) {
      if (value[i] == nullptr) {
        std::cout << "PluginError: GlobalParameter::value[" << std::to_string(i)
                  << "] is nullptr. Forgetting initialization?\n";
        std::exit(EXIT_FAILURE);
      }
    }
  }
};