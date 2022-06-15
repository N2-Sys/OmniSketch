/**
 * @file test_flowkey.cpp
 * @author dromniscience (you@domain.com)
 * @brief Test FlowKey
 *
 * @copyright Copyright (c) 2022
 *
 */
#include "test_factory.h"
#include <common/flowkey.h>
#include <unordered_map>

/**
 * @cond TEST
 *
 */
void TestConsturctFlowKey() {
  using namespace OmniSketch;

  try {
    FlowKey<13> a;
    VERIFY(a.getSrcIp() == 0);
    VERIFY(a.getDstIp() == 0);
    VERIFY(a.getProtocol() == 0);
    VERIFY(a.getSrcPort() == 0);
    VERIFY(a.getDstPort() == 0);
    for (int32_t i = 0; i < 13; ++i) {
      VERIFY(a.getBit(i) == 0);
    }

    FlowKey<8> b;
    for (int32_t i = 0; i < 8; ++i) {
      VERIFY(b.getBit(i) == 0);
    }
    VERIFY(b.getSrcIp() == 0);
    VERIFY(b.getDstIp() == 0);
  } catch (const std::exception &exp) {
    VERIFY_NO_EXCEPTION(exp);
  }

  // 1-tuple
  try {
    FlowKey<4> a(4);
    VERIFY(a.getIp() == 4);

    const int8_t b[4] = {0x12, 0x34, 0x56, 0x78};
    const char c[5] = "\x12\x34\x56\x78";
    FlowKey<4> d(b);
    VERIFY(d.getIp() == *reinterpret_cast<const int32_t *>(c));

    // bit manipulation
    for (int i = 0; i < 4 * 8; ++i) {
      bool bit = (c[i >> 3] >> (i & 7)) & 1;
      VERIFY(d.getBit(i) == bit);
    }
    for (int i = 0; i < 4 * 8; ++i) {
      bool bit = (c[i >> 3] >> (i & 7)) & 1;
      d.setBit(i, !bit);
      VERIFY(d.getBit(i) != bit);
    }

  } catch (const FlowKeyMismtach &exp) {
    VERIFY_NO_EXCEPTION(exp);
  }
  try {
    FlowKey<4> a(4, 8);
    SET_FAILURE_FLAG;
  } catch (const FlowKeyMismtach &exp) {
    VERIFY_EXCEPTION(exp);
  }
  try {
    FlowKey<4> a(4, 8, 12, 16, 20);
    SET_FAILURE_FLAG;
  } catch (const FlowKeyMismtach &exp) {
    VERIFY_EXCEPTION(exp);
  }
  try {
    FlowKey<4> a(0);
    int32_t src_ip = a.getSrcIp();
    SET_FAILURE_FLAG;
  } catch (const FlowKeyMismtach &exp) {
    VERIFY_EXCEPTION(exp);
  }
  try {
    FlowKey<4> a(0);
    int32_t src_ip = a.getDstIp();
    SET_FAILURE_FLAG;
  } catch (const FlowKeyMismtach &exp) {
    VERIFY_EXCEPTION(exp);
  }
  try {
    FlowKey<4> a(0);
    int16_t src_ip = a.getSrcPort();
    SET_FAILURE_FLAG;
  } catch (const FlowKeyMismtach &exp) {
    VERIFY_EXCEPTION(exp);
  }
  try {
    FlowKey<4> a(0);
    int16_t src_ip = a.getDstPort();
    SET_FAILURE_FLAG;
  } catch (const FlowKeyMismtach &exp) {
    VERIFY_EXCEPTION(exp);
  }
  try {
    FlowKey<4> a(0);
    int8_t src_ip = a.getProtocol();
    SET_FAILURE_FLAG;
  } catch (const FlowKeyMismtach &exp) {
    VERIFY_EXCEPTION(exp);
  }

  // 2-tuple
  try {
    FlowKey<8> a(4, 8);
    VERIFY(a.getSrcIp() == 4 && a.getDstIp() == 8);

    const int8_t b[8] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x08};
    const char c[9] = "\x11\x22\x33\x44\x55\x66\x77\x08";
    FlowKey<8> d(b);
    VERIFY(d.getSrcIp() == *reinterpret_cast<const int32_t *>(c));
    VERIFY(d.getDstIp() == *reinterpret_cast<const int32_t *>(c + 4));

    // bit manipulation
    for (int i = 0; i < 8 * 8; ++i) {
      bool bit = (c[i >> 3] >> (i & 7)) & 1;
      VERIFY(d.getBit(i) == bit);
    }
    for (int i = 0; i < 8 * 8; ++i) {
      bool bit = (c[i >> 3] >> (i & 7)) & 1;
      d.setBit(i, !bit);
      VERIFY(d.getBit(i) != bit);
    }

  } catch (const FlowKeyMismtach &exp) {
    VERIFY_NO_EXCEPTION(exp);
  }
  try {
    FlowKey<8> a(4);
    SET_FAILURE_FLAG;
  } catch (const FlowKeyMismtach &exp) {
    VERIFY_EXCEPTION(exp);
  }
  try {
    FlowKey<8> a(4, 8, 12, 16, 20);
    SET_FAILURE_FLAG;
  } catch (const FlowKeyMismtach &exp) {
    VERIFY_EXCEPTION(exp);
  }
  try {
    FlowKey<8> a(4, 8);
    int16_t b = a.getSrcPort();
    SET_FAILURE_FLAG;
  } catch (const FlowKeyMismtach &exp) {
    VERIFY_EXCEPTION(exp);
  }
  try {
    FlowKey<8> a(4, 8);
    int16_t b = a.getDstPort();
    SET_FAILURE_FLAG;
  } catch (const FlowKeyMismtach &exp) {
    VERIFY_EXCEPTION(exp);
  }
  try {
    FlowKey<8> a(4, 8);
    int8_t b = a.getProtocol();
    SET_FAILURE_FLAG;
  } catch (const FlowKeyMismtach &exp) {
    VERIFY_EXCEPTION(exp);
  }
  try {
    FlowKey<8> a(4, 8);
    int32_t b = a.getIp();
    SET_FAILURE_FLAG;
  } catch (const FlowKeyMismtach &exp) {
    VERIFY_EXCEPTION(exp);
  }

  // 5-tuple
  try {
    FlowKey<13> a(4, 8, 12, 16, 20);
    VERIFY(a.getSrcIp() == 4 && a.getDstIp() == 8 && a.getSrcPort() == 12 &&
           a.getDstPort() == 16 && a.getProtocol() == 20);

    const int8_t b[13] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
                          0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d};
    const char c[14] = "\x11\x22\x33\x44\x55\x66\x77\x08\x09\x0a\x0b\x0c\x0d";
    FlowKey<13> d(b);
    VERIFY(d.getSrcIp() == *reinterpret_cast<const int32_t *>(c));
    VERIFY(d.getDstIp() == *reinterpret_cast<const int32_t *>(c + 4));
    VERIFY(d.getSrcPort() == *reinterpret_cast<const int16_t *>(c + 8));
    VERIFY(d.getDstPort() == *reinterpret_cast<const int16_t *>(c + 10));
    VERIFY(d.getProtocol() == *reinterpret_cast<const int8_t *>(c + 12));

    // bit manipulation
    for (int i = 0; i < 13 * 8; ++i) {
      bool bit = (c[i >> 3] >> (i & 7)) & 1;
      VERIFY(d.getBit(i) == bit);
    }
    for (int i = 0; i < 13 * 8; ++i) {
      bool bit = (c[i >> 3] >> (i & 7)) & 1;
      d.setBit(i, !bit);
      VERIFY(d.getBit(i) != bit);
    }

  } catch (const FlowKeyMismtach &exp) {
    VERIFY_NO_EXCEPTION(exp);
  }
  try {
    FlowKey<13> a(4);
    SET_FAILURE_FLAG;
  } catch (const FlowKeyMismtach &exp) {
    VERIFY_EXCEPTION(exp);
  }
  try {
    FlowKey<13> a(4, 8);
    SET_FAILURE_FLAG;
  } catch (const FlowKeyMismtach &exp) {
    VERIFY_EXCEPTION(exp);
  }
  try {
    FlowKey<13> a(4, 8, 12, 16, 20);
    int32_t b = a.getIp();
    SET_FAILURE_FLAG;
  } catch (const FlowKeyMismtach &exp) {
    VERIFY_EXCEPTION(exp);
  }
}

