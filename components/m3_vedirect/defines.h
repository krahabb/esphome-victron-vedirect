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

}  // namespace m3_vedirect
}  // namespace esphome
