/*
  This file is part of the Fairytale project

  Copyright (C) 2018 Andrew Epstein

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

#include "magic.h"

MagicParser::MagicParser(void) {
  priority = PARSER_PRIORITY_JSON;
}

bool MagicParser::parse(Block* block, ParseData* data, StorageManager* manager) {
  assert(!block->done);
  assert(block->type == BlockType::DEFAULT);
  assert(block->data != nullptr && (block->level == 0 || !((HybridStream*)block->data)->wasPurged()));
  int64_t i = 0;
  int64_t blockLength = block->length;
  position = block->offset;
  try {
    block->data->setPos(position);
  }
  catch (ExhaustedStorageException const&) {
    return false;
  }
  while (i < blockLength) {
    int k = 0;
    auto bytesRead = (int)block->data->blockRead(&buffer[0], GENERIC_BUFFER_SIZE);
    while (k < bytesRead && i < blockLength) {
      uint8_t c = buffer[k++];
      if(c == 0x47) {
      if(buffer[k] == 0x49) {
	      LOG("gif1\n");
      if(buffer[k+1] == 0x46) {
	      LOG("gif2\n");
      if(buffer[k+2] == 0x38) {
	      LOG("gif3\n");
      if(buffer[k+3] == 0x37 || buffer[k+3] == 0x39) {
	      LOG("gif4\n");
      if(buffer[k+4] == 0x61) {
	      LOG("gif5\n");
      }
      }
      }
      }
      }
      }
      i++;
      position++;
    }
  }

  return false;
}
