/**
 * @file test_config.cpp
 * @author dromniscience (you@domain.com)
 * @brief Test config parser
 *
 * @copyright Copyright (c) 2022
 *
 */
#include "test_factory.h"
#include <common/utils.h>

#define LOOP_TIMES_CONFIG 6

/**
 * @cond TEST
 */
void TestParseNonExistConfig() {
  using namespace OmniSketch::Util;
  bool success;
  try {
    ConfigParser parser("non_exists.toml");
    VERIFY(parser.succeed() == false);
  } catch (const std::exception &exp) {
    VERIFY_NO_EXCEPTION(exp);
  }
}

void TestParseIllFormattedConfig() {
  using namespace OmniSketch::Util;
  try {
    ConfigParser parser("ill_formatted.toml");
    VERIFY(parser.succeed() == false);
  } catch (const std::exception &exp) {
    VERIFY_NO_EXCEPTION(exp);
  }
}

void TestParseWellFormattedConfig() {
  using namespace OmniSketch::Util;
  using std::string_view_literals::operator""sv;

  try {
    ConfigParser parser("well_formatted.toml");
    VERIFY(parser.succeed() == true);

    for (int i = 0; i < LOOP_TIMES_CONFIG; ++i) {
      // set working node
      if (i & 1)
        parser.setWorkingNode("Section.SubSection");
      else
        parser.setWorkingNode();

      // parse
      int32_t a = 0;
      VERIFY(parser.parseConfig(a, "integer") == true);
      VERIFY((i & 1) ? (a == 2022) : (a == 2020));
      VERIFY(parser.parseConfig(a, "double") == false);
      size_t aa = 0;
      VERIFY(parser.parseConfig(aa, "size") == true);
      VERIFY((i & 1) ? (aa == 10) : (aa == 0x100000000ULL));
      VERIFY(parser.parseConfig(aa, "double") == false);

      double b = 0.0;
      VERIFY(parser.parseConfig(b, "integer") == true);
      VERIFY((i & 1) ? (b == 2022.0) : b == 2020.0);
      VERIFY(parser.parseConfig(b, "double") == true);
      VERIFY((i & 1) ? (b == 2020.0) : b == 2022.0);
      VERIFY(parser.parseConfig(b, "float") == false);

      bool c = false;
      VERIFY(parser.parseConfig(c, "boolean") == true);
      VERIFY(c == (i & 1));
      VERIFY(parser.parseConfig(c, "integer") == false);

      std::string d;
      VERIFY(parser.parseConfig(d, "string") == true);
      VERIFY(d == ((i & 1) ? "This is OmniSketch!" : "This is toml!"));
      VERIFY(parser.parseConfig(d, "vector") == false);

      std::vector<int> e, e1 = {1, 1, 2, 3, 5, 8, 13},
                          e2 = {1, 1, 0, 1, -1, 2, -3};
      VERIFY(parser.parseConfig(e, "vector_int") == true);
      VERIFY(e == ((i & 1) ? e2 : e1));
      VERIFY(parser.parseConfig(e, "vector_double") == false);

      std::vector<double> f, f1 = {1.0, 0.5, 0.25, 0.125, 0.0625},
                             f2 = {1, -0.5, 2.0, -0.25, 4};
      VERIFY(parser.parseConfig(f, "vector_double") == true);
      VERIFY(f == ((i & 1) ? f2 : f1));
      VERIFY(parser.parseConfig(f, "vector_string") == false);

      std::vector<std::string> g,
          g1 = {"OmniSketch", "is", "all", "you", "need"},
          g2 = {"Toml", "is", "all", "you", "need"};
      VERIFY(parser.parseConfig(g, "vector_string") == true);
      VERIFY(g == ((i & 1) ? g2 : g1));
      VERIFY(parser.parseConfig(g, "vector_int") == false);

      toml::array h, hh;
      const std::string_view str = (i & 1) ? R"(
                  name = [true, 0.25, "3", 2]
                )"sv
                                           : R"(
                  name = [2, "3", 0.25, true]
                )"sv;
      toml::table tbl = toml::parse(str);
      hh = *tbl["name"].as_array();
      VERIFY(parser.parseConfig(h, "vector") == true);
      VERIFY(h == hh);
      VERIFY(parser.parseConfig(h, "") == false);
    }

  } catch (const std::exception &exp) {
    VERIFY_NO_EXCEPTION(exp);
  }
}

OMNISKETCH_DECLARE_TEST(config) {
  for (int i = 0; i < g_repeat; ++i) {
    TestParseWellFormattedConfig();
    TestParseIllFormattedConfig();
    TestParseNonExistConfig();
  }
}
/** @endcond */