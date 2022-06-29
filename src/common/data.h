/**
 * @file data.h
 * @author dromniscience (you@domain.com)
 * @brief Program-level representation of streaming data
 *
 * @copyright Copyright (c) 2022
 *
 */
#pragma once

#include "flowkey.h"
#include "logger.h"
#include "utils.h"
#include <boost/bimap.hpp>
#include <boost/bimap/unordered_set_of.hpp>
#include <boost/bimap/vector_of.hpp>
#include <filesystem>
#include <fmt/core.h>

/**
 * @brief Miscellaneous tools for processing data.
 *
 */
namespace OmniSketch::Data {
/**
 * @brief Specify the counting method
 *
 */
enum CntMethod {
  InLength /** Count for (header + payload) in bytes */,
  InPacket /** Each packet is counted as 1 */
};
/**
 * @brief Specify the defining rule of heavy X (X = [hitters|changers])
 *
 */
enum HXMethod {
  TopK /** Top K flow(s) */,
  Percentile /** Flows that exceed a certain fraction of all counters */
};

/**
 * @brief Struct of a single record (i.e., a packet in a segment of streaming
 * data)
 *
 * @tparam key_len length of flowkey
 */
template <int32_t key_len> struct Record {
  /**
   * @brief the flowkey associated with the record
   *
   */
  FlowKey<key_len> flowkey;
  /**
   * @brief measured in microsecond (i.e., 1e-6s)
   *
   */
  int64_t timestamp;
  /**
   * @brief length of IP datagram (in bytes)
   *
   * @note Both header and payload are counted.
   */
  int64_t length;
};

/**
 * @brief Parse the format in config file
 *
 * @attention Specifications in config file must be in compliance with the
 * following rules. Otherwise an exception would be thrown.
 * | Field Name | Viable Length | Further Constraints                   |
 * |:-----------|:--------------|:--------------------------------------|
 * | flowkey    | 4, 8, 13      | Specify exactly once                  |
 * | timestamp  | 1, 2, 4, 8    | In microseconds. Specify at most once |
 * | length     | 1, 2, 4, 8    | Specify at most once                  |
 * | padding    | > 0           | None                                  |
 */
class DataFormat {
  /**
   * @brief Map different fields into array index
   *
   */
  enum { KEYLEN, TIMESTAMP, LENGTH, ENDALL };
  /**
   * @brief `offset` records the offset of the field. `length` notes field
   * length.
   *
   * @details If the field is not present, `offset` has its value `-1`.
   */
  int32_t offset[ENDALL], length[ENDALL];
  /**
   * @brief Total length of a record in byte array
   *
   */
  int32_t total;

public:
  /**
   * @brief Get the length of a record
   *
   */
  [[nodiscard]] int32_t getRecordLength() const { return total; }
  /**
   * @brief Get the key length
   *
   */
  [[nodiscard]] int32_t getKeyLength() const { return length[KEYLEN]; }
  /**
   * @brief Construct by specifications in config file
   *
   * @details It is suggested that `toml::array` is fetched via a call to
   * Util::ConfigParser::parseConfig(T &, const std::string_view) const with `T
   * = toml::array`.
   *
   */
  DataFormat(const toml::array &array);
  /**
   * @brief Unscramble byte string in given format
   *
   * @tparam key_len  length of flowkey
   * @param record    the record to be stored to
   * @param byte      pointer to byte string
   * @return pointer to the head of unprocessed byte string
   *
   * @attention An exception would be thrown if `key_len` does not match that
   * in DataFormat.
   */
  template <int32_t key_len>
  const int8_t *readAsFormat(Record<key_len> &record, const int8_t *byte) const;
  /**
   * @brief Scramble the record in given format
   *
   * @tparam key_len  length of flowkey
   * @param record    the record to be deserialized
   * @param byte      to store the deserialized message
   * @return right after the end of the message
   *
   * @attention An exception would be thrown if `key_len` does not match
   * that in DataFormat.
   */
  template <int32_t key_len>
  const int8_t *writeAsFormat(const Record<key_len> &record,
                              int8_t *byte) const;
};

/**
 * @brief Store the formatted streaming data
 *
 * @tparam key_len length of flowkey
 */
template <int32_t key_len> class StreamData {
  /**
   * @brief Internally shorthand a vector of flowkey as Stream.
   *
   */
  using Stream = std::vector<Record<key_len>>;
  /**
   * @brief Store a row of records in order
   *
   */
  Stream records;
  /**
   * @brief Whether the data is parsed successfully
   *
   */
  bool is_parsed;

public:
  /**
   * @brief Construct by specifying input file as well as the data format
   *
   * @param file_name   path to the input file
   * @param format      data format
   *
   * @note  On failure, records are left empty. Possible reasons for a failure:
   * - File does not exist.
   * - File is garbled. [i.e., its size is not a multiple of record size]
   */
  StreamData(const std::string_view file_name, const DataFormat &format);
  /**
   * @brief Return whether data file is successfully parsed
   *
   */
  [[nodiscard]] bool succeed() const { return is_parsed; }
  /**
   * @brief Check whether the records are read
   *
   * @return `true` if not empty. `false` otherwise.
   */
  [[nodiscard]] bool empty() const { return records.empty(); }
  /**
   * @brief Return the number of records in StreamData
   */
  [[nodiscard]] size_t size() const { return records.size(); }
  /**
   * @brief Return an iterator pointed to the very first record
   *
   * @return A random access iterator
   */
  [[nodiscard]] typename Stream::const_iterator begin() const {
    return records.cbegin();
  }
  /**
   * @brief Return an iterator pointed to the one after the very last record (in
   * cpp STL manner)
   *
   * @return A random access iterator
   */
  [[nodiscard]] typename Stream::const_iterator end() const {
    return records.cend();
  }
  /**
   * @brief Return an iterator pointed to the record at given offset
   *
   * @param offset offset into the record
   * @return A random access iterator
   *
   * @note If the index is out of range, an exception would be thrown.
   */
  [[nodiscard]] typename Stream::const_iterator diff(size_t offset) const {
    if (offset > records.size()) {
      throw std::out_of_range("Index Out Of Range: Expected to be in [0, " +
                              std::to_string(records.size()) + "], but got " +
                              std::to_string(offset) + " instead.");
    }
    return records.cbegin() + offset;
  }
};

/**
 * @brief Ground truth of the streaming data
 *
 * @details This class encapsulates
 * [boost::bimap](https://theboostcpplibraries.com/boost.bimap), a
 * time-efficient implementation of bidirectional map. See also [this
 * page](https://valelab4.ucsf.edu/svn/3rdpartypublic/boost/libs/bimap/doc/html/boost_bimap/the_tutorial.html)
 * for more information. It is left signature-compatible with
 * `std::unordered_map` and right signature-compatible with `std::vector`.
 * Besides, values are in descending order. Sensible users should not bother
 * with underlying data structure, since it is the black box! Moreover, the
 * class supports range-expression. It means that you may iterate all the
 * flowkeys in the following manner:
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~.cpp
 * using namespace OmniSketch::Data;
 * GndTruth<13, int32_t> gnd_truth;
 * for(const auto &kv: gnd_truth){
 *    kv.get_left(); // FlowKey<13>, equivalent to kv.second
 *    kv.get_right(); // int32_t, equivalent to kv.first
 * }
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 * @tparam T        type of counter
 * @tparam key_len  length of flowkey
 *
 * @note
 * - `T` is suggested to be `int32_t` when the counting method is chosen to be
 * `InPacket` and be `int64_t` on `InLength`, so as to avoid overflow.
 * - **Rationale:** It seems to be odd that so many methods, such as finding
 * heavy hitters, heavy changers & super spreaders, are designed to be
 * the class methods of GndTruth, in lieu of those of StreamData. Well, the
 * design is quite intuitive once you keep in mind that GndTruth should be
 * provided with streaming data to find the truth. Besides, GndTruth can be
 * extracted from another GndTruth. *See example below.*
 * ### Example
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~.cpp
 * using namespace OmniSketch::Data;
 *
 * // read data
 * StreamData<8> data("record.bin");
 * if(!data.succeed()) exit(-1);
 *
 * GndTruth<8, int32_t> flow_summary, heavy_hitter;
 *
 * // 1. flow summary
 * flow_summary.getGndTruth(data.begin(), data.end(), InLength);
 *
 * // 2. heavy hitter, extract from flow summary
 * heavy_hitter.getHeavyHitter(flow_summary, 0.1, TopK);
 *
 * // 2. is equivalent to:
 * //
 * // heavy_hitter.getHeavyHitter(data.begin(), data.end(), 0.1, TopK);
 * // // whose time complexity is about 1.+2.
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * @warning
 * - On each instance of GndTruth, you should call getXXX(...) method
 * **at most once**. Otherwise, you should define a new instance and call the
 * method upon the new one. *See example below.* The only exception happens when
 * you swap two GndTruth instances. In that case, their calling histories are
 * swapped as well. See swap().
 * - Any instance that is about to call getHeavyChanger() has to declare `T` as
 * *an signed type*, no matter the size, since there could be negative
 * values half way in arithemetic operations. In any other cases, declaring `T`
 * as *unsigned* is fine after all.
 *
 * ### A Second Example
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~.cpp
 * using namespace OmniSketch::Data;
 *
 * // read data
 * StreamData<8> data("record.bin");
 * if(!data.succeed()) exit(-1);
 *
 * GndTruth flow_summary, heavy_hitter;
 *
 * // 1. get ground truth
 * flow_summary.getGndTruth(data.begin(), data.end(), InLength);
 *
 * // Buggy code!!! Since getXXX(...) is called upon flow_summary a second time.
 * // 2. extract heavy hitter
 * flow_summary.getHeavyHitter(flow_summary, 0.1, Data::TopK);
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 */
template <int32_t key_len, typename T = int64_t> class GndTruth {
protected:
  using BidirMap =
      boost::bimaps::bimap<boost::bimaps::unordered_set_of<
                               FlowKey<key_len>, std::hash<FlowKey<key_len>>,
                               std::equal_to<FlowKey<key_len>>>,
                           boost::bimaps::vector_of<T>>;
  using RightVal = typename BidirMap::right_value_type;
  using LeftVal = typename BidirMap::left_value_type;
  using RightConstIterator = typename BidirMap::right_const_iterator;
  /**
   * @brief The internal bidirectional map
   *
   */
  BidirMap my_map;
  /**
   * @brief Sum of all counters
   *
   */
  int64_t tot_value = 0;
  /**
   * @brief How many times that getXXX are called on this instance
   *
   */
  int64_t called = 0;

private:
  /**
   * @brief Absolute difference between two flow summaries
   * @details Order of the right view is kept in descending order. Besides,
   * `tot_value` is updated accordingly.
   */
  GndTruth &operator-=(const GndTruth &other);

public:
  /**
   * @brief Return whether the instance is empty
   *
   */
  bool empty() const { return my_map.empty(); }
  /**
   * @brief Return the minimum value
   * @details Calling this function on an empty instance causes undefined
   * behavior.
   *
   */
  T min() const { return my_map.right.back().get_right(); }
  /**
   * @brief Return the maximum value
   * @details Calling this function on an empty instance causes undefined
   * behavior.
   */
  T max() const { return my_map.right.front().get_right(); }
  /**
   * @brief Return the sum of values of all flowkeys
   *
   */
  int64_t totalValue() const { return tot_value; }
  /**
   * @brief Swap content, note that calling histories are swapped as well
   *
   * @param other the other GndTruth instance to be swapped with
   *
   * @note It is fine if a GndTruth instance swap with itself. No effect at all.
   */
  void swap(GndTruth &other);
  /**
   * @brief Return a random access iterator pointed to the first element
   *
   * @details The next two examples show how to traverse the ground truth and
   * fetch length for flowkey:
   *
   * ### Examples
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~.cpp
   * // GndTruth<4> gnd_truth;
   * for(auto ptr = gnd_truth.begin(); ptr != gnd_truth.end(); ptr++){
   *    std::cout << ptr->get_left().getIp() << " " << ptr->get_right() <<
   *              std::endl;
   *    // Equivalently,
   *    std::cout << ptr->second.getIp() << " " << ptr->first << std::endl;
   * }
   * // ptr->second (same as ptr->get_left()): const FlowKey &
   * // ptr->first (same as ptr->get_right()): const T &
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   *
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~.cpp
   * // GndTruth<4> gnd_truth;
   * for(const auto &kv : gnd_truth){
   *    std::cout << kv.get_left().getIp() << " " << kv.get_right() <<
   *              std::endl;
   *    // Equivalently,
   *    std::cout << kv.second.getIp() << " " << kv.first << std::endl;
   * }
   * // kv.second (same as kv.get_left()): const FlowKey &
   * // kv.first (same as kv.get_right()): const T &
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   *
   */
  [[nodiscard]] RightConstIterator begin() const {
    return my_map.right.begin();
  }
  /**
   * @brief Return a random access const iterator pointed to the very end
   *
   * @see begin()
   */
  [[nodiscard]] RightConstIterator end() const { return my_map.right.end(); }
  /**
   * @brief return the number of flows
   */
  size_t size() const { return my_map.size(); }
  /**
   * @brief return whether a flowkey is in the streaming data or not
   *
   * @details Always `0` or `1` in this case.
   */
  size_t count(const FlowKey<key_len> &flowkey) const {
    return my_map.left.count(flowkey);
  }
  /**
   * @brief Get the value of a certain key
   *
   * @details If the key does not exist, an out-of-range exception would be
   * thrown.
   */
  T at(const FlowKey<key_len> &flowkey) const;
  /**
   * @brief Return all flowkeys that share a single value
   *
   * @details Time complexity is logarithm in the number of flowkeys
   *
   * @param value the value to query
   * @return A pair of random access iterator pointed to the range [start, end).
   * Below is a code snippet demonstrating how to iterate over the range and
   * extract flowkeys one by one.
   *
   * ### Example
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~.cpp
   * using namespace OmniSketch::Data;
   * // GndTruth<4> gndtruth;
   *
   * const auto pair = gndtruth.equalRange(20);
   * for(auto ptr = pair.first; ptr != pair.second; ptr++) {
   *    std::cout << ptr->get_left().getIp() << " " << ptr->get_right() <<
   *        std::endl;
   *    // Equivalently,
   *    std::cout << ptr->second.getIp() << " " << ptr->first << std::endl;
   * }
   *
   * // ptr->second (same as ptr->get_left()): const flowkey &
   * // ptr->first (same as ptr->get_right()): const T &
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   */
  [[nodiscard]] std::pair<RightConstIterator, RightConstIterator>
  equalRange(T value);
  /**
   * @brief Get the ground truth of the given stream
   *
   * @attention On success, the left view is a hash table that maps flowkey to
   * its value; the right view a sorted vector that maps value to flowkey in
   * descending order.
   *
   * @param begin       the beginning iterator (recommended to be return value
   * of StreamData<key_len>::begin() or of StreamData<key_len>::diff())
   * @param end         the ending iterator (recommended to be return value
   * of StreamData<key_len>::diff() or of StreamData<key_len>::end())
   * @param cnt_method  counting method
   *
   * @note
   * - The function will log flows whose `length<=0 || length > 1500` if
   * `cnt_method` is `InLength`. In this scenario, it is likely that either one
   * of the data format or the raw data is corrupted.
   * - **Rationale:** When `cnt_method` equals `InPacket`, even if there is
   * length info in the data, the method will not do the range check.
   * - Also, the function will complain for counter overflow or calling twice.
   * See warning in the comment of this class for more info.
   */
  void
  getGroundTruth(typename std::vector<Record<key_len>>::const_iterator begin,
                 typename std::vector<Record<key_len>>::const_iterator end,
                 CntMethod cnt_method);
  /**
   * @brief Get heavy hitters of the given stream (from flow summary)
   *
   * @details If `hh_method` equals `TopK`, the heaviest `threshold` flows are
   * to be counted as heavy hitters. Otherwise, all the flows who
   * contribute a fraction *strictly* greater than `threshold` are to be
   * counted.
   *
   * @param flow_summary  summary of flow, typically the one returned by
   * getGroundTruth()
   * @param threshold     threshold value
   * @param hh_method     definition of heavy hitter
   *
   * @note `threshold` should be in [1, infty) when `hh_method` equals `TopK`,
   * and should be in [0, 1] when `hh_method` equals `Percentile`. Otherwise, an
   * exception would be thrown.
   */
  void getHeavyHitter(const GndTruth &flow_summary, double threshold,
                      HXMethod hh_method);
  /**
   * @brief Get heavy hitters of the given stream (from flow summary)
   *
   * @see getHeavyHitter(const GndTruth &, double, HXMethod);
   *
   * @note This function is more time-efficient compared with its const
   * reference version (cf. getHeavyHitter(const GndTruth &, double, HXMethod))
   * since it does *not* copy any flowkeys from flow summary. Rather, it *moves*
   * them. Hence if your `flow_summary` is no longer in use, move it!
   */
  void getHeavyHitter(GndTruth &&flow_summary, double threshold,
                      HXMethod hh_method);
  /**
   * @brief Get heavy hitters from streaming data
   *
   * @details A combination of getGroundTruth() and
   * getHeavyHitter(GndTruth<key_len, T> &&, double, HXMethod). Provided for
   * the user's convenience.
   */
  void
  getHeavyHitter(typename std::vector<Record<key_len>>::const_iterator begin,
                 typename std::vector<Record<key_len>>::const_iterator end,
                 CntMethod cnt_method, double threshold, HXMethod hh_method);
  /**
   * @brief Get heavy changers of the given stream (from flow summary)
   *
   * @details Heavy changers are with respect to the two flow summaries. If
   * `hc_method` equals `TopK`, the heaviest `threshold` flows are to be counted
   * as heavy changers. Otherwise, all the flows whose deviation
   * contributes to the whole a fraction *strictly* greater than `threshold` are
   * counted.
   *
   * @param flow_summary_1  the first flow summary
   * @param flow_summary_2  the second flow summary
   * @param threshold       threshold value
   * @param hc_method       definition of heavy changers
   *
   * @note `threshold` should be in [1, infty) when `hh_method` is `TopK`,
   * and should be in [0, 1] when `hh_method` is `Percentile`. Otherwise, an
   * exception would be thrown.
   */
  void getHeavyChanger(const GndTruth &flow_summary_1,
                       const GndTruth &flow_summary_2, double threshold,
                       HXMethod hc_method);
  /**
   * @brief Get heavy changers of the given stream (from flow summary)
   *
   * @param flow_summary_1  the first flow summary
   * @param flow_summary_2  the second flow summary
   * @param threshold       threshold value
   * @param hc_method       definition of heavy changers
   *
   * @note This function is more time-efficient compared with its const
   * reference version (cf. getHeavyChanger(const GndTruth &, const GndTruth &,
   * double, HXMethod)) since it does *not* copy flowkeys from the first flow
   * summary. Rather, it *moves* them. Hence if your flow summaries are no
   * longer in use, move it!
   */
  void getHeavyChanger(GndTruth &&flow_summary_1, GndTruth &&flow_summary_2,
                       double threshold, HXMethod hc_method);
  /**
   * @brief Get heavy hitters from streaming data
   *
   * @details This function is provided for the user's convenience.
   *
   * @note What differs from getGroundTruth() is that this function does not
   * check counter overflow in the very detail. But it does check for spurious
   * packet length if `cnt_method` equals `InLength`.
   */
  void
  getHeavyChanger(typename std::vector<Record<key_len>>::const_iterator begin_1,
                  typename std::vector<Record<key_len>>::const_iterator end_1,
                  typename std::vector<Record<key_len>>::const_iterator begin_2,
                  typename std::vector<Record<key_len>>::const_iterator end_2,
                  CntMethod cnt_method, double threshold, HXMethod hc_method);
};

/**
 * @brief Output of sketch as estimation of ground truth
 *
 * @details This class provides an interface of bidirectional map similar to a
 * C++ hash table. What's more, right view need not to be a sorted vector,
 * reducing the algorithmic complexity.
 *
 * @tparam T        type of counter
 * @tparam key_len  length of flowkey
 */
template <int32_t key_len, typename T = int64_t>
class Estimation : private GndTruth<key_len, T> {
  using GndTruth<key_len, T>::my_map;

public:
  /**
   * @brief Return a random access iterator pointed to the first element
   * @see GndTruth::begin()
   *
   */
  [[nodiscard]] typename GndTruth<key_len, T>::RightConstIterator
  begin() const {
    return GndTruth<key_len, T>::begin();
  }
  /**
   * @brief Return a random access iterator pointed to the very end
   * @see GndTruth::end()
   */
  [[nodiscard]] typename GndTruth<key_len, T>::RightConstIterator end() const {
    return GndTruth<key_len, T>::end();
  }

