#include "binary_sensor.h"
#include "esphome/core/application.h"
#if ESPHOME_VERSION_CODE < VERSION_CODE(2025, 11, 0)
#ifdef USE_API
#include "esphome/components/api/api_server.h"
#endif
#endif

#include "../manager.h"

#include <cinttypes>

namespace esphome {
namespace m3_vedirect {

Register *BinarySensor::build_entity(Manager *manager, const REG_DEF *reg_def, const char *name) {
#if ESPHOME_VERSION_CODE >= VERSION_CODE(2025, 8, 0)
  if (App.get_binary_sensors().size() >= ESPHOME_ENTITY_BINARY_SENSOR_COUNT) {
    return Register::drop_platform(manager, Platform::BinarySensor);
  }
#endif
  auto entity = new BinarySensor(manager);
  manager->init_entity(entity, reg_def, name);
  App.register_binary_sensor(entity);
#if ESPHOME_VERSION_CODE < VERSION_CODE(2025, 11, 0)
// See https://github.com/esphome/esphome/pull/11772
#if defined(USE_API)
  entity->add_on_state_callback([entity](bool state) {
#if ESPHOME_VERSION_CODE >= VERSION_CODE(2025, 7, 0)
    api::global_api_server->on_binary_sensor_update(entity);
#else
    api::global_api_server->on_binary_sensor_update(entity, state);
#endif
  });
#endif
#endif
  return entity;
}

void BinarySensor::link_disconnected_() {
#if ESPHOME_VERSION_CODE >= VERSION_CODE(2025, 7, 0)
  // Assuming the StatefulEntityBase implementation was released here.
  this->invalidate_state();
#else
  this->set_has_state(false);
#endif
}

void BinarySensor::init_reg_def_() {
  switch (this->reg_def_->cls) {
    case REG_DEF::CLASS::BITMASK:
#if defined(VEDIRECT_USE_HEXFRAME)
      this->parse_hex_ = parse_hex_bitmask_;
#endif
#if defined(VEDIRECT_USE_TEXTFRAME)
      this->parse_text_ = parse_text_bitmask_;
#endif
      break;
    case REG_DEF::CLASS::ENUM:
#if defined(VEDIRECT_USE_HEXFRAME)
      this->parse_hex_ = parse_hex_enum_;
#endif
#if defined(VEDIRECT_USE_TEXTFRAME)
      this->parse_text_ = parse_text_enum_;
#endif
      break;
    default:
      break;
  }
}

#if defined(VEDIRECT_USE_HEXFRAME)
void BinarySensor::parse_hex_default_(Register *hex_register, const RxHexFrame *hex_frame) {
  static_assert(RxHexFrame::ALLOCATED_DATA_SIZE >= 1, "HexFrame storage might lead to access overflow");
  // By default considering the register as a BOOLEAN
  static_cast<BinarySensor *>(hex_register)->publish_state(hex_frame->data_t<ENUM_DEF::enum_t>());
}

void BinarySensor::parse_hex_bitmask_(Register *hex_register, const RxHexFrame *hex_frame) {
  // BITMASK registers have storage up to 4 bytes
  static_assert(RxHexFrame::ALLOCATED_DATA_SIZE >= 4, "HexFrame storage might lead to access overflow");
  static_cast<BinarySensor *>(hex_register)->parse_bitmask_(hex_frame->safe_data_u32());
}

void BinarySensor::parse_hex_enum_(Register *hex_register, const RxHexFrame *hex_frame) {
  static_assert(RxHexFrame::ALLOCATED_DATA_SIZE >= 1, "HexFrame storage might lead to access overflow");
  static_cast<BinarySensor *>(hex_register)->parse_enum_(hex_frame->data_t<ENUM_DEF::enum_t>());
}
#endif  // defined(VEDIRECT_USE_HEXFRAME)

#if defined(VEDIRECT_USE_TEXTFRAME)
void BinarySensor::parse_text_default_(Register *hex_register, const char *text_value) {
  static_cast<BinarySensor *>(hex_register)->publish_state(!strcasecmp(text_value, "ON"));
}

void BinarySensor::parse_text_bitmask_(Register *hex_register, const char *text_value) {
  char *endptr;
  BITMASK_DEF::bitmask_t bitmask_value = strtoumax(text_value, &endptr, 0);
  if (*endptr == 0) {
    static_cast<BinarySensor *>(hex_register)->parse_bitmask_(bitmask_value);
  }
}

void BinarySensor::parse_text_enum_(Register *hex_register, const char *text_value) {
  char *endptr;
  ENUM_DEF::enum_t enum_value = strtoumax(text_value, &endptr, 0);
  if (*endptr == 0) {
    static_cast<BinarySensor *>(hex_register)->parse_enum_(enum_value);
  }
}
#endif  // defined(VEDIRECT_USE_TEXTFRAME)

}  // namespace m3_vedirect
}  // namespace esphome
