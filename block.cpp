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

#include "block.h"
#include "transformfactory.h"
#include "crc32.h"

bool Block::attemptRevival(StorageManager* manager) {
  assert(data != nullptr && level > 0 && parent != nullptr && manager != nullptr);
  HybridStream* stream = (HybridStream*)data;
  if (!stream->wasPurged())
    return true;

  bool parentWasDormant = false;
  // recreate parent stream, if needed
  if (level > 1) {
    if (parent->data == nullptr || (((HybridStream*)parent->data)->wasPurged() && !parent->attemptRevival(manager)))
      return false;
    // don't let it be purged
    ((HybridStream*)parent->data)->setPurgeStatus(false);
  }
  else if ((parentWasDormant = ((FileStream*)parent->data)->dormant()) && !manager->wakeUp((FileStream*)parent->data))
    return false;

  assert(stream->getPreviousSize() > 0);

  // when parsing, should always have enough storage for the parent stream and this stream.
  // however, when deduping, we might have to restore 2 blocks at once, so this may fail
  if (manager->getTempStorage(stream->getPreviousSize(), stream) == nullptr) {
    if (level > 1)
      ((HybridStream*)parent->data)->setPurgeStatus(true);
    else if (parentWasDormant)
      ((FileStream*)parent->data)->goToSleep();
    return false;
  }

  bool res = false;
  Transform* transform = TransformFactory::createTransform(parent->type);
  if (transform != nullptr) {
    parent->data->setPos(parent->offset);
    res = transform->apply(parent->data, stream, parent->info);
    delete transform;
    if (res) {
      manager->UpdateStorageBudget(stream);
      stream->setPurgeStatus(false);
    }
    else {
      // something went terribly wrong, panic
      throw std::logic_error("Failed to recover transformed stream");
    }
  }

  if (level > 1)
    ((HybridStream*)parent->data)->setPurgeStatus(true);
  else if (parentWasDormant)
    ((FileStream*)parent->data)->goToSleep();

  return res;
}

Block* Block::segmentAround(const off_t pos, const int64_t size, const BlockType newType, void* blInfo, const size_t sizeInfo, Stream* childStream, const BlockType childType, const bool childDone, void* childInfo, const size_t childSizeInfo) {
  Block* block = this;
  if (pos > block->offset) {
    Block* n = new Block;
    memcpy(n, block, sizeof(Block));
    block->length = pos - block->offset;
    block->next = n;
    block->child = nullptr;
    if (block->level > 0)
      ((HybridStream*)block->data)->incRefCount();
    block->calculateHash();
    block = n;
  }

  if (pos - block->offset + size < block->length) {
    Block* n = new Block;
    memcpy(n, block, sizeof(Block));
    n->next = block->next, block->next = n;
    n->offset = pos + size;
    n->length -= n->offset - block->offset;
    n->data = block->data;
    n->parent = block->parent;
    n->child = nullptr;
    n->level = block->level;
    if (block->level > 0)
      ((HybridStream*)block->data)->incRefCount();
  }

  block->type = newType;
  block->offset = pos;
  block->length = size;
  if (sizeInfo > 0) {
    block->info = ::operator new(sizeInfo);
    memcpy(block->info, blInfo, sizeInfo);
  }
  block->calculateHash();
  block->done = true;

  if (childStream != nullptr) {
    Block* c = (block->child = new Block);
    memset(c, 0, sizeof(Block));
    c->data = childStream;
    c->length = childStream->getSize();
    c->parent = block;
    c->level = block->level + 1;
    c->calculateHash();
    c->done = childDone;
    c->type = childType;
    if (childSizeInfo > 0) {
      c->info = ::operator new(childSizeInfo);
      memcpy(c->info, childInfo, childSizeInfo);
    }
  }

  return block->next;
}

Block* Block::nextAtLevel(const uint32_t l, const bool skipDone) {
  Block* block = this;
  while (block == this || block->level != l || block->type == BlockType::DEDUP || (skipDone && block->done)) {
    if (block->child != nullptr)
      block = block->child;
    else if (block->next != nullptr)
      block = block->next;
    else if (block->parent != nullptr && block->parent->next != nullptr)
      block = block->parent->next;
    else
      return nullptr;
  }
  return block;
}

void Block::freeInfo() {
  switch (type) {
    case BlockType::DEFLATE: {
      delete (DeflateInfo*)info;
      break;
    }
    default: {}
  }
}

void Block::freeChilds(StorageManager* manager) {
  if (child == nullptr)
    return;
  Block* block = child;
  while (block != nullptr) {
    block->freeInfo();
    block->freeChilds(manager);
    if (block->level > 0)
      manager->UpdateStorageBudget((HybridStream*)block->data, false);
    block->data->close();
    Block* next = block->next;
    delete block;
    block = next;
  }
  child = nullptr;
}

void Block::calculateHash() {
  // assumes data stream is available
  hash = CRC32::process(data, offset, length);
  hashed = true;
}
