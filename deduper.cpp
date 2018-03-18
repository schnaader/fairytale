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

#include "deduper.h"

bool Deduper::match(Block* block1, Block* block2, StorageManager* manager) {
  // start by checking full hashes and lengths
  if (block1->hash!=block2->hash || block1->length!=block2->length || block1==block2)
    return false;

  bool b1wasDormant = false, b2wasDormant = false;
  // attempt to revive streams if needed
  if (block1->level>0) {
    if (((HybridStream*)block1->data)->wasPurged()) {
      if (!block1->attemptRevival(manager))
        return false;
    }
    else
      ((HybridStream*)block1->data)->setPurgeStatus(false);
  }
  else {
    if ((b1wasDormant=((FileStream*)block1->data)->dormant()) && !manager->wakeUp((FileStream*)block1->data))
      return false;
  }

  if (block2->level>0) {
    if (((HybridStream*)block2->data)->wasPurged()) {
      if (!block2->attemptRevival(manager)) {
        if (block1->level>0)
          ((HybridStream*)block1->data)->setPurgeStatus(true);
        return false;
      }
    }
    else
      ((HybridStream*)block2->data)->setPurgeStatus(false);
  }
  else {
    if ((b2wasDormant=((FileStream*)block2->data)->dormant()) && !manager->wakeUp((FileStream*)block2->data)) {
      if (block1->level>0)
        ((HybridStream*)block1->data)->setPurgeStatus(true);
      else if (b1wasDormant)
        ((FileStream*)block1->data)->goToSleep();
      return false;
    }
  }
  // now proceed to compare them
  bool ret = true;
  int64_t length = block1->length;
  off_t off1=block1->offset, off2=block2->offset;

  try {
    while (length>0&&ret) {
      int blsize = (int)min(GENERIC_BUFFER_SIZE, length);
      block1->data->setPos(off1);
      int l = (int)block1->data->blockRead(&buffer[0], blsize);
      block2->data->setPos(off2);
      ret = (l==(int)block2->data->blockRead(&buffer[GENERIC_BUFFER_SIZE], blsize));
      if (ret) {
        ret = memcmp(&buffer[0], &buffer[GENERIC_BUFFER_SIZE], blsize)==0;
        off1+=l, off2+=l;
        length-=l;
      }
    }
  }
  catch (ExhaustedStorageException const&) { ret = false; }

  // clean up
  if (block1->level>0)
    ((HybridStream*)block1->data)->setPurgeStatus(true);
  else if (b1wasDormant)
    ((FileStream*)block1->data)->goToSleep();

  if (block2->level>0)
    ((HybridStream*)block2->data)->setPurgeStatus(true);
  else if (b2wasDormant)
    ((FileStream*)block2->data)->goToSleep();
  return ret;
}

Deduper::Deduper(void) : table(DEDUP_HASH_SIZE), buffer(GENERIC_BUFFER_SIZE*2) {}

Deduper::~Deduper() {
  LinkedList *l, *n;
  for (size_t i=0; i<table.size(); i++) {
    l = table[i].next;
    while (l!=nullptr) {
      n = l->next;
      delete l;
      l = n;
    }
  }
}

void Deduper::process(Block* start, Block* end, StorageManager* manager) {
  if (start==nullptr || start->data==nullptr || start->level>=MAX_RECURSION_LEVEL || manager==nullptr)
    return;

  Block* block = start;
  LinkedList* entry;
  while (block!=nullptr && block!=end) {
    assert(block->hashed);
    entry = &table[block->hash&DEDUP_HASH_MASK];
    // first entry?
    if (entry->item==nullptr)
      entry->item = block;
    else {
      // loop through list
      while (entry!=nullptr) {
        if (entry->item==block)
          break;
        // check for match
        else if (match(entry->item, block, manager)) {
          // free any previously allocated info for this block
          block->freeInfo();
          // now free any childs
          block->freeChilds(manager);
          if (block->level>0) {
            // free the stream if possible
            if (block->offset==0 && block->length==block->data->getSize()) {
              manager->UpdateStorageBudget((HybridStream*)block->data, false);
              ((HybridStream*)block->data)->close();
            }
            // otherwise just decrease its reference count
            else
              ((HybridStream*)block->data)->decRefCount();
          }
          block->type = BlockType::DEDUP;
          // info now points to the block we deduplicated from
          block->info = entry->item;
          block->done = true;
          break;
        }
        // end of the list and no match found?
        else if (entry->next==nullptr) {
          // append this block
          entry->next = new LinkedList{ block, nullptr };
          break;
        }
        else
          entry = entry->next;
      }
    }
    // recurse if possible
    if (block->child!=nullptr)
      process(block->child, nullptr, manager);
    block = block->next;
  }
}