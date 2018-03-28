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

#ifndef BLOCK_H
#define BLOCK_H

#include "storagemanager.h"

enum class BlockType : uint8_t
{
  DEFAULT = 0,
  DEDUP,
  DEFLATE,
  JPEG,
  IMAGE,
  AUDIO,
  TEXT,
  DDS,
  JSON,
  Count
};

class Block {
public:
  int64_t id, length;
  off_t offset;
  Stream* data;
  Block *parent, *next, *child;
  void* info;
  BlockType type;
  uint32_t refCount, hash;
  uint8_t level;
  bool done, hashed;
  bool attemptRevival(StorageManager* manager);
  Block* segmentAround(const off_t pos, const int64_t size, const BlockType newType, void* blInfo = nullptr, const size_t sizeInfo = 0, Stream* childStream = nullptr, const BlockType childType = BlockType::DEFAULT, const bool childDone = false, void* childInfo = nullptr, const size_t childSizeInfo = 0);
  Block* nextAtLevel(const uint32_t l, const bool skipDone = true);
  void freeInfo();
  void freeChilds(StorageManager* manager);
  void calculateHash();
};

#endif
