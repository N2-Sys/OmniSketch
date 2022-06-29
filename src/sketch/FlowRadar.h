/**
 * @file FlowRadar.h
 * @author dromniscience (you@domain.com)
 * @brief
 * @date 2022-06-13
 *
 * @copyright Copyright (c) 2022
 *
 */
#pragma once

#include <common/hash.h>
#include <sketch/BloomFilter.h>

namespace OmniSketch::Sketch {
/**
 * @brief Flow Radar
 *
 * @tparam key_len  length of flowkey
 * @tparam T        type of the counter
 * @tparam hash_t   hashing class
 */
template <int32_t key_len, typename T, typename hash_t = Hash::AwareHash>
class FlowRadar : public SketchBase<key_len, T> {
private:
  struct CountTableEntry {
    FlowKey<key_len> flowXOR;
    T flow_count;
    T packet_count;
    CountTableEntry() : flowXOR(), flow_count(0), packet_count(0) {}
  };

  const int32_t num_bitmap;
  const int32_t num_bit_hash;
  const int32_t num_count_table;
  const int32_t num_count_hash;
  int32_t num_flows;

  hash_t *hash_fns;
  BloomFilter<key_len, hash_t> *flow_filter;
  CountTableEntry *count_table;

  FlowRadar(const FlowRadar &) = delete;
  FlowRadar(FlowRadar &&) = delete;

public:
  /**
   * @brief Construct a new Flow Radar object
   *
   * @param flow_filter_size Number of bits in flow filter (a Bloom Filter)
   * @param flow_filter_hash Number of hash functions in flow filter
   * @param count_table_size Number of elements in count table
   * @param count_table_hash Number of hash functions in count table
   */
  FlowRadar(int32_t flow_filter_size, int32_t flow_filter_hash,
            int32_t count_table_size, int32_t count_table_hash);
  /**
   * @brief Destructor
   *
   */
  ~FlowRadar();
  /**
   * @brief Update a flowkey with a certain value
   *
   */
  void update(const FlowKey<key_len> &flowkey, T val) override;
  /**
   * @brief Decode flowkey and its value
   *
   */
  Data::Estimation<key_len, T> decode() override;
  /**
   * @brief Reset the sketch
   *
   */
  void clear();
  /**
   * @brief Get the size of the sketch
   *
   */
  size_t size() const override;
};

} // namespace OmniSketch::Sketch

//-----------------------------------------------------------------------------
//
///                        Implementation of templated methods
//
//-----------------------------------------------------------------------------

namespace OmniSketch::Sketch {

template <int32_t key_len, typename T, typename hash_t>
FlowRadar<key_len, T, hash_t>::FlowRadar(int32_t flow_filter_size,
                                         int32_t flow_filter_hash,
                                         int32_t count_table_size,
                                         int32_t count_table_hash)
    : num_bitmap(Util::NextPrime(flow_filter_size)),
      num_bit_hash(flow_filter_hash),
      num_count_table(Util::NextPrime(count_table_size)),
      num_count_hash(count_table_hash), num_flows(0) {
  hash_fns = new hash_t[num_count_hash];
  // flow filter
  flow_filter = new BloomFilter<key_len, hash_t>(num_bitmap, num_bit_hash);
  // count table
  count_table = new CountTableEntry[num_count_table]();
}

template <int32_t key_len, typename T, typename hash_t>
FlowRadar<key_len, T, hash_t>::~FlowRadar() {
  delete[] hash_fns;
  delete flow_filter;
  delete[] count_table;
}

template <int32_t key_len, typename T, typename hash_t>
void FlowRadar<key_len, T, hash_t>::update(const FlowKey<key_len> &flowkey,
                                           T val) {
  bool exist = flow_filter->lookup(flowkey);
  // a new flow
  if (!exist) {
    flow_filter->insert(flowkey);
    num_flows++;
  }

  for (int32_t i = 0; i < num_count_hash; i++) {
    int32_t index = hash_fns[i](flowkey) % num_count_table;
    // a new flow
    if (!exist) {
      count_table[index].flow_count++;
      count_table[index].flowXOR ^= flowkey;
    }
    // increment packet count
    count_table[index].packet_count += val;
  }
}

template <int32_t key_len, typename T, typename hash_t>
Data::Estimation<key_len, T> FlowRadar<key_len, T, hash_t>::decode() {
  // an optimized implementation
  class CompareFlowCount {
  public:
    bool operator()(CountTableEntry *ptr1, CountTableEntry *ptr2) const {
      if (ptr1->flow_count == ptr2->flow_count) {
        return ptr1 < ptr2;
      } else
        return ptr1->flow_count < ptr2->flow_count;
    }
  };
  std::set<CountTableEntry *, CompareFlowCount> set;
  for (int32_t i = 0; i < num_count_table; ++i) {
    set.insert(count_table + i);
  }

  Data::Estimation<key_len, T> est;
  while (!set.empty()) {
    int32_t index = *set.begin() - count_table;
    T value = count_table[index].flow_count;
    // no decodable flow count
    if (value > 1) {
      break;
    }

    set.erase(set.begin());
    // ignore vacant counts
    if (value == 0)
      continue;

    FlowKey<key_len> flowkey = count_table[index].flowXOR;
    T size = count_table[index].packet_count;
    for (int i = 0; i < num_count_hash; ++i) {
      int l = hash_fns[i](flowkey) % num_count_table;
      set.erase(count_table + l);
      count_table[l].flow_count--;
      count_table[l].packet_count -= size;
      count_table[l].flowXOR ^= flowkey;
      set.insert(count_table + l);
    }
    est[flowkey] = size;
  }
  return est;
}

template <int32_t key_len, typename T, typename hash_t>
size_t FlowRadar<key_len, T, hash_t>::size() const {
  return sizeof(*this)                                 // instance
         + num_count_hash * sizeof(hash_t)             // hashing class
         + num_count_table * (sizeof(T) * 2 + key_len) // count table
         + flow_filter->size();                        // flow filter
}

template <int32_t key_len, typename T, typename hash_t>
void FlowRadar<key_len, T, hash_t>::clear() {
  // reset flow counter
  num_flows = 0;
  // reset flow filter
  flow_filter->clear();
  // reset count table
  delete[] count_table;
  count_table = new CountTableEntry[num_count_table]();
}

} // namespace OmniSketch::Sketch