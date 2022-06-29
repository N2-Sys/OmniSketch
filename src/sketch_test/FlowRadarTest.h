/**
 * @file FlowRadarTest.h
 * @author dromniscience (you@domain.com)
 * @brief Test Flow Radar
 *
 * @copyright Copyright (c) 2022
 *
 */
#pragma once

#include <common/test.h>
#include <sketch/FlowRadar.h>

#define FR_PARA_PATH "FlowRadar.para"
#define FR_TEST_PATH "FlowRadar.test"
#define FR_DATA_PATH "FlowRadar.data"

namespace OmniSketch::Test {

/**
 * @brief Testing class for Flow Radar
 *
 */
template <int32_t key_len, typename T, typename hash_t = Hash::AwareHash>
class FlowRadarTest : public TestBase<key_len, T> {
  using TestBase<key_len, T>::config_file;

public:
  /**
   * @brief Constructor
   * @details Names from left to right are
   * - show name
   * - config file
   * - path to the node that contains metrics of interest (concatenated with
   * '.')
   */
  FlowRadarTest(const std::string_view config_file)
      : TestBase<key_len, T>("Flow Radar", config_file, FR_TEST_PATH) {}

  /**
   * @brief Test Flow Radar
   * @details An overriden method
   */
  void runTest() override;
};

} // namespace OmniSketch::Test

//-----------------------------------------------------------------------------
//
///                        Implementation of templated methods
//
//-----------------------------------------------------------------------------

namespace OmniSketch::Test {

template <int32_t key_len, typename T, typename hash_t>
void FlowRadarTest<key_len, T, hash_t>::runTest() {
  // for convenience only
  using StreamData = Data::StreamData<key_len>;

  // parse config
  int32_t flow_filter_bit, flow_filter_hash, count_table_num,
      count_table_hash;  // sketch config
  std::string data_file; // data config
  toml::array arr;       // shortly we will convert it to format

  Util::ConfigParser parser(config_file);
  if (!parser.succeed()) {
    return;
  }

  parser.setWorkingNode(FR_PARA_PATH);
  if (!parser.parseConfig(flow_filter_bit, "flow_filter_bit"))
    return;
  if (!parser.parseConfig(flow_filter_hash, "flow_filter_hash"))
    return;
  if (!parser.parseConfig(count_table_num, "count_table_num"))
    return;
  if (!parser.parseConfig(count_table_hash, "count_table_hash"))
    return;

  // prepare data
  parser.setWorkingNode(FR_DATA_PATH);
  if (!parser.parseConfig(data_file, "data"))
    return;
  if (!parser.parseConfig(arr, "format"))
    return;
  Data::DataFormat format(arr);
  StreamData data(data_file, format);
  if (!data.succeed())
    return;
  Data::GndTruth<key_len, T> gnd_truth;
  gnd_truth.getGroundTruth(data.begin(), data.end(), Data::InPacket);
  fmt::print("DataSet: {:d} records with {:d} keys ({})\n", data.size(),
             gnd_truth.size(), data_file);

  std::unique_ptr<Sketch::SketchBase<key_len, T>> ptr(
      new Sketch::FlowRadar<key_len, T, hash_t>(
          flow_filter_bit, flow_filter_hash, count_table_num,
          count_table_hash));

  this->testSize(ptr);
  this->testUpdate(ptr, data.begin(), data.end(), Data::InPacket);
  this->testDecode(ptr, gnd_truth);
  // show
  this->show();

  return;
}

} // namespace OmniSketch::Test

#undef FR_PARA_PATH
#undef FR_TEST_PATH
#undef FR_DATA_PATH

// Driver instance:
//      AUTHOR: dromniscience
//      CONFIG: sketch_config.toml  # with respect to the `src/` directory
//    TEMPLATE: <13, int32_t, Hash::AwareHash>