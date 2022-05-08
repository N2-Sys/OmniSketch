/**
 * @file test.h
 * @author dromniscience (you@domain.com)
 * @brief Testing classes and metrics
 *
 * @copyright Copyright (c) 2022
 *
 */
#pragma once

// A bunch of files to include!
#include "sketch.h"
#include <boost/any.hpp>
#include <ctime>
#include <map>
#include <memory>
#include <set>

/**
 * @brief Testing classes and metrics
 *
 */
namespace OmniSketch::Test {

/**
 * @brief Metrics
 *
 * @details The second column expounds the meaning of each metric.
 *
 */
enum Metric {
  SIZE /** size (in bytes) */,
  TIME /** time (in microseconds, 1e-6s) */,
  RATE /** processing rate (packets per second) */,
  ARE /** average relative error (in numeric) */,
  AAE /** average absolute error (in numeric) */,
  ACC /** correct rate (in percentile) */,
  TP /** true positive (in percentile) */,
  FP /** false positive (in percentile) */,
  TN /** true negative (in percentile) */,
  FN /** false negative (in percentile) */,
  PRC /** precision := (TP) over (TP + FP) (in numeric) */,
  RCL /** recall := (TP) over (TP + FN) (in numeric) */,
  F1 /** F1Score :=  harmonic mean of precision & recall (in numeric)*/,
  DIST /** distribution of error (vector) */,
  PODF /** portion of desired flow (in percentile) */,
};

/**
 * @brief Metric vector
 *
 * @details Holding metrics that are of interest. Manipulated by TestBase.
 *
 */
class MetricVec {
public:
  /**
   * @brief set of interested metrics
   *
   */
  std::set<Metric> metric_set;
  /**
   * @brief threshold for podf
   *
   */
  double podf;
  /**
   * @brief quantiles of distribution
   *
   */
  std::vector<double> quantiles;

