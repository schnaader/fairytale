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

#include "storagemanager.h"

void StorageManager::doRefBiasedPurge(const int64_t storageRequested) {
  struct {
    size_t scores[PURGE_SLIDING_WINDOW] = { 0 };
    size_t indexes[PURGE_SLIDING_WINDOW] = { 0 };
    int count = 0;
  } stats;
  size_t memUsed;
  int64_t sizeUsed;
  int index;
  TRACE("Received storage request for %" PRIu64 "KB, total available storage is %" PRIu64 "KB\n", storageRequested>>10, available.total>>10);
  do {
    int j = 0;
    do {
      while (pruneIndex<streams.size() && stats.count<PURGE_SLIDING_WINDOW) {
        sizeUsed = SIZE_BLOCKS(streams[pruneIndex]->getSize());
        if (!streams[pruneIndex]->mustKeepAlive() && sizeUsed>0) {
          bool skip = false;
          for (int i=0; i<stats.count; skip|=(pruneIndex==stats.indexes[i]), i++);
          if (!skip) {
            stats.scores[stats.count] = sizeUsed/max(1u, streams[pruneIndex]->getRefCount());
            stats.scores[stats.count]*=streams[pruneIndex]->getPriority();
            stats.indexes[stats.count++] = pruneIndex;
          }
        }
        pruneIndex++;
      }
      j++;
      if (stats.count<PURGE_SLIDING_WINDOW)
        pruneIndex = 0;
    } while (stats.count<PURGE_SLIDING_WINDOW && j<2);

    if (stats.count>0) {
      index = 0;
      for (int i=1; i<stats.count; i++) {
        if (stats.scores[i]>stats.scores[index])
          index = i;
      }
      available.total+=SIZE_BLOCKS(sizeUsed = streams[stats.indexes[index]]->getSize());
      available.memory+=(memUsed = (streams[stats.indexes[index]]->inMemory())?streams[stats.indexes[index]]->getMemUsage():0);
      streams[stats.indexes[index]]->close();
      TRACE("Purged stream %zu, recovered %" PRIu64 "KB of storage and %zuKB of RAM, %" PRIu64 "KB now available, (RAM for caching: %zuKB)\n", stats.indexes[index], sizeUsed>>10, memUsed>>10, available.total>>10, available.memory>>10);
      if (index!=(stats.count-1)) {
        stats.scores[index] = stats.scores[stats.count-1];
        stats.indexes[index] = stats.indexes[stats.count-1];
      }
      stats.count--;
    }
  } while (stats.count>0 && available.total<storageRequested);
}

void StorageManager::doForcedPurge(uint32_t purgeRequested) {
  pruneIndex-=(pruneIndex>0);
  uint32_t i=0, j=0;
  int64_t sizeUsed;
  purgeRequested = min(purgeRequested, FORCE_PURGE_MAXIMUM);
  do {
    while (pruneIndex<streams.size() && i<purgeRequested) {
      if (!streams[pruneIndex]->mustKeepAlive() && !streams[pruneIndex]->wasPurged() && !streams[pruneIndex]->inMemory()) {
        sizeUsed = SIZE_BLOCKS(streams[pruneIndex]->getSize());
        if (sizeUsed>0) {
          available.total+=sizeUsed;
          streams[pruneIndex]->close();
          i++;
        }
      }
      pruneIndex++;
    }
    j++;
    if (i<purgeRequested)
      pruneIndex = 0;
  } while (i<purgeRequested && j<2);
}


StorageManager::StorageManager(const size_t maxMemUsage, const int64_t maxTotalUsage) : streams(0), available({maxMemUsage, maxTotalUsage}), limit({maxMemUsage, maxTotalUsage}), pruneIndex(0) {
  assert((uint64_t)maxTotalUsage>MEM_BLOCK_SIZE && (maxTotalUsage&(MEM_BLOCK_SIZE-1))==0);
  assert((int64_t)maxMemUsage<=maxTotalUsage && (maxMemUsage&(MEM_BLOCK_SIZE-1))==0);
  LOG("Created StorageManager using %zuMB of RAM, %" PRIu64 "MB of total storage\n", maxMemUsage>>20, maxTotalUsage>>20);
}

StorageManager::~StorageManager() {
  for (size_t i=0; i<streams.size(); i++)
    delete streams[i];
}

int64_t StorageManager::capacity() { return limit.total; }

bool StorageManager::wakeUp(FileStream* stream) {
  if (!stream->dormant())
    return true;
  int i = 0;
  do {
    if (!stream->wakeUp())
      doForcedPurge(1);
    i++;
  } while (stream->dormant() && i<4);
  return !stream->dormant();
}

void StorageManager::disposeOf(const HybridStream* stream) {
  size_t index = streams.size();
  assert(index>0);
  index--;
  assert(stream==streams[index]);
  delete streams[index];
  streams.pop_back();
}

void StorageManager::UpdateStorageBudget(HybridStream* stream, const bool consuming) {
  if (stream!=nullptr) {
    if (consuming) {
      if (stream->inMemory())
        available.memory-=stream->getMemUsage();
      available.total-=SIZE_BLOCKS(stream->getSize());
    }
    else {
      if (stream->inMemory())
        available.memory+=stream->getMemUsage();
      available.total+=SIZE_BLOCKS(stream->getSize());
      assert(available.total<=limit.total);
    }
    assert((available.total&(MEM_BLOCK_SIZE-1))==0);
  }
}

HybridStream* StorageManager::getTempStorage(int64_t storageRequested, HybridStream* stream) {
  assert(available.total<=limit.total);
  storageRequested = SIZE_BLOCKS(storageRequested);

  if (limit.total<storageRequested)
    return nullptr;
  if (available.total<storageRequested) {
    doRefBiasedPurge(storageRequested);

    if (available.total<storageRequested)
      return nullptr;
  }

  size_t memRequested = min(available.memory, MEM_LIMIT((size_t)storageRequested));
  if ((int64_t)memRequested > available.total)
    memRequested = available.total;
  else if (memRequested > max(MEM_BLOCK_SIZE*2, min(DEFAULT_TEMP_MEM_PER_STREAM, available.memory>>3)))
    memRequested = 0;

  assert((memRequested&(MEM_BLOCK_SIZE-1))==0);
  if (stream==nullptr) {
    HybridStream* res;
    try {
      res = new HybridStream(memRequested, available.total);
    }
    catch (ExhaustedStorageException const&) {
      doForcedPurge();
      try {
        res = new HybridStream(memRequested, available.total);
      }
      catch (ExhaustedStorageException const&) {
        return nullptr;
      }
    }
    streams.push_back(res);
    return res;
  }
  else {
    try {
      if ((int64_t)memRequested<storageRequested)
        doForcedPurge(1);
      stream->restore(memRequested, available.total);
    }
    catch (ExhaustedStorageException const&) {
      doForcedPurge();
      try {
        stream->restore(memRequested, available.total);
      }
      catch (ExhaustedStorageException const&) {
        return nullptr;
      }
    }
    return stream;
  }
}
