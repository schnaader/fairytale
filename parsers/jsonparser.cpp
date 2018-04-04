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

#include "jsonparser.h"

JsonParser::JsonParser(void) {
  priority = PARSER_PRIORITY_JSON;
}

bool JsonParser::parse(Block* block, ParseData* data, StorageManager* manager) {
  assert(!block->done);
  assert(block->type == BlockType::DEFAULT);
  assert(block->data != nullptr && (block->level == 0 || !((HybridStream*)block->data)->wasPurged()));
  int64_t i = 0;
  int64_t blockLength = block->length;
  position = block->offset;
  int32_t curlyBraceCount = 0;
  int32_t squareBracketCount = 0;
  int64_t firstBracePosition = -1;
  int64_t lastBracePosition = 0;
  uint64_t quoteCount = 0;
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
      switch (c) {
        case '{': {
          curlyBraceCount++;
          if (firstBracePosition == -1) {
            squareBracketCount = 0;
            firstBracePosition = position;
          }
          break;
        }
        case '}': {
          curlyBraceCount--;
          if (firstBracePosition != -1)
            lastBracePosition = position + 1;
          break;
        }
        case '"': {
          quoteCount++;
          break;
        }
        case '[': {
          squareBracketCount++;
          break;
        }
        case ']': {
          squareBracketCount--;
          break;
        }
        default: break;
      }
      i++;
      position++;
    }
  }
  if (curlyBraceCount == 0 && squareBracketCount == 0 && (quoteCount % 2 == 0) && firstBracePosition != -1 && lastBracePosition > firstBracePosition) {
    LOG("Possible JSON detection at %" PRIu64 ", %" PRIu64 " bytes\n", firstBracePosition, lastBracePosition - firstBracePosition);
    block = block->segmentAround(firstBracePosition, lastBracePosition - firstBracePosition, BlockType::JSON);
    return true;
  }

  return false;
}
