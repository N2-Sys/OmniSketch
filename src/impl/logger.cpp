/**
 * @file logger.cpp
 * @author FerricIon (you@domain.com)
 * @brief Implementation of the logger
 *
 * @copyright Copyright (c) 2022
 *
 */

#include <common/logger.h>
#include <fmt/color.h>
#include <fmt/core.h>

void Log(LogLevel level, const std::string &msg, const std::string &file,
         int lineno) {
  static const std::pair<fmt::text_style, std::string> LevelIndicators[] = {
      {fmt::text_style(), "VERBOSE"},
      {fmt::bg(fmt::terminal_color::blue), "INFO"},
      {fmt::bg(fmt::terminal_color::yellow), "WARNING"},
      {fmt::bg(fmt::terminal_color::red), "ERROR"},
      {fmt::bg(fmt::terminal_color::white) |
           fmt::fg(fmt::terminal_color::black),
       "FATAL"},
      {fmt::emphasis::italic, "UNKNOWN"}};

  if (level < VERBOSE || level > FATAL)
    level = UNKNOWN;

  fmt::print(stderr, LevelIndicators[level].first, "{:>7}|",
             LevelIndicators[level].second);
  fmt::print(stderr, " {} @{}:{}\n", msg, file, lineno);
}