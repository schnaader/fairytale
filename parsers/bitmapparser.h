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

#ifndef BITMAPPARSER_H
#define BITMAPPARSER_H

#include "../parser.h"
#include "../image.h"

#ifndef BI_RGB
#define BI_RGB 0L
#endif
#ifndef BI_RLE8
#define BI_RLE8 1L
#endif
#ifndef BI_RLE4
#define BI_RLE4 1L
#endif

#define BITMAP_INFO_HEADER 0x28
#define BITMAP_FILE_HEADER 0xE

template <const bool HdrRequired> class BitmapParser : public Parser<ParserType::Strict> {
private:
  uint8_t buffer[GENERIC_BUFFER_SIZE];
  off_t position;
  struct BMP {
    uint16_t colorPlanes, bpp;
    uint32_t compression, size, hres, vres, paletteEntries;
  } bmp;
public:
  explicit BitmapParser(void) {
    priority = PARSER_PRIORITY_BITMAP;
  }
  bool parse(Block* block, ParseData* data, StorageManager* manager) {
    assert(!block->done);
    assert(block->type==BlockType::DEFAULT);
    assert(block->data!=nullptr && (block->level==0 || !((HybridStream*)block->data)->wasPurged()));
    int64_t i=0, l=block->length;
    if (l<0x100)
      return false; // skip if less than 256 bytes
    uint64_t wnd[3] = { 0 };
    uint32_t imgOffset;
    bool res = false, noFileHdr = false;
    position = block->offset;
    try { block->data->setPos(position); }
    catch (ExhaustedStorageException const&) { return false; }
    while (i<l) {
      int k=0, bytesRead=(int)block->data->blockRead(&buffer[0], GENERIC_BUFFER_SIZE);
      while (k<bytesRead && i<l) {
        wnd[2] = (wnd[2]<<8)|(wnd[1]>>56);
        wnd[1] = (wnd[1]<<8)|(wnd[0]>>56);
        wnd[0] = (wnd[0]<<8)|buffer[k++];
        i++, position++;

        if ((
          (wnd[2]&0xFFFF)==0x424D /*BM*/ &&
          ((imgOffset=(wnd[0]>>32))&0xB7FBFFFF)==0x36000000 &&
          betoh32(imgOffset) <= (BITMAP_FILE_HEADER+BITMAP_INFO_HEADER+4*256) &&
          (wnd[0]&0xFFFFFFFF)==0x28000000
          ) ||
          (!HdrRequired && (noFileHdr=((wnd[0]&0xFFFFFFFF)==0x28000000)))
          )
        {
          off_t offset = position + (noFileHdr?(off_t)(imgOffset=0)+BITMAP_INFO_HEADER-4:(off_t)imgOffset-BITMAP_FILE_HEADER-4);
          try { block->data->setPos(position); }
          catch (ExhaustedStorageException const&) { break; }
          if (
            block->data->blockRead(&data->image.width, 4)!=4 || letoh32(data->image.width)>0x4000 ||
            block->data->blockRead(&data->image.height, 4)!=4 || abs(letoh32(data->image.height))>0x4000 ||
            block->data->blockRead(&bmp.colorPlanes, 2)!=2 || letoh16(bmp.colorPlanes)!=1 ||
            block->data->blockRead(&bmp.bpp, 2)!=2 || (letoh16(bmp.bpp)!=1 && bmp.bpp!=4 && bmp.bpp!=8 && bmp.bpp!=24 && bmp.bpp!=32) ||
            (data->image.stride=((data->image.width*(data->image.bpp=(uint8_t)bmp.bpp)+31)&~31)>>3)*data->image.height<0x100 || // skip if less than 256 bytes
            block->data->blockRead(&bmp.compression, 4)!=4 || bmp.compression!=BI_RGB ||
            block->data->blockRead(&bmp.size, 4)!=4 ||
            block->data->blockRead(&bmp.hres, 4)!=4 || block->data->blockRead(&bmp.vres, 4)!=4 ||
            block->data->blockRead(&bmp.paletteEntries, 4)!=4 || !(letoh32(bmp.paletteEntries)==0 || (bmp.paletteEntries<=(1lu<<bmp.bpp)&&bmp.bpp<=8))
            )
            break;

          data->image.height = abs(data->image.height);
          // possible icon/cursor?
          if (noFileHdr && data->image.width*2==data->image.height && bmp.bpp>1 && (
            (bmp.size>0 && int64_t(bmp.size)==(int64_t(data->image.width*data->image.height*(bmp.bpp+1))>>4)) ||
            ((bmp.size==0 || int64_t(bmp.size)<(int64_t(data->image.width*data->image.height*bmp.bpp)>>3)) && (
            (data->image.width==8)   || (data->image.width==10) || (data->image.width==14) || (data->image.width==16) || (data->image.width==20) ||
              (data->image.width==22)  || (data->image.width==24) || (data->image.width==32) || (data->image.width==40) || (data->image.width==48) ||
              (data->image.width==60)  || (data->image.width==64) || (data->image.width==72) || (data->image.width==80) || (data->image.width==96) ||
              (data->image.width==128) || (data->image.width==256)
              ))
            ))
            data->image.height = data->image.width;

          // if it's a DIB (no file header) and less than 24bpp, we must calculate the data offset based on BPP or num. of entries in color palette
          if (noFileHdr && (bmp.bpp<24))
            offset+=(bmp.paletteEntries>0)?bmp.paletteEntries*4:4<<bmp.bpp;

          if (noFileHdr && bmp.size>0 && int64_t(bmp.size)<(int64_t(data->image.width*data->image.height*bmp.bpp)>>3)) { /*Guard against erroneous DIB detections*/ }
          else {
            int64_t length = data->image.stride*data->image.height;
            if (i+(offset-position)+length<=l) {
              if (bmp.bpp==8) {
                // read color palette to see if image is grayscale
                try {
                  block->data->setPos(position+BITMAP_INFO_HEADER-4);
                  data->image.grayscale = Image::isGrayscalePalette(block->data, (bmp.paletteEntries>0)?bmp.paletteEntries:(1<<bmp.bpp), true);
                }
                catch (ExhaustedStorageException const&) { data->image.grayscale = false; }
              }
              LOG("%dbpp %s%s found at %" PRIu64 ", %dx%d, %" PRIu64 " bytes\n", bmp.bpp, (data->image.grayscale?"grayscale ":""),(noFileHdr?"DIB":"BMP"), offset, data->image.width, data->image.height, length);
              block = block->segmentAround(offset, length, BlockType::IMAGE, &data->image, sizeof(ImageInfo));
              i+=offset-position+length;
              position = offset+length;
              noFileHdr = false;
              wnd[2]=wnd[1]=wnd[0]=0;
              res = true;
            }
          }
          break;
        }
      }
      if (i<l) {
        try { block->data->setPos(position); }
        catch (ExhaustedStorageException const&) { return res; }
      }
    }
    return res;
  }
};

#endif
