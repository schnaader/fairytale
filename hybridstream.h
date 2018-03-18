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

#ifndef HYBRIDSTREAM_H
#define HYBRIDSTREAM_H

#include "filestream.h"
#include "array.h"

#define STREAM_PRIORITY_LOWEST   4
#define STREAM_PRIORITY_LOW      3
#define STREAM_PRIORITY_NORMAL   2
#define STREAM_PRIORITY_HIGH     1
#define STREAM_PRIORITY_HIGHEST  0

class HybridStream: public Stream {
protected:
  FileStream* file;
  Array<uint8_t>* memory;
  off_t filepos;
  int64_t filesize, previousSize;
  int64_t sizeLimit;
  size_t memLimit;
  uint32_t refCount;
  int priority;
  bool keepAlive;
  void freeMemory();
  void freeFile();
  void allocate();
public:
  HybridStream(const size_t maxMemUsage, const int64_t maxSize);
  ~HybridStream();
  void close();
  void restore(const size_t maxMemUsage, const int64_t maxSize);
  bool commitToFile();
  int getChar();
  void putChar(const uint8_t c);
  size_t blockRead(void *ptr, const size_t count);
  void blockWrite(void *ptr, const size_t count);
  void setPos(const off_t newpos);
  void setEnd();
  off_t curPos();
  inline bool eof() { return filepos>=(off_t)filesize; }
  inline bool inMemory() { return memory!=nullptr; }
  inline bool wasPurged() { return (filesize==0 && memory==nullptr && file==nullptr); }
  inline uint32_t getRefCount() { return refCount; }
  void incRefCount();
  void decRefCount();
  int getPriority();
  void setPriority(const int p);
  inline bool mustKeepAlive() { return keepAlive; }
  void setPurgeStatus(const bool allowed);
  size_t getMemUsage();
  int64_t getSize();
  int64_t getPreviousSize();
};

#endif
