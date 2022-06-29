/**
 * @file CountingBloomFilterTest.h
 * @author dromniscience (you@domain.com)
 * @brief Testing CBF
 *
 * @copyright Copyright (c) 2022
 *
 */
#pragma once

#include <common/test.h>
#include <sketch/CountingBloomFilter.h>

#define CBF_PARA_PATH "CBF.para"
#define CBF_TEST_PATH "CBF.test"
#define CBF_DATA_PATH "CBF.data"

namespace OmniSketch::Test {
/**
 * @brief Testing class for Bloom Filter
 *
 */
template <int32_t key_len, typename hash_t = Hash::AwareHash>
class CountingBloomFilterTest : public TestBase<key_len> {
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
  CountingBloomFilterTest(const std::string_view config_file)
      : TestBase<key_len>("CBF", config_file, CBF_TEST_PATH) {}

  /**
   * @brief Test Counting Bloom Filter
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
void CountingBloomFilterTest<key_len, hash_t>::runTest() {
  /**
   * @brief shorthand for convenience
   *
   */
  using StreamData = Data::StreamData<key_len>;

  int32_t ncnt, nhash, nbit; // sketch config
  std::string data_file;     // data config
  toml::array arr;           // shortly we will convert it to format

  Util::ConfigParser parser(config_file);
  if (!parser.succeed()) {
    return;
  }
  parser.setWorkingNode(CBF_PARA_PATH);
  if (!parser.parseConfig(ncnt, "num_cnt"))
    return;
  if (!parser.parseConfig(nhash, "num_hash"))
    return;
  if (!parser.parseConfig(nbit, "cnt_length"))
    return;

  parser.setWorkingNode(CBF_DATA_PATH);
  if (!parser.parseConfig(data_file, "data"))
    return;
  if (!parser.parseConfig(arr, "format"))
    return;
  Data::DataFormat format(arr); // conver from toml::array to Data::DataFormat

  double sample;
  parser.setWorkingNode(CBF_TEST_PATH);
  if (!parser.parseConfig(sample, "sample"))
    return;
  if (sample <= 0. && sample > 1.) {
    throw std::out_of_range(
        "Sample Rate Out Of Range: Should be in (0,1], but got " +
        std::to_string(sample) + " instead.");
  }

  std::unique_ptr<Sketch::SketchBase<key_len>> ptr(
      new Sketch::CountingBloomFilter<key_len, hash_t>(ncnt, nhash, nbit));

  StreamData data(data_file, format);
  if (!data.succeed())
    return;

  auto data_ptr = data.diff(static_cast<std::size_t>(sample * data.size()));
  Data::GndTruth<key_len> gnd_truth, sample_truth;
  gnd_truth.getGroundTruth(data.begin(), data.end(),
                           Data::CntMethod::InPacket); // all flows
  sample_truth.getGroundTruth(
      data.begin(), data_ptr,
      Data::CntMethod::InPacket); // first $sample fraction of records
  // show data info
  fmt::print("DataSet: {:d} records with {:d} keys ({})\n", data.size(),
             gnd_truth.size(), data_file);

  this->testInsert(ptr, data.begin(),
                   data_ptr); // metrics of interest are in config file
  this->testLookup(ptr, gnd_truth,
                   sample_truth); // metrics of interest are in config file
  this->testSize(ptr);
  this->show();

  return;
}

} // namespace OmniSketch::Test

#undef CBF_PARA_PATH
#undef CBF_TEST_PATH
#undef CBF_DATA_PATH

// Driver instance:
//      AUTHOR: dromniscience
//      CONFIG: sketch_config.toml  # with respect to the `src/` directory
//    TEMPLATE: <13, Hash::AwareHash>
