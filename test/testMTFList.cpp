#include <gtest/gtest.h>

#include "../transforms/MTFList.h"

TEST(MTFList, CreatedInOrder) {
  MTFList<4> MTF;
  ASSERT_EQ(MTF.getFirst(), 0);
  ASSERT_EQ(MTF.getNext(), 1);
  ASSERT_EQ(MTF.getNext(), 2);
  ASSERT_EQ(MTF.getNext(), 3);
  ASSERT_EQ(MTF.getNext(), -1);
}

TEST(MTFList, MoveToFront) {
  MTFList<4> MTF;
  MTF.moveToFront(2);
  ASSERT_EQ(MTF.getFirst(), 2);
  ASSERT_EQ(MTF.getNext(), 0);
  ASSERT_EQ(MTF.getNext(), 1);
  ASSERT_EQ(MTF.getNext(), 3);
  ASSERT_EQ(MTF.getNext(), -1);
}
