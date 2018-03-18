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

// This transform is based on paq8px, see https://github.com/hxim/paq8px

#include "zlibtransform.h"

zLibMTF::zLibMTF(void) : Root(0), Index(0) {
  for (int i=0; i<ZLIB_NUM_COMBINATIONS; i++) {
    List[i].Next = i+1;
    List[i].Previous = i-1;
  }
  List[ZLIB_NUM_COMBINATIONS-1].Next = -1;
}

void zLibTransform::clearBuffers() {
  memset(&blockIn[0], 0, ZLIB_BLOCK_SIZE*2);
  memset(&blockOut[0], 0, ZLIB_BLOCK_SIZE);
  memset(&blockRec[0], 0, ZLIB_BLOCK_SIZE*2);
  memset(&penaltyBytes[0], 0, ZLIB_NUM_COMBINATIONS*ZLIB_MAX_PENALTY_BYTES);
  memset(&diffPos[0], 0xFF, ZLIB_NUM_COMBINATIONS*ZLIB_MAX_PENALTY_BYTES*sizeof(uint32_t));
  memset(&main_strm, 0, sizeof(z_stream));
  memset(&rec_strm[0], 0, ZLIB_NUM_COMBINATIONS*sizeof(z_stream));
  memset(&diffCount[0], 0, ZLIB_NUM_COMBINATIONS*sizeof(int));
}

void zLibTransform::setupStream(z_streamp strm) {
  strm->zalloc=Z_NULL, strm->zfree=Z_NULL, strm->opaque=Z_NULL;
  strm->next_in=Z_NULL, strm->avail_in=0;
}

int zLibTransform::parse_zlib_header(const uint16_t header) {
  switch (header) {
    case 0x2815 : return 0;  case 0x2853 : return 1;  case 0x2891 : return 2;  case 0x28cf : return 3;
    case 0x3811 : return 4;  case 0x384f : return 5;  case 0x388d : return 6;  case 0x38cb : return 7;
    case 0x480d : return 8;  case 0x484b : return 9;  case 0x4889 : return 10; case 0x48c7 : return 11;
    case 0x5809 : return 12; case 0x5847 : return 13; case 0x5885 : return 14; case 0x58c3 : return 15;
    case 0x6805 : return 16; case 0x6843 : return 17; case 0x6881 : return 18; case 0x68de : return 19;
    case 0x7801 : return 20; case 0x785e : return 21; case 0x789c : return 22; case 0x78da : return 23;
  }
  return -1;
}

int zLibTransform::inflateInitAs(z_streamp strm, int parameters) {
  if (parameters==-1)
    return inflateInit2(strm, -MAX_WBITS);
  else
    return inflateInit(strm);
}

