#pragma once
#include "esphome/components/text_sensor/text_sensor.h"

#include "../register.h"

namespace esphome {
namespace m3_vedirect {

class TextSensor final : public Register, public esphome::text_sensor::TextSensor {
 public:
#if defined(VEDIRECT_USE_HEXFRAME) && defined(VEDIRECT_USE_TEXTFRAME)
  TextSensor(Manager *Manager) : Register(parse_hex_default_, parse_text_default_) {}
#elif defined(VEDIRECT_USE_HEXFRAME)
  TextSensor(Manager *Manager) : Register(parse_hex_default_) {}
#elif defined(VEDIRECT_USE_TEXTFRAME)
  TextSensor(Manager *Manager) : Register(parse_text_default_) {}
#endif

  static Register *build_entity(Manager *manager, const char *name, const char *object_id);

 protected:
  friend class Manager;
  BITMASK_DEF::bitmask_t raw_value_{BITMASK_DEF::VALUE_UNKNOWN};

  void link_disconnected_() override;
  void init_reg_def_() override;
  inline void parse_bitmask_(BITMASK_DEF::bitmask_t bitmask_value) override;
  inline void parse_enum_(ENUM_DEF::enum_t enum_value) override;
  inline void parse_string_(const char *string_value) override;

#if defined(VEDIRECT_USE_HEXFRAME)
  static void parse_hex_default_(Register *hex_register, const RxHexFrame *hex_frame);
  static void parse_hex_bitmask_(Register *hex_register, const RxHexFrame *hex_frame);
  static void parse_hex_enum_(Register *hex_register, const RxHexFrame *hex_frame);
  static void parse_hex_string_(Register *hex_register, const RxHexFrame *hex_frame);
  static void parse_hex_app_ver_(Register *hex_register, const RxHexFrame *hex_frame);
#endif
#if defined(VEDIRECT_USE_TEXTFRAME)
  static void parse_text_default_(Register *hex_register, const char *text_value);
  static void parse_text_bitmask_(Register *hex_register, const char *text_value);
  static void parse_text_enum_(Register *hex_register, const char *text_value);
  static void parse_text_app_ver_(Register *hex_register, const char *text_value);
#endif
};

}  // namespace m3_vedirect
}  // namespace esphome
