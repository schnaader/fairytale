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

#ifndef TEXTPARSER_H
#define TEXTPARSER_H

#include "../parser.h"

#define MIN_TEXT_SIZE 0x400 //1KB
#define MAX_TEXT_MISSES 2   //number of misses in last 32 bytes before resetting

class TextParser : public Parser<ParserType::Fuzzy> {
private:
  uint8_t buffer[GENERIC_BUFFER_SIZE];
  off_t position;

public:
  explicit TextParser(void);
  bool parse(Block* block, ParseData* data, StorageManager* manager);
};

#endif
