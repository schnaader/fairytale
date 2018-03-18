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

#include "textparser.h"

TextParser::TextParser(void) {
  priority = PARSER_PRIORITY_TEXT;
}

bool TextParser::parse(Block* block, ParseData* data, StorageManager* manager) {
  assert(!block->done);
  assert(block->type==BlockType::DEFAULT);
  assert(block->data!=nullptr && (block->level==0 || !((HybridStream*)block->data)->wasPurged()));
  int64_t i=0, l=block->length;
  if (l<MIN_TEXT_SIZE)
    return false;
  uint32_t last4 = 0;
  bool res = false;
  position = block->offset;
  data->text = { 0 };
  data->text.start = position;
  block->data->setPos(position);
  while (i<l) {
    int k=0, bytesRead=(int)block->data->blockRead(&buffer[0], GENERIC_BUFFER_SIZE);
    while (k<bytesRead && i<l) {
      uint8_t c = buffer[k++];
      last4 = (last4<<8)|c;
      bool isLetter = tolower(c)!=toupper(c);
      bool isUTF8 = ((c!=0xC0 && c!=0xC1 && c<0xF5) && (
        (c<0x80) ||
        // possible 1st byte of UTF8 sequence
        ((last4&0xC000)!=0xC000 && ((c&0xE0)==0xC0 || (c&0xF0)==0xE0 || (c&0xF8)==0xF0)) ||
        // possible 2nd byte of UTF8 sequence
        ((last4&0xE0C0)==0xC080 && (last4&0xFE00)!=0xC000) || (last4&0xF0C0)==0xE080 || ((last4&0xF8C0)==0xF080 && ((last4>>8)&0xFF)<0xF5) ||
        // possible 3rd byte of UTF8 sequence
        (last4&0xF0C0C0)==0xE08080 || ((last4&0xF8C0C0)==0xF08080 && ((last4>>16)&0xFF)<0xF5) ||
        // possible 4th byte of UTF8 sequence
        ((last4&0xF8C0C0C0)==0xF0808080 && (last4>>24)<0xF5)
        ));
      i++, position++;
      data->text.lastNL=(c==NEW_LINE || c==CARRIAGE_RETURN)?0:data->text.lastNL+1;
      data->text.lastSpace=(c==SPACE || c==TAB)?0:data->text.lastSpace+1;
      data->text.wordLength=(isLetter)?data->text.wordLength+1:0;
      data->text.missCount-=data->text.misses>>31;
      data->text.misses<<=1;

      if ((c<SPACE && c!=TAB && data->text.lastNL!=0) || ((last4&0xFF00)==(CARRIAGE_RETURN<<8) && c!=NEW_LINE) || (!isLetter && !isUTF8 && (
        data->text.lastNL>128 ||
        data->text.lastSpace>max(data->text.lastNL, max(data->text.wordLength*8, 64u)) ||
        data->text.wordLength>32
        ))) {
        data->text.misses|=1; data->text.missCount++;
        int64_t length = position-data->text.start -1;
        if ((length<MIN_TEXT_SIZE))
          data->text.start = position;
        else if (data->text.missCount>MAX_TEXT_MISSES) {
          LOG("Possible text detection at %" PRIu64 ", %" PRIu64 " bytes\n", data->text.start, length);
          block = block->segmentAround(data->text.start, length, BlockType::TEXT);
          data->text.start = position;
          res = true;
          break;
        }
      }
    }
    if (i<l)
      block->data->setPos(position);
    else if (position-data->text.start>MIN_TEXT_SIZE) {
      int64_t length = position-data->text.start;
      LOG("Possible text detection at %" PRIu64 ", %" PRIu64 " bytes\n", data->text.start, length);
      block = block->segmentAround(data->text.start, length, BlockType::TEXT);
      res = true;
    }
  }
  return res;
}
