/**
 * @file utils.h
 * @author FerricIon (you@domain.com)
 * @brief Some utils
 *
 * @copyright Copyright (c) 2022
 *
 */
#pragma once

#include <string_view>
#include <toml++/toml.h>
#include <vector>

/**
 * @brief Utils of manipulating integers, parsing configuration files and so on.
 *
 */
namespace OmniSketch::Util {

#define MANGLE_MAGIC ((int32_t)2083697005)
/**
 * @brief Mangling a multi-byte object
 *
 * @tparam T  object type
 * @param key the object
 * @return the mangled object
 */
template <typename T> T Mangle(T key) {
  const size_t n = sizeof(T) >> 1;
  char *s = (char *)&key;
  for (int i = 0; i < n; ++i)
    std::swap(s[i], s[n - 1 - i]);

  return key * MANGLE_MAGIC;
}
#undef MANGLE_MAGIC

/**
 * @brief Compute primality of a 32-bit number
 *
 * @param n     Number to be checked
 * @return `true` if is a prime; `false` otherwise.
 *
 * @warning `n` must be positive. Otherwise an exception would be thrown.
 */
bool IsPrime(int32_t n);
/**
 * @brief Find the next prime number.
 *
 * @param n Starting number
 * @return Next prime number
 *
 * @warning `n` must be positive. Otherwise an exception would be thrown.
 */
int32_t NextPrime(int32_t n);
/**
 * @brief Convert a 2-byte word from network endian to the host endian
 *
 */
uint16_t Net2Host16(uint16_t val);
/**
 * @brief Convert a 4-byte word from network endian to host endian
 *
 */
uint32_t Net2Host32(uint32_t val);
/**
 * @brief Endianness of the platform
 * @return `true` on big endian; `false` on small endian.
 *
 */
inline bool Endianness() {
  const int32_t i = 1;
  return *reinterpret_cast<const int8_t *>(&i) == 0;
}

/**
 * @brief Parse config file and return its configurations in a versatile
 * manner
 *
 * @author dromniscience (you@domain.com)
 */
class ConfigParser {
  /**
   * @brief Store the parsed nested table
   *
   */
  toml::node_view<toml::node> node;
  /**
   * @brief Store the root object for the given config file
   *
   */
  toml::table tbl;
  /**
   * @brief Whether the current parsing succeeds
   * @details May fail due to a wrong opening path or format error in the config
   * file
   *
   */
  bool is_parsed;
  /**
   * @brief Workhouse of parseConfig
   *
   * @return `true` on success; `false` otherwise.
   */
  bool parse(int32_t &arg, const std::string_view arg_name,
             const bool error_logging) const;
  /**
   * @brief Workhouse of parseConfig
   *
   * @return `true` on success; `false` otherwise.
   */
  bool parse(double &arg, const std::string_view arg_name,
             const bool error_logging) const;
  /**
   * @brief Workhouse of parseConfig
   *
   * @return `true` on success; `false` otherwise.
   */
  bool parse(bool &arg, const std::string_view arg_name,
             const bool error_logging) const;
  /**
   * @brief Workhouse of parseConfig
   *
   * @return `true` on success; `false` otherwise.
   */
  bool parse(std::vector<int32_t> &arg, const std::string_view arg_name,
             const bool error_logging) const;
  /**
   * @brief Workhouse of parseConfig
   *
   * @return `true` on success; `false` otherwise.
   */
  bool parse(std::vector<double> &arg, const std::string_view arg_name,
             const bool error_logging) const;
  /**
   * @brief Workhouse of parseConfig
   *
   * @return `true` on success; `false` otherwise.
   */
  bool parse(std::vector<std::string> &arg, const std::string_view arg_name,
             const bool error_logging) const;
  /**
   * @brief Workhouse of parseConfig
   *
   * @return `true` on success; `false` otherwise.
   */
  bool parse(std::string &arg, const std::string_view arg_name,
             const bool error_logging) const;
  /**
   * @brief Workhouse of parseConfig
   *
   * @return `true` on success; `false` otherwise.
   */
  bool parse(toml::array &arg, const std::string_view arg_name,
             const bool error_logging) const;

public:
  /**
   * @brief Open the config file
   *
   * @param config_file path to the config file
   */
  ConfigParser(const std::string_view config_file);
  /**
   * @brief Return whether the parsing succeeds
   *
   */
  [[nodiscard]] bool succeed() const;
  /**
   * @brief Set the working node in the config file.
   * @details The data about to read should always locate in the working node.
   * See example below for a better understanding. An argument of `""` (or
   * void) means the root. This method can be invoked upon a single instances
   * multiple times.
   *
   * @param path path to the working node
   *
   * ### Example
   * Suppose the toml file is
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~.toml
   * # toml.toml
   *
   * key = 2 # then this!
   *
   * [A.B.C]
   * key = 1 # first parse this!
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   * and you want to fetch the value for `key`. This code snippet works for you.
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~.cpp
   * # example.cpp
   * using namespace OmniSketch::Util;
   *
   * // open file and parse
   * auto parser = ConfigParser("toml.toml");
   * // check whether the parsing succeed
   * if(!parser.succeed()) exit(-1);
   *
   * // set working node to [A.B.C]
   * parser.setWorkingNode("A.B.C");
   *
   * int first_key;
   * parser.parseConfig(first_key, "key");
   *
   * // reset working node to the root
   * parser.setWorkingNode();
   * // Equivalently, parser.setWorkingNode("");
   *
   * int second_key;
   * parser.parseConfig(second_key, "key");
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   *
   */
  void setWorkingNode(const std::string_view path = "");
  /**
   * @brief Get the configuration and store it directly into the object.
   *
   * @tparam T        supported types:
   * - int32_t
   * - double
   * - bool
   * - std::string
   * - std::vector<int32_t>
   * - std::vector<double>
   * - std::vector<std::string>
   * - toml::array
   * @param arg       the object to be written to
   * @param arg_name  the name in config file
   * @param error_logging  Enable / disable error logging when parsing fails (by
   * default enabled)
   * @return `true` on success; `false` otherwise. Possible reasons for a
   * `false` return:
   * - Misspell the key name in config file.
   * - Working node unset or is wrong.
   *
   * @attention On failure, the object may be in an inconsistent state,
   * particularly when `arg` is a vector or an toml::array, and an error be
   * logged. To disable error logging, which might be particularly useful when
   * the config is meant to be optional, set `logging_error` to `false`.
   */
  template <typename T>
  bool parseConfig(T &arg, const std::string_view arg_name,
                   const bool error_logging = true) const {
    // need NOT to preclude the situation that parsing config precedes reading
    // if (tbl.empty() || node == toml::node_view<toml::node>())
    //  return false;
    return parse(arg, arg_name, error_logging);
  }
};

/**
 * @brief Integer of any fixed length
 *
 * @details Length of integer is specified at run-time. The content of the
 * counter is always interpreted as a *non-negative* value.
 *
 * @tparam T  Should be large enough to hold arithmetic overflow. This class
 * works with both signed and unsigned integer.
 */
template <typename T> class DynamicIntX {
private:
  T counter;
  size_t bits;

public:
  /**
   * @brief Construct by specifying the length of the integer
   * @details In **bits**. Must be in (0, 8 * sizeof(T) - 1), or an exception
   * would be thrown.
   */
  DynamicIntX(size_t bits);
  /**
   * @brief Update by a certain value
   *
   * @note If `T` is signed and `val` is negative, make sure val is within
   * `[-(2^n - 1), 2^n - 1]`, where `n = 8 * sizeof(T) - 2`. (This is a
   * technical requirement due to the correct interpretation of overflow) For
   * example, the range when `T = int32_t` is `[-1073741823, 1073741823]`.
   *
   * @return the overflowed value
   */
  T operator+(T val);
  /**
   * @brief Get the value of the counter
   *
   */
  T getVal() const { return counter; }
};

} // namespace OmniSketch::Util

