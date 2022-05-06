/**
 * @file test_endian.cpp
 * @author dromniscience (you@domain.com)
 * @brief Test endianness
 *
 * @copyright Copyright (c) 2022
 *
 */
#include "test_factory.h"
#include <common/utils.h>

void TestRunTimeEndian() {
  using namespace OmniSketch::Util;

  volatile uint32_t i = 0x12345678U;
  volatile uint16_t j = 0x2022U;

  if (!Endianness()) {
    VERIFY(Net2Host32(i) == 0x78563412U);
    VERIFY(Net2Host16(static_cast<uint16_t>(i)) == 0x7856U);
    VERIFY(Net2Host16(j) == 0x2220U);
    VERIFY(Net2Host32(j) == 0x22200000U);
  } else {
    VERIFY(Net2Host32(i) == 0x12345678U);
    VERIFY(Net2Host16(static_cast<uint16_t>(i)) == 0x5678U);
    VERIFY(Net2Host16(j) == 0x2022U);
    VERIFY(Net2Host32(j) == 0x2022U);
  }
}

void TestCompileTimeEndian() {
  using namespace OmniSketch::Util;

  constexpr uint32_t i = 0x12345678U;
  constexpr uint16_t j = 0x2022U;

  if (!Endianness()) {
    VERIFY(Net2Host32(i) == 0x78563412U);
    VERIFY(Net2Host16(static_cast<uint16_t>(i)) == 0x7856U);
    VERIFY(Net2Host16(j) == 0x2220U);
    VERIFY(Net2Host32(j) == 0x22200000U);
  } else {
    VERIFY(Net2Host32(i) == 0x12345678U);
    VERIFY(Net2Host16(static_cast<uint16_t>(i)) == 0x5678U);
    VERIFY(Net2Host16(j) == 0x2022U);
    VERIFY(Net2Host32(j) == 0x2022U);
  }
}

OMNISKETCH_DECLARE_TEST(endian) {
  for (int i = 0; i < g_repeat; ++i) {
    TestRunTimeEndian();
    TestCompileTimeEndian();
  }
}