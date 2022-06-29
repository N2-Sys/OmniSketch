/**
 * @file test_metric.cpp
 * @author dromniscience (you@domain.com)
 * @brief Test MetricVec
 *
 * @copyright Copyright (c) 2022
 *
 */
#include "test_factory.h"
#include <common/test.h>

/**
 * @cond TEST
 *
 */
void TestReadMetric() {
  using std::string_view_literals::operator""sv;
  using namespace OmniSketch::Test;

  const char *metric_names[] = {"SIZE", "AAE", "ARE", "ACC",  "TIME",
                                "RATE", "TP",  "FP",  "TN",   "FN",
                                "PRC",  "RCL", "F1",  "DIST", "PODF", "RATIO"};
  const Metric metrics[] = {SIZE, AAE, ARE, ACC, TIME, RATE, TP,  FP,
                            TN,   FN,  PRC, RCL, F1,   DIST, PODF, RATIO};

  int32_t index = 0;
  for (const auto &term : metric_names) {
    char name[L_tmpnam], content[1 << 10];
    std::tmpnam(name);
    double thres_1 = ::rand() % 100 / 2.0;
    double thres_2 = ::rand() % 100 / 2.0 + 0.1;
    double thres_3 = ::rand() % 100 / 2.0 + 0.2;
    double thres_4 = ::rand() % 100 / 2.0 + 0.3;
    if (!::strcmp(term, "DIST")) {
      ::sprintf(content,
                "[Test.XXX]\n  insert = [\"%s\"]\n insert_dist = [%g, %g, %g, "
                "%g, %g]",
                term, thres_1, thres_2, thres_3, thres_4, thres_2);
    } else if (!::strcmp(term, "PODF")) {
      ::sprintf(content, "[Test.XXX]\n  insert = [\"%s\"]\n  insert_podf = %g",
                term, thres_1 / 2.0);
    } else {
      ::sprintf(content, "[Test.XXX]\n  insert = [\"%s\"]\n", term);
    }

    std::ofstream fout(name);
    fout.write(content, ::strlen(content));
    fout.close();

    MetricVec vec(name, "Test.XXX", "insert");
    int32_t cur_index = 0;
    for (const auto &met : metrics) {
      if (cur_index == index) {
        VERIFY(vec.in(met));
        if (metrics[cur_index] == PODF) {
          VERIFY(vec.podf == thres_1 / 2.0);
        } else if (metrics[cur_index] == DIST) {
          VERIFY(vec.quantiles.size() == 5);
          for (int32_t i = 1; i < vec.quantiles.size(); ++i) {
            VERIFY(vec.quantiles[i] > vec.quantiles[i - 1]);
          }
          VERIFY(std::set({thres_1, thres_2, thres_3, thres_4,
                           std::numeric_limits<double>::infinity()}) ==
                 std::set(vec.quantiles.begin(), vec.quantiles.end()));
        }
      } else
        VERIFY(!vec.in(met));
      cur_index++;
    }
    index++;
    std::remove(name);
  }
}

void TestReadMetric2() {
  using std::string_view_literals::operator""sv;
  using namespace OmniSketch::Test;

  const char *metric_names[] = {"SIZE", "AAE", "ARE", "ACC",  "TIME",
                                "RATE", "TP",  "FP",  "TN",   "FN",
                                "PRC",  "RCL", "F1",  "DIST", "PODF"};
  const Metric metrics[] = {SIZE, AAE, ARE, ACC, TIME, RATE, TP,  FP,
                            TN,   FN,  PRC, RCL, F1,   DIST, PODF};

  int32_t index = 0;
  for (const auto &term : metric_names) {
    char name[L_tmpnam], content[1 << 10];
    std::tmpnam(name);
    ::sprintf(content, "[abc]\n  q = [\"%s\"]\n", term);

    std::ofstream fout(name);
    fout.write(content, ::strlen(content));
    fout.close();

    MetricVec vec(name, "abc", "q");
    int32_t cur_index = 0;
    for (const auto &met : metrics) {
      if (cur_index == index && metrics[cur_index] != PODF &&
          metrics[cur_index] != DIST) {
        VERIFY(vec.in(met));
      } else
        VERIFY(!vec.in(met));
      cur_index++;
    }
    index++;
    std::remove(name);
  }
}

OMNISKETCH_DECLARE_TEST(metric) {
  for (int i = 0; i < g_repeat; ++i) {
    TestReadMetric();
    TestReadMetric2();
  }
}
/** @endcond */