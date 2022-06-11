/**
 * @file CountSketch.h
 * @author dromniscience (you@domain.com)
 * @brief Implementation of Count Sketch
 *
 * @copyright Copyright (c) 2022
 *
 */
#pragma once

#include <common/hash.h>
#include <common/sketch.h>

namespace OmniSketch::Sketch {
/**
 * @brief Count Sketch
 *
 * @tparam key_len  length of flowkey
 * @tparam T        type of the counter
 * @tparam hash_t   hashing class
 */
template <int32_t key_len, typename T, typename hash_t = Hash::AwareHash>
class CountSketch : public SketchBase<key_len, T> {
private:
  int32_t depth;
  int32_t width;
  hash_t *hash_fns;
  T **counter;

  CountSketch(const CountSketch &) = delete;
  CountSketch(CountSketch &&) = delete;

public:
  /**
   * @brief Construct by specifying depth and width
   *
   */
  CountSketch(int32_t depth_, int32_t width_);
  /**
   * @brief Release the pointer
   *
   */
  ~CountSketch();
  /**
   * @brief Update a flowkey with certain value
   *
   */
  void update(const FlowKey<key_len> &flowkey, T val) override;
  /**
   * @brief Query a flowkey
   *
   */
  T query(const FlowKey<key_len> &flowkey) const override;
  /**
   * @brief Get the size of the sketch
   *
   */
  size_t size() const override;
  /**
   * @brief Reset the sketch
   *
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

template <int32_t key_len, typename T, typename hash_t>
CountSketch<key_len, T, hash_t>::CountSketch(int32_t depth_, int32_t width_)
    : depth(depth_), width(Util::NextPrime(width_)) {

  // The first depth hash functions: CM
  // The last depth hash function: signed bit
  hash_fns = new hash_t[depth * 2];
  // Allocate continuous memory
  counter = new T *[depth];
  counter[0] = new T[depth * width](); // Init with zero
  for (int32_t i = 1; i < depth; ++i) {
    counter[i] = counter[i - 1] + width;
  }
}

template <int32_t key_len, typename T, typename hash_t>
CountSketch<key_len, T, hash_t>::~CountSketch() {
  delete[] hash_fns;
  delete[] counter[0];
  delete[] counter;
}

template <int32_t key_len, typename T, typename hash_t>
void CountSketch<key_len, T, hash_t>::update(const FlowKey<key_len> &flowkey,
                                             T val) {
  for (int i = 0; i < depth; ++i) {
    int idx = hash_fns[i](flowkey) % width;
    counter[i][idx] +=
        val * (static_cast<int>(hash_fns[depth + i](flowkey) & 1) * 2 - 1);
  }
}

template <int32_t key_len, typename T, typename hash_t>
T CountSketch<key_len, T, hash_t>::query(
    const FlowKey<key_len> &flowkey) const {
  T values[depth];
  for (int i = 0; i < depth; ++i) {
    int idx = hash_fns[i](flowkey) % width;
    values[i] = counter[i][idx] *
                (static_cast<int>(hash_fns[depth + i](flowkey) & 1) * 2 - 1);
  }
  std::sort(values, values + depth);
  if (!(depth & 1)) { // even
    return std::abs((values[depth / 2 - 1] + values[depth / 2]) / 2);
  } else { // odd
    return std::abs(values[depth / 2]);
  }
}

template <int32_t key_len, typename T, typename hash_t>
size_t CountSketch<key_len, T, hash_t>::size() const {
  return sizeof(*this)                // instance
         + depth * 2 * sizeof(hash_t) // hashing class
         + depth * width * sizeof(T); // counter
}

template <int32_t key_len, typename T, typename hash_t>
void CountSketch<key_len, T, hash_t>::clear() {
  std::fill(counter[0], counter[0] + depth * width, 0);
}

} // namespace OmniSketch::Sketch
