#pragma once
#include "esphome/core/version.h"
#include "esphome/core/defines.h"
#include "ve_reg_def.h"

namespace esphome {
namespace m3_vedirect {

using namespace ::m3_ve_reg;

class Manager;
class Register;

// default 'PING' HEX command timeout (millis)
#define VEDIRECT_PING_TIMEOUT_MILLIS 60000

// maximum amount of time (millis) without receiving data
// after which we consider the vedirect link disconnected
#define VEDIRECT_LINK_TIMEOUT_MILLIS 5000

// maximum amount of time (millis) without receiving a SET command
// reply after which we consider the command unsuccesful
#define VEDIRECT_COMMAND_TIMEOUT_MILLIS 1000

// maximum number of pending requests (GET/SET/COMMAND) we can queue/track
#ifndef VEDIRECT_REQUEST_QUEUE_SIZE
#define VEDIRECT_REQUEST_QUEUE_SIZE 5
#endif

// number of pre-allocated buckets in HEX registers map (see HexRegisterMap)
#ifndef VEDIRECT_HEXMAP_SIZE
#define VEDIRECT_HEXMAP_SIZE 64
#endif

// number of pre-allocated buckets in TEXT registers map (see TextRegisterMap)
#ifndef VEDIRECT_TEXTMAP_SIZE
#define VEDIRECT_TEXTMAP_SIZE 16
#endif

// fix wrong static_assert behavior in uninstantiated templates
// https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2023/p2593r1.html
// This would actually be needed only for ESP8266 toolchain (xtensa 10.3.0)
// while ESP32 toolchain seem to behave correctly (as of 2025-11-28).
template<typename T> struct _assert_false : std::false_type {};
#define STATIC_ASSERT_FALSE(T) static_assert(_assert_false<T>::value, "This code path should never be compiled.")
#define TEMPLATE_MEMBER_NEED_OVERRIDE(_Targ, msg) \
  static_assert(_assert_false<_Targ>::value, msg " must be implemented by derived classes.")

}  // namespace m3_vedirect
}  // namespace esphome
