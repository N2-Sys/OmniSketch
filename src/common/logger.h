/**
 * @file logger.h
 * @author FerricIon (you@domain.com)
 * @brief Log to the standard error
 *
 * @copyright Copyright (c) 2022
 *
 */
#pragma once

#include <string>

enum LogLevel {
  VERBOSE /** for verbosity */,
  INFO /** informing messages */,
  WARNING /** potential error */,
  ERROR /** error */,
  FATAL /** fatal error */,
  UNKNOWN /** unknown circumstances */
};

#define LOG(level, msg)                                                        \
  Log((level), (msg), strrchr("/" __FILE__, '/') + 1, __LINE__)

/**
 * @brief Workhorse of logging
 *
 * @details The log shows not only its content and level, but the file and the
 * line where it was generated as well.
 *
 * @param level level of the log, namely
 *
 * - `VERBOSE`
 * - `INFO`
 * - `WARNING`
 * - `ERROR`
 * - `FATAL`
 * - `UNKNOWN`
 *
 * These names are quite self-explanatory.
 * @param msg     content of the message
 * @param file    the file from which the log is generated
 * @param lineno  the line at which the log is generated
 *
 * @warning Never call this function directly. Rather, use the `LOG` macro as
 * follows.
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~.cpp
 * std::string msg = "This is an INFO";
 * LOG(INFO, msg);
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 */
void Log(LogLevel level, const std::string &msg, const std::string &file,
         int lineno);