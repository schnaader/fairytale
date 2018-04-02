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

#ifndef MTFLIST_H
#define MTFLIST_H

template <int size> class MTFList {
  static_assert(size > 0, "MTF list size must be a positive integer");

private:
  struct MTFItem {
    int next, previous;
  };
  MTFItem list[size];
  int root, index;

public:
  MTFList() : root(0), index(0) {
    for (int i = 0; i < size; i++) {
      list[i].next = i + 1;
      list[i].previous = i - 1;
    }
    list[size - 1].next = -1;
  }
  inline int getFirst() {
    index = root;
    return index;
  }
  inline int getNext() {
    if (index >= 0)
      index = list[index].next;
    return index;
  }
  inline void moveToFront(const int i) {
    index = i;
    if (index == root)
      return;
    list[list[index].previous].next = list[index].next;
    if (list[index].next >= 0)
      list[list[index].next].previous = list[index].previous;
    list[root].previous = index;
    list[index].next = root;
    root = index;
    list[root].previous = -1;
  }
};

#endif // MTFLIST_H
