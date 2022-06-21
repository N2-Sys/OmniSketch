/**
 * @file test_hierarchy.cpp
 * @author dromniscience (you@domain.com)
 * @brief Test Counter Hierarchy
 *
 * @copyright Copyright (c) 2022
 *
 */
#include "test_factory.h"
#include <common/hierarchy.h>

/**
 * @cond TEST
 *
 */
class TestHash : public OmniSketch::Hash::HashBase {
private:
  inline static int seed = 0;
  int my_seed;

public:
  TestHash() : my_seed(seed++) {}

  uint64_t hash(const uint8_t *key, const int32_t len) const {
    size_t cnt = *reinterpret_cast<const size_t *>(key);
    switch (my_seed % 4) {
    case 0:
    case 2:
      return cnt;
    case 1:
      return cnt + 1;
    default:
      return (cnt == 2) ? cnt : (cnt + 1);
    }
  }
};

void TestDynamicIntX() {
  using namespace OmniSketch::Util;

  // signed number
  try {
    DynamicIntX<int32_t> a(31);
    SET_FAILURE_FLAG;
  } catch (const std::length_error &exp) {
    VERIFY_EXCEPTION(exp);
  }
  try {
    DynamicIntX<int32_t> a(-1);
    SET_FAILURE_FLAG;
  } catch (const std::length_error &exp) {
    VERIFY_EXCEPTION(exp);
  }
  try {
    DynamicIntX<int32_t> a(0);
    SET_FAILURE_FLAG;
  } catch (const std::length_error &exp) {
    VERIFY_EXCEPTION(exp);
  }

  try {
    DynamicIntX<int32_t> a(30);
    int32_t over = a + (std::numeric_limits<int32_t>::max() >> 1); // a is all 1
    VERIFY(over == 0);
    over = a + (std::numeric_limits<int32_t>::max() >> 1); // a is all 11...10
    VERIFY(over == 1);
    over = a + 2; // a is all 0
    VERIFY(over == 1);
    over = a + -(std::numeric_limits<int32_t>::max() >>
                 1); // borrow 1 to cancel out
    VERIFY(over == -1);
    VERIFY(a.getVal() == 1);
  } catch (const std::overflow_error &exp) {
    VERIFY_NO_EXCEPTION(exp);
  }

  try {
    DynamicIntX<int32_t> a(4);
    int32_t over = a + 0x7F;
    VERIFY(over == 7);
    over = a + 0x235;
    VERIFY(over == 0x24);
    over = a + -0x136;
    VERIFY(over == -0x14);
    VERIFY(a.getVal() == 0xe);
    over = a + -0x10d;
    VERIFY(over == -0x10);
    VERIFY(a.getVal() == 0x1);
  } catch (const std::overflow_error &exp) {
    VERIFY_NO_EXCEPTION(exp);
  }

  try {
    DynamicIntX<int32_t> a(30);
    int32_t over = a + ((std::numeric_limits<int32_t>::max() >> 1) + 1);
    SET_FAILURE_FLAG;
  } catch (const std::overflow_error &exp) {
    VERIFY_EXCEPTION(exp);
  }

  try {
    DynamicIntX<int32_t> a(30);
    int32_t over = a + (-(std::numeric_limits<int32_t>::max() >> 1) - 1);
    SET_FAILURE_FLAG;
  } catch (const std::overflow_error &exp) {
    VERIFY_EXCEPTION(exp);
  }

  // unsigned number
  try {
    DynamicIntX<uint32_t> a(31);
    SET_FAILURE_FLAG;
  } catch (const std::length_error &exp) {
    VERIFY_EXCEPTION(exp);
  }
  try {
    DynamicIntX<uint32_t> a(-1);
    SET_FAILURE_FLAG;
  } catch (const std::length_error &exp) {
    VERIFY_EXCEPTION(exp);
  }
  try {
    DynamicIntX<uint32_t> a(0);
    SET_FAILURE_FLAG;
  } catch (const std::length_error &exp) {
    VERIFY_EXCEPTION(exp);
  }

  try {
    DynamicIntX<uint32_t> a(30);
    int32_t over =
        a + (std::numeric_limits<uint32_t>::max() >> 2); // a is all 1
    VERIFY(over == 0);
    over = a + (std::numeric_limits<uint32_t>::max() >> 2); // a is all 11...10
    VERIFY(over == 1);
    over = a + 2; // a is all 0
    VERIFY(over == 1);
    VERIFY(a.getVal() == 0);
  } catch (const std::overflow_error &exp) {
    VERIFY_NO_EXCEPTION(exp);
  }

  try {
    DynamicIntX<uint32_t> a(4);
    int32_t over = a + 0x7F;
    VERIFY(over == 7);
    over = a + 0x235;
    VERIFY(over == 0x24);
    over = a + 0x136;
    VERIFY(over == 0x13);
    VERIFY(a.getVal() == 0xa);
  } catch (const std::overflow_error &exp) {
    VERIFY_NO_EXCEPTION(exp);
  }

  try {
    DynamicIntX<uint32_t> a(30);
    int32_t over = a + ((std::numeric_limits<uint32_t>::max() >> 2) + 1);
    SET_FAILURE_FLAG;
  } catch (const std::overflow_error &exp) {
    VERIFY_EXCEPTION(exp);
  }
}

