/**
 * @file CountingBloomFilter.h
 * @author KyleLv (you@domain.com)
 * @brief Counting Bloom Filter
 *
 * @copyright Copyright (c) 2022
 *
 */
#pragma once

#include <common/hash.h>
#include <common/hierarchy.h>

namespace OmniSketch::Sketch {
/**
 * @brief Counting Bloom Filter
 *
 * @tparam key_len  length of flowkey
 * @tparam hash_t   hashing class
 */
template <int32_t key_len, typename hash_t = Hash::AwareHash>
class CountingBloomFilter : public SketchBase<key_len> {
  // for convenience
  using T = int64_t;
  using CH = CounterHierarchy<1, T, hash_t>;

private:
  int32_t ncnt;
  int32_t nhash;
  hash_t *hash_fns;
  CH *counter;

  CountingBloomFilter(const CountingBloomFilter &) = delete;
  CountingBloomFilter(CountingBloomFilter &&) = delete;
  CountingBloomFilter &operator=(CountingBloomFilter) = delete;

public:
  /**
   * @brief Construct by specifying #counters, #hash and length of counters
   *
   * @param num_cnt    #counter
   * @param num_hash    #hash
   * @param cnt_length  length of each counter
   */
  CountingBloomFilter(int32_t num_cnt, int32_t num_hash, int32_t cnt_length);
  /**
   * @brief Destructor
   *
   */
  ~CountingBloomFilter();

  /**
   * @brief Insert a flowkey into the bloom filter
   * @details An overriding method
   *
   */
  void insert(const FlowKey<key_len> &flowkey) override;
  /**
   * @brief Look up a flowkey to see whether it exists
   * @details An overriding method
   *
   */
  bool lookup(const FlowKey<key_len> &flowkey) const override;
  /**
   * @brief Remove a flowkey from the bloom filter
   * @details A non-overriding method
   *
   */
  void remove(const FlowKey<key_len> &flowkey);
  /**
   * @brief Size of the sketch
   * @details An overriding method
   */
  size_t size() const override;
  /**
   * @brief Reset the Bloom Filter
   * @details A non-overriding method
   */
  void clear();
};

} // namespace OmniSketch::Sketch

//-----------------------------------------------------------------------------
//
///                        Implementation of templated methods
//
//-----------------------------------------------------------------------------

namespace OmniSketch::Sketch {

template <int32_t key_len, typename hash_t>
CountingBloomFilter<key_len, hash_t>::CountingBloomFilter(int32_t num_cnt,
                                                          int32_t num_hash,
                                                          int32_t cnt_length)
    : ncnt(Util::NextPrime(num_cnt)), nhash(num_hash) {
  // hash functions
  hash_fns = new hash_t[num_hash];
  // counter array
  counter = new CH({static_cast<size_t>(ncnt)},
                   {static_cast<size_t>(cnt_length)}, {});
}

template <int32_t key_len, typename hash_t>
CountingBloomFilter<key_len, hash_t>::~CountingBloomFilter() {
  delete[] hash_fns;
  delete counter;
}

template <int32_t key_len, typename hash_t>
void CountingBloomFilter<key_len, hash_t>::insert(
    const FlowKey<key_len> &flowkey) {
  // if there is a 0
  int32_t i = 0;
  while (i < nhash) {
    int32_t idx = hash_fns[i](flowkey) % ncnt;
    if (counter->getCnt(idx) == 0)
      break;
    i++;
  }
  // increment the buckets
  if (i < nhash) {
    for (int32_t j = 0; j < nhash; ++j) {
      counter->updateCnt(hash_fns[j](flowkey) % ncnt, 1);
    }
  }
}

template <int32_t key_len, typename hash_t>
bool CountingBloomFilter<key_len, hash_t>::lookup(
    const FlowKey<key_len> &flowkey) const {
  // if every counter is non-zero, return true
  for (int32_t i = 0; i < nhash; ++i) {
    int32_t idx = hash_fns[i](flowkey) % ncnt;
    if (counter->getCnt(idx) == 0) {
      return false;
    }
  }
  return true;
}

template <int32_t key_len, typename hash_t>
void CountingBloomFilter<key_len, hash_t>::remove(
    const FlowKey<key_len> &flowkey) {
  // if there is a 0
  int32_t i = 0;
  while (i < nhash) {
    int32_t idx = hash_fns[i](flowkey) % ncnt;
    if (counter->getCnt(idx) == 0)
      break;
    i++;
  }
  // decrement the buckets
  if (i == nhash) {
    for (int32_t j = 0; j < nhash; ++j) {
      counter->updateCnt(hash_fns[j](flowkey) % ncnt, -1);
    }
  }
}

template <int32_t key_len, typename hash_t>
size_t CountingBloomFilter<key_len, hash_t>::size() const {
  return sizeof(*this)            // instance
         + sizeof(hash_t) * nhash // hash functions
         + counter->size();       // counter size
}

template <int32_t key_len, typename hash_t>
void CountingBloomFilter<key_len, hash_t>::clear() {
  counter->clear();
}

} // namespace OmniSketch::Sketch