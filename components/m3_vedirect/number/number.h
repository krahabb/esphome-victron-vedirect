#pragma once
#include "esphome/components/number/number.h"

#include "../register.h"

namespace esphome {
namespace m3_vedirect {

class Number final : public WritableRegister, public NumericRegister, public Register, public esphome::number::Number {
 public:
#if defined(VEDIRECT_USE_HEXFRAME) && defined(VEDIRECT_USE_TEXTFRAME)
  Number(Manager *manager) : WritableRegister(manager), Register(parse_hex_default_, parse_text_empty_) {}
#elif defined(VEDIRECT_USE_HEXFRAME)
  Number(Manager *manager) : WritableRegister(manager), Register(parse_hex_default_) {}
#elif defined(VEDIRECT_USE_TEXTFRAME)
  Number(Manager *manager) : WritableRegister(manager), Register(parse_text_empty_) {}
#endif

  static Register *build_entity(Manager *manager, const char *name, const char *object_id);

 protected:
  friend class Manager;
  void link_disconnected_() override;
  void init_reg_def_() override;

// interface esphome::number::Number
#if defined(VEDIRECT_USE_HEXFRAME)
  void control(float value) override;
  static void request_callback_(void *callback_param, const RxHexFrame *hex_frame);
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