void TestHierarchy() {
  using namespace OmniSketch::Sketch;

  const std::vector<size_t> no_cnt = {7, 5, 3};
  const std::vector<size_t> width_cnt = {10, 10, 10};
  const std::vector<size_t> no_hash = {2, 2};
  CounterHierarchy<3, int32_t, TestHash> ch(no_cnt, width_cnt, no_hash);

  // normal case
  try {
    constexpr int32_t a[7] = {3309568, 356352001, 163842, 10243, 1028, 5, 6};
    for (size_t i = 0; i < 7; ++i) {
      ch.updateCnt(i, a[i] % 10);
    }
    for (size_t i = 0; i < 7; ++i) {
      VERIFY(ch.getCnt(i) == a[i] % 10);
    }
    for (size_t j = 0; j < 10; ++j) {
      for (size_t i = 0; i < 7; ++i) {
        ch.updateCnt(i, a[i] / 10);
      }
      for (size_t i = 0; i < 7; ++i) {
        VERIFY(ch.getCnt(i) == a[i] % 10 + a[i] / 10 * (j + 1));
        VERIFY(ch.getOriginalCnt(i) == a[i] % 10 + a[i] / 10 * (j + 1));
      }
    }
    for (size_t i = 0; i < 7; ++i) {
      VERIFY(ch.getCnt(i) == a[i]);
      VERIFY(ch.getOriginalCnt(i) == a[i]);
    }
  } catch (const std::exception &exp) {
    VERIFY_NO_EXCEPTION(exp);
  }
  try {
    ch.clear();
    constexpr int32_t a[7] = {3305086, 3568800, 14322, 10243, 10238, 125, 216};
    for (size_t j = 0; j < 5; ++j) {
      for (size_t i = 0; i < 7; ++i) {
        ch.updateCnt(i, a[i] / 5);
      }
    }
    for (size_t i = 0; i < 7; ++i) {
      ch.updateCnt(i, a[i]);
    }
    for (size_t i = 0; i < 7; ++i) {
      ch.updateCnt(i, -a[i] + a[i] % 5);
    }
    for (size_t i = 0; i < 7; ++i) {
      VERIFY(ch.getCnt(i) == a[i]);
      VERIFY(ch.getOriginalCnt(i) == a[i]);
    }
  } catch (const std::exception &exp) {
    VERIFY_NO_EXCEPTION(exp);
  }
  try {
    ch.clear();
    constexpr int32_t a[7] = {1086, 1321, 22, 10243, 10238, 1124, 1216};
    for (size_t j = 0; j < 5; ++j) {
      for (size_t i = 0; i < 7; ++i) {
        ch.updateCnt(i, a[i] / 5);
      }
    }
    for (size_t i = 0; i < 7; ++i) {
      VERIFY(ch.getCnt(i) == a[i] / 5 * 5);
      VERIFY(ch.getOriginalCnt(i) == a[i] / 5 * 5);
    }
    ch.clear();
    for (size_t i = 0; i < 7; ++i) {
      ch.updateCnt(i, a[i]);
    }
    for (size_t i = 0; i < 7; ++i) {
      VERIFY(ch.getCnt(i) == a[i]);
      VERIFY(ch.getOriginalCnt(i) == a[i]);
    }
    for (size_t i = 0; i < 7; ++i) {
      ch.updateCnt(i, -a[i] + a[i] % 5);
    }
    for (size_t i = 0; i < 7; ++i) {
      VERIFY(ch.getCnt(i) == a[i] % 5);
      VERIFY(ch.getOriginalCnt(i) == a[i] % 5);
    }
  } catch (const std::exception &exp) {
    VERIFY_NO_EXCEPTION(exp);
  }

  // exception
  try {
    ch.clear();
    constexpr int32_t a[7] = {1048576, 357564416};
    for (int j = 0; j < 10; ++j) {
      for (size_t i = 0; i < 7; ++i) {
        ch.updateCnt(i, a[i] / 10);
      }
    }
    for (size_t i = 0; i < 7; ++i) {
      ch.updateCnt(i, 5);
    }
    ch.updateCnt(0, 1);
    ch.getCnt(0); // lazy-update
  } catch (const std::overflow_error &exp) {
    VERIFY_NO_EXCEPTION(exp);
  }
  try {
    ch.updateCnt(1, 1);
    ch.getCnt(0); // lazy-update
    SET_FAILURE_FLAG;
  } catch (const std::overflow_error &exp) {
    VERIFY_EXCEPTION(exp);
  }
  try {
    CounterHierarchy<1, int32_t, TestHash> ch({}, {3}, {});
    SET_FAILURE_FLAG;
  } catch (const std::exception &exp) {
    VERIFY_EXCEPTION(exp);
  }
  try {
    CounterHierarchy<2, int32_t, TestHash> ch({3, 5}, {3}, {2});
    SET_FAILURE_FLAG;
  } catch (const std::exception &exp) {
    VERIFY_EXCEPTION(exp);
  }
  try {
    CounterHierarchy<2, int32_t, TestHash> ch({3, 5}, {3, 4}, {2, 3, 4});
    SET_FAILURE_FLAG;
  } catch (const std::exception &exp) {
    VERIFY_EXCEPTION(exp);
  }
  try {
    CounterHierarchy<2, int32_t, TestHash> ch({3, 5}, {30, 3}, {2});
    SET_FAILURE_FLAG;
  } catch (const std::exception &exp) {
    VERIFY_EXCEPTION(exp);
  }
  try {
    CounterHierarchy<2, int32_t, TestHash> ch(
        {100, 5}, {20, std::numeric_limits<size_t>::max()}, {2});
    SET_FAILURE_FLAG;
  } catch (const std::exception &exp) {
    VERIFY_EXCEPTION(exp);
  }
  try {
    CounterHierarchy<3, int32_t, TestHash> ch({100, 50, 0}, {20, 5, 5}, {2, 3});
    SET_FAILURE_FLAG;
  } catch (const std::exception &exp) {
    VERIFY_EXCEPTION(exp);
  }
  try {
    CounterHierarchy<3, int32_t, TestHash> ch({100, 50, 10}, {20, 0, 5},
                                              {2, 3});
    SET_FAILURE_FLAG;
  } catch (const std::exception &exp) {
    VERIFY_EXCEPTION(exp);
  }
  try {
    CounterHierarchy<3, int32_t, TestHash> ch({100, 50, 10}, {20, 5, 5},
                                              {0, 3});
    SET_FAILURE_FLAG;
  } catch (const std::exception &exp) {
    VERIFY_EXCEPTION(exp);
  }
}

OMNISKETCH_DECLARE_TEST(hierarchy) {
  for (int i = 0; i < g_repeat; ++i) {
    TestDynamicIntX();
    TestHierarchy();
  }
}

/** @endcond */