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

#ifndef DEDUPER_H
#define DEDUPER_H

#include "block.h"

#define DEDUP_HASH_BITS 16
#define DEDUP_HASH_SIZE (1 << DEDUP_HASH_BITS)
#define DEDUP_HASH_MASK (DEDUP_HASH_SIZE - 1)

class Deduper {
private:
	struct LinkedList {
		Block* item;
		LinkedList* next;
	};
	Array<LinkedList> table;
	Array<uint8_t> buffer;
	bool match(Block* block1, Block* block2, StorageManager* manager);

public:
	explicit Deduper(void);
	~Deduper();
	void process(Block* start, Block* end, StorageManager* manager);
};

#endif
