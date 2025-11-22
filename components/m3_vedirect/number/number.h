#pragma once
#include "esphome/components/number/number.h"

#include "../register.h"

namespace esphome {
namespace m3_vedirect {

class Number final : public WritableRegister, public NumericRegister, public esphome::number::Number {
 public:
#if defined(VEDIRECT_USE_HEXFRAME) && defined(VEDIRECT_USE_TEXTFRAME)
  Number(Manager *manager) : WritableRegister(manager, parse_hex_default_, parse_text_empty_) { this->state = NAN; }
#elif defined(VEDIRECT_USE_HEXFRAME)
  Number(Manager *manager) : WritableRegister(manager, parse_hex_default_) { this->state = NAN; }
#elif defined(VEDIRECT_USE_TEXTFRAME)
  Number(Manager *manager) : WritableRegister(manager, parse_text_empty_) { this->state = NAN; }
#endif

  /// @brief Factory method to build a TextSensor entity for a given Manager
  /// This is installed (see Register::register_platform) by yaml generated code
  ///  when setting up this platform.
  /// @param manager the Manager instance to which this entity will be linked
  /// @param name the name of the entity
  /// @param object_id the object_id of the entity
  /// @return the newly created TextSensor->Register entity
  static Register *build_entity(Manager *manager, const REG_DEF *reg_def, const char *name);

 protected:
  friend class Manager;
  void link_disconnected_() override;
  void init_reg_def_() override;

  // interface esphome::number::Number
#if defined(VEDIRECT_USE_HEXFRAME)
  void control(float value) override;
#else
  void control(float value) override {}
#endif

#if defined(VEDIRECT_USE_HEXFRAME)
  static void parse_hex_default_(Register *hex_register, const RxHexFrame *hex_frame);
  static void parse_hex_kelvin_(Register *hex_register, const RxHexFrame *hex_frame);
  template<typename T> static void parse_hex_t_(Register *hex_register, const RxHexFrame *hex_frame);
  static const parse_hex_func_t DATA_TYPE_TO_PARSE_HEX_FUNC_[];
#endif
};

}  // namespace m3_vedirect
}  // namespace esphome