//-----------------------------------------------------------------------------
//
///                        Implementation of templated methods
//
//-----------------------------------------------------------------------------

namespace OmniSketch::Util {

template <typename T>
DynamicIntX<T>::DynamicIntX(size_t bits) : counter(0), bits(bits) {
  if (!bits || bits >= sizeof(T) * 8 - 1) {
    throw std::length_error(std::string("Length Too Large: Type ") +
                            typeid(T).name() + " expects size > 0 && < " +
                            std::to_string(8 * sizeof(T) - 1) + ", but got " +
                            std::to_string(bits) + " instead.");
  }
}

template <typename T> T DynamicIntX<T>::operator+(T val) {
  const T constant = static_cast<T>(1) << bits;
  constexpr T bound = (static_cast<T>(1) << (sizeof(T) * 8 - 2)) - 1;

  // non-negative update
  if (val >= static_cast<T>(0)) {
    // detect overflow
    if (val > bound) {
      throw std::overflow_error(
          "Overflow: The value being updated is too large. Expected <= 2^" +
          std::to_string(sizeof(T) * 8 - 2) + " - 1, but got " +
          std::to_string(val) + " instead.");
    }

    T overflow = val >> bits;
    T add = counter + (val & (constant - 1));
    counter = add % constant;
    overflow += add / constant;
    return overflow;
  } // negative update, and T must be signed
  else {
    // detect overflow
    if (val < -bound) {
      throw std::overflow_error("Overflow: The value being updated is too "
                                "negative. Expected >= -2^" +
                                std::to_string(sizeof(T) * 8 - 2) +
                                " + 1, but got " + std::to_string(val) +
                                " instead.");
    }

    T negate = -val;
    // borrow bits
    T negate_overflow = negate >> bits;
    T add = constant + counter - (negate & (constant - 1));
    counter = add % constant;
    return -(negate_overflow + 1 - (add / constant));
  }
}

} // namespace OmniSketch::Util