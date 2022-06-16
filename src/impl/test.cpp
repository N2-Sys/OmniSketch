/**
 * @file test.cpp
 * @author dromniscience (you@domain.com)
 * @brief Implementation of some test methods
 *
 * @copyright Copyright (c) 2022
 *
 */
#include <common/test.h>

namespace OmniSketch::Test {

MetricVec::MetricVec(const std::string_view file_path,
                     const std::string_view test_path,
                     const std::string_view term_name) {

  // open config
  Util::ConfigParser parser(file_path);
  if (!parser.succeed())
    return;

  // set working node
  parser.setWorkingNode(test_path);

  std::vector<std::string> arr;
  // return if the parsing failed
  if (!parser.parseConfig(arr, term_name))
    return;
  // collect metric indices
  bool is_podf = false, is_dist = false;
  for (const auto &index : arr) {
    if (!index.compare("SIZE")) {
      metric_set.insert(Metric::SIZE);
    } else if (!index.compare("TIME")) {
      metric_set.insert(Metric::TIME);
    } else if (!index.compare("RATE")) {
      metric_set.insert(Metric::RATE);
    } else if (!index.compare("ARE")) {
      metric_set.insert(Metric::ARE);
    } else if (!index.compare("AAE")) {
      metric_set.insert(Metric::AAE);
    } else if (!index.compare("ACC")) {
      metric_set.insert(Metric::ACC);
    } else if (!index.compare("TP")) {
      metric_set.insert(Metric::TP);
    } else if (!index.compare("FP")) {
      metric_set.insert(Metric::FP);
    } else if (!index.compare("TN")) {
      metric_set.insert(Metric::TN);
    } else if (!index.compare("FN")) {
      metric_set.insert(Metric::FN);
    } else if (!index.compare("PRC")) {
      metric_set.insert(Metric::PRC);
    } else if (!index.compare("RCL")) {
      metric_set.insert(Metric::RCL);
    } else if (!index.compare("F1")) {
      metric_set.insert(Metric::F1);
    } else if (!index.compare("DIST")) {
      is_dist = true;
      metric_set.insert(Metric::DIST);
    } else if (!index.compare("PODF")) {
      is_podf = true;
      metric_set.insert(Metric::PODF);
    } else if (!index.compare("RATIO")) {
      metric_set.insert(Metric::RATIO);
    }
  }
  // If distribution is specified
  if (is_dist) {
    std::string dist_name = std::string(term_name) + "_dist";
    if (!parser.parseConfig(quantiles, dist_name)) {
      LOG(ERROR,
          fmt::format("Bad quantiles for distribution in test {}", term_name));
      metric_set.erase(Metric::DIST);
    } else {
      // sort and unique
      std::sort(quantiles.begin(), quantiles.end());
      auto iter = std::unique(quantiles.begin(), quantiles.end());
      quantiles.erase(iter, quantiles.end());
      // ended with +inf
      if (quantiles.back() != std::numeric_limits<double>::infinity()) {
        quantiles.push_back(std::numeric_limits<double>::infinity());
      }
    }
  }
  // If PODF is specified
  if (is_podf) {
    std::string podf_name = std::string(term_name) + "_podf";
    if (!parser.parseConfig(podf, podf_name)) {
      LOG(ERROR, fmt::format("Bad threshold for PODF in test {}", term_name));
      metric_set.erase(Metric::PODF);
    }
  }
}

} // namespace OmniSketch::Test