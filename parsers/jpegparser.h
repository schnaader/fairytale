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
  enum Markers
  {
    SOF0 = 0xC0,
    SOF1 = 0xC1,
    SOF2 = 0xC2,
    DHT = 0xC4,
    SOI = 0xD8,
    EOI = 0xD9,
    SOS = 0xDA,
    DQT = 0xDB
  };
  uint8_t buffer[GENERIC_BUFFER_SIZE];
  off_t position;

public:
  explicit JpegParser(void) {
    priority = PARSER_PRIORITY_JPEG;
  }

  bool parse(Block* block, ParseData* data, StorageManager* manager) {
    assert(!block->done);
    assert(block->type == BlockType::DEFAULT);
    assert(block->data != nullptr && (block->level == 0 || !((HybridStream*)block->data)->wasPurged()));
    int64_t i = 0, l = block->length;
    if (l < 0x200)
      return false; // skip if less than 512 bytes
    uint32_t last4 = 0;
    uint8_t c;
    bool res = false;
    position = block->offset;
    try {
      block->data->setPos(position);
    }
    catch (ExhaustedStorageException const&) {
      return false;
    }
    while (i < l) {
      int k = 0, bytesRead = (int)block->data->blockRead(&buffer[0], GENERIC_BUFFER_SIZE);
      while (k < bytesRead && i < l) {
        c = buffer[k++];
        last4 = (last4 << 8) | c;
        i++, position++;

        if ((last4 & 0xFFFFFF00) == 0xFFD8FF00 && ((c == SOF0) || (c == SOF1) || (Progressive && c == SOF2) || (c == DHT) || (c >= DQT && c <= 0xFE))) {
          bool done = false, found = false, hasQuantTable = (c == DQT), progressive = (c == SOF2);
          off_t start = position, offset = start - 2;
          do {
            try {
              block->data->setPos(offset);
            }
            catch (ExhaustedStorageException const&) {
              break;
            }
            if (block->data->blockRead(&buffer[0], 5) != 5 || buffer[0] != 0xFF)
              break;

            int length = (int)(buffer[2] * 256) + (int)buffer[3];
            switch (buffer[1]) {
              case DQT: {
                // FF DB XX XX QtId ...
                // Marker length (XX XX) must be = 2 + multiple of 65 <= 260
                // QtId:
                // bit 0..3: number of QT (0..3, otherwise error)
                // bit 4..7: precision of QT, 0 = 8 bit, otherwise 16 bit
                if (length <= 262 && ((length - 2) % 65) == 0 && buffer[4] <= 3) {
                  hasQuantTable = true;
                  offset += length + 2;
                }
                else
                  done = true;
                break;
              }
              case DHT: {
                done = ((buffer[4] & 0xF) > 3 || (buffer[4] >> 4) > 1);
                offset += length + 2;
                break;
              }
              case SOS: found = hasQuantTable;
              case EOI: done = true; break; //EOI with no SOS?
              case SOF2: {
                if (Progressive)
                  progressive = true;
                else {
                  done = true;
                  break;
                }
              }
              case SOF0: done = (buffer[4] != 0x08);
              default: offset += length + 2;
            }
          } while (!done);

          if (found) {
            found = done = false;
            offset += 5;

            bool isMarker = (buffer[4] == 0xFF);
            bytesRead = 0;
            while (!done && (bytesRead = (int)block->data->blockRead(&buffer[0], GENERIC_BUFFER_SIZE))) {
              for (int j = 0; !done && (j < bytesRead); j++) {
                offset++;
                if (!isMarker)
                  isMarker = (buffer[j] == 0xFF);
                else {
                  done = (buffer[j] && ((buffer[j] & 0xF8) != 0xD0 /*skip Restart Markers RST0 to RST7*/) && ((progressive) ? (buffer[j] != DHT) && (buffer[j] != SOS) : true));
                  found = (buffer[j] == EOI);
                  isMarker = false;
                }
              }
            }
          }

          if (found) {
            start -= 4;
            int64_t length = offset - start;
            LOG("%s JPEG found at %" PRIu64 ", length: %" PRIu64 " bytes\n", (progressive ? "Progressive" : "Baseline"), start, length);
            block = block->segmentAround(start, length, BlockType::JPEG);
            i += length - 4;
            position = start + length;
            res = true;
          }
          last4 = 0;
          break;
        }
      }
      if (i < l) {
        try {
          block->data->setPos(position);
        }
        catch (ExhaustedStorageException const&) {
          return res;
        }
      }
    }
    return res;
  }
};

#endif
