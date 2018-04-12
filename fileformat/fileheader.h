/*
  This file is part of the Fairytale project

  Copyright (C) 2018 Andrew Epstein

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

#ifndef FILEHEADER_H
#define FILEHEADER_H

#include "../common.h"

class FileHeader {
private:
  uint8_t magicSignature[3];
  uint8_t version;
  uint8_t flags;
  int64_t dataSize;

public:
  uint8_t* getMagicSignature();
  uint8_t getVersion();
  uint8_t getFlags();
  bool checkFlag(const uint8_t flagId) {
    return (flags & flagId) != 0;
  }
  int64_t getDataSize();
};

#endif // FILEHEADER_H
