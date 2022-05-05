/**
 * @file test_prime.cpp
 * @author dromniscience (you@domain.com)
 * @brief Test routines in utils.hpp
 *
 * @copyright Copyright (c) 2022
 *
 */
#include "test_factory.h"
#include <common/utils.h>

#define LOOP_TIMES_PRIME 6

/**
 * @cond TEST
 * @brief Test IsPrime()
 *
 */
void TestIsPrime() {
  using OmniSketch::Util::IsPrime;

  try {
    // Small instances
    const int32_t a = 1, b = 2, c = 3, d = 4;
    VERIFY(IsPrime(a) == false);
    VERIFY(IsPrime(b) == true);
    VERIFY(IsPrime(c) == true);
    VERIFY(IsPrime(d) == false);

    // Big instances
    const int32_t e = 12340033, f = 98473769, g = 98473768, h = 178431937,
                  i = 3808279;
    VERIFY(IsPrime(e) == true);
    VERIFY(IsPrime(f) == true);
    VERIFY(IsPrime(g) == false);
    VERIFY(IsPrime(h) == false);
    VERIFY(IsPrime(i) == false);
  } catch (const std::invalid_argument &exp) {
    VERIFY_NO_EXCEPTION(exp);
  }

  // invalid argument
  try {
    const int32_t a = 0;
    VERIFY(IsPrime(a));
    SET_FAILURE_FLAG;
  } catch (const std::invalid_argument &exp) {
    VERIFY_EXCEPTION(exp);
  }
  try {
    const int32_t a = -1;
    VERIFY(IsPrime(a));
    SET_FAILURE_FLAG;
  } catch (const std::invalid_argument &exp) {
    VERIFY_EXCEPTION(exp);
  }
  try {
    const int32_t a = static_cast<int32_t>(1) << 31;
    VERIFY(IsPrime(a));
    SET_FAILURE_FLAG;
  } catch (const std::invalid_argument &exp) {
    VERIFY_EXCEPTION(exp);
  }
}

/**
 * @brief Test NextPrime()
 *
 */
void TestNextPrime() {
  using OmniSketch::Util::NextPrime;

  try {
    // Small instances
    const int a = 1, b = 2, c = 3, d = 4;
    VERIFY(NextPrime(a) == 2);
    VERIFY(NextPrime(b) == 2);
    VERIFY(NextPrime(c) == 3);
    VERIFY(NextPrime(d) == 5);

    // Big instances
    const int e = 1326727, f = 1328237, g = 84857147;
    VERIFY(NextPrime(e) == e);
    VERIFY(NextPrime(e + 1) == 1326739);
    VERIFY(NextPrime(f) == f);
    VERIFY(NextPrime(f + 1) == 1328269);
    VERIFY(NextPrime(g) == g);
    VERIFY(NextPrime(g + 2) == 84857177);
  } catch (const std::invalid_argument &exp) {
    VERIFY_NO_EXCEPTION(exp);
  }

  // invalid argument
  try {
    const int32_t a = 0;
    VERIFY(NextPrime(a));
    SET_FAILURE_FLAG;
  } catch (const std::invalid_argument &exp) {
    VERIFY_EXCEPTION(exp);
  }
  try {
    const int32_t a = -1;
    VERIFY(NextPrime(a));
    SET_FAILURE_FLAG;
  } catch (const std::invalid_argument &exp) {
    VERIFY_EXCEPTION(exp);
  }
  try {
    const int32_t a = static_cast<int32_t>(1) << 31;
    VERIFY(NextPrime(a));
    SET_FAILURE_FLAG;
  } catch (const std::invalid_argument &exp) {
    VERIFY_EXCEPTION(exp);
  }
}

/**
 * @brief Test Mangle()
 *
 */
void TestMangle() {
  using OmniSketch::Util::Mangle;

  try {
    const int32_t a = 123456789, b = Mangle(a);
    const int64_t c = 987654321, d = Mangle(c);
    const uint32_t e = 234567890, f = Mangle(e);
    const uint64_t g = 876543210, h = Mangle(g);
    for (int i = 0; i < LOOP_TIMES_PRIME; i++) {
      VERIFY(Mangle(a) == b);
      VERIFY(Mangle(c) == d);
      VERIFY(Mangle(e) == f);
      VERIFY(Mangle(g) == h);
    }
  } catch (const std::exception &exp) {
    VERIFY_NO_EXCEPTION(exp);
  }
}

/**
 * @brief Prime test
 *
 */
OMNISKETCH_DECLARE_TEST(prime) {
  for (int i = 0; i < g_repeat; ++i) {
    TestIsPrime();
    TestNextPrime();
  }
}
/** @endcond */
