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

#include "ddsparser.h"

#include <algorithm>

DDSParser::DDSParser(void) {
  priority = PARSER_PRIORITY_DDS;
}

bool DDSParser::parse(Block* block, ParseData* data, StorageManager* manager) {
  assert(!block->done);
  assert(block->type == BlockType::DEFAULT);
  assert(block->data != nullptr && (block->level == 0 || !((HybridStream*)block->data)->wasPurged()));
  int64_t i = 0, l = block->length;
  if (l < 0x100)
    return false;
  uint64_t last8 = 0;
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
      last8 = (last8 << 8) | buffer[k++];
      i++, position++;

      if (last8 == 0x444453207C000000 /*"DDS ", 124 in L.E.*/) {
        try {
          block->data->setPos(position - 4);
        }
        catch (ExhaustedStorageException const&) {
          break;
        }

        if (block->data->blockRead(&header, sizeof(DDS_HEADER))==sizeof(DDS_HEADER) &&
          letoh32(header.dwSize)==sizeof(DDS_HEADER) &&
          (letoh32(header.dwFlags)&(DDSD_CAPS|DDSD_HEIGHT|DDSD_WIDTH|DDSD_PIXELFORMAT))==(DDSD_CAPS|DDSD_HEIGHT|DDSD_WIDTH|DDSD_PIXELFORMAT) &&
          (letoh32(header.dwCaps)&DDSCAPS_TEXTURE)>0 &&
          letoh32(header.dwWidth)>0 && header.dwWidth<0x10000 &&
          letoh32(header.dwHeight)>0 && header.dwHeight<0x10000 &&
          (letoh32(header.dwMipMapCount)==0 || ((header.dwCaps&DDSCAPS_MIPMAP)>0 && (1u<<(header.dwMipMapCount-1))<= std::max<uint32_t>(header.dwWidth, header.dwHeight))) &&
          letoh32(header.ddspf.dwSize)==sizeof(DDS_PIXELFORMAT) && (
          (letoh32(header.ddspf.dwFlags)==DDPF_FOURCC && (
            ((letoh32(header.ddspf.dwFourCC)&0xF0FFFFFFu)==0x30545844 /*DXT?*/ && ((header.ddspf.dwFourCC>>24)&0xF)>0 && ((header.ddspf.dwFourCC>>24)&0xF)<6) ||
            ((header.ddspf.dwFourCC&0xFCFFFFFFu)==0x30495441 /*ATI?*/ && ((header.ddspf.dwFourCC>>24)&0x3)>0 && ((header.ddspf.dwFourCC>>24)&0x3)<3)
            )) || (
            (header.ddspf.dwFlags&DDPF_RGB)>0 && ((header.ddspf.dwABitMask==0xFF000000u && header.ddspf.dwRGBBitCount==32) || header.ddspf.dwRGBBitCount==24) &&
              header.ddspf.dwRBitMask==0xFF0000u && header.ddspf.dwGBitMask==0xFF00u && header.ddspf.dwBBitMask==0xFFu
              ) || (
              ((header.ddspf.dwFlags&DDPF_ALPHA)>0 || (header.ddspf.dwFlags&DDPF_ALPHAPIXELS)>0) && header.ddspf.dwRGBBitCount==8 && header.ddspf.dwABitMask==0xFFu
              )
            )
          ) {
          int64_t length = sizeof(DDS_HEADER) + 4;
          uint32_t w = header.dwWidth, h = header.dwHeight;

          if (header.ddspf.dwFlags == DDPF_FOURCC) {
            int64_t blockSize = (((header.ddspf.dwFourCC >> 24) & 0xF) == 1) ? 8 : 16;
            uint32_t j = 0;
            do {
              length += ((w + 3) / 4) * ((h + 3) / 4) * blockSize;
              w = std::max<uint32_t>(1u, w >> 1), h = std::max<uint32_t>(1u, h >> 1);
              j++;
            } while (j < header.dwMipMapCount);
            if (i - 8 + length >= l)
              break;

            if ((header.ddspf.dwFourCC & 0xFF) == 0x44)
              LOG(
              "DDS DXT%d texture found at %" PRIu64 ", %ux%u, %u MipMaps, %" PRIu64 " bytes\n", (header.ddspf.dwFourCC >> 24) & 0xF, position - 8,
              header.dwWidth, header.dwHeight, header.dwMipMapCount, length);
            else
              LOG(
              "DDS ATI%d texture found at %" PRIu64 ", %ux%u, %u MipMaps, %" PRIu64 " bytes\n", (header.ddspf.dwFourCC >> 24) & 0xF, position - 8,
              header.dwWidth, header.dwHeight, header.dwMipMapCount, length);
          }
          else {
            uint32_t j = 0;
            do {
              length += ((w * header.ddspf.dwRGBBitCount + 7) / 8) * h;
              w = std::max<uint32_t>(1u, w >> 1), h = std::max<uint32_t>(1u, h >> 1);
              j++;
            } while (j < header.dwMipMapCount);
            if (i - 8 + length >= l)
              break;

            LOG(
            "DDS %dbpp uncompressed texture found at %" PRIu64 ", %ux%u, %u MipMaps, %" PRIu64 " bytes\n", header.ddspf.dwRGBBitCount, position - 8,
            header.dwWidth, header.dwHeight, header.dwMipMapCount, length);
          }
          block = block->segmentAround(position - 8, length, BlockType::DDS);
          i = i - 8 + length;
          position = position - 8 + length;
          last8 = 0;
          res = true;
        }
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
