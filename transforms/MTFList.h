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

template<int size>
class MTFList {
 private:
  struct MTFItem {
    int Next, Previous;
  };
  MTFItem List[size];
  int Root, Index;

 public:
  MTFList() : Root(0), Index(0) {
    for (int i = 0; i < size; i++) {
      List[i].Next = i + 1;
      List[i].Previous = i - 1;
    }
    List[size - 1].Next = -1;
  }
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

#endif // MTFLIST_H