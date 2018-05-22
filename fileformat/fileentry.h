/*
  This file is part of the Fairytale project

  Copyright (C) 2018 Andrew Epstein

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

#ifndef FILEENTRY_H
#define FILEENTRY_H

#include "../common.h"
#include "metadata.h"

class FileEntry {
private:
  VLI directoryId;
  VLI length; // Length of name, in bytes
  std::vector<uint8_t> name;
  Metadata metadata;
  VLI numberOfBlocks;
  std::vector<VLI> blockIds;

public:
  int64_t getDirectoryId();
  int64_t getLength();
  std::vector<uint8_t> getName();
  Metadata getMetadata();
  int64_t getNumberOfBlocks();
  std::vector<int64_t> getBlockIds();
};

#endif // FILEENTRY_H
