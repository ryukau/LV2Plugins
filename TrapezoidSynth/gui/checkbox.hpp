// (c) 2019 Takamitsu Endo
//
// This file is part of TrapezoidSynth.
//
// TrapezoidSynth is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// TrapezoidSynth is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with TrapezoidSynth.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "valuewidget.hpp"

class CheckBox : public ValueWidget {
public:
  explicit CheckBox(NanoWidget *group, PluginUI *ui, const char *labelText, FontId fontId)
    : ValueWidget(group, ui, 0.0f), labelText(labelText), fontId(fontId)
  {
  }

  void onNanoDisplay() override
  {
    resetTransform();
    translate(getAbsoluteX(), getAbsoluteY());

    const auto height = getHeight();
    const auto centerY = height / 2;

    // Background.
    beginPath();
    rect(0, 0, getWidth(), height);
    fillColor(colorBack);
    fill();

    // Box.
    beginPath();
    rect(2, centerY - boxSize / 2, boxSize, boxSize);
    strokeColor(isMouseEntered ? highlightColor : colorFore);
    strokeWidth(2.0f);
    stroke();

    if (value) {
      const auto innerBoxSize = boxSize - 4;
      beginPath();
      rect(
        2 + (boxSize - innerBoxSize) / 2, centerY - innerBoxSize / 2, innerBoxSize,
        innerBoxSize);
      fillColor(isMouseEntered ? highlightColor : colorFore);
      fill();
    }

    // Text.
    if (labelText == nullptr) return;
    fillColor(colorFore);
    fontFaceId(fontId);
    fontSize(textSize);
    textAlign(align);
    text(boxSize + textSize / 2.0f, height / 2, labelText, nullptr);
  }

  bool onMouse(const MouseEvent &ev) override
  {
    if (ev.press && contains(ev.pos)) {
      value = value == 0.0;
      updateValue();
      repaint();
      return true;
    }
    return false;
  }

  bool onMotion(const MotionEvent &ev) override
  {
    isMouseEntered = contains(ev.pos);
    repaint();
    return false;
  }

  bool onScroll(const ScrollEvent &ev) override
  {
    if (!contains(ev.pos)) return false;
    if (ev.delta.getY() < 0)
      value = true;
    else if (ev.delta.getY() > 0)
      value = false;
    updateValue();
    repaint();
    return true;
  }

  void setHighlightColor(Color color) { highlightColor = color; }
  void setForegroundColor(Color color) { colorFore = color; }
  void setTextSize(float size) { textSize = size < 0.0f ? 0.0f : size; }

protected:
  Color highlightColor{0x33, 0xaa, 0xff};
  Color colorFore{0, 0, 0};
  Color colorBack{0xff, 0xff, 0xff};

  const char *labelText = nullptr;
  int align = ALIGN_LEFT | ALIGN_MIDDLE;
  int32_t boxSize = 10;
  float textSize = 14.0f;
  FontId fontId = -1;

  bool isMouseEntered = false;
};