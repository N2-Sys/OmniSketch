/**
 * @file BloomFilter.h
 * @author FerricIon (you@domain.com)
 * @brief Bloom Filter
 *
 * @copyright Copyright (c) 2022
 *
 */
#pragma once

#include <common/hash.h>
#include <common/sketch.h>

#define BYTE(n) ((n) >> 3)
#define BIT(n) ((n)&7)

namespace OmniSketch::Sketch {
/**
 * @brief Bloom Filter
 *
 * @tparam key_len  length of flowkey
 * @tparam hash_t   hashing class
 */
template <int32_t key_len, typename hash_t = Hash::AwareHash>
class BloomFilter : public SketchBase<key_len> {

private:
  int32_t nbits;
  int32_t num_hash;
  int32_t nbytes;
  uint8_t *arr;
  hash_t *hash_fns;

  BloomFilter(const BloomFilter &) = delete;
  BloomFilter(BloomFilter &&) = delete;
  BloomFilter &operator=(BloomFilter) = delete;

  /**
   * @brief Set a bit
   *
   */
  void setBit(int32_t pos) { arr[BYTE(pos)] |= (1 << BIT(pos)); }
  /**
   * @brief Fetch a bit
   *
   * @return `true` if it is `1`; `false` otherwise.
   */
  bool getBit(int32_t pos) const { return (arr[BYTE(pos)] >> BIT(pos)) & 1; }

public:
  /**
   * @brief Construct by specifying # of bits and hash classes
   *
   * @param num_bits        # bit
   * @param num_hash_class  # hash classes
   */
  BloomFilter(int32_t num_bits, int32_t num_hash_class);
  /**
   * @brief Destructor
   *
   */
  ~BloomFilter();

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
BloomFilter<key_len, hash_t>::BloomFilter(int32_t num_bits,
                                          int32_t num_hash_class)
    : nbits(num_bits), num_hash(num_hash_class) {
  nbits = Util::NextPrime(nbits);
  nbytes = (nbits + 7) >> 3; // ceil(nbits / 8)
  hash_fns = new hash_t[num_hash];
  // Allocate memory, zero initialized
  arr = new uint8_t[nbytes]();
}

template <int32_t key_len, typename hash_t>
BloomFilter<key_len, hash_t>::~BloomFilter() {
  delete[] hash_fns;
  delete[] arr;
}

template <int32_t key_len, typename hash_t>
void BloomFilter<key_len, hash_t>::insert(const FlowKey<key_len> &flowkey) {
  for (int32_t i = 0; i < num_hash; ++i) {
    int32_t idx = hash_fns[i](flowkey) % nbits;
    setBit(idx);
  }
}

template <int32_t key_len, typename hash_t>
bool BloomFilter<key_len, hash_t>::lookup(
    const FlowKey<key_len> &flowkey) const {
  // If every bit is on, return true
  for (int32_t i = 0; i < num_hash; ++i) {
    int32_t idx = hash_fns[i](flowkey) % nbits;
    if (!getBit(idx)) {
      return false;
    }
  }
  return true;
}

template <int32_t key_len, typename hash_t>
size_t BloomFilter<key_len, hash_t>::size() const {
  return sizeof(*this)                // Instance
         + nbytes * sizeof(uint8_t)   // arr
         + num_hash * sizeof(hash_t); // hash_fns
}

template <int32_t key_len, typename hash_t>
void BloomFilter<key_len, hash_t>::clear() {
  std::fill(arr, arr + nbytes, 0);
}

} // namespace OmniSketch::Sketch

#undef BYTE
#undef BIT