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

#ifndef ANALYSER_H
#define ANALYSER_H

#include "deduper.h"
#include "parser.h"

class Analyser {
private:
  Array<Parser<ParserType::Strict>*> strict;
  Array<Parser<ParserType::Fuzzy>*> fuzzy;
  ParseData data;
public:
  explicit Analyser(const Array<Parsers> *parsers);
  bool analyse(Block* block, StorageManager* manager, Deduper* deduper = nullptr);
};

#endif
