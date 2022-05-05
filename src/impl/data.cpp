/**
 * @file data.cpp
 * @author dromniscience (you@domain.com)
 * @brief Implementation of some class methods in data.h
 *
 * @copyright Copyright (c) 2022
 *
 */
#include <common/data.h>
#include <sstream>

namespace OmniSketch::Data {

DataFormat::DataFormat(const toml::array &array) {
  std::fill(offset, offset + ENDALL, -1);
  std::fill(length, length + ENDALL, -1);

  const toml::array *ptr0, *ptr1;
  int32_t off = 0;
  int32_t len = 0;
  int32_t size = 0;
  std::string str;
  bool b_flow = false, b_len = false, b_time = false;

  if (array.size() != 2)
    goto error;
  if (!array[0].is_array() || !array[1].is_array())
    goto error;

  ptr0 = array[0].as_array();
  ptr1 = array[1].as_array();
  size = ptr0->size();
  if (size != ptr1->size())
    goto error;
  for (int32_t i = 0; i < size; i++) {
    if (!(*ptr0)[i].is_string() || !((*ptr1)[i].is_integer()))
      goto error;
    str = (*ptr0)[i].as_string()->get();
    len = (*ptr1)[i].as_integer()->get();
    if (!str.compare("flowkey")) {
      if (b_flow)
        goto error;
      if (len == 4 || len == 8 || len == 13) {
        offset[KEYLEN] = off;
        length[KEYLEN] = len;
        off += len;
        b_flow = true;
      } else
        goto error;
    } else if (!str.compare("timestamp")) {
      if (b_time)
        goto error;
      if (len == 1 || len == 2 || len == 4 || len == 8) {
        offset[TIMESTAMP] = off;
        length[TIMESTAMP] = len;
        off += len;
        b_time = true;
      } else
        goto error;
    } else if (!str.compare("length")) {
      if (b_len)
        goto error;
      if (len == 1 || len == 2 || len == 4 || len == 8) {
        offset[LENGTH] = off;
        length[LENGTH] = len;
        off += len;
        b_len = true;
      } else
        goto error;
    } else if (!str.compare("padding") && len > 0) {
      off += len;
    } else
      goto error;
  }
  total = off;
  if (!b_flow)
    goto error;
  return;
error:
  // assemble error message
  std::stringstream ioss;
  ioss << array;
  std::getline(ioss, str);
  throw std::runtime_error(
      fmt::format("Runtime Error: Unknown format {}", str));
  /*
  // set to default format
  std::fill(offset, offset + ENDALL, -1);
  std::fill(length, length + ENDALL, -1);
  LOG(WARNING,
      fmt::format("Switch to default format: [ [ 'flowkey' ], [ 13 ] ]"));
  offset[KEYLEN] = 0;
  length[KEYLEN] = 13;
  total = length[KEYLEN];*/
}

} // namespace OmniSketch::Data