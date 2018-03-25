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

#ifndef DDSPARSER_H
#define DDSPARSER_H

#include "../parser.h"

struct DDS_PIXELFORMAT {
  uint32_t dwSize;
  uint32_t dwFlags;
  uint32_t dwFourCC;
  uint32_t dwRGBBitCount;
  uint32_t dwRBitMask;
  uint32_t dwGBitMask;
  uint32_t dwBBitMask;
  uint32_t dwABitMask;
};

typedef struct {
  uint32_t dwSize;
  uint32_t dwFlags;
  uint32_t dwHeight;
  uint32_t dwWidth;
  uint32_t dwPitchOrLinearSize;
  uint32_t dwDepth;
  uint32_t dwMipMapCount;
  uint32_t dwReserved1[11];
  DDS_PIXELFORMAT ddspf;
  uint32_t dwCaps;
  uint32_t dwCaps2;
  uint32_t dwCaps3;
  uint32_t dwCaps4;
  uint32_t dwReserved2;
} DDS_HEADER;

class DDSParser : public Parser<ParserType::Strict> {
private:
  uint8_t buffer[GENERIC_BUFFER_SIZE];
  off_t position;
  DDS_HEADER header;
  enum DDSHeaderFlags : uint32_t
  {
    DDSD_CAPS = 0x01,
    DDSD_HEIGHT = 0x02,
    DDSD_WIDTH = 0x04,
    DDSD_PIXELFORMAT = 0x1000
  };
  enum DDSCaps : uint32_t
  {
    DDSCAPS_TEXTURE = 0x1000,
    DDSCAPS_MIPMAP = 0x400000
  };
  enum DDSPxFmtFlags : uint32_t
  {
    DDPF_ALPHAPIXELS = 0x01,
    DDPF_ALPHA = 0x02,
    DDPF_FOURCC = 0x04,
    DDPF_RGB = 0x40
  };

public:
  explicit DDSParser(void);
  bool parse(Block* block, ParseData* data, StorageManager* manager);
};

#endif