void TestCompareFlowKey() {
  using namespace OmniSketch;

  int8_t a[8] = {0x01, 0x20, 0x31, 0x42, 0x5a, 0x67, 0x76, 0x45};
  FlowKey<8> b(a);
  FlowKey<8> c(a);
  VERIFY(b == c);
  VERIFY(!(b < c));

  a[3] = 0x43;
  FlowKey<8> d(a);
  VERIFY(!(b == d));
  VERIFY(b < d);

  a[5] = 0x66;
  FlowKey<8> e(a);
  VERIFY(!(e == d));
  VERIFY(e < d);

  b ^= c;
  VERIFY(b == FlowKey<8>());
  b ^= c;
  VERIFY(b == c);
  b ^= d;
  int8_t tmp[8] = {0x0, 0x0, 0x0, 0x1, 0x0, 0x0, 0x0, 0x0};
  VERIFY(b == FlowKey<8>(tmp));
}

void TestCopyFlowKey() {
  using namespace OmniSketch;
  const int8_t a[13] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                        0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d};
  const char b[14] = "\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d";

  try {
    FlowKey<8> c(a);
    FlowKey<13> d;
    d.copy(0, a, 13);
    VERIFY(d.getSrcIp() == *reinterpret_cast<const int32_t *>(b));
    VERIFY(d.getDstIp() == *reinterpret_cast<const int32_t *>(b + 4));
    VERIFY(d.getSrcPort() == *reinterpret_cast<const int16_t *>(b + 8));
    VERIFY(d.getDstPort() == *reinterpret_cast<const int16_t *>(b + 10));
    VERIFY(d.getProtocol() == *reinterpret_cast<const int8_t *>(b + 12));

    VERIFY(c.getSrcIp() != c.getDstIp());
    c.copy(4, a, 4);
    VERIFY(c.getSrcIp() == c.getDstIp());
    c.copy(0, d, 8, 4);
    VERIFY(c.getSrcIp() == *reinterpret_cast<const int32_t *>(b + 8));

  } catch (const FlowKeyOutOfRange &exp) {
    VERIFY_NO_EXCEPTION(exp);
  }

  try {
    FlowKey<8> c(a);
    FlowKey<13> d(a);
    c.copy(0, d, 10, 4);
    SET_FAILURE_FLAG;
  } catch (const FlowKeyOutOfRange &exp) {
    VERIFY_EXCEPTION(exp);
  }

  try {
    FlowKey<8> c(a);
    FlowKey<13> d(a);
    c.copy(6, d, 10, 3);
    SET_FAILURE_FLAG;
  } catch (const FlowKeyOutOfRange &exp) {
    VERIFY_EXCEPTION(exp);
  }

  try {
    FlowKey<8> c(a);
    FlowKey<13> d(a);
    c.copy(-1, d, 0, 4);
    SET_FAILURE_FLAG;
  } catch (const FlowKeyOutOfRange &exp) {
    VERIFY_EXCEPTION(exp);
  }

  try {
    FlowKey<8> c(a);
    FlowKey<13> d(a);
    c.copy(0, d, -1, 4);
    SET_FAILURE_FLAG;
  } catch (const FlowKeyOutOfRange &exp) {
    VERIFY_EXCEPTION(exp);
  }

  try {
    FlowKey<8> c(a);
    FlowKey<13> d(a);
    c.copy(8, d, 0, 4);
    SET_FAILURE_FLAG;
  } catch (const FlowKeyOutOfRange &exp) {
    VERIFY_EXCEPTION(exp);
  }

  try {
    FlowKey<8> c(a);
    FlowKey<13> d(a);
    c.copy(0, d, 13, 4);
    SET_FAILURE_FLAG;
  } catch (const FlowKeyOutOfRange &exp) {
    VERIFY_EXCEPTION(exp);
  }
}

