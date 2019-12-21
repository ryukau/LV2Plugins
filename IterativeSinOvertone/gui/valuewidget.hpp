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

#include "../ui.hpp"
#include "Widget.hpp"

#include <iostream>
#include <vector>

class ValueWidget : public NanoWidget {
public:
  uint32_t id;

  explicit ValueWidget(NanoWidget *group, PluginUI *ui, double value)
    : NanoWidget(group), ui(ui), value(value)
  {
  }

  virtual double getValue() { return value; }
  virtual void setValue(double value) { this->value = value; }
  virtual void updateValue()
  {
    if (ui == nullptr) return;
    ui->updateValue(id, value);
  }

protected:
  PluginUI *ui = nullptr;
  double value = 0.0;
};

class ArrayWidget : public NanoWidget {
protected:
  PluginUI *ui = nullptr;

public:
  std::vector<uint32_t> id;
  std::vector<double> value;

  explicit ArrayWidget(
    NanoWidget *group, PluginUI *ui, std::vector<uint32_t> id, std::vector<double> value)
    : NanoWidget(group), ui(ui), id(id), value(value)
  {
    if (id.size() != value.size())
      std::cout << "Error ArrayWidget::ArrayWidget: Mismatch size of id and value."
                << std::endl;
  }

  virtual double getValueAt(size_t index)
  {
    if (index >= value.size()) return 0;
    return value[index];
  }

  virtual void setValueAt(size_t index, double normalized)
  {
    if (index < value.size())
      value[index] = normalized < 0.0 ? 0.0 : normalized > 1.0 ? 1.0 : normalized;
  }

  virtual void updateValueAt(size_t index)
  {
    if (ui == nullptr) return;
    ui->updateValue(id[index], value[index]);
  }

  virtual void updateValue()
  {
    if (ui == nullptr || id.size() != value.size()) return;
    for (size_t i = 0; i < id.size(); ++i) ui->updateValue(id[i], value[i]);
  }
};