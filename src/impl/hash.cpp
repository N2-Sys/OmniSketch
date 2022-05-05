/**
 * @file hash.cpp
 * @author Dustin-He (you@domain.com)
 * @brief Implementation of hashing classes
 *
 * @copyright Copyright (c) 2022
 *
 */
#include <common/hash.h>
#include <common/utils.h>

//-----------------------------------------------------------------------------
//
//                       Implementation of class method
//
//-----------------------------------------------------------------------------

namespace OmniSketch::Hash {

AwareHash::AwareHash() {
  static const int32_t GEN_INIT_MAGIC = 388650253;
  static const int32_t GEN_SCALE_MAGIC = 388650319;
  static const int32_t GEN_HARDENER_MAGIC = 1176845762;
  static int32_t index = 0;
  static uint64_t seed = 0;
  seed = rand();
  static AwareHash gen_hash(GEN_INIT_MAGIC, GEN_SCALE_MAGIC,
                            GEN_HARDENER_MAGIC);

  uint64_t mangled;
  mangled = Util::Mangle(seed + (index++));
  init = gen_hash((const uint8_t *)&mangled, sizeof(uint64_t));
  mangled = Util::Mangle(seed + (index++));
  scale = gen_hash((const uint8_t *)&mangled, sizeof(uint64_t));
  mangled = Util::Mangle(seed + (index++));
  hardener = gen_hash((const uint8_t *)&mangled, sizeof(uint64_t));
}

uint64_t AwareHash::hash(const uint8_t *data, const int32_t n) const {
  int32_t len = n;
  uint64_t result = init;
  while (len--) {
    result *= scale;
    result += *data++;
  }
  return result ^ hardener;
}

} // namespace OmniSketch::Hash
