/**
 * @file hierarchy.h
 * @author dromniscience (you@domain.com)
 * @brief Counter Hierarchy
 *
 * @copyright Copyright (c) 2022
 *
 */
#pragma once

#include "hash.h"
#include "utils.h"
#include <Eigen/Dense>
#include <Eigen/IterativeLinearSolvers>
#include <Eigen/SparseCore>
#include <boost/dynamic_bitset.hpp>
#include <map>

namespace OmniSketch::Sketch {
/**
 * @brief Use the counter hierarchy to better save space while preserving
 * accuracy!
 *
 * @tparam no_layer   Number of layers in CH
 * @tparam T          Counter Type of CH
 * @tparam hash_t     Hashing classes used internally
 *
 * @note
 * - In CH, counters are serialized, so it is the user's job to convert the
 * index of a possibly multi-dimensional array into a unique serial number.
 * Since CH uses 0-based array internally, this serial number is chosen to
 * have type `size_t`.
 * - CH uses **lazy update policy**. That is, only when you try to get a counter
 * value in CH will the updates genuinely be propagated to the higher layers
 * and then decoded.
 * - Note that CH cannot bring better accuracy. Ideally, if the first layer in
 * CH never overflows, your sketch achieves the same accuracy as does without
 * it.
 *
 * @warning
 * - To apply CH, make sure values of the original counters are always
 * non-negative during the whole process, though negative update is OK.
 * (i.e., only *cash register case* and the *non-negative case*
 * in *turnstile model* are allowed) Otherwise, errors of decoding can be
 * unbounded.
 * - It is highly recommended that on each layer the number of counters is
 * prime.
 */
template <int32_t no_layer, typename T, typename hash_t = Hash::AwareHash>
class CounterHierarchy {
private:
  using CarryOver = std::map<std::size_t, T>;
  /**
   * @brief Number of counters on each layer, from low to high.
   *
   */
  const std::vector<size_t> no_cnt;
  /**
   * @brief Width of counters on each layer, from low to high.
   *
   */
  const std::vector<size_t> width_cnt;
  /**
   * @brief Number of hash function used on each layer, from low to
   * high (except for the last layer)
   *
   */
  const std::vector<size_t> no_hash;
  /**
   * @brief array of hashing classes
   *
   */
  std::vector<hash_t> *hash_fns;
  /**
   * @brief counters in CH
   *
   */
  std::vector<Util::DynamicIntX<T>> *cnt_array;
  /**
   * @brief Status bits
   *
   */
  boost::dynamic_bitset<uint8_t> *status_bits;
  /**
   * @brief Original counters
   *
   */
  std::vector<T> original_cnt;
  /**
   * @brief get decoded counter
   * @details `double` will round to `T` after decoding each layer. The reason
   * why `double` here is to facilitate NZE decoding.
   */
  std::vector<double> decoded_cnt;
  /**
   * @brief For lazy update policy
   *
   */
  CarryOver lazy_update;

private:
  /**
   * @brief Update a layer (aggregation)
   *
   * @details An overflow error would be thrown if there is an overflow at the
   * last layer. For the other layers, since a counter may first oveflow and
   * then be substracted to withdraw any carry over, the number of overflows of
   * counter whose status bit is set is not assumed to be 1 at least.
   *
   * @param layer   the current layer
   * @param updates updates to be propagated to the current layer
   * @return updates to be propagated to the next layer
   */
  [[nodiscard]] CarryOver updateLayer(const int32_t layer, CarryOver &&updates);
  /**
   * @brief Decode a layer
   *
   * @param layer   the current layer to decode
   * @param higher  decoded results of the higher layer
   * @return results of the current layer
   */
  [[nodiscard]] std::vector<double>
  decodeLayer(const int32_t layer, std::vector<double> &&higher) const;

public:
  /**
   * @brief Construct by specifying detailed architectural parameters
   *
   * @param no_cnt      number of counters on each layer, from low to high
   * @param width_cnt   width of counters on each layer, from low to high
   * @param no_hash     number of hash functions used on each layer, from low
   * to high (except for the last layer)
   *
   * @details The meaning of the three parameters stipulates the following
   * requirements:
   * - size of `no_cnt` should equal `no_layer`.
   * - size of `width_cnt` should equal `no_layer`.
   * - size of `no_hash` should equal `no_layer - 1`.
   *
   * If any of these is violated, an exception would be thrown. Other
   * conditions that trigger an exception:
   * - Items in these vectors contains a 0.
   * - `no_layer <= 0`
   * - Sum of `width_cnt` exceeds `sizeof(T) * 8`. This constraint is imposed to
   * guarantee proper shifting of counters when decoding.
   */
  CounterHierarchy(const std::vector<size_t> &no_cnt,
                   const std::vector<size_t> &width_cnt,
                   const std::vector<size_t> &no_hash);
  /**
   * @brief Destructor
   *
   */
  ~CounterHierarchy();
  /**
   * @brief Update a counter
   *
   * @param index Serialized index of a counter. It is the user's job to get the
   * index serialized in advance.
   * @param val   Value to be updated
   *
   * @note
   * - Use the lazy update policy.
   * - An out-of-range exception would be thrown if `index` is out of range.
   */
  void updateCnt(size_t index, T val);
  /**
   * @brief Get the value of counters in CH
   *
   * @details An overflow exception would be thrown if there is an overflow at
   * the last layer. An out-of-range exception would be thrown if the index is
   * out of range.
   *
   * @param index Serialized index of a counter. It is the user's job to get the
   * index serialized in advance.
   */
  T getCnt(size_t index);
  /**
   * @brief Get the original value of counters.
   *
   * @details I.e., the value of the counter without CH.
   *
   * @param index Serialized index of a counter. It is the user's job to get
   * the index serialized in advance.
   */
  T getOriginalCnt(size_t index) const;
  /**
   * @brief Size of CH.
   *
   */
  size_t size() const;
  /**
   * @brief Size of counters without CH.
   *
   */
  size_t originalSize() const;
  /**
   * @brief Reset CH.
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

template <int32_t no_layer, typename T, typename hash_t>
typename CounterHierarchy<no_layer, T, hash_t>::CarryOver
CounterHierarchy<no_layer, T, hash_t>::updateLayer(const int32_t layer,
                                                   CarryOver &&updates) {
  CarryOver ret; // aggregate all updates on the current layer

  for (const auto &kv : updates) {
    T overflow = cnt_array[layer][kv.first] + kv.second;
    if (overflow) {
      // mark status bits
      status_bits[layer][kv.first] = true;
      if (layer == no_layer - 1) { // last layer
        throw std::overflow_error(
            "Counter overflow at the last layer in CH, overflow by " +
            std::to_string(overflow) + ".");
      } else { // hash to upper-layer counters
        for (size_t i = 0; i < no_hash[layer]; i++) {
          std::size_t index = hash_fns[layer][i](kv.first) % no_cnt[layer + 1];
          ret[index] += overflow;
        }
      }
    }
  }
  return ret;
}

template <int32_t no_layer, typename T, typename hash_t>
std::vector<double> CounterHierarchy<no_layer, T, hash_t>::decodeLayer(
    const int32_t layer, std::vector<double> &&higher) const {
  // make sure the size match
  if (higher.size() != no_cnt[layer + 1]) {
    throw std::length_error("Size Error: Expect a vector of size " +
                            std::to_string(no_cnt[layer + 1]) +
                            ", but got one of size " +
                            std::to_string(higher.size()) + " instead.");
  }

  // solver
  Eigen::LeastSquaresConjugateGradient<Eigen::SparseMatrix<double>>
      solver_sparse;

  Eigen::VectorXd X(no_cnt[layer]),
      b = Eigen::Map<Eigen::VectorXd, Eigen::Unaligned>(higher.data(),
                                                        higher.size());
  Eigen::SparseMatrix<double> A(no_cnt[layer + 1], no_cnt[layer]);
  std::vector<Eigen::Triplet<double>> tripletlist;

  for (size_t i = 0; i < no_cnt[layer]; i++) {
    if (!status_bits[layer][i])
      continue; // no overflow
    // hash to higher-layer counter
    for (size_t j = 0; j < no_hash[layer]; ++j) {
      size_t k = hash_fns[layer][j](i) % no_cnt[layer + 1];
      tripletlist.push_back(Eigen::Triplet<double>(k, i, 1.0));
    }
  }
  // duplicates are summed up, see
  // https://eigen.tuxfamily.org/dox/classEigen_1_1SparseMatrix.html#a8f09e3597f37aa8861599260af6a53e0
  A.setFromTriplets(tripletlist.begin(), tripletlist.end());
  A.makeCompressed();
  solver_sparse.compute(A);
  X = solver_sparse.solve(b);

  std::vector<double> ret(no_cnt[layer]);
  for (size_t i = 0; i < no_cnt[layer]; ++i) {
    ret[i] = status_bits[layer][i]
                 ? static_cast<double>(static_cast<T>(X[i] + 0.5)
                                       << width_cnt[layer])
                 : 0.0;
    ret[i] += cnt_array[layer][i].getVal();
  }
  return ret;
}

template <int32_t no_layer, typename T, typename hash_t>
CounterHierarchy<no_layer, T, hash_t>::CounterHierarchy(
    const std::vector<size_t> &no_cnt, const std::vector<size_t> &width_cnt,
    const std::vector<size_t> &no_hash)
    : no_cnt(no_cnt), width_cnt(width_cnt), no_hash(no_hash) {
  // validity check
  if (no_layer < 1) {
    throw std::invalid_argument(
        "Invalid Template Argument: `no_layer` must > 1, got " +
        std::to_string(no_layer) + ".");
  }
  if (no_cnt.size() != no_layer) {
    throw std::invalid_argument(
        "Invalid Argument: `no_cnt` should be of size " +
        std::to_string(no_layer) + ", but got size " +
        std::to_string(no_cnt.size()) + ".");
  }
  if (width_cnt.size() != no_layer) {
    throw std::invalid_argument(
        "Invalid Argument: `width_cnt` should be of size " +
        std::to_string(no_layer) + ", but got size " +
        std::to_string(width_cnt.size()) + ".");
  }
  if (no_hash.size() != no_layer - 1) {
    throw std::invalid_argument(
        "Invalid Argument: `no_hash` should be of size " +
        std::to_string(no_layer - 1) + ", but got size " +
        std::to_string(no_hash.size()) + ".");
  }
  for (auto i : no_cnt) {
    if (i == 0) {
      throw std::invalid_argument(
          "Invalid Argument: There is a zero in `no_cnt`.");
    }
  }
  for (auto i : width_cnt) {
    if (i == 0) {
      throw std::invalid_argument(
          "Invalid Argument: There is a zero in `width_cnt`.");
    }
  }
  for (auto i : no_hash) {
    if (i == 0) {
      throw std::invalid_argument(
          "Invalid Argument: There is a zero in `no_hash`.");
    }
  }
  size_t length = 0;
  for (auto i : width_cnt) {
    size_t tmp = length + i;
    if (tmp < length || tmp > sizeof(T) * 8) {
      throw std::invalid_argument(
          "Invalid Argument: Aggregate length of `width_cnt` is too large.");
    }
    length = tmp;
  }

  // allocate in heap
  hash_fns = new std::vector<hash_t>[no_layer - 1];
  for (int32_t i = 0; i < no_layer - 1; ++i) {
    hash_fns[i] = std::vector<hash_t>(no_hash[i]);
  }
  cnt_array = new std::vector<Util::DynamicIntX<T>>[no_layer];
  for (int32_t i = 0; i < no_layer; ++i) {
    cnt_array[i] = std::vector<Util::DynamicIntX<T>>(no_cnt[i], {width_cnt[i]});
  }
  status_bits = new boost::dynamic_bitset<uint8_t>[no_layer];
  for (int32_t i = 0; i < no_layer; ++i) {
    status_bits[i].resize(no_cnt[i], false);
  }
  // original counters, value initialized
  original_cnt.resize(no_cnt[0]);
  // decoded counters, value initialized
  decoded_cnt.resize(no_cnt[0]);
}

template <int32_t no_layer, typename T, typename hash_t>
CounterHierarchy<no_layer, T, hash_t>::~CounterHierarchy() {
  if (hash_fns)
    delete[] hash_fns;
  if (cnt_array)
    delete[] cnt_array;
  if (status_bits)
    delete[] status_bits;
}

template <int32_t no_layer, typename T, typename hash_t>
void CounterHierarchy<no_layer, T, hash_t>::updateCnt(size_t index, T val) {
  if (index >= no_cnt[0]) {
    throw std::out_of_range("Index Out of Range: Should be in [0, " +
                            std::to_string(no_cnt[0] - 1) + "], but got " +
                            std::to_string(index) + " instead.");
  }
  // lazy update policy
  lazy_update[index] += val;
  // original counters
  original_cnt[index] += val;
}

template <int32_t no_layer, typename T, typename hash_t>
T CounterHierarchy<no_layer, T, hash_t>::getCnt(size_t index) {
  if (index >= no_cnt[0]) {
    throw std::out_of_range("Index Out of Range: Should be in [0, " +
                            std::to_string(no_cnt[0] - 1) + "], but got " +
                            std::to_string(index) + " instead.");
  }

  // lazy update
  if (!lazy_update.empty()) {
    for (int32_t i = 0; i < no_layer; i++) {
      lazy_update = updateLayer(i, std::move(lazy_update)); // throw exception
    }
    lazy_update.clear();
    // decode
    decoded_cnt = std::vector<double>(no_cnt.back());
    for (size_t i = 0; i < no_cnt.back(); ++i) {
      decoded_cnt[i] = static_cast<double>(cnt_array[no_layer - 1][i].getVal());
    }
    for (int32_t i = no_layer - 2; i >= 0; i--) {
      decoded_cnt = decodeLayer(i, std::move(decoded_cnt));
    }
  }
  return static_cast<T>(decoded_cnt[index]);
}

template <int32_t no_layer, typename T, typename hash_t>
T CounterHierarchy<no_layer, T, hash_t>::getOriginalCnt(size_t index) const {
  return original_cnt[index];
}

template <int32_t no_layer, typename T, typename hash_t>
size_t CounterHierarchy<no_layer, T, hash_t>::size() const {
  // counters + status bits
  size_t tot = 0; // first in bits
  for (int32_t i = 0; i < no_layer; ++i) {
    tot += no_cnt[i] * width_cnt[i];
    tot += no_cnt[i];
  }
  tot >>= 3; // to bytes
  // hash functions
  for (int32_t i = 0; i < no_layer - 1; ++i) {
    tot += sizeof(hash_t) * no_hash[i];
  }
  return tot;
}

template <int32_t no_layer, typename T, typename hash_t>
size_t CounterHierarchy<no_layer, T, hash_t>::originalSize() const {
  return sizeof(T) * no_cnt[0];
}

template <int32_t no_layer, typename T, typename hash_t>
void CounterHierarchy<no_layer, T, hash_t>::clear() {
  // reset counters
  for (int32_t i = 0; i < no_layer; ++i) {
    cnt_array[i] = std::vector<Util::DynamicIntX<T>>(no_cnt[i], width_cnt[i]);
  }
  // reset status bits
  for (int32_t i = 0; i < no_layer; ++i) {
    status_bits[i].reset();
  }
  // reset original counters
  original_cnt = std::vector<T>(no_cnt[0]);
  // // reset decoded counters
  // decoded_cnt = std::vector<double>(no_cnt[0]);
  // reset lazy_update
  lazy_update.clear();
}

} // namespace OmniSketch::Sketch