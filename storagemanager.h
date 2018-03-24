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

#ifndef STORAGEMANAGER_H
#define STORAGEMANAGER_H

#include "hybridstream.h"

#define MEM_BLOCK_SIZE (0x100ull)                  //256 bytes
#define DEFAULT_TEMP_MEM_PER_STREAM (0x0400000ull) //4MB
#define FORCE_PURGE_MAXIMUM 8u
#define PURGE_SLIDING_WINDOW 4
#define SIZE_BLOCKS(x) ((x) + (MEM_BLOCK_SIZE - 1)) & (~(MEM_BLOCK_SIZE - 1));

class StorageManager final {
private:
  Array<HybridStream*> streams;
  struct {
    size_t memory;
    int64_t total;
  } available, limit;
  size_t pruneIndex;
  void doRefBiasedPurge(const int64_t storageRequested);
  void doForcedPurge(uint32_t purgeRequested = FORCE_PURGE_MAXIMUM);

public:
  StorageManager(const size_t maxMemUsage, const int64_t maxTotalUsage);
  ~StorageManager();
  int64_t capacity();
  bool wakeUp(FileStream* stream);
  void disposeOf(const HybridStream* stream);
  void UpdateStorageBudget(HybridStream* stream, const bool consuming = true);
  HybridStream* getTempStorage(int64_t storageRequested, HybridStream* stream = nullptr);
};

#endif
