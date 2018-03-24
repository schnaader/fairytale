/*
  This file is part of the Fairytale project

  Copyright (C) 2018 Márcio Pais

  This library is free software: you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef IMAGE_H
#define IMAGE_H

#include "stream.h"

class Image {
public:
  static bool isGrayscalePalette(Stream* input, const int n = 256, const bool isRGBA = false) {
    off_t offset = input->curPos();
    int stride = 3 + (int)isRGBA, res = (n > 0) << 8, order = 1;
    for (int i = 0; i < n * stride && (res >> 8) > 0; i++) {
      int b = input->getChar();
      if (b == EOF) {
        res = 0;
        break;
      }
      if (i == 0) {
        res = 0x100 | b;
        order = 1 - 2 * (b > 0);
        continue;
      }

      //"j" is the index of the current byte in this color entry
      int j = i % stride;
      if (j == 0) {
        // load first component of this entry
        res = (res & ((b - (res & 0xFF) == order) << 8));
        res |= (res) ? b : 0;
      }
      else if (j == 3)
        res &= ((b == 0 || b == 0xFF) << 9) - 1; // alpha/attribute component must be zero or 0xFF
      else
        res &= ((b == (res & 0xFF)) << 9) - 1;
    }
    input->setPos(offset);
    return (res >> 8) > 0;
  }
};

#endif