  /**
   * @brief Read and parse the metric vector
   *
   * @param file_path   Path to the config file
   * @param test_path   Path to the node containing testing term in the file
   * (concatenated with '.', see example below)
   * @param term_name   Name of the testing term
   *
   * @note Once Metric::PODF is declared in the current testing term, say
   * ```
   * XXX = [..., "PODF", ...]
   * ```
   * you have to add to the same node a line
   * ```
   * XXX_podf = [a threshold]
   * ```
   * for MetricVec to read. Likewise, with Metric::DIST you have to add to the
   * node a line
   * ```
   * XXX_dist = [a vector of double]
   * ```
   *
   * ### Example
   * If the toml file is like this,
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~.toml
   * # toml.toml
   * [MySketch.test]
   * insert = ["RATE"]
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   * The proper way of creating an MetricVec is `("toml.toml", "MySketch.test",
   * "insert")`.
   */
  MetricVec(const std::string_view file_path, const std::string_view test_path,
            const std::string_view term_name);
  /**
   * @brief Whether a metric is specified
   *
   */
  bool in(const Metric metric) const { return metric_set.count(metric); }
};

/**
 * @brief Collection of metrics
 *
 * @todo Docs for supported types of statistics
 *
 * @tparam key_len  length of flowkey
 * @tparam T        type of the counter
 *
 *|Test method |Required overriden function|Available Metrics (Test::Metric)|
 *|:-----------|:--------------------------|:-------------------------------|
 *|testSize()  |Sketch::SketchBase::size() | SIZE                           |
 *|testInsert()|Sketch::SketchBase::insert()|RATE                           |
 *|testUpdate()|Sketch::SketchBase::update()|RATE                           |
 *|testQuery() |Sketch::SketchBase::query()|RATE, ARE, AAE, ACC, PODF, DIST |
 *|testLookup()|Sketch::SketchBase::lookup()|RATE, TP, FP, PRC              |
 *|testHeavyHitter()|Sketch::SketchBase::getHeavyHitter()|TIME,ARE,PRC,RCL,F1|
 *|testHeavyChanger()|Sketch::SketchBase::getHeavyChanger()|TIME,ARE,PRC,RCL,F1|
 *
 */
template <int32_t key_len, typename T = int64_t> class TestBase {
private:
  using Vec = std::map<Metric, boost::any>;

  Vec size;
  Vec insert;
  Vec lookup;
  Vec update;
  Vec query;
  Vec heavy_hitter;
  Vec heavy_changer;

protected:
  const std::string_view show_name;
  const std::string_view config_file;
  const std::string_view test_path;

public:
  /**
   * @brief Construct by specifying all necessary tables in config file
   *
   * @param show_name           The name to be shown in the output (may contain
   * whitespaces)
   * @param config_file         Path to the config file
   * @param test_path           Path to the node which stores the testing
   * metrics (concatenated with '.', see example below)
   *
   * ### Example
   * If the toml file is as follows,
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~.toml
   * # toml.toml
   * [MySketch]
   *
   *    [MySketch.para]
   *    arg1 = 1
   *    arg2 = 2
   *
   *    [MySketch.test]
   *    insert = ["RATE"]
   *    arg3 = 3
   *
   *    [MySketch.data]
   *    data = "../record.bin"
   *    format = [["flowkey", "padding"], [13, 3]]
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   * and you want the readable name "My Tiny Little Sketch" when showing
   * results, then you should call constructor in this way:
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~.cpp
   * TestBase<...>("My Tiny Little Sketch", "toml.toml", "MySketch.test");
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   *
   * @note
   * - Except for `show_name`, all other strings should contain no whitespaces.
   */
  TestBase(const std::string_view show_name, const std::string_view config_file,
           const std::string_view test_path)
      : show_name(show_name), config_file(config_file), test_path(test_path) {}
  /**
   * @brief Display metrics in a human-readable manner
   * @todo DIST
   */
  virtual void show() const final;
  /**
   * @brief Run the test and collect statistics
   * @details Make sure this method is overriden in subclass.
   *
   */
  virtual void runTest();
  /**
   * @brief Get the size of the sketch
   *
   * @details You should override the Sketch::SketchBase::size() method.
   *
   * @param ptr_sketch  pointer to the sketch
   */
  virtual void
  testSize(std::unique_ptr<Sketch::SketchBase<key_len, T>> &ptr_sketch) final;
  /**
   * @brief Insert a row of records
   * @details Records in [begin, end) will be sequentially inserted. You should
   * override the Sketch::SketchBase::insert() method.
   *
   * @param ptr_sketch  pointer to the sketch
   * @param begin       [begin, end)
   * @param end         [begin, end)
   */
  virtual void testInsert(
      std::unique_ptr<Sketch::SketchBase<key_len, T>> &ptr_sketch,
      typename std::vector<Data::Record<key_len>>::const_iterator begin,
      typename std::vector<Data::Record<key_len>>::const_iterator end) final;
  /**
   * @brief Update a row of records (with values to the sketch)
   * @details Records in [begin, end) will be sequentially updated. You should
   * override the Sketch::SketchBase::update() method.
   *
   */
  virtual void
  testUpdate(std::unique_ptr<Sketch::SketchBase<key_len, T>> &ptr_sketch,
             typename std::vector<Data::Record<key_len>>::const_iterator begin,
             typename std::vector<Data::Record<key_len>>::const_iterator end,
             Data::CntMethod cnt_method) final;
  /**
   * @brief Query for each flow in ground truth
   * @details You should override the Sketch::SketchBase::query() method.
   *
   * @param ptr_sketch  pointer to the sketch
   * @param gnd_truth   ground truth
   */
  virtual void
  testQuery(std::unique_ptr<Sketch::SketchBase<key_len, T>> &ptr_sketch,
            const Data::GndTruth<key_len, T> &gnd_truth) final;
  /**
   * @brief Lookup each flow in ground truth
   * @details You should override the Sketch::SketchBase::lookup() method.
   *
   * @param ptr_sketch  pointer to the sketch
   * @param gnd_truth   ground truth
   * @param sample      sampled ground truth
   */
  virtual void
  testLookup(std::unique_ptr<Sketch::SketchBase<key_len, T>> &ptr_sketch,
             const Data::GndTruth<key_len, T> &gnd_truth,
             const Data::GndTruth<key_len, T> &sample) final;
  /**
   * @brief Test heavy hitters
   * @details You should override the Sketch::SketchBase::getHeavyHitter()
   * method.
   *
   * @param ptr_sketch  pointer to the sketch
   * @param threshold   threshold value of heavy hitters
   * @param gnd_truth_heavy_hitters ground truth of heavy hitters
   */
  virtual void
  testHeavyHitter(std::unique_ptr<Sketch::SketchBase<key_len, T>> &ptr_sketch,
                  double threshold,
                  Data::GndTruth<key_len, T> gnd_truth_heavy_hitters) final;
  /**
   * @brief Test heavy changers
   * @details You should override the Sketch::SketchBase::getHeavyChangers()
   * method.
   *
   * @param ptr_sketch_1  pointer to the first sketch
   * @param ptr_sketch_2  pointer to the second sketch
   * @param threshold     threshold value of heavy changers
   * @param gnd_truth_heavy_changers  ground truth of heavy changers (relative
   * to the first sketch)
   */
  virtual void testHeavyChanger(
      std::unique_ptr<Sketch::SketchBase<key_len, T>> &ptr_sketch_1,
      std::unique_ptr<Sketch::SketchBase<key_len, T>> &ptr_sketch_2,
      double threshold,
      Data::GndTruth<key_len, T> gnd_truth_heavy_changers) final;
};

} // namespace OmniSketch::Test