HybridStream* zLibTransform::attempt(Stream* input, StorageManager* manager, void* info) {
  if (input==nullptr || manager==nullptr || info==nullptr)
    return nullptr;
  clearBuffers();
  off_t savedPos = input->curPos();
  DeflateInfo* data = (DeflateInfo*)info;
  data->zlibCombination = 0xFF;

  // setup parameters
  int window = (data->zlibParameters==-1)?0:MAX_WBITS+10+data->zlibParameters/4;
  int ctype = data->zlibParameters%4;
  int minclevel = (window==0)?1:(ctype==3)?7:(ctype==2)?6:(ctype==1)?2:1;
  int maxclevel = (window==0)?9:(ctype==3)?9:(ctype==2)?6:(ctype==1)?5:1;
  int index=-1, trials=0;
  bool found = false;

  int main_ret=Z_STREAM_END;
  if (inflateInitAs(&main_strm, data->zlibParameters)!=Z_OK)
    return nullptr;

  for (int i=0; i<ZLIB_NUM_COMBINATIONS; i++) {
    int clevel = (i/9)+1;
    // Early skip if invalid parameter
    if (clevel<minclevel || clevel>maxclevel){
      diffCount[i] = ZLIB_MAX_PENALTY_BYTES;
    }
    recPos[i] = ZLIB_BLOCK_SIZE*2;
  }

  // now try to find a combination that can reproduce the original stream
  for (uint32_t i=0; i<data->lengthIn; i+=ZLIB_BLOCK_SIZE) {
    uint32_t blockSize = min(data->lengthIn-i, ZLIB_BLOCK_SIZE);
    trials = 0;

    for (int j=0; j<ZLIB_NUM_COMBINATIONS; j++) {
      if (diffCount[j]>=ZLIB_MAX_PENALTY_BYTES)
        continue;
      trials++;
      if (recPos[j]>=(int)ZLIB_BLOCK_SIZE)
        recPos[j]-=ZLIB_BLOCK_SIZE;
    }
    // early break if nothing left to test
    if (trials==0)
      break;
    memmove(&blockRec[0], &blockRec[ZLIB_BLOCK_SIZE], ZLIB_BLOCK_SIZE);
    memmove(&blockIn[0], &blockIn[ZLIB_BLOCK_SIZE], ZLIB_BLOCK_SIZE);

    // Read block from input file
    input->blockRead(&blockIn[ZLIB_BLOCK_SIZE], blockSize);

    // Decompress/inflate block
    main_strm.next_in=&blockIn[ZLIB_BLOCK_SIZE], main_strm.avail_in=blockSize;
    do {
      main_strm.next_out=&blockOut[0], main_strm.avail_out=ZLIB_BLOCK_SIZE;
      main_ret = inflate(&main_strm, Z_FINISH);
      trials = 0;
      // Recompress/deflate block with all possible valid parameters
      for (int j=MTF.getFirst(); j>=0; j=MTF.getNext()) {
        if (rec_strm[j].next_out==nullptr) {
          int ret = deflateInit2(&rec_strm[j], (j/9)+1, Z_DEFLATED, window-MAX_WBITS, (j%9)+1, Z_DEFAULT_STRATEGY);
          if (ret!=Z_OK) {
            diffCount[j] = ZLIB_MAX_PENALTY_BYTES;
            continue;
          }
        }
        else if (diffCount[j]>=ZLIB_MAX_PENALTY_BYTES)
          continue;
        trials++;
        rec_strm[j].next_in=&blockOut[0], rec_strm[j].avail_in=ZLIB_BLOCK_SIZE-main_strm.avail_out;
        rec_strm[j].next_out=&blockRec[recPos[j]], rec_strm[j].avail_out=ZLIB_BLOCK_SIZE*2-recPos[j];

        ret = deflate(&rec_strm[j], (main_strm.total_in==data->lengthIn)?Z_FINISH:Z_NO_FLUSH);

        if (ret!=Z_BUF_ERROR && ret!=Z_STREAM_END && ret!=Z_OK) {
          diffCount[j] = ZLIB_MAX_PENALTY_BYTES;
          continue;
        }

        // Compare
        int end = 2*ZLIB_BLOCK_SIZE - (int)rec_strm[j].avail_out;
        int tail = max((main_ret==Z_STREAM_END)?(int)data->lengthIn-(int)rec_strm[j].total_out:0, 0);
        int k=recPos[j], len=(end+tail)&(-8);
        uint64_t *pRec=(uint64_t*)&blockRec[k], *pIn=(uint64_t*)&blockIn[k];
        while (k+8<len && (*pRec)==(*pIn)) {
          pRec+=8, pIn+=8, k+=8;
        }
        for (; k<end; k++) {
          if (i+k-ZLIB_BLOCK_SIZE < data->lengthIn && blockRec[k]!=blockIn[k]) {
            if (++diffCount[j] < ZLIB_MAX_PENALTY_BYTES) {
              const int p = j*ZLIB_MAX_PENALTY_BYTES + diffCount[j];
              diffPos[p] = i+k-ZLIB_BLOCK_SIZE;
              penaltyBytes[p] = blockIn[k];
            }
            else {
              // break, no point in continuing if we've already reached
              //the maximum allowed number of penalty bytes
              tail = 0;
              break;
            }
          }
        }
        if (diffCount[j]+tail < ZLIB_MAX_PENALTY_BYTES) {
          for (k=0; k<tail; k++) {
            diffCount[j]++;
            const int p = j*ZLIB_MAX_PENALTY_BYTES + diffCount[j];
            diffPos[p] = i+k-ZLIB_BLOCK_SIZE;
            penaltyBytes[p] = blockIn[k];

          }
        }
        else {
          diffCount[j] = ZLIB_MAX_PENALTY_BYTES;
          continue;
        }

        // Early break on perfect match
        if (main_ret==Z_STREAM_END && !diffCount[j]){
          index=j, found=true;
          break;
        }
        recPos[j] = 2*ZLIB_BLOCK_SIZE - rec_strm[j].avail_out;
      }
    } while (main_strm.avail_out==0 && main_ret==Z_BUF_ERROR && trials>0);
    if ((main_ret!=Z_BUF_ERROR && main_ret!=Z_STREAM_END) || trials==0)
      break;
  }
  // clean-up and get best match
  int minCount;
  if (found) {
    minCount = 0;
    for (int i=ZLIB_NUM_COMBINATIONS-1; i>=0; i--) {
      if (rec_strm[i].next_out!=nullptr)
        deflateEnd(&rec_strm[i]);
    }
  }
  else {
    minCount = ZLIB_MAX_PENALTY_BYTES;
    for (int i=ZLIB_NUM_COMBINATIONS-1; i>=0; i--) {
      if (rec_strm[i].next_out!=nullptr)
        deflateEnd(&rec_strm[i]);
      if (diffCount[i]<minCount)
        minCount = diffCount[index=i];
    }
  }
  inflateEnd(&main_strm);
  if (!validate(data->lengthIn, data->lengthOut, minCount))
    return nullptr;

  assert(index>=0 && index<ZLIB_NUM_COMBINATIONS);
  MTF.moveToFront(index);
  // save reconstruction info
  data->penaltyBytesUsed = minCount;
  data->zlibWindow = window;
  data->zlibCombination = index;
  for (int i=0; i<=minCount; i++)
    data->posDiff[i] = int(diffPos[index*ZLIB_MAX_PENALTY_BYTES+i+1]-diffPos[index*ZLIB_MAX_PENALTY_BYTES+i])-1;
  data->posDiff[minCount] = int(data->lengthIn-diffPos[index*ZLIB_MAX_PENALTY_BYTES+minCount]);
  for (int i=0; i<minCount; i++)
    data->penaltyBytes[i] = penaltyBytes[index*ZLIB_MAX_PENALTY_BYTES+i+1];
  // now try to output the decompressed stream
  input->setPos(savedPos);
  HybridStream* output = manager->getTempStorage(data->lengthOut);
  if (output==nullptr)
    return output;
  if (!apply(input, output, info)) {
    manager->disposeOf(output);
    return nullptr;
  }
  else {
    manager->UpdateStorageBudget(output);
    return output;
  }
}