void TestHashFlowKey() {
  using namespace OmniSketch;

  std::unordered_map<FlowKey<4>, int32_t> map1;
  std::unordered_map<int8_t, int32_t> map2;

  for (int8_t i = 0; i < std::numeric_limits<int8_t>::max(); ++i) {
    int32_t val = rand();

    int8_t a[4];
    a[0] = i & 0x3;
    a[1] = (i >> 2) & 0x3;
    a[2] = (i >> 4) & 0x3;
    a[3] = (i >> 6) & 0x3;

    map1[FlowKey<4>(a)] += val;
    map2[i] += val;
  }
  for (int8_t i = 0; i < std::numeric_limits<int8_t>::max(); ++i) {
    int32_t val = rand();

    int8_t a[4];
    a[0] = i & 0x3;
    a[1] = (i >> 2) & 0x3;
    a[2] = (i >> 4) & 0x3;
    a[3] = (i >> 6) & 0x3;

    map1[FlowKey<4>(a)] += val;
    map2[i] += val;
  }
  for (int8_t i = 0; i < std::numeric_limits<int8_t>::max(); ++i) {
    int8_t a[4];
    a[0] = i & 0x3;
    a[1] = (i >> 2) & 0x3;
    a[2] = (i >> 4) & 0x3;
    a[3] = (i >> 6) & 0x3;

    VERIFY(map1[FlowKey<4>(a)] == map2[i]);
  }
}

