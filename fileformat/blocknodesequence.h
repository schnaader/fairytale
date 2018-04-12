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

#ifndef BLOCKNODESEQUENCE_H
#define BLOCKNODESEQUENCE_H

#include "../common.h"
#include "blocknodeentry.h"
#include "metadata.h"

class BlockNodeSequence {
private:
  VLI blockCount;
  VLI blockType;
  Metadata metadata;
  BlockNodeEntry BlockNodeEntries[];

public:
  int64_t getBlockCount();
  int64_t getBlockType();
  Metadata getMetadata();
  BlockNodeEntry* getBlockNodeEntries;
};

#endif // BLOCKNODESEQUENCE_H
