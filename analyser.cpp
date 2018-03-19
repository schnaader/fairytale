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

#include "analyser.h"
#include "parsers/deflateparser.h"
#include "parsers/jpegparser.h"
#include "parsers/bitmapparser.h"
#include "parsers/textparser.h"
#include "parsers/ddsparser.h"
#include "parsers/modparser.h"

Analyser::Analyser(const Array<Parsers> *parsers) : strict(0), fuzzy(0), data{{ 0 }} {
  size_t l = parsers->size();
  bool used[static_cast<size_t>(Parsers::Count)] = { 0 };
  for (size_t i=0; i<l; i++) {
    Parsers parser = ((*parsers)[i]);
    if (parser>=Parsers::Count || used[static_cast<size_t>(parser)])
      continue;

    switch (parser) {
      case Parsers::DEFLATE : {
        if (!used[static_cast<size_t>(Parsers::DEFLATE_BRUTE)])
          strict.push_back(new DeflateParser<false>());
        break;
      }
      case Parsers::DEFLATE_BRUTE : {
        if (!used[static_cast<size_t>(Parsers::DEFLATE)])
          strict.push_back(new DeflateParser<true>());
        break;
      }
      case Parsers::JPEG : {
        if (!used[static_cast<size_t>(Parsers::JPEG_PROGRESSIVE)])
          strict.push_back(new JpegParser<false>());
        break;
      }
      case Parsers::JPEG_PROGRESSIVE : {
        if (!used[static_cast<size_t>(Parsers::JPEG)])
          strict.push_back(new JpegParser<true>());
        break;
      }
      case Parsers::BITMAP : {
        if (!used[static_cast<size_t>(Parsers::BITMAP_NOHDR)])
          strict.push_back(new BitmapParser<true>());
        break;
      }
      case Parsers::BITMAP_NOHDR : {
        if (!used[static_cast<size_t>(Parsers::BITMAP)])
          strict.push_back(new BitmapParser<false>());
        break;
      }
      case Parsers::TEXT : {
        fuzzy.push_back(new TextParser());
        break;
      }
      case Parsers::DDS : {
        strict.push_back(new DDSParser());
        break;
      }
      case Parsers::MOD : {
        strict.push_back(new ModParser());
        break;
      }
      default: {};
    }
  }
  // todo: sort by priorities
}

bool Analyser::analyse(Block* block, StorageManager* manager, Deduper* deduper) {
  if (block==nullptr || block->data==nullptr || block->level>=MAX_RECURSION_LEVEL || manager==nullptr)
    return false;
  if (block->level>0 && ((HybridStream*)block->data)->wasPurged() && (!block->attemptRevival(manager)))
    return false;
  if (deduper!=nullptr)
    deduper->process(block, nullptr, manager);
  uint32_t level = block->level;
  bool result = false;    // true if we found anything at all
  bool detected = false;  // true if we found anything in this recursion level
  Block *b, *next;

  do {
    detected = false;
    for (int stage=0; stage<2; stage++) {
      LOG("Starting %s analysis at recursion level %u\n", (stage)?"fuzzy":"strict", level);
      size_t len = (stage)?fuzzy.size():strict.size();
      for (size_t i=0; i<len; i++) {
        b = block;
        if (b->level!=level || b->done)
          b = b->nextAtLevel(level);
        while (b!=nullptr) {
          if (level>0) {
            // attempt stream revival if needed
            if (((HybridStream*)b->data)->wasPurged()) {
              if (!b->attemptRevival(manager))
                break;
            }
            else
              // don't let it be purged from storage
              ((HybridStream*)b->data)->setPurgeStatus(false);
          }
          else if (!manager->wakeUp((FileStream*)b->data))
            break;

          // get current reference to next block at this recursion level,
          // the segmentation may change that
          next = b->nextAtLevel(level);

          bool found = (stage?fuzzy[i]->parse(b, &data, manager):strict[i]->parse(b, &data, manager)); // true if this parser found anything
          result|=detected|=found;

          if (found || next==nullptr) {
            Block* cur = b;
            while (cur!=nullptr && cur!=next) {
              if (!cur->hashed)
                cur->calculateHash();
              cur = cur->next;
            }
          }

          if (deduper!=nullptr && found)
            deduper->process(b, next, manager);

          if (next==nullptr || next->data!=b->data) {
            if (level>0)
              ((HybridStream*)b->data)->setPurgeStatus(true);
            else
              ((FileStream*)b->data)->goToSleep();
          }

          b = next;
        }
      }
    }
    level++;
  } while (detected && level<=MAX_RECURSION_LEVEL);
  return result;
}