  /**
   * @brief Insert a flowkey
   * @details Calling this function implies that values are uninterested. If the
   * flowkey does not exists yet, counters are initialized to zero by default.
   *
   * @return `false` if the flowkey has already existed; `true` otherwise.
   */
  bool insert(const FlowKey<key_len> &flowkey);
  /**
   * @brief Update a flowkey with a value specified
   * @details If the flowkey does not exists yet, counters are initialized to
   * zero by default.
   *
   * @return `false` if the flowkey has already existed; `true` otherwise.
   *
   */
  bool update(const FlowKey<key_len> &flowkey, T val);
  /**
   * @brief A more user-friendly interface.
   *
   * @details It serves as a substitute for insert() and update() methods. For
   * methods invocable on a const object, use at() instead.
   * ### Examples
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~.cpp
   * using namespace OmniSketch;
   * Data::Estimate<13> est;
   * FlowKey<13> flowkey;
   *
   * est[flowkey];      // Equivalent to: est.insert(flowkey);
   * est[flowkey] += 5; // Equivalent to: est.update(flowkey, 5);
   * est[flowkey] = 5;  // No equivalence
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   */
  T &operator[](const FlowKey<key_len> &flowkey);
  /**
   * @brief Return whether the flowkey exists in this estimate
   *
   * @details Always `0` or `1` in this case
   */
  size_t count(const FlowKey<key_len> &flowkey) const;
  /**
   * @brief Get the value of a certain key
   *
   * @details If the key does not exist, an out-of-range exception would be
   * thrown.
   */
  const T &at(const FlowKey<key_len> &flowkey) const;
  /**
   * @brief Return the number of flows
   */
  size_t size() const;
};

} // namespace OmniSketch::Data

