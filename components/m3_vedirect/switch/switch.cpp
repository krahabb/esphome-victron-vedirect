#include "switch.h"
#include "esphome/core/application.h"
#ifdef USE_API
#include "esphome/components/api/api_server.h"
#endif

#include "../manager.h"

#include <cinttypes>

namespace esphome {
namespace m3_vedirect {

#ifdef ESPHOME_LOG_HAS_DEBUG
static const char *const TAG = "m3_vedirect.switch";
#endif

Register *Switch::build_entity(Manager *manager, const REG_DEF *reg_def, const char *name) {
#if ESPHOME_VERSION_CODE >= VERSION_CODE(2025, 8, 0)
  if (App.get_switches().size() >= ESPHOME_ENTITY_SWITCH_COUNT) {
    return Register::drop_platform(manager, Platform::Switch);
  }
#endif
  auto entity = new Switch(manager);
  manager->init_entity(entity, reg_def, name);
  App.register_switch(entity);
#ifdef USE_API
  entity->add_on_state_callback([entity](bool state) { api::global_api_server->on_switch_update(entity, state); });
#endif
  return entity;
}

void Switch::link_disconnected_() { this->set_has_state(false); }

void Switch::init_reg_def_() {
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

void Switch::parse_bitmask_(BITMASK_DEF::bitmask_t bitmask_value) {
  if (this->raw_value_ != bitmask_value) {
    this->raw_value_ = bitmask_value;
    this->publish_state_(bitmask_value & this->mask_);
  }
}

void Switch::parse_enum_(ENUM_DEF::enum_t enum_value) {
  if (this->raw_value_ != enum_value) {
    this->raw_value_ = enum_value;
    this->publish_state_(enum_value == this->mask_);
  }
}

#if defined(VEDIRECT_USE_HEXFRAME)
void Switch::write_state(bool state) {
  // This code should work for both ENUM-like and BITMASK-like registers
  // For the latter, actual bits are preserved so that we can toggle individual bits
  // inside the register. The 'mask' too might be used to control multiple bits at once.
  uint32_t hexvalue;
  switch (this->reg_def_->cls) {
    case REG_DEF::CLASS::BITMASK:
      hexvalue = state ? this->raw_value_ | this->mask_ : this->raw_value_ & ~this->mask_;
      break;
    case REG_DEF::CLASS::ENUM:
      // what's a reasonable negation of mask_ ?
      hexvalue = state ? this->mask_ : 0;
      break;
    default:
      // consider BOOLEAN
      hexvalue = state ? 1 : 0;
      break;
  }
  this->request_set_(hexvalue, [this](const HexFrame *frame, uint8_t error) {
    if (error) {
      // Error or timeout..resend actual state since it looks like HA esphome does optimistic
      // updates in it's HA entity instance...
      this->republish_state_();
    } else {
      // Invalidate our state so that the subsequent dispatching/parsing goes through
      // an effective publish_state. This is needed (again) since the frontend already
      // optimistically updated the entity to the new value but even in case of success,
      // the device might 'force' a different setting if the request was for an unsupported
      // value
      this->raw_value_ = BITMASK_DEF::VALUE_UNKNOWN;
    }
  });
}

void Switch::parse_hex_default_(Register *hex_register, const RxHexFrame *hex_frame) {
  static_assert(RxHexFrame::ALLOCATED_DATA_SIZE >= 1, "HexFrame storage might lead to access overflow");
  static_cast<Switch *>(hex_register)->publish_state_(hex_frame->data_t<uint8_t>());
}

void Switch::parse_hex_bitmask_(Register *hex_register, const RxHexFrame *hex_frame) {
  // BITMASK registers have storage up to 4 bytes
  static_assert(RxHexFrame::ALLOCATED_DATA_SIZE >= 4, "HexFrame storage might lead to access overflow");
  static_cast<Switch *>(hex_register)->parse_bitmask_(hex_frame->safe_data_u32());
}

void Switch::parse_hex_enum_(Register *hex_register, const RxHexFrame *hex_frame) {
  static_assert(RxHexFrame::ALLOCATED_DATA_SIZE >= 1, "HexFrame storage might lead to access overflow");
  static_cast<Switch *>(hex_register)->parse_enum_(hex_frame->data_t<ENUM_DEF::enum_t>());
}
#endif  // defined(VEDIRECT_USE_HEXFRAME)

#if defined(VEDIRECT_USE_TEXTFRAME)
void Switch::parse_text_default_(Register *hex_register, const char *text_value) {
  static_cast<Switch *>(hex_register)->publish_state_(!strcasecmp(text_value, "ON"));
}

void Switch::parse_text_bitmask_(Register *hex_register, const char *text_value) {
  char *endptr;
  BITMASK_DEF::bitmask_t bitmask_value = strtoumax(text_value, &endptr, 0);
  if (*endptr == 0) {
    static_cast<Switch *>(hex_register)->parse_bitmask_(bitmask_value);
  }
}

void Switch::parse_text_enum_(Register *hex_register, const char *text_value) {
  char *endptr;
  ENUM_DEF::enum_t enum_value = strtoumax(text_value, &endptr, 0);
  if (*endptr == 0) {
    static_cast<Switch *>(hex_register)->parse_enum_(enum_value);
  }
}
#endif  // defined(VEDIRECT_USE_TEXTFRAME)

void Switch::publish_state_(bool state) {
  state = state != this->inverted_;
  if (this->state != state) {
    this->state = state;
    ESP_LOGD(TAG, "'%s': Sending state %s", this->name_.c_str(), ONOFF(state));
    this->state_callback_.call(state);
  }
}

void Switch::republish_state_() {
  ESP_LOGD(TAG, "'%s': Sending state %s", this->name_.c_str(), ONOFF(this->state));
  this->state_callback_.call(this->state);
}

}  // namespace m3_vedirect
}  // namespace esphome
