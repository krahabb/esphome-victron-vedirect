#pragma once
#include "ve_reg_def.h"
#include <stddef.h>
#include <cstring>

namespace esphome {
namespace m3_vedirect {

using namespace ::m3_ve_reg;

class Manager;
class Register;

// maximum amount of time (millis) without receiving data
// after which we consider the vedirect link disconnected
#define VEDIRECT_LINK_TIMEOUT_MILLIS 5000

// maximum amount of time (millis) without receiving a SET command
// reply after which we consider the command unsuccesful
#define VEDIRECT_COMMAND_TIMEOUT_MILLIS 1000

// Helpers for unordered_map with const char* key
#if __cpp_constexpr >= 201304L
#define _RELAXEDCONSTEXPR constexpr
#else
#define _RELAXEDCONSTEXPR
#endif
#define _HASH_SHIFT 16u
#define _HASH_MUL 23456789u

struct cstring_hash {
  _RELAXEDCONSTEXPR size_t operator()(const char *s) const {
    size_t h = 0;
    for (; *s; ++s) {
      h = h * _HASH_MUL + static_cast<unsigned char>(*s);
      h ^= h >> _HASH_SHIFT;
    }
    return h *= _HASH_MUL;
  }
};

struct cstring_eq {
  _RELAXEDCONSTEXPR
  bool operator()(const char *__x, const char *__y) const { return !strcmp(__x, __y); }
};

}  // namespace m3_vedirect
}  // namespace esphome