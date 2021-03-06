// (c) 2019-2020 Takamitsu Endo
//
// This file is part of SyncSawSynth.
//
// SyncSawSynth is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SyncSawSynth is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with SyncSawSynth.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../common/dsp/constants.hpp"
#include "../../common/dsp/smoother.hpp"
#include "../../common/dsp/somemath.hpp"

namespace SomeDSP {

// t in [0, 1].
template<typename Sample> inline Sample cosinterp(Sample t)
{
  return 0.5 * (1.0 - somecos<Sample>(pi * t));
}

// When using float, time will be shorten.
// env(t) := exp(-beta * t)
template<typename Sample> class ExpADSREnvelope {
public:
  // attackTime, decayTime, releaseTime and declickTime are in seconds.
  // sustainLevel in [0, 1]. 0.0 < threshold < 1.0.
  ExpADSREnvelope(
    Sample sampleRate,
    Sample attackTime,
    Sample decayTime,
    Sample sustainLevel,
    Sample releaseTime,
    Sample declickTime = 0.001,
    Sample threshold = 1e-5)
    : sampleRate(sampleRate)
  {
    reset(attackTime, decayTime, sustainLevel, releaseTime, declickTime, threshold);
  }

  void reset(
    Sample attackTime,
    Sample decayTime,
    Sample sustainLevel,
    Sample releaseTime,
    Sample declickTime = 0.001,
    Sample threshold = 1e-5)
  {
    state = State::attack;
    value = threshold;
    lastAttack = value;
    adTransitionCounter = adTransitionLength - 1;
    set(attackTime, decayTime, sustainLevel, releaseTime, declickTime, threshold);
  }

  // This method is slow.
  void set(
    Sample attackTime,
    Sample decayTime,
    Sample sustainLevel,
    Sample releaseTime,
    Sample declickTime = 0.001,
    Sample threshold = 1e-5)
  {
    this->threshold = threshold;

    const auto sampleLength = Sample(4.0) / sampleRate;

    if (attackTime < sampleLength) attackTime = sampleLength;

    this->decayTime = (decayTime < sampleLength) ? sampleLength : decayTime;

    sustainLevel = std::max<Sample>(0.0, std::min<Sample>(sustainLevel, Sample(1.0)));
    sustain.push(sustainLevel);

    declickLength = int32_t(declickTime * sampleRate);

    if (releaseTime * sampleRate <= declickLength)
      releaseAlpha = threshold;
    else
      releaseAlpha = somepow<Sample>(
        threshold, Sample(1.0) / (releaseTime * sampleRate - declickLength));

    switch (state) {
      case State::attack:
        alpha = somepow<Sample>(
          Sample(1.0) / threshold, Sample(1.0) / (attackTime * sampleRate));
        break;

      case State::adTransition:
      case State::decay:
        alpha = somepow<Sample>(threshold, Sample(1.0) / (decayTime * sampleRate));
        break;

      case State::release:
        alpha = releaseAlpha;
        break;

      default:
        break;
    }
  }

  void release()
  {
    switch (state) {
      case State::attack:
      case State::adTransition:
        releaseRange = value;
        break;

      case State::decay:
        releaseRange = value - value * sustain.getValue() + sustain.getValue();
        break;

      case State::terminated:
        return;

      default:
        releaseRange = sustain.getValue();
        break;
    }

    value = Sample(1.0);
    alpha = releaseAlpha;
    state = State::release;
  }

  bool isAttacking() { return state == State::attack; }
  bool isTerminated() { return state == State::terminated; }

  Sample process()
  {
    sustain.process();

    Sample output;
    switch (state) {
      case State::attack:
        lastAttack = value;
        value *= alpha;
        if (value >= Sample(1.0)) {
          state = State::adTransition;
          value = lastAttack;
          adRange = Sample(1.0) - lastAttack;
          goto AD_TRANSITION;
        }
        output = value;
        break;

      case State::adTransition:
      AD_TRANSITION:
        adRange *= Sample(0.5);
        value += adRange;
        adTransitionCounter -= 1;
        if (adTransitionCounter < 0) {
          state = State::decay;
          alpha = somepow<Sample>(threshold, Sample(1.0) / (decayTime * sampleRate));
        }
        output = value;
        break;

      case State::decay:
        value *= alpha;
        output = value - value * sustain.getValue() + sustain.getValue();
        if (output > sustain.getValue() + threshold) break;
        state = State::sustain;
        break;

      case State::sustain:
        return sustain.getValue();

      case State::release:
        value *= alpha;
        if (value > threshold) {
          output = value * releaseRange;
          break;
        }
        value *= releaseRange;
        state = State::declickOut;
        output = value;
        break;

      case State::declickOut:
        value *= alpha;
        declickCounter -= 1;
        if (declickCounter > 0)
          return value * cosinterp<Sample>(declickCounter / (Sample)declickLength);
        value = 0.0;
        state = State::terminated;
        return value;

      default:
        output = 0;
        break;
    }

    if (state != State::declickOut && declickCounter < declickLength) {
      declickCounter += 1;
      output *= cosinterp<Sample>(declickCounter / (Sample)declickLength);
    }

    return output;
  }

protected:
  enum class State : int32_t {
    attack,
    adTransition,
    decay,
    sustain,
    release,
    declickOut,
    terminated
  };

  static const int32_t adTransitionLength = 16;

  int32_t declickLength;
  int32_t declickCounter = 0;
  int32_t adTransitionCounter = 0;

  State state = State::attack;
  Sample sampleRate;
  Sample lastAttack;
  Sample adRange;
  Sample decayTime;
  Sample releaseAlpha;
  Sample releaseRange = 1.0;
  Sample alpha;
  Sample value;
  Sample threshold = 1e-5;
  LinearSmoother<Sample> sustain;
};

template<typename Sample> class LinearEnvelope {
public:
  LinearEnvelope(
    Sample sampleRate,
    Sample attackTime,
    Sample decayTime,
    Sample sustainLevel,
    Sample releaseTime)
    : sampleRate(sampleRate)
  {
    reset(attackTime, decayTime, sustainLevel, releaseTime);
  }

  void reset(Sample attackTime, Sample decayTime, Sample sustainLevel, Sample releaseTime)
  {
    set(attackTime, decayTime, sustainLevel, releaseTime);
    state = State::attack;
  }

  void set(Sample attackTime, Sample decayTime, Sample sustainLevel, Sample releaseTime)
  {
    sustain = std::max<Sample>(0.0, std::min<Sample>(sustainLevel, 1.0));
    decayRange = 1.0 - sustain;

    attackDelta = Sample(1.0) / attackTime / sampleRate;
    decayDelta = Sample(1.0) / decayTime / sampleRate;
    releaseDelta = Sample(1.0) / releaseTime / sampleRate;
  }

  void release()
  {
    state = State::release;
    releaseRange = value;
  }

  bool isTerminated() { return state == State::terminated; }

  Sample process()
  {
    switch (state) {
      case State::attack:
        value += attackDelta;
        if (value >= 1.0) {
          state = State::decay;
          value = 1.0;
        }
        return value;

      case State::decay:
        value -= decayDelta * decayRange;
        if (value <= sustain) {
          state = State::sustain;
          value = sustain;
        }
        return value;

      case State::sustain:
        return sustain;

      case State::release:
        value -= releaseDelta * releaseRange;
        if (value < 0.0) {
          state = State::terminated;
          value = 0.0;
        }
        return value;

      default:
        break;
    }
    return 0.0;
  }

protected:
  enum class State : int32_t { attack, decay, sustain, release, terminated };
  State state = State::attack;
  Sample value = 0.0;
  Sample sampleRate;
  Sample sustain;
  Sample attackDelta;
  Sample decayDelta;
  Sample decayRange;
  Sample releaseDelta;
  Sample releaseRange;
};

// Almost always use double. float may be used if attack and curve is small.
// env(t) := t^alpha * exp(-beta * t)
// TODO: Find optimal range of alpha and beta.
template<typename Sample> class PolyExpEnvelope {
public:
  // attack is in seconds. curve is arbitrary value.
  PolyExpEnvelope(Sample sampleRate, Sample attack, Sample curve) : sampleRate(sampleRate)
  {
    reset(attack, curve);
  }

  bool isReleasing() { return time >= attack; }

  void reset(Sample attack, Sample curve)
  {
    alpha = attack * curve;
    peak = somepow<Sample>(alpha / curve, alpha) * someexp<Sample>(-alpha);
    gamma = someexp<Sample>(-curve / sampleRate);
    tick = 1.0 / sampleRate;

    time = 0.0;
    value = 1.0;
  }

  Sample process()
  {
    auto output = somepow<Sample>(time, alpha) * value / peak;
    if (!std::isfinite(output)) return 0.0; // TODO
    time += tick;
    value *= gamma;
    return output;
  }

protected:
  Sample sampleRate;
  Sample value;
  Sample peak;
  Sample gamma;
  Sample attack;
  Sample tick;
  Sample alpha;
  Sample time;
};

} // namespace SomeDSP
