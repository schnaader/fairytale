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

#include "hybridstream.h"

void HybridStream::freeMemory() {
  if (memory) {
    delete memory;
    memory = nullptr;
  }
}

void HybridStream::freeFile() {
  if (file) {
    file->close();
    delete file;
    file = nullptr;
  }
}

void HybridStream::allocate() {
  assert(sizeLimit>0);
  assert((int64_t)memLimit<=sizeLimit);
  if (memLimit>0)
    memory = new Array<uint8_t>(memLimit);
  else {
    file = new FileStream();
    if (!file->getTempFile()) {
      freeFile();
      throw ExhaustedStorageException();
    }
  }
}

HybridStream::HybridStream(const size_t maxMemUsage, const int64_t maxSize) :
  file(nullptr), memory(nullptr), filepos(0), filesize(0), previousSize(0),
  sizeLimit(maxSize), memLimit(MEM_LIMIT(maxMemUsage)), refCount(1), priority(STREAM_PRIORITY_NORMAL), keepAlive(false)
{
  allocate();
}

HybridStream::~HybridStream() { close(); }

void HybridStream::close() {
  freeMemory();
  freeFile();
  if (previousSize==0)
    previousSize = filesize;
  filepos=0, filesize=0;
}

void HybridStream::restore(const size_t maxMemUsage, const int64_t maxSize) {
  assert(memory==nullptr);
  assert(file==nullptr);
  assert(filepos==0);
  assert(filesize==0);
  memLimit = MEM_LIMIT(maxMemUsage);
  sizeLimit = maxSize;
  allocate();
}

bool HybridStream::commitToFile(){
  assert(file==nullptr);
  assert(memory!=nullptr);
  file = new FileStream();
  if (!file->getTempFile()){
    // panic: we failed to get physical storage
    close();
    return false;
  }
  if (filesize>0)
    file->blockWrite(&((*memory)[0]), size_t(filesize));
  file->setPos(filepos);
  freeMemory();
  return true;
}
int HybridStream::getChar() {
  if (filepos>=(off_t)filesize)
    return EOF;
  if (memory)
    return (*memory)[filepos++];
  else {
    assert(file!=nullptr);
    return filepos++, file->getChar();
  }
}

void HybridStream::putChar(const uint8_t c) {
  if (memory) {
    if (filepos<(off_t)memLimit) {
      (*memory)[filepos] = c;
      if (filepos==(off_t)filesize)
        filesize++;
      filepos++;
      return;
    }
    else {
      if (!commitToFile())
        throw ExhaustedStorageException();
    }
  }
  if (filepos<(off_t)sizeLimit) {
    assert(file!=nullptr);
    file->putChar(c);
    if (filepos==(off_t)filesize)
      filesize++;
    filepos++;
  }
  else {
    close();
    throw ExhaustedStorageException();
  }
}

size_t HybridStream::blockRead(void *ptr, const size_t count) {
  if (memory) {
    if (count>0) {
      uint64_t total = filesize-filepos;
      if (total>uint64_t(count))
        total = uint64_t(count);
      memcpy(ptr, &((*memory)[filepos]), size_t(total));
      filepos+=off_t(total);
      return size_t(total);
    }
    else
      return 0;
  }
  else {
    assert(file!=nullptr);
    size_t total = file->blockRead(ptr, count);
    filepos+=off_t(total);
    return total;
  }
}

void HybridStream::blockWrite(void *ptr, const size_t count) {
  if (memory) {
    if (filepos+count<=memLimit) {
      if (count>0) {
        memcpy(&((*memory)[filepos]), ptr, count);
        filepos+=off_t(count);
        if (filepos>(off_t)filesize)
          filesize = filepos;
      }
      return;
    }
    else {
      if (!commitToFile())
        throw ExhaustedStorageException();
    }
  }
  if (filepos+count<=(size_t)sizeLimit) {
    assert(file!=nullptr);
    file->blockWrite(ptr, count);
    filepos+=off_t(count);
    if (filepos>(off_t)filesize)
      filesize = filepos;
  }
  else {
    close();
    throw ExhaustedStorageException();
  }
}

void HybridStream::setPos(const off_t newpos) {
  if (memory) {
    filepos = newpos;
    if (filepos>(off_t)filesize && !commitToFile())
      throw ExhaustedStorageException();
    else
      return;
  }
  if (newpos<(off_t)sizeLimit) {
    assert(file!=nullptr);
    file->setPos(newpos);
    filepos = newpos;
  }
}

void HybridStream::setEnd() {
  filepos = off_t(filesize);
  if (file!=nullptr)
    file->setEnd();
}

off_t HybridStream::curPos() { return (memory)?filepos:(assert(file!=nullptr), file->curPos()); }

void HybridStream::incRefCount() { refCount++; }

void HybridStream::decRefCount() { refCount-=(refCount>0); }

int HybridStream::getPriority() { return priority; }

void HybridStream::setPriority(const int p) { priority = p; }

void HybridStream::setPurgeStatus(const bool allowed) { keepAlive = !allowed; }

size_t HybridStream::getMemUsage() { return memLimit; }

int64_t HybridStream::getSize() { return filesize; }

int64_t HybridStream::getPreviousSize() { return previousSize; }
