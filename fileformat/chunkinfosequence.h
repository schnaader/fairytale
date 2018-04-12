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

#ifndef CHUNKINFOSEQUENCE_H
#define CHUNKINFOSEQUENCE_H

#include "../common.h"
#include "blockinfoentry.h"
#include "metadata.h"

class ChunkInfoSequence {
private:
  VLI size;
  uint32_t checksum;
  VLI codecSequenceId;
  VLI blockCount;
  VLI blockType;
  Metadata metadata;
  BlockInfoEntry BlockInfoEntries[];

public:
  int64_t getSize();
  uint32_t getChecksum();
  int64_t getCodecSequenceId();
  int64_t getBlockCount();
  int64_t getBlockType();
  Metadata getMetadata();
  BlockInfoEntry* getBlockInfoEntries;
};

#endif // CHUNKINFOSEQUENCE_H
