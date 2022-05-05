/**
 * @file BloomFilterTest.h
 * @author dromniscience (you@domain.com)
 * @brief Testing Bloom Filter
 *
 * @copyright Copyright (c) 2022
 *
 */
#pragma once

#include <common/test.h>
#include <sketch/BloomFilter.h>

#define BF_PARA_PATH "BF.para"
#define BF_TEST_PATH "BF.test"
#define BF_DATA_PATH "BF.data"

namespace OmniSketch::Test {
/**
 * @brief Testing class for Bloom Filter
 *
 */
template <int32_t key_len, typename hash_t = Hash::AwareHash>
class BloomFilterTest : public TestBase<key_len> {
  using TestBase<key_len>::config_file;

public:
  /**
   * @brief Constructor
   * @details Names from left to right are
   * - show name
   * - config file
   * - path to the node that contains metrics of interest (concatenated with
   * '.')
   */
  BloomFilterTest(const std::string_view config_file)
      : TestBase<key_len>("Bloom Filter", config_file, BF_TEST_PATH) {}

  /**
   * @brief Test Bloom Filter
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

template <int32_t key_len, typename hash_t>
void BloomFilterTest<key_len, hash_t>::runTest() {
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
  int32_t nbit, nhash;   // sketch config
  std::string data_file; // data config
  toml::array arr;       // shortly we will convert it to format
  /// Step ii. Open the config file
  Util::ConfigParser parser(config_file);
  if (!parser.succeed()) {
    return;
  }
  /// Step iii. Set the working node of the parser.
  parser.setWorkingNode(BF_PARA_PATH);
  /// Step iv. Parse num_bits and num_hash
  if (!parser.parseConfig(nbit, "num_bits"))
    return;
  if (!parser.parseConfig(nhash, "num_hash"))
    return;
  /// Step v. Ready to read data configurations
  parser.setWorkingNode(BF_DATA_PATH);
  /// Step vi. Parse data and format
  if (!parser.parseConfig(data_file, "data"))
    return;
  if (!parser.parseConfig(arr, "format"))
    return;
  Data::DataFormat format(arr); // conver from toml::array to Data::DataFormat
  /// [Optional] User-defined rules
  ///
  /// In BF, we still have an parameter to control how much it samples. It is in
  /// [BF.data] node.
  ///
  /// Step vii. Parse other testing configurations.
  double sample;
  parser.setWorkingNode(BF_TEST_PATH);
  if (!parser.parseConfig(sample, "sample"))
    return;
  if (sample <= 0. && sample > 1.) {
    throw std::out_of_range(
        "Sample Rate Out Of Range: Should be in (0,1], but got " +
        std::to_string(sample) + " instead.");
  }

  /// Part II.
  ///   Prepare sketch and data
  ///
  /// Step i. Initialize a sketch
  std::unique_ptr<Sketch::SketchBase<key_len>> ptr(
      new Sketch::BloomFilter<key_len, hash_t>(nbit, nhash));
  /// remember that the left ptr must point to the base class in order to call
  /// the methods in it

  /// Step ii. Get the ground truth
  ///
  ///       1. read data
  StreamData data(data_file,
                  format); // specifying both data file and data format
  if (!data.succeed())
    return;
  ///       2. find ending point of the sampling
  auto data_ptr = data.diff(static_cast<std::size_t>(sample * data.size()));
  ///       3. use the whole stream as the ground truth, but only a $sample
  ///       fraction as the sample
  Data::GndTruth<key_len> gnd_truth, sample_truth;
  gnd_truth.getGroundTruth(data.begin(), data.end(),
                           Data::CntMethod::InPacket); // all flows
  sample_truth.getGroundTruth(
      data.begin(), data_ptr,
      Data::CntMethod::InPacket); // first $sample fraction of records
  ///       4. [optional] show data info
  fmt::print("DataSet: {:d} records with {:d} keys ({})\n", data.size(),
             gnd_truth.size(), data_file);

  /// Step iii. Insert the samples and then look up all the flows
  ///
  ///        1. insert the sampled records
  this->testInsert(ptr, data.begin(),
                   data_ptr); // metrics of interest are in config file
  ///        2. look up all the flows
  this->testLookup(ptr, gnd_truth,
                   sample_truth); // metrics of interest are in config file
  ///        3. test size
  this->testSize(ptr);
  ///        4. show metrics
  this->show();

  return;
}

} // namespace OmniSketch::Test

#undef BF_PARA_PATH
#undef BF_TEST_PATH
#undef BF_DATA_PATH

// Driver instance:
//      AUTHOR: dromniscience
//      CONFIG: sketch_config.toml  # with respect to the `src/` directory
//    TEMPLATE: <13, Hash::AwareHash>
