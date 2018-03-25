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

#ifndef ZLIBTRANSFORM_H
#define ZLIBTRANSFORM_H

#include <zlib.h>
#include "../structs.h"
#include "../transform.h"

#define ZLIB_BLOCK_SIZE (0x10000ul)
#define ZLIB_NUM_COMBINATIONS 81

class zLibMTF {
private:
  struct MTFItem {
    int Next, Previous;
  };
  MTFItem List[ZLIB_NUM_COMBINATIONS];
  int Root, Index;

public:
  explicit zLibMTF(void);
  inline int getFirst() {
    return Index = Root;
  }
  inline int getNext() {
    return (Index >= 0) ? Index = List[Index].Next : Index;
  }
  inline void moveToFront(const int i) {
    if ((Index = i) == Root)
      return;
    List[List[Index].Previous].Next = List[Index].Next;
    if (List[Index].Next >= 0)
      List[List[Index].Next].Previous = List[Index].Previous;
    List[Root].Previous = Index;
    List[Index].Next = Root;
    List[Root = Index].Previous = -1;
  }
};

class zLibTransform : public Transform {
private:
  uint8_t blockIn[ZLIB_BLOCK_SIZE * 2];
  uint8_t blockOut[ZLIB_BLOCK_SIZE];
  uint8_t blockRec[ZLIB_BLOCK_SIZE * 2];
  uint8_t penaltyBytes[ZLIB_NUM_COMBINATIONS * ZLIB_MAX_PENALTY_BYTES];
  uint32_t diffPos[ZLIB_NUM_COMBINATIONS * ZLIB_MAX_PENALTY_BYTES];
  z_stream main_strm, rec_strm[ZLIB_NUM_COMBINATIONS];
  int diffCount[ZLIB_NUM_COMBINATIONS], recPos[ZLIB_NUM_COMBINATIONS];
  zLibMTF MTF;
  int ret;
  void clearBuffers();
  void setupStream(z_streamp strm);
  inline bool validate(const uint32_t in, const uint32_t out, const uint8_t penalty) {
    return (penalty < ZLIB_MAX_PENALTY_BYTES) && (int64_t(in * 8) < int64_t(out * 9 + 8 + penalty * 5) || in > 512u);
  }

public:
  int parse_zlib_header(const uint16_t header);
  int inflateInitAs(z_streamp strm, int parameters);
  HybridStream* attempt(Stream* input, StorageManager* manager, void* info = nullptr);
  bool apply(Stream* input, Stream* output, void* info = nullptr);
  bool undo(Stream* input, Stream* output, void* info = nullptr);
};

#endif
