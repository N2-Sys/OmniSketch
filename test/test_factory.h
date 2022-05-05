/**
 * @file test_factory.h
 * @author dromniscience (you@domain.com)
 * @brief Test framework
 *
 * @copyright Copyright (c) 2022
 *
 */
#pragma once

#include <getopt.h>
#include <iostream>
#include <string_view>
#include <vector>

namespace OmniSketch {

/**
 * @cond TEST
 * @brief Test factory
 *
 */
class OmniSketchTest final {
  /**
   * @brief All registered tests
   *
   */
  inline static std::vector<OmniSketchTest *> m_registered_tests;

public:
  /**
   * @brief Get all registered tests
   *
   */
  static const std::vector<OmniSketchTest *> &get_registered_tests() {
    return m_registered_tests;
  }
  /**
   * @brief Get test name
   *
   */
  const std::string_view name() const { return m_name; }
  /**
   * @brief Call test routine
   *
   */
  void operator()() const { m_func(); }

  OmniSketchTest() = delete;
  /**
   * @brief Construct with test name and test routine
   *
   */
  OmniSketchTest(const std::string_view name, void (*const func)(void))
      : m_name(name), m_func(func) {
    OmniSketchTest::m_registered_tests.push_back(this);
  }

private:
  /**
   * @brief Name of the test
   *
   */
  const std::string_view m_name;
  /**
   * @brief Test routine
   *
   */
  void (*const m_func)(void);
};

/**
 * @brief Catenate literal strings
 *
 */
#define OMNISKETCH_CAT(a, b) a##b
/**
 * @brief Literal string
 *
 */
#define OMNISKETCH_MAKESTRING(a) #a
/**
 * @brief Declare and register a test
 *
 * @details For example, `OMNISKETCH_DECLARE_TEST(mytest) {...}`
 * will create a function `void test_mytest() { ... }` that will be
 * automatically called.
 *
 */
#define OMNISKETCH_DECLARE_TEST(X)                                             \
  void OMNISKETCH_CAT(test_, X)();                                             \
  static OmniSketch::OmniSketchTest OMNISKETCH_CAT(test_handler_, X)(          \
      "test_" OMNISKETCH_MAKESTRING(X), &OMNISKETCH_CAT(test_, X));            \
  void OMNISKETCH_CAT(test_, X)()
/**
 * @brief Set failure flag
 *
 */
#define SET_FAILURE_FLAG g_success = false
/** @endcond */

} // namespace OmniSketch

/**
 * @cond TEST
 * @brief Global flags
 *
 */
extern int g_repeat;
extern unsigned g_seed;
extern bool g_success;
extern std::string_view g_test_name;
/** @endcond */

/**
 * @cond TEST
 * @brief Verify implementation
 */
inline void verify_impl(bool condition, const std::string_view testname,
                        const char *file, int line,
                        const char *condition_as_string) {
  if (!condition) {
    std::cerr << "Test " << testname << " failed in " << file << " (" << line
              << ")" << std::endl
              << "    " << condition_as_string << std::endl;
    SET_FAILURE_FLAG;
  }
}

/**
 * @brief Shorthand for verify_impl()
 *
 */
#define VERIFY(boolean_exp)                                                    \
  ::verify_impl(boolean_exp, g_test_name, __FILE__, __LINE__,                  \
                OMNISKETCH_MAKESTRING(boolean_exp))
#define VERIFY_NO_EXCEPTION(exp)                                               \
  std::cerr << exp.what() << std::endl;                                        \
  SET_FAILURE_FLAG
#define VERIFY_EXCEPTION(exp)                                                  \
  std::cerr << "Thrown as expected: " << exp.what() << std::endl

/** @endcond */