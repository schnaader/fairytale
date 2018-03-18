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

#ifndef DEFLATEPARSER_H
#define DEFLATEPARSER_H

#include "../parser.h"
#include "../transforms/zlibtransform.h"

#define WINDOW_LOOKBACK     32
#define BRUTE_LOOKBACK      256
#define WINDOW_SIZE         (BRUTE_LOOKBACK+WINDOW_LOOKBACK)
#define WINDOW_ACCESS_MASK  (BRUTE_LOOKBACK-1)
#define BRUTE_ROUNDS        (BRUTE_LOOKBACK>>6)
#define BUFFER(x)           (window[(wndPos-WINDOW_LOOKBACK+(x))&WINDOW_ACCESS_MASK])
#if (BRUTE_LOOKBACK&WINDOW_ACCESS_MASK) || (BRUTE_LOOKBACK<=64)
#error BRUTE_LOOKBACK must be a power of 2 bigger than 64
#endif
#if (WINDOW_LOOKBACK>BRUTE_LOOKBACK) || (WINDOW_LOOKBACK<32)
#error WINDOW_LOOKBACK cannot be bigger than BRUTE_LOOKBACK and must be >=32
#endif

template <const bool BruteMode> class DeflateParser : public Parser<ParserType::Strict> {
private:
  zLibTransform transform;
  uint8_t buffer[GENERIC_BUFFER_SIZE];
  uint8_t window[WINDOW_SIZE], blockIn[ZLIB_BLOCK_SIZE], blockOut[ZLIB_BLOCK_SIZE];
  int histogram[256];
  struct gZip {
    enum Flags { NONE=0, CRC=2, EXTRA=4, NAME=8, COMMENT=16 } flags;
    off_t offset;
    uint8_t options;
  } gzip;
  off_t zipPos;
  off_t index, position;
  int wndPos;
  void clearBuffers() {
    memset(&blockIn[0], 0, ZLIB_BLOCK_SIZE);
    memset(&blockOut[0], 0, ZLIB_BLOCK_SIZE);
    memset(&window[0], 0, WINDOW_SIZE);
    memset(&histogram[0], 0, 256*sizeof(int));
    memset(&gzip, 0, sizeof(gZip));
    wndPos=0, zipPos=0;
  }
  void processByte(const uint8_t b) {
    histogram[b]++;
    if (index>=256) {
      assert(histogram[window[wndPos]]>0);
      histogram[window[wndPos]]--;
    }
    window[wndPos] = b;
    if (wndPos<WINDOW_LOOKBACK)
      window[wndPos+BRUTE_LOOKBACK] = b;
    wndPos=(wndPos+1)&WINDOW_ACCESS_MASK;
  }
  void doBruteModeSearch(bool* result) {
    uint8_t BTYPE = (window[wndPos]&7)>>1;
    if (((*result)=(BTYPE==1 || BTYPE==2))){
      int maximum=0, used=0, offset=wndPos;
      for (int i=0; i<BRUTE_ROUNDS; i++, offset+=64){
        for (int j=0; j<64; j++){
          int freq = histogram[window[(offset+j)&WINDOW_ACCESS_MASK]];
          used+=(freq>0);
          maximum+=(freq>maximum);
        }
        if (maximum>=((12+i)<<i) || (used*(6-i))<((i+1)*64)){
          (*result) = false;
          break;
        }
      }
    }
  }
  inline bool validate(const uint32_t in, const uint32_t out, const bool brute) {
    return (in>max(32ul/max(1ul, out/max(1ul, in)), uint32_t(brute)<<7));
  }
  void getStreamInfo(Block* block, DeflateInfo* info, const bool brute) {
    int ret = 0;
    z_stream strm;
    strm.zalloc=Z_NULL, strm.zfree=Z_NULL, strm.opaque=Z_NULL, strm.next_in=Z_NULL, strm.avail_in=0;
    // Quick check possible stream by decompressing first WINDOW_LOOKBACK bytes
    if (transform.inflateInitAs(&strm, info->zlibParameters)==Z_OK) {
      strm.next_in=&window[(wndPos-(brute?0:WINDOW_LOOKBACK))&WINDOW_ACCESS_MASK], strm.avail_in=WINDOW_LOOKBACK;
      strm.next_out=blockOut, strm.avail_out=ZLIB_BLOCK_SIZE;
      ret = inflate(&strm, Z_FINISH);
      ret = (inflateEnd(&strm)==Z_OK && (ret==Z_STREAM_END || ret==Z_BUF_ERROR) && strm.total_in>=16);
    }
    // Verify valid stream and determine stream length
    if (ret) {
      strm.zalloc=Z_NULL, strm.zfree=Z_NULL, strm.opaque=Z_NULL;
      strm.next_in=Z_NULL, strm.avail_in=0, strm.total_in=strm.total_out=0;
      if (transform.inflateInitAs(&strm, info->zlibParameters)==Z_OK) {
        block->data->setPos(position-(brute?BRUTE_LOOKBACK:WINDOW_LOOKBACK));
        do {
          size_t blockSize = block->data->blockRead(blockIn, ZLIB_BLOCK_SIZE);
          strm.next_in=blockIn, strm.avail_in=(uint32_t)blockSize;
          do {
            strm.next_out=blockOut, strm.avail_out=ZLIB_BLOCK_SIZE;
            ret=inflate(&strm, Z_FINISH);
          } while (strm.avail_out==0 && ret==Z_BUF_ERROR);
          if (ret==Z_STREAM_END && validate(strm.total_in, strm.total_out, brute))
            info->lengthIn=strm.total_in, info->lengthOut=strm.total_out;
          if (ret!=Z_BUF_ERROR)
            break;
        } while (!block->data->eof());
        if (inflateEnd(&strm)!=Z_OK)
          info->lengthIn = info->lengthOut = 0;
      }
    }
  }
public:
  explicit DeflateParser(void) : window{ 0 }, blockIn{ 0 }, blockOut{ 0 }, histogram{ 0 }, gzip{ gZip::NONE, 0, 0 }, zipPos(0), index(0), wndPos(0) {
    priority = PARSER_PRIORITY_DEFLATE;
  }

  bool parse(Block* block, ParseData* data, StorageManager* manager) {
    assert(!block->done);
    assert(block->type==BlockType::DEFAULT);
    assert(block->data!=nullptr && (block->level==0 || !((HybridStream*)block->data)->wasPurged()));
    int64_t i=0, l=block->length;
    if (l<WINDOW_LOOKBACK)
      return false;
    index = 0;
    bool res = false;
    clearBuffers();
    position = block->offset;
    block->data->setPos(position);
    while (i<l) {
      int k=0, bytesRead=(int)block->data->blockRead(&buffer[0], GENERIC_BUFFER_SIZE);
      while (k<bytesRead && i<l) {
        processByte(buffer[k++]);
        index++, i++, position++;

        data->deflate.zlibParameters = transform.parse_zlib_header((BUFFER(0)<<8)|BUFFER(1));
        data->deflate.lengthIn=0, data->deflate.lengthOut=0;
        bool valid = (index>=(WINDOW_LOOKBACK-1) && data->deflate.zlibParameters!=-1);

        if (BruteMode && !valid && index>=WINDOW_ACCESS_MASK)
          doBruteModeSearch(&valid);
        bool brute = (data->deflate.zlibParameters==-1 && index!=zipPos && index!=gzip.offset);

        if (valid || (zipPos>0 && index==zipPos) || (gzip.offset>0 && index==gzip.offset))
          getStreamInfo(block, &data->deflate, brute);

        if (data->deflate.lengthIn>0) {
          off_t offset=position-(brute?BRUTE_LOOKBACK:WINDOW_LOOKBACK);
          block->data->setPos(offset);
          HybridStream* output = transform.attempt(block->data, manager, &data->deflate);
          if (output!=nullptr) {
            LOG("zLib stream found at %" PRIu64 ", length %u bytes, decompresses to %u bytes, %d penalty bytes\n", offset, data->deflate.lengthIn, data->deflate.lengthOut, data->deflate.penaltyBytesUsed);
            res = true;
            block = block->segmentAround(offset, data->deflate.lengthIn, output, &data->deflate, sizeof(DeflateInfo), BlockType::DEFLATE);
            output->setPriority(STREAM_PRIORITY_HIGH);
            if (block==nullptr)
              return res;
          }
          // we succeeded, or it was a valid stream but we didn't have enough storage budget for it, so skip it anyway
          if (output!=nullptr || data->deflate.zlibCombination!=0xFF){
            index = 0;
            i+=data->deflate.lengthIn, i-=(brute?BRUTE_LOOKBACK:WINDOW_LOOKBACK);
            position = offset + data->deflate.lengthIn;
            clearBuffers();
            bytesRead = 0; // to ensure we break
          }
        }

        if (index>gzip.offset)
          gzip.offset = 0;
        if (index>zipPos)
          zipPos = 0;

        if (index>0 && data->deflate.zlibParameters==-1) {
          // detect ZIP streams
          if (zipPos==0 && BUFFER(0)=='P' && BUFFER(1)=='K' && BUFFER(2)=='\x3' && BUFFER(3)=='\x4' && BUFFER(8)=='\x8' && BUFFER(9)=='\0')
          {
            int nlen = (int)BUFFER(26)+(int)BUFFER(27)*256+(int)BUFFER(28)+(int)BUFFER(29)*256;
            if (nlen<256 && block->offset+index+30+nlen < (off_t)block->data->getSize())
              zipPos = index+30+nlen;
          }
          // detect gZip streams
          else if (gzip.offset==0 && BUFFER(0)==0x1F && BUFFER(1)==0x8B && BUFFER(2)==0x08 && ((gzip.options=BUFFER(3))&0xC0)==0)
          {
            gzip.offset = index+10;
            if (gzip.options&gZip::EXTRA)
              gzip.offset+=2+BUFFER(10)+BUFFER(11)*256;

            if (gzip.offset>=(off_t)block->length)
              gzip.offset = 0;
            else {
              block->data->setPos(position+gzip.offset-(2*WINDOW_LOOKBACK-1));

              if (gzip.options&gZip::NAME) {
                do {
                  gzip.offset++;
                } while (block->data->getChar()>0);
                if (gzip.offset>=(off_t)block->length)
                  gzip.offset = 0;
              }
              if (gzip.offset && gzip.options&gZip::COMMENT) {
                do {
                  gzip.offset++;
                } while (block->data->getChar()>0);
                if (gzip.offset>=(off_t)block->length)
                  gzip.offset = 0;
              }
              if (gzip.offset && gzip.options&gZip::CRC) {
                if ((gzip.offset+=2)>=(off_t)block->length)
                  gzip.offset = 0;
              }
            }
          }
        }
      }
      if (i<l)
        block->data->setPos(position);
    }
    return res;
  }
};

#undef WINDOW_SIZE
#undef WINDOW_ACCESS_MASK
#undef BUFFER

#endif