//-----------------------------------------------------------------------------
//
///                        Implementation of templated methods
//
//-----------------------------------------------------------------------------

namespace OmniSketch::Data {

template <int32_t key_len>
const int8_t *DataFormat::readAsFormat(Record<key_len> &record,
                                       const int8_t *byte) const {
  auto convert = [](const int8_t *ptr, const int8_t len) -> int64_t {
    switch (len) { // never fall through
    case 1:
      return static_cast<int64_t>(*reinterpret_cast<const uint8_t *>(ptr));
    case 2:
      return static_cast<int64_t>(*reinterpret_cast<const uint16_t *>(ptr));
    case 4:
      return static_cast<int64_t>(*reinterpret_cast<const uint32_t *>(ptr));
    default:
      return *reinterpret_cast<const int64_t *>(ptr);
    }
  };
  if (key_len != length[KEYLEN]) {
    throw std::runtime_error("Runtime Error: Keylen of Record(" +
                             std::to_string(key_len) + ") and of DataFormat(" +
                             std::to_string(length[KEYLEN]) + ") mismatch.");
  }

  if (offset[KEYLEN] >= 0) {
    record.flowkey.copy(0, byte + offset[KEYLEN], length[KEYLEN]);
  }
  if (offset[TIMESTAMP] >= 0) {
    record.timestamp = convert(byte + offset[TIMESTAMP], length[TIMESTAMP]);
  }
  if (offset[LENGTH] >= 0) {
    record.length = convert(byte + offset[LENGTH], length[LENGTH]);
  }
  return byte + total;
}

template <int32_t key_len>
const int8_t *DataFormat::writeAsFormat(const Record<key_len> &record,
                                        int8_t *byte) const {
  auto my_memset = [](int8_t *ptr, int64_t value, const int8_t len) -> void {
    switch (len) { // never fall through
    case 1:
      *ptr = static_cast<int8_t>(value);
      return;
    case 2:
      *reinterpret_cast<int16_t *>(ptr) = static_cast<int16_t>(value);
      return;
    case 4:
      *reinterpret_cast<int32_t *>(ptr) = static_cast<int32_t>(value);
      return;
    default:
      *reinterpret_cast<int64_t *>(ptr) = static_cast<int64_t>(value);
      return;
    }
  };

  if (key_len != length[KEYLEN]) {
    throw std::runtime_error("Runtime Error: Keylen of Record(" +
                             std::to_string(key_len) + ") and of DataFormat(" +
                             std::to_string(length[KEYLEN]) + ") mismatch.");
  }

  ::memset(byte, 0, total);
  if (offset[KEYLEN] >= 0) {
    ::memcpy(byte + offset[KEYLEN], record.flowkey.cKey(), key_len);
  }
  if (offset[TIMESTAMP] >= 0) {
    my_memset(byte + offset[TIMESTAMP], record.timestamp, length[TIMESTAMP]);
  }
  if (offset[LENGTH] >= 0) {
    my_memset(byte + offset[LENGTH], record.length, length[LENGTH]);
  }
  return byte + total;
}

template <int32_t key_len>
StreamData<key_len>::StreamData(const std::string_view file_name,
                                const DataFormat &format) {
  // records are always empty

  LOG(VERBOSE, "Preparing test data...");
  // open files
  LOG(INFO, fmt::format("Loading records from {}...", file_name));
  std::ifstream fin(std::string(file_name), std::ios::binary);
  if (!fin.is_open()) {
    LOG(FATAL, fmt::format("Failed to open record file {}.", file_name));
    is_parsed = false;
    return; // fin automatically closed
  }
  // check if file size is a multiple of record size
  int32_t size = format.getRecordLength();
  auto file_size =
      std::filesystem::file_size(file_name); // do NOT use fseek, tellg or seekg
  if (file_size % size) {
    LOG(FATAL, fmt::format("Length of the file is not a multiple of that of "
                           "records. {} could have been garbled.",
                           file_name));
    is_parsed = false;
    return; // fin automatically closed
  }

  // read records in turn
  int8_t buf[size];
  Record<key_len> record;
  while (fin.read((char *)&buf, size)) {
    format.readAsFormat(record, buf);
    records.emplace_back(std::move(record));
  }
  LOG(VERBOSE, "Records Loaded.");
  is_parsed = true;
  return; // fin automatically closed
}

#define CHECK_CALLED_ONCE                                                      \
  called++;                                                                    \
  if (called > 1) {                                                            \
    LOG(WARNING,                                                               \
        fmt::format("getXXX(...) is called for the {:d}{} "                    \
                    "time. The instance is left unmodified.",                  \
                    called,                                                    \
                    (called % 10 == 2)                                         \
                        ? "nd"                                                 \
                        : ((called % 10 == 3)                                  \
                               ? "rd"                                          \
                               : ((called % 10 == 1) ? "st" : "th"))));        \
    return;                                                                    \
  }

#define ASSERT_AND_TRUNCATE_MYSELF_TO_THE_FIRST_K_ELEMENTS                     \
  if (threshold < 1.0) {                                                       \
    throw std::invalid_argument(                                               \
        "Invalid Argument: Threshold should >= 1.0 (Top-K), but got " +        \
        std::to_string(threshold) + " intsead.");                              \
  }                                                                            \
  auto size = my_map.size();                                                   \
  size_t no = std::min(size, static_cast<size_t>(threshold));                  \
  my_map.right.erase(my_map.right.begin() + no, my_map.right.end());           \
  for (auto ptr = my_map.begin(); ptr != my_map.end(); ptr++) {                \
    tot_value += ptr->get_right();                                             \
  }

#define ASSERT_AND_TRUNCATE_MYSELF_TO_ELEMENTS_WITH_GIVEN_VALUE                \
  if (!(threshold >= 0.0 && threshold <= 1.0)) {                               \
    throw std::invalid_argument("Invalid Argument: Threshold should be in "    \
                                "[0,1] (Percentile), but got " +               \
                                std::to_string(threshold) + " intsead.");      \
  }                                                                            \
  T thres = threshold * save;                                                  \
  const auto end =                                                             \
      std::lower_bound(my_map.right.begin(), my_map.right.end(), thres,        \
                       [](const RightVal &p, const T &val) {                   \
                         return std::greater<T>()(p.first, val);               \
                       });                                                     \
  my_map.right.erase(end, my_map.right.end());                                 \
  for (auto ptr = my_map.begin(); ptr != my_map.end(); ptr++) {                \
    tot_value += ptr->get_right();                                             \
  }

template <int32_t key_len, typename T>
void GndTruth<key_len, T>::swap(GndTruth &other) {
  my_map.swap(other.my_map);

  int64_t tmp = tot_value;
  tot_value = other.tot_value;
  other.tot_value = tmp;

  tmp = called;
  called = other.called;
  other.called = tmp;
}

template <int32_t key_len, typename T>
std::pair<typename GndTruth<key_len, T>::RightConstIterator,
          typename GndTruth<key_len, T>::RightConstIterator>
GndTruth<key_len, T>::equalRange(T value) {
  return std::equal_range(my_map.right.begin(), my_map.right.end(),
                          RightVal(value, FlowKey<key_len>()),
                          [](const RightVal &p, const RightVal &q) {
                            return std::greater<T>()(p.first, q.first);
                          });
}

template <int32_t key_len, typename T>
GndTruth<key_len, T> &GndTruth<key_len, T>::operator-=(const GndTruth &other) {
  for (const auto kv : other.my_map) {
    const auto &flowkey = kv.get_left();
    if (my_map.left.count(flowkey)) {
      tot_value -= my_map.left[flowkey];
      my_map.left[flowkey] = std::abs(my_map.left[flowkey] - kv.get_right());
      tot_value += my_map.left[flowkey];
    } else {
      my_map.left[flowkey] = kv.get_right();
      tot_value += kv.get_right();
    }
  }
  // sorted in descending order
  my_map.right.sort(std::greater<T>());
  return *this;
}

template <int32_t key_len, typename T>
T GndTruth<key_len, T>::at(const FlowKey<key_len> &flowkey) const {
  if (my_map.left.count(flowkey)) {
    return my_map.left.at(flowkey);
  } else {
    throw std::out_of_range(fmt::format("Flowkey Out Of Range: Not found in "
                                        "OmniSketch::Data::GndTruth<{:d}, {}>!",
                                        key_len, typeid(T).name()));
  }
  // return my_map.right.begin()->get_right(); // always return a value
}

template <int32_t key_len, typename T>
void GndTruth<key_len, T>::getGroundTruth(
    typename std::vector<Record<key_len>>::const_iterator begin,
    typename std::vector<Record<key_len>>::const_iterator end,
    CntMethod cnt_method) {
  CHECK_CALLED_ONCE;

  bool spurious_len = false, overflow = false;

  for (auto ptr = begin; ptr != end; ptr++) {
    if (cnt_method == InLength) {
      // check packet length
      if (ptr->length <= 0 || ptr->length > 1500) {
        spurious_len = true;
      }
      my_map.left[ptr->flowkey] += ptr->length;
      tot_value += ptr->length;
      // a more stringent condition on counter
      // apply to both unsigned and signed value
      if (my_map.left[ptr->flowkey] & static_cast<T>(1)
                                          << (sizeof(T) * 8 - 1)) {
        overflow = true;
      }
    } else {
      my_map.left[ptr->flowkey] += 1;
      tot_value += 1;
      // a more stringent condition on counter
      // apply to both unsigned and signed value
      if (my_map.left[ptr->flowkey] & static_cast<T>(1)
                                          << (sizeof(T) * 8 - 1)) {
        overflow = true;
      }
    }
  }

  if (spurious_len) {
    LOG(WARNING, "There are some flows with spurious length. Please check "
                 "the raw data.");
  }
  if (overflow) {
    LOG(WARNING,
        "Some counters overflew when getting ground truth. Try larger T.");
  }

  // sort the vector in descending order
  my_map.right.sort(std::greater<T>());
}

template <int32_t key_len, typename T>
void GndTruth<key_len, T>::getHeavyHitter(const GndTruth &flow_summary,
                                          double threshold,
                                          HXMethod hh_method) {
  CHECK_CALLED_ONCE;

  if (hh_method == TopK) {
    if (threshold < 1.0) {
      throw std::invalid_argument(
          "Invalid Argument: Threshold should >= 1.0 (Top-K), but got " +
          std::to_string(threshold) + " intsead.");
    }
    auto size = flow_summary.my_map.size();
    size_t no = std::min(size, static_cast<size_t>(threshold));
    auto ptr = flow_summary.my_map.right.begin();
    for (int i = 0; i < no; ++i, ptr++) {
      my_map.right.push_back(*ptr);
      tot_value += ptr->get_right();
    }
    // automatically sorted
    // my_map.right.sort(std::greater<T>());
  } else {
    if (!(threshold >= 0.0 && threshold <= 1.0)) {
      throw std::invalid_argument("Invalid Argument: Threshold should be in "
                                  "[0,1] (Percentile), but got " +
                                  std::to_string(threshold) + " intsead.");
    }
    T thres = threshold * flow_summary.tot_value;
    // Use the property: [cf. std::greater<T>()]
    // - Let x be an integer and y a floating point, then
    // (x > y) <=> (x > floor(y))
    const auto end = std::lower_bound(flow_summary.my_map.right.begin(),
                                      flow_summary.my_map.right.end(), thres,
                                      [](const RightVal &p, const T &val) {
                                        return std::greater<T>()(p.first, val);
                                      });
    for (auto ptr = flow_summary.my_map.right.begin(); ptr != end; ptr++) {
      my_map.right.push_back(*ptr);
      tot_value += ptr->get_right();
    }
    // automatically sorted
    // my_map.right.sort(std::greater<T>());
  }
}

template <int32_t key_len, typename T>
void GndTruth<key_len, T>::getHeavyHitter(GndTruth &&flow_summary,
                                          double threshold,
                                          HXMethod hh_method) {
  CHECK_CALLED_ONCE;
  // Why swapping rather than perfect forwarding?
  // It is quite a surprise that no operator=(bimap &&) is defined! Jesus!
  my_map.swap(flow_summary.my_map);
  int64_t save = flow_summary.tot_value;
  flow_summary.tot_value = 0;

  if (hh_method == TopK) {
    ASSERT_AND_TRUNCATE_MYSELF_TO_THE_FIRST_K_ELEMENTS;
    // automatically sorted
    // my_map.right.sort(std::greater<T>());
  } else {
    ASSERT_AND_TRUNCATE_MYSELF_TO_ELEMENTS_WITH_GIVEN_VALUE;
    // automatically sorted
    // my_map.right.sort(std::greater<T>());
  }
}

template <int32_t key_len, typename T>
void GndTruth<key_len, T>::getHeavyHitter(
    typename std::vector<Record<key_len>>::const_iterator begin,
    typename std::vector<Record<key_len>>::const_iterator end,
    CntMethod cnt_method, double threshold, HXMethod hh_method) {
  CHECK_CALLED_ONCE;
  // magic: erase calling history
  called--;
  this->getGroundTruth(begin, end, cnt_method);
  // magic: erase calling history
  called--;
  // it is fine if my_map swap with itself
  this->getHeavyHitter(std::move(*this), threshold, hh_method);
}

template <int32_t key_len, typename T>
void GndTruth<key_len, T>::getHeavyChanger(const GndTruth &flow_summary_1,
                                           const GndTruth &flow_summary_2,
                                           double threshold,
                                           HXMethod hc_method) {
  CHECK_CALLED_ONCE;

  // maybe time-costly
  my_map = flow_summary_1.my_map;
  tot_value = flow_summary_1.tot_value;
  (*this) -= flow_summary_2;
  int64_t save = tot_value;
  tot_value = 0;

  if (hc_method == TopK) {
    ASSERT_AND_TRUNCATE_MYSELF_TO_THE_FIRST_K_ELEMENTS;
    // automatically sorted
    // my_map.right.sort(std::greater<T>());
  } else {
    ASSERT_AND_TRUNCATE_MYSELF_TO_ELEMENTS_WITH_GIVEN_VALUE;
    // automatically sorted
    // my_map.right.sort(std::greater<T>());
  }
}

template <int32_t key_len, typename T>
void GndTruth<key_len, T>::getHeavyChanger(GndTruth &&flow_summary_1,
                                           GndTruth &&flow_summary_2,
                                           double threshold,
                                           HXMethod hc_method) {
  CHECK_CALLED_ONCE;
  // Why swapping rather than perfect forwarding?
  // It is quite a surprise that no operator=(bimap &&) is defined! Jesus!
  my_map.swap(flow_summary_1.my_map);
  tot_value = flow_summary_1.tot_value;
  flow_summary_1.tot_value = 0;
  (*this) -= flow_summary_2;
  int64_t save = tot_value;
  tot_value = 0;

  if (hc_method == TopK) {
    ASSERT_AND_TRUNCATE_MYSELF_TO_THE_FIRST_K_ELEMENTS;
    // automatically sorted
    // my_map.right.sort(std::greater<T>());
  } else {
    ASSERT_AND_TRUNCATE_MYSELF_TO_ELEMENTS_WITH_GIVEN_VALUE;
    // automatically sorted
    // my_map.right.sort(std::greater<T>());
  }
}

template <int32_t key_len, typename T>
void GndTruth<key_len, T>::getHeavyChanger(
    typename std::vector<Record<key_len>>::const_iterator begin_1,
    typename std::vector<Record<key_len>>::const_iterator end_1,
    typename std::vector<Record<key_len>>::const_iterator begin_2,
    typename std::vector<Record<key_len>>::const_iterator end_2,
    CntMethod cnt_method, double threshold, HXMethod hc_method) {
  CHECK_CALLED_ONCE;
  // magic: erase calling history
  called--;
  // can be called twice
  this->getGroundTruth(begin_1, end_1, cnt_method);

  bool spurious_len = false;

  // find the difference
  for (auto ptr = begin_2; ptr != end_2; ptr++) {
    int64_t size = (cnt_method == InLength) ? ptr->length : 1;
    // check length if count in length
    if (cnt_method == InLength) {
      if (ptr->length <= 0 || ptr->length > 1500) {
        spurious_len = true;
      }
    }
    // flip the negative value at the very last
    // at this point some value can be negative
    my_map.left[ptr->flowkey] -= size;
    tot_value -= size;
  }
  // flip the negative value
  for (auto ptr = my_map.left.begin(); ptr != my_map.left.end(); ptr++) {
    if (ptr->second < 0) {
      ptr->second = -ptr->second;
      tot_value += 2 * ptr->second;
    }
  }
  // sorted in descending order
  my_map.right.sort(std::greater<T>());

  // report spurious length
  if (spurious_len) {
    LOG(WARNING, "There are some flows with spurious length. Please check "
                 "the raw data.");
  }

  int64_t save = tot_value;
  tot_value = 0;

  if (hc_method == TopK) {
    ASSERT_AND_TRUNCATE_MYSELF_TO_THE_FIRST_K_ELEMENTS;
    // automatically sorted
    // my_map.right.sort(std::greater<T>());
  } else {
    ASSERT_AND_TRUNCATE_MYSELF_TO_ELEMENTS_WITH_GIVEN_VALUE;
    // automatically sorted
    // my_map.right.sort(std::greater<T>());
  }
}

template <int32_t key_len, typename T>
bool Estimation<key_len, T>::insert(const FlowKey<key_len> &flowkey) {
  if (my_map.left.count(flowkey))
    return false;
  my_map.left[flowkey] = 0;
  return true;
}

template <int32_t key_len, typename T>
bool Estimation<key_len, T>::update(const FlowKey<key_len> &flowkey, T val) {
  bool not_existed = !my_map.left.count(flowkey);
  my_map.left[flowkey] += val;
  return not_existed;
}

template <int32_t key_len, typename T>
T &Estimation<key_len, T>::operator[](const FlowKey<key_len> &flowkey) {
  if (!my_map.left.count(flowkey)) {
    my_map.left[flowkey] = 0;
  }
  return my_map.left[flowkey];
}

template <int32_t key_len, typename T>
size_t Estimation<key_len, T>::count(const FlowKey<key_len> &flowkey) const {
  return my_map.left.count(flowkey);
}

template <int32_t key_len, typename T>
const T &Estimation<key_len, T>::at(const FlowKey<key_len> &flowkey) const {
  if (my_map.left.count(flowkey)) {
    return my_map.left.at(flowkey);
  } else {
    throw std::out_of_range(
        fmt::format("Flowkey Out Of Range: Not found in "
                    "OmniSketch::Data::Estimation<{:d}, {}>!",
                    key_len, typeid(T).name()));
  }
  // return my_map.right.begin()->get_right(); // always return a value
}

template <int32_t key_len, typename T>
size_t Estimation<key_len, T>::size() const {
  return my_map.size();
}

#undef ASSERT_AND_TRUNCATE_MYSELF_TO_ELEMENTS_WITH_GIVEN_VALUE
#undef ASSERT_AND_TRUNCATE_MYSELF_TO_THE_FIRST_K_ELEMENTS
#undef CHECK_CALLED_ONCE

} // namespace OmniSketch::Data