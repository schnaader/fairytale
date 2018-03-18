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

#ifndef JPEGPARSER_H
#define JPEGPARSER_H

#include "../parser.h"

template <const bool Progressive> class JpegParser : public Parser<ParserType::Strict> {
private:
  uint8_t buffer[GENERIC_BUFFER_SIZE];
  off_t position;
public:
  explicit JpegParser(void) {
    priority = PARSER_PRIORITY_JPEG;
  }

  bool parse(Block* block, ParseData* data, StorageManager* manager) {
    assert(!block->done);
    assert(block->type==BlockType::DEFAULT);
    assert(block->data!=nullptr && (block->level==0 || !((HybridStream*)block->data)->wasPurged()));
    int64_t i=0, l=block->length;
    if (l<0x200)
      return false; // skip if less than 512 bytes
    uint32_t last4 = 0;
    uint8_t c;
    bool res = false;
    position = block->offset;
    block->data->setPos(position);
    while (i<l) {
      int k=0, bytesRead=(int)block->data->blockRead(&buffer[0], GENERIC_BUFFER_SIZE);
      while (k<bytesRead && i<l) {
        c = buffer[k++];
        last4 = (last4<<8)|c;
        i++, position++;

        if ((last4&0xFFFFFF00)==0xFFD8FF00 && (
          (c&0xFE)==0xC0 ||
          (Progressive && c==0xC2) ||
          (c==0xC4) ||
          (c>=0xDB && c<=0xFE)
          ))
        {
          bool done = false, found = false, hasQuantTable = (c==0xDB), progressive = (c==0xC2);
          off_t start=position, offset=start-2;
          do {
            block->data->setPos(offset);
            if (block->data->blockRead(&buffer[0], 5)!=5 || buffer[0]!=0xFF)
              break;

            int length = (int)(buffer[2]*256)+(int)buffer[3];
            switch (buffer[1]) {
              case 0xDB : {
                // FF DB XX XX QtId ...
                // Marker length (XX XX) must be = 2 + multiple of 65 <= 260
                // QtId:
                // bit 0..3: number of QT (0..3, otherwise error)
                // bit 4..7: precision of QT, 0 = 8 bit, otherwise 16 bit
                if (length<=262 && ((length-2)%65)==0 && buffer[4]<=3) {
                  hasQuantTable = true;
                  offset += length+2;
                }
                else
                  done = true;
                break;
              }
              case 0xC4 : {
                done = ((buffer[4]&0xF)>3 || (buffer[4]>>4)>1);
                offset += length+2;
                break;
              }
              case 0xDA : found = hasQuantTable;
              case 0xD9 : done = true; break; //EOI with no SOS?
              case 0xC2 : {
                if (Progressive)
                  progressive = true;
                else {
                  done = true;
                  break;
                }
              }
              case 0xC0 : done = (buffer[4]!=0x08);
              default: offset += length+2;
            }
          } while (!done);

          if (found) {
            found = done = false;
            offset+=5;

            bool isMarker = (buffer[4] == 0xFF);
            bytesRead = 0;
            while (!done && (bytesRead = (int)block->data->blockRead(&buffer[0], GENERIC_BUFFER_SIZE))) {
              for (int j=0; !done && (j<bytesRead); j++) {
                offset++;
                if (!isMarker)
                  isMarker = (buffer[j]==0xFF);
                else {
                  done = (buffer[j] && ((buffer[j]&0xF8)!=0xD0) && ((progressive)?(buffer[j]!=0xC4) && (buffer[j]!=0xDA):true));
                  found = (buffer[j]==0xD9);
                  isMarker = false;
                }
              }
            }
          }

          if (found) {
            start-=4;
            int64_t length = offset-start;
            LOG("%s JPEG found at %" PRIu64 ", length: %" PRIu64 " bytes\n", (progressive?"Progressive":"Baseline"), start, length);
            block = block->segmentAround(start, length, nullptr, nullptr, 0, BlockType::JPEG);
            i+=length-4;
            position = start+length;
            res = true;
          }
          last4 = 0;
          break;
        }
      }
      if (i<l)
        block->data->setPos(position);
    }
    return res;
  }
};

#endif
