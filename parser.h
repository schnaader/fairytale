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

#ifndef PARSER_H
#define PARSER_H

#include "structs.h"
#include "block.h"

#define PARSER_PRIORITY_BITMAP  0
#define PARSER_PRIORITY_MOD     1
#define PARSER_PRIORITY_DDS     2
#define PARSER_PRIORITY_DEFLATE 3
#define PARSER_PRIORITY_JPEG    4
#define PARSER_PRIORITY_TEXT   -1

enum class ParserType {
  Strict,
  Fuzzy
};

enum class Parsers : size_t {
  DEFLATE,
  DEFLATE_BRUTE,
  JPEG,
  JPEG_PROGRESSIVE,
  BITMAP,
  BITMAP_NOHDR,
  MOD,
  DDS,
  TEXT,
  Count
};

template <const ParserType type> class Parser {
protected:
  int priority;
public:
  virtual ~Parser() {}
  virtual bool parse(Block* block, ParseData* data, StorageManager* manager) = 0;
  int getPriority() { return priority; }
};

#endif
