#pragma once
#include <stdint.h>
/// @brief m3_ve_reg namespace is a collection of api's independent of esphome
/// (so that it could esily be used as a library) . It's purpose is to provide a
/// framework to manage vedirect registers through both HEX frames and TEXT frames
namespace m3_ve_reg {

typedef unsigned short register_id_t;
typedef unsigned char group_id_t;

// Helper to get the number of elements in static arrays
#define ARRAY_COUNT(_array) (sizeof(_array) / sizeof(_array[0]))

#pragma pack(push, 1)
/// @brief basic HEXFRAME prototype declaration
struct HEXFRAME {
  enum COMMAND : uint8_t {
    Ping = 0x1,
    Done = 0x1,  // response
    AppVersion = 0x3,
    Unknown = 0x3,  // response
    ProductId = 0x4,
    Error = 0x4,     // response
    PingResp = 0x5,  // response
    Restart = 0x6,
    Get = 0x7,
    Set = 0x8,
    Async = 0xA,
  };

  enum DATA_TYPE : uint8_t {
    VARIADIC = 0,  // used for strings or unknown/untyped registers
    UN8 = 1,
    UN16 = 2,
    UN32 = 3,
    SN8 = 4,
    SN16 = 5,
    SN32 = 6,
    _COUNT = 7,
  };
  static const uint8_t DATA_TYPE_TO_SIZE[];
  template<typename T> static constexpr DATA_TYPE DATA_TYPE_OF();
  template<typename T> static constexpr T DATA_UNKNOWN();

  COMMAND command;
  union {
    uint8_t rawdata[0];
    struct {
      register_id_t register_id;
      uint8_t flags;
      union {
        uint8_t data_u8;
        int16_t data_i16;
        uint16_t data_u16;
        uint32_t data_u32;
        uint8_t data[0];
        char data_string[0];
      };
    };
  };

  // member helper for generalized casting of payload data
  template<typename Tcast, typename Traw> Tcast get_data_t() { return static_cast<Tcast>(*(Traw *) this->data); }
  // static helper for generalized casting of payload data
  template<typename Tcast, typename Traw> static Tcast get_data_t(const HEXFRAME *hexframe) {
    return static_cast<Tcast>(*(Traw *) hexframe->data);
  }

  typedef int (*get_data_int_func_t)(const HEXFRAME *);
  static const get_data_int_func_t GET_DATA_AS_INT[DATA_TYPE::_COUNT];
};
#pragma pack(pop)

template<> constexpr HEXFRAME::DATA_TYPE HEXFRAME::DATA_TYPE_OF<uint8_t>() { return DATA_TYPE::UN8; }
template<> constexpr HEXFRAME::DATA_TYPE HEXFRAME::DATA_TYPE_OF<uint16_t>() { return DATA_TYPE::UN16; }
template<> constexpr HEXFRAME::DATA_TYPE HEXFRAME::DATA_TYPE_OF<uint32_t>() { return DATA_TYPE::UN32; }
template<> constexpr HEXFRAME::DATA_TYPE HEXFRAME::DATA_TYPE_OF<int8_t>() { return DATA_TYPE::SN8; }
template<> constexpr HEXFRAME::DATA_TYPE HEXFRAME::DATA_TYPE_OF<int16_t>() { return DATA_TYPE::SN16; }
template<> constexpr HEXFRAME::DATA_TYPE HEXFRAME::DATA_TYPE_OF<int32_t>() { return DATA_TYPE::SN32; }

template<> constexpr uint8_t HEXFRAME::DATA_UNKNOWN<uint8_t>() { return 0xFFu; }
template<> constexpr uint16_t HEXFRAME::DATA_UNKNOWN<uint16_t>() { return 0xFFFFu; }
template<> constexpr uint32_t HEXFRAME::DATA_UNKNOWN<uint32_t>() { return 0xFFFFFFFFu; }
template<> constexpr int8_t HEXFRAME::DATA_UNKNOWN<int8_t>() { return 0x7F; }
template<> constexpr int16_t HEXFRAME::DATA_UNKNOWN<int16_t>() { return 0x7FFF; }
template<> constexpr int32_t HEXFRAME::DATA_UNKNOWN<int32_t>() { return 0x7FFFFFFF; }

}  // namespace m3_ve_reg