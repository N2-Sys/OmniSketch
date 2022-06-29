/**
 * @file flowkey.h
 * @author FerricIon (you@domain.com)
 * @brief Template of flowkeys
 *
 * @copyright Copyright (c) 2022
 *
 */
#pragma once

#include <stdexcept>
#include <string>

namespace OmniSketch {
/**
 * @brief Flowkey-out-of-range exception
 *
 * @details This exception is thrown whenever indexing into flowkey is out of
 * range. It stores detailed description of the exception.
 *
 */
class FlowKeyOutOfRange : public std::out_of_range {
public:
  /**
   * @brief Construct a new FlowKeyOutOfRange object
   *
   * @param pos         offset of the base
   * @param offset      offset as an index
   * @param total_len   length of flowkey
   */
  FlowKeyOutOfRange(int32_t pos, int32_t offset, int32_t total_len)
      : std::out_of_range("FlowKey Out of Range: pos: " + std::to_string(pos) +
                          ", offset: " + std::to_string(offset) +
                          ", total_len: " + std::to_string(total_len)) {}
};

/**
 * @brief FlowKey mismatch exception
 *
 * @details This exception is thrown if the arguments given to a flowkey is bad.
 *
 */
class FlowKeyMismtach : public std::invalid_argument {
public:
  FlowKeyMismtach(int32_t length, int32_t given)
      : std::invalid_argument("FlowKey Length Mismatch: Required " +
                              std::to_string(length) + ", given " +
                              std::to_string(given)) {}
  FlowKeyMismtach(int32_t length1, int32_t length2, int32_t given)
      : std::invalid_argument("FlowKey Length Mismatch: Required " +
                              std::to_string(length1) + " or " +
                              std::to_string(length2) + ", given " +
                              std::to_string(given)) {}
};

/**
 * @brief Template class for flowkey
 *
 * @tparam key_len length of flowkey
 */
template <int32_t key_len> class FlowKey {
private:
  int8_t key_[key_len];
  template <int32_t other_len> friend class FlowKey;

public:
  /**
   * @brief Void constructor
   * @details Initialized to an all-zero flowkey
   */
  FlowKey();
  /**
   * @brief Construct by copying from a pointer
   *
   * @param key the pointer to be copied from
   */
  FlowKey(const int8_t *key);
  /**
   * @brief Construct by a 1-tuple
   *
   * @param ipaddr IP address of the flow
   * @warning Be sure to have `key_len=4` when invoking. Otherwise an exception
   * would be thrown.
   */
  FlowKey(int32_t ipaddr);
  /**
   * @brief Construct by a 2-tuple
   *
   * @param srcip source IP address
   * @param dstip destination IP address
   *
   * @warning Be sure to have `key_len=8` when invoking. Otherwise an exception
   * would be thrown.
   */
  FlowKey(int32_t srcip, int32_t dstip);
  /**
   * @brief Construct by a 5-tuple
   *
   * @param srcip     source IP address
   * @param dstip     destination IP address
   * @param srcport   source port
   * @param dstport   destination port
   * @param protocol  code of network protocol in use
   *
   * @warning Be sure to have `key_len=13` when invoking. Otherwise an exception
   * would be thrown.
   */
  FlowKey(int32_t srcip, int32_t dstip, int16_t srcport, int16_t dstport,
          int8_t protocol);
  /**
   * @brief See if two flowkeys are identical
   *
   * @param otherkey  the second flowkey to compare with (of the same length)
   * @return `true` if equal; `false` otherwise.
   */
  bool operator==(const FlowKey &otherkey) const;
  /**
   * @brief See if the first flowkey is less than the second key in
   * lexicographic order.
   *
   * @param otherkey  the second flowkey to compare with (of the same length)
   * @return `false` if is smaller; `true` if equal or is greater
   */
  bool operator<(const FlowKey &otherkey) const;
  /**
   * @brief XOR two flowkeys in place
   *
   */
  FlowKey<key_len> &operator^=(const FlowKey &otherkey);
  /**
   * @brief Copy from another flowkey (probably of different
   * length)
   *
   * @tparam other_len  Length of the other flowkey
   * @param pos        destination offset (into the current flowkey)
   * @param other_key  the other flowkey to be copied from
   * @param o_pos      source offset (into the other flowkey)
   * @param len        length of the copied interval (in bytes)
   *
   * @return the copied flowkey itself
   */
  template <int32_t other_len>
  FlowKey<key_len> &copy(int32_t pos, const FlowKey<other_len> &other_key,
                         int32_t o_pos, int32_t len);
  /**
   * @brief Copy from a given pointer
   *
   * @param pos destination offset (into the current flowkey)
   * @param key the pointer to be copied from
   * @param len length of the copied interval (in bytes)
   * @return the copied flowkey itself
   */
  FlowKey<key_len> &copy(int32_t pos, const int8_t *key, int32_t len);
  /**
   * @brief Swap the content of two flowkey
   * @details No effect if one flowkey swaps with itself
   *
   */
  FlowKey<key_len> &swap(FlowKey<key_len> &the_other_key);
  /**
   * @brief Get a single bit in flowkey
   *
   * @param pos   Offset into the flowkey
   * @return `1` if the bit is on; `0` otherwise.
   */
  int8_t getBit(int32_t pos) const;
  /**
   * @brief Set a single bit in flowkey
   *
   * @param pos Offset into the flowkey
   * @param one `1` if set to be on; `0` otherwise.
   */
  void setBit(int32_t pos, bool one);
  /**
   * @brief Get the flowkey as a char array
   *
   * @return const int8_t* pointer to flowkey
   */
  const int8_t *cKey() const { return key_; }
  /**
   * @brief Get the IP address
   *
   * @return IP address as 1-tuple
   *
   * @warning Be sure to have `key_len=4` when invoking
   */
  int32_t getIp() const {
    if (key_len != 4) {
      throw FlowKeyMismtach(4, key_len);
    }
    return *(int32_t *)key_;
  }
  /**
   * @brief Get the src IP address
   *
   * @return source IP address
   *
   * @warning Be sure to have `key_len=8` or `key_len=13` when invoking
   */
  int32_t getSrcIp() const {
    if (key_len != 8 && key_len != 13) {
      throw FlowKeyMismtach(8, 13, key_len);
    }
    return *(int32_t *)key_;
  }
  /**
   * @brief Get the dst IP address
   *
   * @return destination IP address
   *
   * @warning Be sure to have `key_len=8` or `key_len=13` when invoking
   */
  int32_t getDstIp() const {
    if (key_len != 8 && key_len != 13) {
      throw FlowKeyMismtach(8, 13, key_len);
    }
    return *(int32_t *)(key_ + 4);
  }
  /**
   * @brief Get the src port
   *
   * @return source port no
   *
   * @warning Be sure to have `key_len=13` when invoking
   */
  int16_t getSrcPort() const {
    if (key_len != 13) {
      throw FlowKeyMismtach(13, key_len);
    }
    return *(int16_t *)(key_ + 8);
  }
  /**
   * @brief Get the dst port
   *
   * @return destination port no
   *
   * @warning Be sure to have `key_len=13` when invoking
   */
  int16_t getDstPort() const {
    if (key_len != 13) {
      throw FlowKeyMismtach(13, key_len);
    }
    return *(int16_t *)(key_ + 10);
  }
  /**
   * @brief Get the protocol code
   *
   * @return the protocol code of the flow
   *
   * @warning Be sure to have `key_len=13` when invoking
   */
  int8_t getProtocol() const {
    if (key_len != 13) {
      throw FlowKeyMismtach(13, key_len);
    }
    return *(int8_t *)(key_ + 12);
  }
};

} // namespace OmniSketch

