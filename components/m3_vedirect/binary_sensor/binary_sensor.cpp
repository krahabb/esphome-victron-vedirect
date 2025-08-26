#include "binary_sensor.h"
#include "esphome/core/application.h"
#include "esphome/core/version.h"
#ifdef USE_API
#include "esphome/components/api/api_server.h"
#endif

#include "../manager.h"

#include <cinttypes>

namespace esphome {
namespace m3_vedirect {

Register *BinarySensor::build_entity(Manager *manager, const char *name, const char *object_id) {
  auto entity = new BinarySensor(manager);
  Register::dynamic_init_entity_(entity, name, object_id, manager->get_vedirect_name(), manager->get_vedirect_id());
  App.register_binary_sensor(entity);
#ifdef USE_API
  if (api::global_api_server) {
#if ESPHOME_VERSION_CODE >= VERSION_CODE(2025, 7, 0)
    entity->add_on_state_callback([entity](bool state) { api::global_api_server->on_binary_sensor_update(entity); });
#else
    entity->add_on_state_callback(
        [entity](bool state) { api::global_api_server->on_binary_sensor_update(entity, state); });
#endif
  }
#endif
  return entity;
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
