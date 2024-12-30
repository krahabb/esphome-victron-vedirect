#pragma once
#include "esphome/components/sensor/sensor.h"

#include "../register.h"

namespace esphome {
namespace m3_vedirect {

class Sensor final : public NumericRegister, public Register, public esphome::sensor::Sensor {
 public:
  // configuration symbols for numeric sensors
  static const sensor::StateClass UNIT_TO_STATE_CLASS[REG_DEF::UNIT::UNIT_COUNT];
  static const uint8_t SCALE_TO_DIGITS[REG_DEF::SCALE::SCALE_COUNT];

#if defined(VEDIRECT_USE_HEXFRAME) && defined(VEDIRECT_USE_TEXTFRAME)
  Sensor(Manager *Manager) : Register(parse_hex_default_, parse_text_default_) {}
#elif defined(VEDIRECT_USE_HEXFRAME)
  Sensor(Manager *Manager) : Register(parse_hex_default_) {}
#elif defined(VEDIRECT_USE_TEXTFRAME)
  Sensor(Manager *Manager) : Register(parse_text_default_) {}
#endif

  static Register *build_entity(Manager *manager, const char *name, const char *object_id);

 protected:
  friend class Manager;

  float text_scale_{1.};

  void link_disconnected_() override;

  void init_reg_def_() override;
#if defined(VEDIRECT_USE_HEXFRAME)
  static void parse_hex_default_(Register *hex_register, const RxHexFrame *hex_frame);
  static void parse_hex_kelvin_(Register *hex_register, const RxHexFrame *hex_frame);
  template<typename T> static void parse_hex_t_(Register *hex_register, const RxHexFrame *hex_frame);
  static const parse_hex_func_t DATA_TYPE_TO_PARSE_HEX_FUNC_[];
#endif
#if defined(VEDIRECT_USE_TEXTFRAME)
  static void parse_text_default_(Register *hex_register, const char *text_value);
#endif
};

}  // namespace m3_vedirect
}  // namespace esphome
