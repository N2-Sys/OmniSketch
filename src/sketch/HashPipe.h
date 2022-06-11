/**
 * @file HashPipe.h
 * @author KyleLv (you@domain.com)
 * @brief Hash Pipe
 *
 * @copyright Copyright (c) 2022
 *
 */
#pragma once

#include <common/hash.h>
#include <common/sketch.h>

namespace OmniSketch::Sketch {
/**
 * @brief Hash Pipe
 *
 * @tparam key_len  length of flowkey
 * @tparam T        type of the counter
 * @tparam hash_t   hashing class
 */
template <int32_t key_len, typename T, typename hash_t = Hash::AwareHash>
class HashPipe : public SketchBase<key_len, T> {
private:
  class Entry {
  public:
    FlowKey<key_len> flowkey;
    T val;
  };
  int32_t depth;
  int32_t width;
  hash_t *hash_fns;
  Entry **slots;

  HashPipe(const HashPipe &) = delete;
  HashPipe(HashPipe &&) = delete;
  HashPipe &operator=(HashPipe) = delete;

public:
  /**
   * @brief Construct by specifying depth and width
   *
   */
  HashPipe(int32_t depth_, int32_t width_);
  /**
   * @brief Release the pointer
   *
   */
  ~HashPipe();
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
   * @brief Get Heavy Hitter
   * @param threshold A flowkey is a HH iff its counter `>= threshold`
   *
   */
  Data::Estimation<key_len, T> getHeavyHitter(double threshold) const override;
  /**
   * @brief Get sketch size
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
///                        Implementation of template methods
//
//-----------------------------------------------------------------------------

namespace OmniSketch::Sketch {

template <int32_t key_len, typename T, typename hash_t>
HashPipe<key_len, T, hash_t>::HashPipe(int32_t depth_, int32_t width_)
    : depth(depth_), width(Util::NextPrime(width_)) {

  hash_fns = new hash_t[depth];
  // Allocate continuous memory
  slots = new Entry *[depth];
  slots[0] = new Entry[depth * width](); // Init with zero
  for (int32_t i = 1; i < depth; ++i) {
    slots[i] = slots[i - 1] + width;
  }
}

template <int32_t key_len, typename T, typename hash_t>
HashPipe<key_len, T, hash_t>::~HashPipe() {
  delete[] hash_fns;
  delete[] slots[0];
  delete[] slots;
}

template <int32_t key_len, typename T, typename hash_t>
void HashPipe<key_len, T, hash_t>::update(const FlowKey<key_len> &flowkey,
                                          T val) {
  // The first stage
  int idx = hash_fns[0](flowkey) % width;
  FlowKey<key_len> empty_key;
  FlowKey<key_len> c_key;
  T c_val;
  if (slots[0][idx].flowkey == flowkey) {
    // flowkey hit
    slots[0][idx].val += val;
    return;
  } else if (slots[0][idx].flowkey == empty_key) {
    // empty
    slots[0][idx].val = val;
    slots[0][idx].flowkey = flowkey;
    return;
  } else {
    // swap
    c_key = slots[0][idx].flowkey;
    c_val = slots[0][idx].val;
    slots[0][idx].flowkey = flowkey;
    slots[0][idx].val = val;
  }
  // Later stages
  for (int i = 1; i < depth; ++i) {
    idx = hash_fns[i](c_key) % width;
    if (slots[i][idx].flowkey == c_key) {
      slots[i][idx].val += c_val;
      return;
    } else if (slots[i][idx].flowkey == empty_key) {
      slots[i][idx].flowkey = c_key;
      slots[i][idx].val = c_val;
      return;
    } else if (slots[i][idx].val < c_val) {
      // swap
      // slots[i][idx].swap(c_key);
      auto tmpkey = c_key;
      c_key = slots[i][idx].flowkey;
      slots[i][idx].flowkey = tmpkey;
      std::swap(c_val, slots[i][idx].val);
    }
  }
}

template <int32_t key_len, typename T, typename hash_t>
T HashPipe<key_len, T, hash_t>::query(const FlowKey<key_len> &flowkey) const {
  T ret = 0;
  for (int i = 0; i < depth; ++i) {
    int idx = hash_fns[i](flowkey) % width;
    if (slots[i][idx].flowkey == flowkey) {
      ret += slots[i][idx].val;
    }
  }
  return ret;
}

template <int32_t key_len, typename T, typename hash_t>
Data::Estimation<key_len, T>
HashPipe<key_len, T, hash_t>::getHeavyHitter(double threshold) const {
  Data::Estimation<key_len, T> heavy_hitters;
  std::set<FlowKey<key_len>> checked;
  for (int i = 0; i < depth; ++i) {
    for (int j = 0; j < width; ++j) {
      const auto &flowkey = slots[i][j].flowkey;
      if (checked.find(flowkey) != checked.end()) {
        continue;
      }
      checked.insert(flowkey);
      auto estimate_val = query(flowkey);
      if (estimate_val >= threshold) {
        heavy_hitters[flowkey] = estimate_val;
      }
    }
  }
  return heavy_hitters;
}

template <int32_t key_len, typename T, typename hash_t>
size_t HashPipe<key_len, T, hash_t>::size() const {
  return sizeof(*this)                    // instance
         + depth * sizeof(hash_t)         // hashing class
         + depth * width * sizeof(Entry); // slots
}

template <int32_t key_len, typename T, typename hash_t>
void HashPipe<key_len, T, hash_t>::clear() {
  FlowKey<key_len> empty_key;
  for (int i = 0; i < depth; ++i) {
    for (int j = 0; j < width; ++j) {
      slots[i][j].flowkey = empty_key;
      slots[i][j].val = 0;
    }
  }
}

} // namespace OmniSketch::Sketch