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

#ifndef STREAM_H
#define STREAM_H

#include "common.h"

class Stream {
public:
	virtual ~Stream() {}
	virtual void close() = 0;
	virtual int getChar() = 0;
	virtual void putChar(const uint8_t c) = 0;
	virtual size_t blockRead(void *ptr, const size_t count) = 0;
	virtual void blockWrite(void *ptr, const size_t count) = 0;
	virtual void setPos(const off_t newpos) = 0;
	virtual void setEnd() = 0;
	virtual off_t curPos() = 0;
	virtual bool eof() = 0;
	virtual int64_t getSize() = 0;
};

#endif