//-----------------------------------------------------------------------------
//
//                       Implementation of template method
//
//-----------------------------------------------------------------------------

#define BYTE(n) ((n) >> 3)
#define BIT(n) ((n)&7)

namespace OmniSketch {

template <int32_t key_len> FlowKey<key_len>::FlowKey() {
  std::fill(key_, key_ + key_len, 0);
}

template <int32_t key_len> FlowKey<key_len>::FlowKey(const int8_t *key) {
  std::copy(key, key + key_len, key_);
}

template <int32_t key_len> FlowKey<key_len>::FlowKey(int32_t ipaddr) {
  if (key_len != 4) {
    throw FlowKeyMismtach(4, key_len);
  }
  *(int32_t *)key_ = ipaddr;
}

template <int32_t key_len>
FlowKey<key_len>::FlowKey(int32_t srcip, int32_t dstip) {
  if (key_len != 8) {
    throw FlowKeyMismtach(8, key_len);
  }
  *(int32_t *)key_ = srcip;
  *(int32_t *)(key_ + 4) = dstip;
}

template <int32_t key_len>
FlowKey<key_len>::FlowKey(int32_t srcip, int32_t dstip, int16_t srcport,
                          int16_t dstport, int8_t protocol) {
  if (key_len != 13) {
    throw FlowKeyMismtach(13, key_len);
  }
  *(int32_t *)key_ = srcip;
  *(int32_t *)(key_ + 4) = dstip;
  *(int16_t *)(key_ + 8) = srcport;
  *(int16_t *)(key_ + 10) = dstport;
  *(int8_t *)(key_ + 12) = protocol;
}

template <int32_t key_len>
bool FlowKey<key_len>::operator==(const FlowKey &otherkey) const {
  for (int32_t i = 0; i < key_len; ++i) {
    if (key_[i] != otherkey.key_[i]) {
      return false;
    }
  }
  return true;
}

template <int32_t key_len>
bool FlowKey<key_len>::operator<(const FlowKey &otherkey) const {
  for (int32_t i = 0; i < key_len; ++i) {
    if (key_[i] < otherkey.key_[i]) {
      return true;
    } else if (key_[i] > otherkey.key_[i]) {
      return false;
    }
  }
  return false;
}

template <int32_t key_len>
FlowKey<key_len> &FlowKey<key_len>::operator^=(const FlowKey &otherkey) {
  for (int32_t i = 0; i < key_len; ++i) {
    key_[i] ^= otherkey.key_[i];
  }
  return *this;
}

template <int32_t key_len>
template <int32_t other_len>
FlowKey<key_len> &FlowKey<key_len>::copy(int32_t pos,
                                         const FlowKey<other_len> &other_key,
                                         int32_t o_pos, int32_t len) {
  if (pos < 0 || pos + len > key_len) {
    throw FlowKeyOutOfRange(pos, len, key_len);
  }
  if (o_pos < 0 || o_pos + len > other_len) {
    throw FlowKeyOutOfRange(o_pos, len, other_len);
  }
  auto o_key = other_key.cKey();
  std::copy(o_key + o_pos, o_key + o_pos + len, key_ + pos);
  return *this;
}

template <int32_t key_len>
FlowKey<key_len> &FlowKey<key_len>::copy(int32_t pos, const int8_t *key,
                                         int32_t len) {
  if (pos < 0 || pos + len > key_len) {
    throw FlowKeyOutOfRange(pos, len, key_len);
  }
  std::copy(key, key + len, key_ + pos);
  return *this;
}

template <int32_t key_len>
FlowKey<key_len> &FlowKey<key_len>::swap(FlowKey<key_len> &the_other_key) {
  std::swap(key_, the_other_key.key_);
  return *this;
}

template <int32_t key_len> int8_t FlowKey<key_len>::getBit(int32_t pos) const {
  if (BYTE(pos) >= key_len) {
    throw FlowKeyOutOfRange(BYTE(pos), 0, key_len);
  }
  return (key_[BYTE(pos)] >> BIT(pos)) & 1;
}

template <int32_t key_len>
void FlowKey<key_len>::setBit(int32_t pos, bool one) {
  if (BYTE(pos) >= key_len) {
    throw FlowKeyOutOfRange(BYTE(pos), 0, key_len);
  }
  if (one) {
    key_[BYTE(pos)] |= (1 << BIT(pos));
  } else {
    key_[BYTE(pos)] &= ~(1 << BIT(pos));
  }
}

} // namespace OmniSketch

