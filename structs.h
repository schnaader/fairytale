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

#ifndef STRUCTS_H
#define STRUCTS_H

#include "common.h"

#define ZLIB_MAX_PENALTY_BYTES 64

struct DeflateInfo {
  int zlibParameters;
  uint8_t zlibCombination;
  uint8_t zlibWindow;
  uint8_t penaltyBytesUsed;
  uint8_t penaltyBytes[ZLIB_MAX_PENALTY_BYTES];
  uint32_t posDiff[ZLIB_MAX_PENALTY_BYTES];
  uint32_t lengthIn, lengthOut;
};

struct ImageInfo {
  int32_t width, height, stride;
  uint8_t bpp;
  bool grayscale;
};

struct AudioInfo {
  uint8_t channels, bps, mode;
};

struct TextInfo {
  off_t start;
  uint32_t lastSpace;
  uint32_t lastNL;
  uint32_t wordLength;
  uint32_t misses;
  uint32_t missCount;
};

struct ParseData {
  DeflateInfo deflate;
  ImageInfo image;
  AudioInfo audio;
  TextInfo text;
};

#endif