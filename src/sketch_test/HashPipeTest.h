/**
 * @file HashPipeTest.h
 * @author KyleLv (you@domain.com)
 * @brief Testing Bloom Filter
 *
 * @copyright Copyright (c) 2022
 *
 */
#pragma once

#include <common/test.h>
#include <sketch/HashPipe.h>

#define HP_PARA_PATH "HP.para"
#define HP_TEST_PATH "HP.test"
#define HP_DATA_PATH "HP.data"

namespace OmniSketch::Test {
/**
 * @brief Testing class for Bloom Filter
 *
 */
template <int32_t key_len, typename T, typename hash_t = Hash::AwareHash>
class HashPipeTest : public TestBase<key_len, T> {
  using TestBase<key_len, T>::config_file;

public:
  /**
   * @brief Constructor
   * @details Names from left to right are
   * - name for showing
   * - config file [This one should be passed from the .cpp file]
   * - table name in the config file
   * - sub-table name (also a node name) under the table, containing metrics
   * of interest
   * - sub-table name (also a node name) under the table, containing data info
   */
  HashPipeTest(const std::string_view config_file)
      : TestBase<key_len, T>("Hash Pipe", config_file, HP_TEST_PATH) {}

  /**
   * @brief Test Bloom Filter
   * @details An overriden method
   */
  void runTest() override;
};

} // namespace OmniSketch::Test

//-----------------------------------------------------------------------------
//
///                        Implementation of template methods
//
//-----------------------------------------------------------------------------

namespace OmniSketch::Test {

template <int32_t key_len, typename T, typename hash_t>
void HashPipeTest<key_len, T, hash_t>::runTest() {
  /**
   * @brief shorthand for convenience
   *
   */
  using StreamData = Data::StreamData<key_len>;

  /// Part I.
  ///   Parse the config file
  ///
  /// Step i.  First we list the variables to parse, namely:
  ///
  int32_t depth, width; // sketch config
  double num_heavy_hitter;
  std::string data_file; // data config
  toml::array arr;       // shortly we will convert it to format
  /// Step ii. Open the config file
  Util::ConfigParser parser(config_file);
  if (!parser.succeed()) {
    return;
  }
  /// Step iii. Set the working node of the parser.
  parser.setWorkingNode(
      HP_PARA_PATH); // do not forget to to enclose it with braces
  /// Step iv. Parse num_bits and num_hash
  if (!parser.parseConfig(depth, "depth"))
    return;
  if (!parser.parseConfig(width, "width"))
    return;
  /// Step v. To know about the data, we  switch to [HP.data].
  parser.setWorkingNode(HP_DATA_PATH);
  /// Step vi. Parse data and format
  if (!parser.parseConfig(num_heavy_hitter, "threshold_heavy_hitter"))
    return;
  if (!parser.parseConfig(data_file, "data"))
    return;
  if (!parser.parseConfig(arr, "format"))
    return;
  Data::DataFormat format(arr); // conver from toml::array to Data::DataFormat
  /// [Optional] User-defined rules
  std::string method;
  Data::HXMethod hx_method = Data::TopK;
  if (!parser.parseConfig(method, "hx_method"))
    return;
  if (!method.compare("Percentile")) {
    hx_method = Data::Percentile;
  }
  Data::CntMethod cnt_method = Data::InLength;
  if (!parser.parseConfig(method, "cnt_method"))
    return;
  if (!method.compare("InPacket")) {
    cnt_method = Data::InPacket;
  }

  /// Part II.
  ///   Prepare sketch and data
  ///
  /// Step i. Initialize a sketch
  std::unique_ptr<Sketch::SketchBase<key_len, T>> ptr(
      new Sketch::HashPipe<key_len, T, hash_t>(depth, width));
  /// remember that the left ptr must point to the base class in order to call
  /// the methods in it

  /// Step ii. Get ground truth
  ///
  ///       1. read data
  StreamData data(data_file, format); // specify both data file and data format
  if (!data.succeed())
    return;
  Data::GndTruth<key_len, T> gnd_truth, gnd_truth_heavy_hitters;
  gnd_truth.getGroundTruth(data.begin(), data.end(), cnt_method);
  gnd_truth_heavy_hitters.getHeavyHitter(gnd_truth, num_heavy_hitter,
                                         hx_method);
  ///       2. [optional] show data info
  fmt::print("DataSet: {:d} records with {:d} keys ({})\n", data.size(),
             gnd_truth.size(), data_file);
  /// Step iii. Insert the samples and then look up all the flows
  ///
  ///        1. update records into the sketch
  this->testUpdate(ptr, data.begin(), data.end(),
                   cnt_method); // metrics of interest are in config file
  ///        2. query for all the flowkeys
  if (hx_method == Data::TopK) {
    this->testHeavyHitter(
        ptr, gnd_truth_heavy_hitters.min(),
        gnd_truth_heavy_hitters); // metrics of interest are in config file
  } else {
    this->testHeavyHitter(
        ptr, std::floor(gnd_truth.totalValue() * num_heavy_hitter + 1),
        gnd_truth_heavy_hitters); // gnd_truth_heavy_hitter: >, yet HashPipe: >=
  }
  ///        3. size
  this->testSize(ptr);
  ///        3. show metrics
  this->show();

  return;
}

} // namespace OmniSketch::Test

#undef HP_PARA_PATH
#undef HP_TEST_PATH
#undef HP_DATA_PATH

// Driver instance:
//      AUTHOR: KyleLv
//      CONFIG: sketch_config.toml  # with respect to the `src/` directory
//    TEMPLATE: <13, int32_t, Hash::AwareHash>