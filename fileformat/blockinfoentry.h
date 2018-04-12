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

#ifndef BLOCKINFOENTRY_H
#define BLOCKINFOENTRY_H

#include "../common.h"

class BlockInfoEntry {
private:
  VLI size;
  uint8_t info[];

public:
  int64_t getSize();
  uint8_t* getInfo();
};

#endif // BLOCKINFOENTRY_H