namespace std {
/**
 * @brief Hash a flow key
 *
 * @details Useful if flowkey is used as hash key
 *
 * @tparam key_len length of flow key
 */
template <int32_t key_len> struct hash<OmniSketch::FlowKey<key_len>> {
  /**
   * @brief The workhorse
   */
  size_t operator()(const OmniSketch::FlowKey<key_len> &flowkey) const {
    /*
    // Caveat: This hash function is too slow!
    size_t ret = 0;
    int32_t i = 0;
    for (; i + sizeof(size_t) <= key_len; i += sizeof(size_t)) {
      ret = std::hash<size_t>()(
                *reinterpret_cast<const size_t *>(flowkey.cKey() + i) + ret) ^
            ret;
    }
    for (; i < key_len; i++) {
      ret = std::hash<size_t>()(static_cast<size_t>(flowkey.cKey()[i]) + ret) ^
            ret;
    }
    return ret;
    */
    constexpr size_t InitialFNV = 2166136261U;
    constexpr size_t FNVMultiple = 16777619;
    const int8_t *const ptr = flowkey.cKey();

    size_t hash = InitialFNV;
    for (int32_t i = 0; i < key_len; i++) {
      hash = hash ^ (ptr[i]);    /* xor  the low 8 bits */
      hash = hash * FNVMultiple; /* multiply by the magic number */
    }
    return hash;
  }
};

/**
 * @brief Compare if two flowkeys equal
 *
 * @tparam key_len length of flowkey
 */
template <int32_t key_len> struct equal_to<OmniSketch::FlowKey<key_len>> {
  /**
   * @brief The workhorse
   *
   */
  bool operator()(const OmniSketch::FlowKey<key_len> &flowkey1,
                  const OmniSketch::FlowKey<key_len> &flowkey2) const {
    return flowkey1 == flowkey2;
  }
};

} // namespace std

#undef BYTE
#undef BIT