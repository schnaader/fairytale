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

#ifndef CODECDEFINITION_H
#define CODECDEFINITION_H

#include "../common.h"
#include "codecsequenceentry.h"

class CodecDefinition {
private:
  VLI numberOfCodecSequenceEntries;
  CodecEntry CodecSequenceEntries[];

public:
  int64_t getNumberOfCodecSequenceEntries();
  CodecEntry* getCodecSequenceEntries;
};

#endif // CODECDEFINITION_H