//-----------------------------------------------------------------------------
//
///                        Implementation of templated methods
//
//-----------------------------------------------------------------------------

namespace OmniSketch::Test {

#define DEFINE_TIMERS                                                          \
  auto timer = std::chrono::microseconds::zero();                              \
  auto tick = std::chrono::steady_clock::now();                                \
  auto tock = std::chrono::steady_clock::now();
#define START_TIMER tick = std::chrono::steady_clock::now();
#define STOP_TIMER                                                             \
  tock = std::chrono::steady_clock::now();                                     \
  timer += std::chrono::duration_cast<std::chrono::microseconds>(tock - tick);
#define TIMER_RESULT static_cast<int64_t>(timer.count())

template <int32_t key_len, typename T> void TestBase<key_len, T>::runTest() {
  LOG(ERROR, "You should override TestBase::runTest() in subclass.");
  return;
}

template <int32_t key_len, typename T> void TestBase<key_len, T>::show() const {
  auto foo = [](const Vec &vec, const std::string_view prefix) {
    if (vec.count(SIZE)) {
      assert(vec.at(SIZE).type() == typeid(size_t));
      size_t size = boost::any_cast<size_t>(vec.at(SIZE));
      if (size < 1024) {
        fmt::print("{:>15}: {:d} B\n", "Mem Footprint", size);
      } else if (size < (1 << 20)) {
        fmt::print("{:>15}: {:g} kB\n", "Mem Footprint", size / 1024.0);
      } else {
        fmt::print("{:>15}: {:g} MB\n", "Mem Footprint",
                   size / 1024.0 / 1024.0);
      }
    }
    if (vec.count(TIME)) {
      assert(vec.at(TIME).type() == typeid(int64_t));
      int64_t time = boost::any_cast<int64_t>(vec.at(TIME));
      if (time < 1e3) {
        fmt::print("{:>15}: {:d} us\n", fmt::format("{} Time", prefix), time);
      } else if (time < 1e6) {
        fmt::print("{:>15}: {:g} ms\n", fmt::format("{} Time", prefix),
                   time / 1e3);
      } else {
        fmt::print("{:>15}: {:g} s\n", fmt::format("{} Time", prefix),
                   time / 1e6);
      }
    }
    if (vec.count(RATE)) {
      assert(vec.at(RATE).type() == typeid(double));
      double rate = boost::any_cast<double>(vec.at(RATE));
      if (rate < 1e3) {
        fmt::print("{:>15}: {:g} pac/s\n", fmt::format("{} Rate", prefix),
                   rate);
      } else if (rate < 1e6) {
        fmt::print("{:>15}: {:g} Kpac/s\n", fmt::format("{} Rate", prefix),
                   rate / 1e3);
      } else {
        fmt::print("{:>15}: {:g} Mpac/s\n", fmt::format("{} Rate", prefix),
                   rate / 1e6);
      }
    }
    if (vec.count(ARE)) {
      assert(vec.at(ARE).type() == typeid(double));
      fmt::print("{:>15}: {:g}\n", fmt::format("{} ARE", prefix),
                 boost::any_cast<double>(vec.at(ARE)));
    }
    if (vec.count(AAE)) {
      assert(vec.at(AAE).type() == typeid(double));
      fmt::print("{:>15}: {:g}\n", fmt::format("{} AAE", prefix),
                 boost::any_cast<double>(vec.at(AAE)));
    }
    if (vec.count(ACC)) {
      assert(vec.at(ACC).type() == typeid(double));
      fmt::print("{:>15}: {:g}%\n", fmt::format("{} Acc", prefix),
                 boost::any_cast<double>(vec.at(ACC)) * 1e2);
    }
    if (vec.count(TP)) {
      assert(vec.at(TP).type() == typeid(double));
      fmt::print("{:>15}: {:g}%\n", fmt::format("{} TP", prefix),
                 boost::any_cast<double>(vec.at(TP)) * 1e2);
    }
    if (vec.count(FP)) {
      assert(vec.at(FP).type() == typeid(double));
      fmt::print("{:>15}: {:g}%\n", fmt::format("{} FP", prefix),
                 boost::any_cast<double>(vec.at(FP)) * 1e2);
    }
    if (vec.count(TN)) {
      assert(vec.at(TN).type() == typeid(double));
      fmt::print("{:>15}: {:g}%\n", fmt::format("{} TN", prefix),
                 boost::any_cast<double>(vec.at(TN)) * 1e2);
    }
    if (vec.count(FN)) {
      assert(vec.at(FN).type() == typeid(double));
      fmt::print("{:>15}: {:g}%\n", fmt::format("{} FN", prefix),
                 boost::any_cast<double>(vec.at(FN)) * 1e2);
    }
    if (vec.count(PRC)) {
      assert(vec.at(PRC).type() == typeid(double));
      fmt::print("{:>15}: {:g}%\n", fmt::format("{} Prec", prefix),
                 boost::any_cast<double>(vec.at(PRC)) * 1e2);
    }
    if (vec.count(RCL)) {
      assert(vec.at(RCL).type() == typeid(double));
      fmt::print("{:>15}: {:g}%\n", fmt::format("{} RCL", prefix),
                 boost::any_cast<double>(vec.at(RCL)) * 1e2);
    }
    if (vec.count(F1)) {
      assert(vec.at(F1).type() == typeid(double));
      fmt::print("{:>15}: {:g}\n", fmt::format("{} F1", prefix),
                 boost::any_cast<double>(vec.at(F1)));
    }
    /**
     * @todo DIST
     */
    if (vec.count(PODF)) {
      assert(vec.at(PODF).type() == typeid(std::pair<double, double>));
      std::pair<double, double> tmp =
          boost::any_cast<std::pair<double, double>>(vec.at(PODF));
      fmt::print("{:>15}: {:g}%\n",
                 fmt::format("{} <={:g}%", prefix, tmp.first * 1e2),
                 tmp.second * 1e2);
    }
  };
  // prologue
  fmt::print("============ {:^18} ============\n", show_name);

  // size
  foo(size, "Size");
  // insert
  foo(insert, "Insert");
  // lookup
  foo(lookup, "Lookup");
  // update
  foo(update, "Update");
  // query
  foo(query, "Query");
  // heavy_hitter
  foo(heavy_hitter, "HH");
  // heavy_changer
  foo(heavy_changer, "HC");
  // epilogue
  fmt::print("============================================\n");
}

template <int32_t key_len, typename T>
void TestBase<key_len, T>::testSize(
    std::unique_ptr<Sketch::SketchBase<key_len, T>> &ptr_sketch) {
  size[Metric::SIZE] = ptr_sketch->size();
}

template <int32_t key_len, typename T>
void TestBase<key_len, T>::testInsert(
    std::unique_ptr<Sketch::SketchBase<key_len, T>> &ptr_sketch,
    typename std::vector<Data::Record<key_len>>::const_iterator begin,
    typename std::vector<Data::Record<key_len>>::const_iterator end) {
  // config
  MetricVec metric_vec(config_file, test_path, "insert");

  DEFINE_TIMERS;
  for (auto ptr = begin; ptr != end; ptr++) {
    START_TIMER;
    ptr_sketch->insert(ptr->flowkey);
    STOP_TIMER;
  }
  if (metric_vec.in(Metric::RATE)) {
    insert[Metric::RATE] = 1.0 * (end - begin) / TIMER_RESULT * 1e6;
  }
}

template <int32_t key_len, typename T>
void TestBase<key_len, T>::testUpdate(
    std::unique_ptr<Sketch::SketchBase<key_len, T>> &ptr_sketch,
    typename std::vector<Data::Record<key_len>>::const_iterator begin,
    typename std::vector<Data::Record<key_len>>::const_iterator end,
    Data::CntMethod cnt_method) {
  // config
  MetricVec metric_vec(config_file, test_path, "update");

  DEFINE_TIMERS;
  for (auto ptr = begin; ptr != end; ptr++) {
    START_TIMER;
    ptr_sketch->update(ptr->flowkey,
                       cnt_method == Data::InLength ? ptr->length : 1);
    STOP_TIMER;
  }
  if (metric_vec.in(Metric::RATE))
    update[Metric::RATE] = 1.0 * (end - begin) / TIMER_RESULT * 1e6;
}

template <int32_t key_len, typename T>
void TestBase<key_len, T>::testQuery(
    std::unique_ptr<Sketch::SketchBase<key_len, T>> &ptr_sketch,
    const Data::GndTruth<key_len, T> &gnd_truth) {
  // config
  MetricVec metric_vec(config_file, test_path, "query");

  DEFINE_TIMERS;
  double ARE = 0.0, AAE = 0.0, corr = 0, podf_cnt = 0;
  const bool measure_dist = metric_vec.in(Metric::DIST);
  std::vector<double> dist(metric_vec.quantiles.size()); // zero initialized

  for (const auto &kv : gnd_truth) {
    START_TIMER;
    T estimated_size = ptr_sketch->query(kv.get_left());
    STOP_TIMER;
    // update RE, AE, Correct Rate, PODF
    double RE = static_cast<double>(std::abs(kv.get_right() - estimated_size)) /
                kv.get_right();
    if (RE <= metric_vec.podf)
      podf_cnt += 1.0;
    ARE += RE;
    AAE += std::abs(kv.get_right() - estimated_size);
    corr += (kv.get_right() == estimated_size);
    // update Distribution
    if (measure_dist) {
      auto ptr = std::lower_bound(metric_vec.quantiles.begin(),
                                  metric_vec.quantiles.end(), RE);
      dist[ptr - metric_vec.quantiles.begin()] += 1.0;
    }
  }
  // add statistics
  if (metric_vec.in(Metric::RATE)) {
    query[Metric::RATE] = 1.0 * gnd_truth.size() / TIMER_RESULT * 1e6;
  }
  if (metric_vec.in(Metric::ARE)) {
    query[Metric::ARE] = ARE / gnd_truth.size();
  }
  if (metric_vec.in(Metric::AAE)) {
    query[Metric::AAE] = AAE / gnd_truth.size();
  }
  if (metric_vec.in(Metric::ACC)) {
    query[Metric::ACC] = corr / gnd_truth.size();
  }
  if (metric_vec.in(PODF)) {
    query[Metric::PODF] =
        std::make_pair(metric_vec.podf, podf_cnt / gnd_truth.size());
  }
  if (measure_dist) {
    for (auto &v : dist)
      v /= gnd_truth.size();
    query[Metric::DIST] = std::make_pair(metric_vec.quantiles, dist);
  }
}

template <int32_t key_len, typename T>
void TestBase<key_len, T>::testLookup(
    std::unique_ptr<Sketch::SketchBase<key_len, T>> &ptr_sketch,
    const Data::GndTruth<key_len, T> &gnd_truth,
    const Data::GndTruth<key_len, T> &sample) {
  // config
  MetricVec metric_vec(config_file, test_path, "lookup");

  DEFINE_TIMERS;
  double TP = 0.0, FP = 0.0;
  for (const auto &kv : gnd_truth) {
    START_TIMER;
    bool existed = ptr_sketch->lookup(kv.get_left());
    STOP_TIMER;
    // update TP, FP
    if (existed) {
      if (sample.count(kv.get_left()))
        TP += 1.0;
      else
        FP += 1.0;
    }
  }
  // add statistics
  if (metric_vec.in(Metric::RATE)) {
    lookup[Metric::RATE] = 1.0 * gnd_truth.size() / TIMER_RESULT * 1e6;
  }
  if (metric_vec.in(Metric::TP)) {
    lookup[Metric::TP] = TP / gnd_truth.size();
  }
  if (metric_vec.in(Metric::FP)) {
    lookup[Metric::FP] = FP / gnd_truth.size();
  }
  if (metric_vec.in(PRC)) {
    lookup[Metric::PRC] = TP / (TP + FP);
  }
}

template <int32_t key_len, typename T>
void TestBase<key_len, T>::testHeavyHitter(
    std::unique_ptr<Sketch::SketchBase<key_len, T>> &ptr_sketch,
    double threshold, Data::GndTruth<key_len, T> gnd_truth_heavy_hitters) {
  // config
  MetricVec metric_vec(config_file, test_path, "heavyhitter");

  double TP = 0.0, FP = 0.0, FN = 0.0, ARE = 0.0;

  DEFINE_TIMERS;
  START_TIMER;
  Data::Estimation<key_len, T> detected = ptr_sketch->getHeavyHitter(threshold);
  STOP_TIMER;

  for (const auto &kv : gnd_truth_heavy_hitters) {
    if (detected.count(kv.get_left())) {
      TP += 1.0;
      ARE += static_cast<double>(
                 std::abs(detected.at(kv.get_left()) - kv.get_right())) /
             kv.get_right();
    } else {
      FN += 1.0;
    }
  }
  FP = detected.size() - TP;

  double precision = TP / (TP + FP);
  double recall = TP / (TP + FN);

  if (metric_vec.in(Metric::TIME)) {
    heavy_hitter[Metric::TIME] = TIMER_RESULT;
  }
  if (metric_vec.in(Metric::ARE)) {
    heavy_hitter[Metric::ARE] = ARE / TP;
  }
  if (metric_vec.in(Metric::PRC)) {
    heavy_hitter[Metric::PRC] = precision;
  }
  if (metric_vec.in(Metric::RCL)) {
    heavy_hitter[Metric::RCL] = recall;
  }
  if (metric_vec.in(Metric::F1)) {
    heavy_hitter[Metric::F1] = 2 * precision * recall / (precision + recall);
  }
}

template <int32_t key_len, typename T>
void TestBase<key_len, T>::testHeavyChanger(
    std::unique_ptr<Sketch::SketchBase<key_len, T>> &ptr_sketch_1,
    std::unique_ptr<Sketch::SketchBase<key_len, T>> &ptr_sketch_2,
    double threshold, Data::GndTruth<key_len, T> gnd_truth_heavy_changers) {
  // config
  MetricVec metric_vec(config_file, test_path, "heavychanger");

  double TP = 0.0, FP = 0.0, FN = 0.0, ARE = 0.0;

  DEFINE_TIMERS;
  START_TIMER;
  Data::Estimation<key_len, T> detected =
      ptr_sketch_1->getHeavyChanger(ptr_sketch_2, threshold);
  STOP_TIMER;

  for (const auto &kv : gnd_truth_heavy_changers) {
    if (detected.count(kv.get_left())) {
      TP += 1.0;
      ARE += static_cast<double>(
                 std::abs(detected.at(kv.get_left()) - kv.get_right())) /
             kv.get_right();
    } else {
      FN += 1.0;
    }
  }
  FP = detected.size() - TP;

  double precision = TP / (TP + FP);
  double recall = TP / (TP + FN);

  if (metric_vec.in(Metric::TIME)) {
    heavy_changer[Metric::TIME] = TIMER_RESULT;
  }
  if (metric_vec.in(Metric::ARE)) {
    heavy_changer[Metric::ARE] = ARE / TP;
  }
  if (metric_vec.in(Metric::PRC)) {
    heavy_changer[Metric::PRC] = precision;
  }
  if (metric_vec.in(Metric::RCL)) {
    heavy_changer[Metric::RCL] = recall;
  }
  if (metric_vec.in(Metric::F1)) {
    heavy_changer[Metric::F1] = 2 * precision * recall / (precision + recall);
  }
}

#undef DEFINE_TIMERS
#undef START_TIMER
#undef STOP_TIMER
#undef TIMER_RESULT

} // namespace OmniSketch::Test