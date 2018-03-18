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

#ifndef FILESTREAM_H
#define FILESTREAM_H

#include "stream.h"

class FileStream: public Stream {
protected:
  FILE* file;
  char* name;
public:
  FileStream();
  ~FileStream();
  bool open(const char *filename);
  bool create(const char *filename);
  bool getTempFile();
  void close();
  inline int getChar() { assert(file!=nullptr); return fgetc(file); }
  inline void putChar(const uint8_t c) { assert(file!=nullptr); if (fputc(c, file)==EOF) throw ExhaustedStorageException(); }
  size_t blockRead(void *ptr, const size_t count);
  void blockWrite(void *ptr, const size_t count);
  void setPos(const off_t newpos);
  void setEnd();
  off_t curPos();
  inline bool eof() { assert(file!=nullptr); return feof(file)!=0; }
  inline bool dormant() { return (file==nullptr) && (name!=nullptr); }
  bool wakeUp();
  void goToSleep();
  int64_t getSize();
};

#endif