void TestSwapFlowKey() {
  using namespace OmniSketch;
  const int8_t a[13] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                        0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d};
  const int8_t b[13] = {0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
                        0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d};
  // swap two flowkeys
  try {
    FlowKey<13> c(a);
    FlowKey<13> d(b);
    d.swap(c);
    VERIFY(d.getSrcIp() == *reinterpret_cast<const int32_t *>(a));
    VERIFY(d.getDstIp() == *reinterpret_cast<const int32_t *>(a + 4));
    VERIFY(d.getSrcPort() == *reinterpret_cast<const int16_t *>(a + 8));
    VERIFY(d.getDstPort() == *reinterpret_cast<const int16_t *>(a + 10));
    VERIFY(d.getProtocol() == *reinterpret_cast<const int8_t *>(a + 12));

    VERIFY(c.getSrcIp() == *reinterpret_cast<const int32_t *>(b));
    VERIFY(c.getDstIp() == *reinterpret_cast<const int32_t *>(b + 4));
    VERIFY(c.getSrcPort() == *reinterpret_cast<const int16_t *>(b + 8));
    VERIFY(c.getDstPort() == *reinterpret_cast<const int16_t *>(b + 10));
    VERIFY(c.getProtocol() == *reinterpret_cast<const int8_t *>(b + 12));

    c.swap(d);
    VERIFY(c.getSrcIp() == *reinterpret_cast<const int32_t *>(a));
    VERIFY(c.getDstIp() == *reinterpret_cast<const int32_t *>(a + 4));
    VERIFY(c.getSrcPort() == *reinterpret_cast<const int16_t *>(a + 8));
    VERIFY(c.getDstPort() == *reinterpret_cast<const int16_t *>(a + 10));
    VERIFY(c.getProtocol() == *reinterpret_cast<const int8_t *>(a + 12));

    VERIFY(d.getSrcIp() == *reinterpret_cast<const int32_t *>(b));
    VERIFY(d.getDstIp() == *reinterpret_cast<const int32_t *>(b + 4));
    VERIFY(d.getSrcPort() == *reinterpret_cast<const int16_t *>(b + 8));
    VERIFY(d.getDstPort() == *reinterpret_cast<const int16_t *>(b + 10));
    VERIFY(d.getProtocol() == *reinterpret_cast<const int8_t *>(b + 12));

  } catch (const std::exception &exp) {
    VERIFY_NO_EXCEPTION(exp);
  }

  // swap with itself
  try {
    FlowKey<13> c(a);
    c.swap(c);
    VERIFY(c.getSrcIp() == *reinterpret_cast<const int32_t *>(a));
    VERIFY(c.getDstIp() == *reinterpret_cast<const int32_t *>(a + 4));
    VERIFY(c.getSrcPort() == *reinterpret_cast<const int16_t *>(a + 8));
    VERIFY(c.getDstPort() == *reinterpret_cast<const int16_t *>(a + 10));
    VERIFY(c.getProtocol() == *reinterpret_cast<const int8_t *>(a + 12));
  } catch (const std::exception &exp) {
    VERIFY_NO_EXCEPTION(exp);
  }
}

OMNISKETCH_DECLARE_TEST(flowkey) {
  for (int i = 0; i < g_repeat; ++i) {
    TestConsturctFlowKey();
    TestCompareFlowKey();
    TestCopyFlowKey();
    TestHashFlowKey();
    TestSwapFlowKey();
  }
}
/** @endcond */
