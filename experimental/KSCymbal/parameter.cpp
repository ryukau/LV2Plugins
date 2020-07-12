// (c) 2020 Takamitsu Endo
//
// This file is part of KSCymbal.
//
// KSCymbal is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// KSCymbal is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with KSCymbal.  If not, see <https://www.gnu.org/licenses/>.

#include "parameter.hpp"
#include "../../common/dsp/constants.hpp"

using namespace SomeDSP;

IntScale<double> Scales::boolScale(1);
LinearScale<double> Scales::defaultScale(0.0, 1.0);

LogScale<double> Scales::gain(0.0, 1.0, 0.5, 0.2);
LinearScale<double> Scales::boost(0.0, 4.0);

LogScale<double> Scales::decay(0.0, 8.0, 0.5, 1.0);
LogScale<double> Scales::attack(0.001, 0.2, 0.5, 0.1);

LinearScale<double> Scales::combTime(0.0001, 0.002);
LogScale<double> Scales::frequency(12.0, 1000.0, 0.5, 100.0);
LogScale<double> Scales::lowpassCutoffHz(20.0, 20000.0, 0.5, 1000.0);

LogScale<double> Scales::distance(0.002, 0.2, 0.5, 0.02);
IntScale<double> Scales::seed(16777215); // 2^24 - 1

IntScale<double> Scales::octave(16);
IntScale<double> Scales::semitone(168);
IntScale<double> Scales::milli(2000);
IntScale<double> Scales::equalTemperament(119);