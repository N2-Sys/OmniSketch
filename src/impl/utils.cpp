/**
 * @file utils.cpp
 * @author FerricIon (you@domain.com)
 * @brief Implementation of utils
 *
 * @copyright Copyright (c) 2022
 *
 */
#include <cassert>
#include <common/logger.h>
#include <common/utils.h>
#include <fmt/core.h>

//-----------------------------------------------------------------------------
//
///                        Implementation of some utils
//
//-----------------------------------------------------------------------------

namespace OmniSketch::Util {

bool IsPrime(int32_t n) {
  if (n <= 0) {
    throw std::invalid_argument(
        "Invalid Argument: Argument of IsPrime() should be positive, but got " +
        std::to_string(n) + " instead.");
  }

  if (!(n & 1)) // if n is even
    return n == 2;
  for (int32_t i = 3; static_cast<int64_t>(i) * i <= n; i += 2)
    if (n % i == 0)
      return false;
  return n != 1;
}

int32_t NextPrime(int32_t n) {
  while (!IsPrime(n))
    ++n;
  return n;
}

uint16_t Net2Host16(uint16_t val) {
  if (Endianness())
    return val;
  else
    return (val >> 8) | (val << 8);
}

uint32_t Net2Host32(uint32_t val) {
  if (Endianness())
    return val;
  else
    return (static_cast<uint32_t>(Net2Host16(static_cast<uint16_t>(val)))
            << 16) |
           static_cast<uint32_t>(Net2Host16(static_cast<uint16_t>(val >> 16)));
}

ConfigParser::ConfigParser(const std::string_view config_file) {
  static bool emitted = false; // avoid burst of logging
  if (!emitted) {
    LOG(INFO, fmt::format("Loading config from {}...", config_file));
  }
  try {
    tbl = toml::parse_file(config_file);
  } catch (const toml::parse_error &err) {
    if (!emitted) {
      LOG(FATAL, fmt::format("Parsing failed: {}", err.description()));
      emitted = true;
    }
    is_parsed = false;
    return;
  }
  is_parsed = true;
  if (!emitted) {
    LOG(VERBOSE, "Config loaded.");
    emitted = true;
  }
  return;
}

bool ConfigParser::succeed() const { return is_parsed; }

void ConfigParser::setWorkingNode(const std::string_view path) {
  auto split = [](const std::string_view strv) {
    std::vector<std::string_view> output;
    size_t first = 0;
    while (first < strv.size()) {
      const auto second = strv.find_first_of(".", first);

      if (first != second)
        output.emplace_back(strv.substr(first, second - first));

      if (second == std::string_view::npos)
        break;

      first = second + 1;
    }
    return output;
  };
  const auto vec_path = split(path);

  // toml::table -> toml::node_view<toml::node>
  node = toml::node_view<toml::node>(tbl);
  for (auto str : vec_path) {
    node = node[str];
  }
}

bool ConfigParser::parse(int32_t &arg, const std::string_view arg_name,
                         const bool error_logging) const {
  toml::node_view term = node[arg_name];
  if (!term.is_integer()) {
    if (error_logging) {
      LOG(ERROR,
          fmt::format("Fail to parse \"{}\" as type `int32_t`.", arg_name));
    }
    return false;
  }
  arg = static_cast<int32_t>(term.as_integer()->get());
  return true;
}

bool ConfigParser::parse(size_t &arg, const std::string_view arg_name,
                         const bool error_logging) const {
  toml::node_view term = node[arg_name];
  if (!term.is_integer()) {
    if (error_logging) {
      LOG(ERROR,
          fmt::format("Fail to parse \"{}\" as type `size_t`.", arg_name));
    }
    return false;
  }
  arg = static_cast<size_t>(term.as_integer()->get());
  return true;
}

bool ConfigParser::parse(double &arg, const std::string_view arg_name,
                         const bool error_logging) const {
  toml::node_view term = node[arg_name];
  if (!term.is_number()) {
    if (error_logging) {
      LOG(ERROR,
          fmt::format("Fail to parse \"{}\" as type `double`.", arg_name));
    }
    return false;
  }
  if (term.is_integer()) {
    arg = static_cast<double>(term.as_integer()->get());
  } else {
    arg = term.as_floating_point()->get();
  }
  return true;
}

bool ConfigParser::parse(bool &arg, const std::string_view arg_name,
                         const bool error_logging) const {
  toml::node_view term = node[arg_name];
  if (!term.is_boolean()) {
    if (error_logging) {
      LOG(ERROR, fmt::format("Fail to parse \"{}\" as type `bool`.", arg_name));
    }
    return false;
  }
  arg = term.as_boolean()->get();
  return true;
}

bool ConfigParser::parse(std::vector<int32_t> &arg,
                         const std::string_view arg_name,
                         const bool error_logging) const {
  arg.clear();

  toml::node_view term = node[arg_name];
  if (!term.is_array()) {
    if (error_logging) {
      LOG(ERROR, fmt::format("Fail to parse \"{}\" as a vector.", arg_name));
    }
    return false;
  }
  toml::array *arr = term.as_array();
  for (toml::node &elem : *arr) {
    if (!elem.is_integer()) {
      if (error_logging) {
        LOG(ERROR,
            fmt::format(
                "Fail to parse some elements in \"{}\" as type `int32_t`.",
                arg_name));
      }
      return false;
    }
    arg.push_back(static_cast<int32_t>(elem.as_integer()->get()));
  }
  return true;
}

bool ConfigParser::parse(std::vector<size_t> &arg,
                         const std::string_view arg_name,
                         const bool error_logging) const {
  arg.clear();

  toml::node_view term = node[arg_name];
  if (!term.is_array()) {
    if (error_logging) {
      LOG(ERROR, fmt::format("Fail to parse \"{}\" as a vector.", arg_name));
    }
    return false;
  }
  toml::array *arr = term.as_array();
  for (toml::node &elem : *arr) {
    if (!elem.is_integer()) {
      if (error_logging) {
        LOG(ERROR,
            fmt::format(
                "Fail to parse some elements in \"{}\" as type `size_t`.",
                arg_name));
      }
      return false;
    }
    arg.push_back(static_cast<size_t>(elem.as_integer()->get()));
  }
  return true;
}

bool ConfigParser::parse(std::vector<double> &arg,
                         const std::string_view arg_name,
                         const bool error_logging) const {
  arg.clear();

  toml::node_view term = node[arg_name];
  if (!term.is_array()) {
    if (error_logging) {
      LOG(ERROR, fmt::format("Fail to parse \"{}\" as a vector.", arg_name));
    }
    return false;
  }
  toml::array *arr = term.as_array();
  for (toml::node &elem : *arr) {
    if (!elem.is_number()) {
      if (error_logging) {
        LOG(ERROR,
            fmt::format(
                "Fail to parse some elements in \"{}\" as type `double`.",
                arg_name));
      }
      return false;
    }
    if (elem.is_integer()) {
      arg.push_back(static_cast<double>(elem.as_integer()->get()));
    } else {
      arg.push_back(elem.as_floating_point()->get());
    }
  }
  return true;
}

bool ConfigParser::parse(std::vector<std::string> &arg,
                         const std::string_view arg_name,
                         const bool error_logging) const {
  arg.clear();

  toml::node_view term = node[arg_name];
  if (!term.is_array()) {
    if (error_logging) {
      LOG(ERROR, fmt::format("Fail to parse \"{}\" as a vector.", arg_name));
    }
    return false;
  }
  toml::array *arr = term.as_array();
  for (toml::node &elem : *arr) {
    if (!elem.is_string()) {
      if (error_logging) {
        LOG(ERROR,
            fmt::format(
                "Fail to parse some elements in \"{}\" as type `std::string`.",
                arg_name));
      }
      return false;
    }
    arg.push_back(elem.as_string()->get());
  }
  return true;
}

bool ConfigParser::parse(std::string &arg, const std::string_view arg_name,
                         const bool error_logging) const {
  toml::node_view term = node[arg_name];
  if (!term.is_string()) {
    if (error_logging) {
      LOG(ERROR,
          fmt::format("Fail to parse \"{}\" as type `std::string`.", arg_name));
    }
    return false;
  }
  arg = term.as_string()->get();
  return true;
}

bool ConfigParser::parse(toml::array &arg, const std::string_view arg_name,
                         const bool error_logging) const {
  toml::node_view term = node[arg_name];
  if (!term.is_array()) {
    if (error_logging) {
      LOG(ERROR,
          fmt::format("Fail to parse \"{}\" as type `toml::array`.", arg_name));
    }
    return false;
  }
  arg = *term.as_array();
  return true;
}

} // namespace OmniSketch::Util