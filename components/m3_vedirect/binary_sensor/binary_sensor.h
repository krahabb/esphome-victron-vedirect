#pragma once
#include "esphome/components/binary_sensor/binary_sensor.h"

#include "../register.h"

namespace esphome {
namespace m3_vedirect {

class BinarySensor final : public Register, public esphome::binary_sensor::BinarySensor {
 public:
#if defined(VEDIRECT_USE_HEXFRAME) && defined(VEDIRECT_USE_TEXTFRAME)
  BinarySensor(Manager *Manager) : Register(parse_hex_default_, parse_text_default_) {}
#elif defined(VEDIRECT_USE_HEXFRAME)
  BinarySensor(Manager *Manager) : Register(parse_hex_default_) {}
#elif defined(VEDIRECT_USE_TEXTFRAME)
  BinarySensor(Manager *Manager) : Register(parse_text_default_) {}
#endif

  static Register *build_entity(Manager *manager, const char *name, const char *object_id);

  void set_mask(uint32_t mask) { this->mask_ = mask; }

 protected:
  friend class Manager;

  uint32_t mask_{0xFFFFFFFF};

  void init_reg_def_() override;
  inline void parse_bitmask_(BITMASK_DEF::bitmask_t bitmask_value) override {
    this->publish_state(bitmask_value & this->mask_);
  }
  inline void parse_enum_(ENUM_DEF::enum_t enum_value) override { this->publish_state(enum_value == this->mask_); }

#if defined(VEDIRECT_USE_HEXFRAME)
  static void parse_hex_default_(Register *hex_register, const RxHexFrame *hex_frame);
  static void parse_hex_bitmask_(Register *hex_register, const RxHexFrame *hex_frame);
  static void parse_hex_enum_(Register *hex_register, const RxHexFrame *hex_frame);
#endif
#if defined(VEDIRECT_USE_TEXTFRAME)
  static void parse_text_default_(Register *hex_register, const char *text_value);
  static void parse_text_bitmask_(Register *hex_register, const char *text_value);
  static void parse_text_enum_(Register *hex_register, const char *text_value);
#endif
};

}  // namespace m3_vedirect
}  // namespace esphome
