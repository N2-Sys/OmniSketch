/**
 * @file hash.h
 * @author Dustin-He (you@domain.com)
 * @brief Warehouse of hashing classes
 *
 * @copyright Copyright (c) 2022
 *
 */
#pragma once

#include "flowkey.h"

/**
 * @brief Warehouse of hashing classes
 *
 * @details All hashing classes in this warehouse are capable of hashing
 * flowkey, integer and byte array to an `uint64_t` or an `uint32_t` value
 * (implementation-defined). To craft a new hashing class, please inherit
 * from HashBase, and the derived class only has to provide a simple interface
 * looked exactly like `uint64_t hash(const uint8_t *key, const int32_t len)
 * const;` (also a pure virtual function in HashBase). See the example below.
 *
 * ### Example
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~.cpp
 * // hash.h (this file)
 * class MyHash: public HashBase {
 * private:
 *   // Any internal data and methods
 *
 *   uint64_t hash(const uint8_t *key, const int32_t len) const;
 *
 * public:
 *   // Constructors & destructors (if needed)
 * };
 *
 *
 * // src/hash.cpp
 * uint64_t MyHash::hash(const uint8_t *key, const int32_t len) const {
 *   // Implementation here
 * }
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 * @note The reason why `uint8_t *`, rather than `int8_t *` or `char *`, is
 * chosen for byte array is that multitudinous hash functions (if not all) are
 * built upon unsigned integers.
 *
 * @see HashBase
 *
 */
namespace OmniSketch::Hash {
/**
 * @brief Base class for all hashing classes
 *
 * @details Any derived class only has to implement a private
 * `uint64_t hash(const uint8_t *, const int32_t) const` method. Note that
 *
 * - operator()(const uint8_t *, const int32_t) const
 * - operator()(const size_t) const
 * - operator()(const FlowKey<key_len> &) const
 *
 * are automatically inherited so that they internally call
 * `hash(const uint8_t *, const int32_t) const` to hash the input.
 *
 * ### Example
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~.cpp
 * // hash.h (this file)
 * class MyHash: public HashBase {
 * private:
 *   // Any internal data and methods
 *
 *   uint64_t hash(const uint8_t *key, const int32_t len) const;
 *
 * public:
 *   // Constructors & destructors (if needed)
 * };
 *
 *
 * // impl/hash.cpp
 * uint64_t MyHash::hash(const uint8_t *key, const int32_t len) const {
 *   // Implementation here
 * }
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 */
class HashBase {
private:
  /**
   * @brief Hash byte array.
   *
   * @details To be overriden in derived class
   *
   * @param key pointer to the byte array
   * @param len length of the byte array
   * @return hashed value of the byte array
   *
   * @note This function has to be declared as a constant class function.
   */
  virtual uint64_t hash(const uint8_t *key, const int32_t len) const = 0;

public:
  /**
   * @brief Hash byte array
   *
   * @param key pointer to the byte array
   * @param len length of the byte array
   * @return hashed value of the byte array
   *
   * @attention Do not override this function in derived classes.
   */
  virtual uint64_t operator()(const uint8_t *key,
                              const int32_t len) const final {
    return this->hash(key, len);
  };
  /**
   * @brief Hash an integer
   *
   * @param val integer to hash
   * @return hashed value of the integer
   *
   * @attention Do not override this function in derived classes.
   */
  virtual uint64_t operator()(const size_t val) const final {
    return this->hash(reinterpret_cast<const uint8_t *>(&val), sizeof(size_t));
  }
  /**
   * @brief Hash a flowkey
   *
   * @tparam key_len length of flowkey
   * @param flowkey the flowkey to hash
   * @return hashed value of the flowkey
   *
   * @attention Do not override this function in derived classes.
   */
  template <int32_t key_len>
  uint64_t operator()(const FlowKey<key_len> &flowkey) const {
    return this->hash(reinterpret_cast<const uint8_t *>(flowkey.cKey()),
                      key_len);
  }
  /**
   * @brief Randomize the seed of the pseudo-randomness generator
   *
   * @note Currently not invoked.
   *
   */
  static void random_seed() { ::srand((unsigned)time(NULL)); }
};

/**
 * @brief Aware hash
 *
 * @author FerricIon (you@domain.com)
 *
 * @details **Refs are wanted!**
 *
 */
class AwareHash : public HashBase {
  uint64_t init;
  uint64_t scale;
  uint64_t hardener;
  /**
   * @brief Construct by a 3-tuple.
   *
   * @details The constructor should only be called as the util of generating
   * randomized value for `init`, `scale` and `hardener`.
   *
   */
  AwareHash(uint64_t init, uint64_t scale, uint64_t hardener)
      : init(init), scale(scale), hardener(hardener) {}
  /**
   * @see HashBase::hash(const uint8_t *, const int32_t) const
   */
  uint64_t hash(const uint8_t *data, const int32_t n) const;

public:
  /**
   * @brief Construct an AwareHash instance
   *
   * @details Seeds are internally mangled and hashed so that fewer
   * hash collisions are expected.
   *
   */
  AwareHash();
};

} // namespace OmniSketch::Hash