bool zLibTransform::apply(Stream* input, Stream* output, void* info) {
  if (input==nullptr || output==nullptr || info==nullptr)
    return false;
  DeflateInfo* data = (DeflateInfo*)info;
  z_stream strm;
  setupStream(&strm);
  if (inflateInitAs(&strm, data->zlibParameters)!=Z_OK)
    return false;
  for (uint32_t i=0; i<data->lengthIn; i+=ZLIB_BLOCK_SIZE) {
    uint32_t blockSize = min(data->lengthIn-i, ZLIB_BLOCK_SIZE);
    input->blockRead(&blockIn[0], blockSize);
    strm.next_in=&blockIn[0], strm.avail_in=blockSize;
    do {
      strm.next_out=&blockOut[0], strm.avail_out=ZLIB_BLOCK_SIZE;
      ret = inflate(&strm, Z_FINISH);
      try {
        output->blockWrite(&blockOut[0], ZLIB_BLOCK_SIZE-strm.avail_out);
      }
      catch (ExhaustedStorageException const&) {
        return false;
      }
    } while (strm.avail_out==0 && ret==Z_BUF_ERROR);
    if (ret!=Z_BUF_ERROR && ret!=Z_STREAM_END)
      break;
  }
  return (ret==Z_STREAM_END);
}

bool zLibTransform::undo(Stream* input, Stream* output, void* info) {
  //TODO
  return false;
}
