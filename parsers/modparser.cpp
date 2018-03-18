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

#include "modparser.h"

ModParser::ModParser(void) {
  priority = PARSER_PRIORITY_MOD;
}

bool ModParser::parse(Block* block, ParseData* data, StorageManager* manager) {
  assert(!block->done);
  assert(block->type==BlockType::DEFAULT);
  assert(block->data!=nullptr && (block->level==0 || !((HybridStream*)block->data)->wasPurged()));
  int64_t i=0, l=block->length;
  if (l<0x800)
    return false;
  index = 0;
  uint32_t last4[2] = { 0 };
  memset(&window[0], 0, sizeof(window));
  bool res = false;
  position = block->offset;
  block->data->setPos(position);
  while (i<l) {
    int k=0, bytesRead=(int)block->data->blockRead(&buffer[0], GENERIC_BUFFER_SIZE);
    while (k<bytesRead && i<l) {
      uint8_t c = buffer[k++];
      last4[1] = (last4[1]<<8)|(last4[0]>>24);
      last4[0] = (last4[0]<<8)|c;
      window[index&WINDOW_ACCESS_MASK] = c;
      index++, i++, position++;

      if (index>=1084 && (last4[1]&0xC0C0C0C0)==0 && (
        last4[0]==0x4D2E4B2E /*M.K.*/ || last4[0]==0x4D214B21 /*M!K! (ProTracker, when more than 64 patterns are used*/ ||
        ((last4[0]&0xF1FFFFFF)==0x3043484E /*xCHN, x is even*/ && ((last4[0]>>24)&0xE)<10) ||
        last4[0]==0x464C5434 /*FLT4*/ || last4[0]==0x464C5438 /*FLT8*/ ||
        (last4[0]&0xFFF7FFFF)==0x4F435441 /*OCTA or OKTA*/ || last4[0]==0x43443831 /*CD81*/ ||
        (last4[0]&0xFFFFFFFC)==0x54445A30 /*TDZx, x in 1-3*/ ||
        ((last4[0]&0xF0F0FFF9)==0x30304348 && (c==0x48 || c==0x4E) && ((last4[0]>>24)<0x3A && ((last4[0]>>16)&0xFF)<0x3A)/*xxCH or xxCN, x in 0-9*/)
        ) && window[(index-134)&WINDOW_ACCESS_MASK]<=0x80
        ) {
        uint32_t channels = (((last4[0]>>24)&0xF1)==0x30 /*even*/ && (last4[0]&0xFFF9)!=0x4348 /*..CH or ..CN*/)?
          (last4[0]>>24)&0x0F:
          (c==0x38 /*FLT8*/ || c==0x41 /*OCTA,OKTA*/ || (last4[0]&0xFFFF)==0x3831 /*CD81*/)?
          8:
          ((last4[0]&0xFFF9)==0x4348 /*..CH or ..CN*/)?
          ((last4[0]>>24)&0xF)*10+((last4[0]>>16)&0xF):
          ((last4[0]>>24)==0x54 /*TDZx*/)?
          c&0xF:4;

        if (channels==0 || ((last4[0]&0xFFFF)==4348 /*..CH*/ && (channels&1)>0 /*odd*/))
          continue;

        uint32_t length=0, j=0;
        do {
          off_t p = index-1084+42+j*30; //skip sample name
          uint32_t sampleLength = window[p&WINDOW_ACCESS_MASK]*512 + window[(p+1)&WINDOW_ACCESS_MASK]*2;
          if (sampleLength>0 && (window[(p+2)&WINDOW_ACCESS_MASK]>0xF /*sample finetune, 0..0xF*/ || window[(p+3)&WINDOW_ACCESS_MASK]>0x40 /*sample volume, 0..0x40*/))
            break;
          length+=sampleLength;
          j++;
        } while (j<31);
        if (j<31 || length==0)
          continue;

        uint32_t nPatterns = 1;
        for (j=0; j<0x80; j++) {
          uint32_t n = window[(index-132+j)&WINDOW_ACCESS_MASK]+1;
          nPatterns = max(nPatterns, n);
        }

        if (nPatterns<=(64u<<uint32_t((last4[0]&0xFFFF)==0x4348 || (last4[0]&0xFFFF)==0x4B21))){
          off_t offset = position + nPatterns*channels*256;
          LOG("MOD audio found at %" PRIu64 ", %u bytes\n", offset, length);
          data->audio.bps = 8;
          data->audio.channels = 1;
          data->audio.mode = 4;
          block = block->segmentAround(offset, length, nullptr, &data->audio, sizeof(AudioInfo), BlockType::AUDIO);
          index = 0;
          i+=offset+length-position;
          position = offset+length;
          last4[1]=last4[0]=0;
          memset(&window[0], 0, sizeof(window));
          break;
        }
      }
    }
    if (i<l)
      block->data->setPos(position);
  }
  return res;
}

#undef WINDOW_SIZE
#undef WINDOW_ACCESS_MASK
