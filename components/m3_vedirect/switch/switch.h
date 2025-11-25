#pragma once
#include "esphome/components/switch/switch.h"

#include "../register.h"

namespace esphome {
namespace m3_vedirect {

class Switch final : public WritableRegister, public esphome::switch_::Switch {
 public:
#if defined(VEDIRECT_USE_HEXFRAME) && defined(VEDIRECT_USE_TEXTFRAME)
  Switch(Manager *manager)
      : WritableRegister(manager, parse_hex_default_, parse_text_default_), esphome::switch_::Switch() {
    this->restore_mode = esphome::switch_::SwitchRestoreMode::SWITCH_RESTORE_DISABLED;
  }
#elif defined(VEDIRECT_USE_HEXFRAME)
  Switch(Manager *manager) : WritableRegister(manager, parse_hex_default_), esphome::switch_::Switch() {
    this->restore_mode = esphome::switch_::SwitchRestoreMode::SWITCH_RESTORE_DISABLED;
  }
#elif defined(VEDIRECT_USE_TEXTFRAME)
  Switch(Manager *manager) : WritableRegister(manager, parse_text_default_), esphome::switch_::Switch() {
    this->restore_mode = esphome::switch_::SwitchRestoreMode::SWITCH_RESTORE_DISABLED;
  }
#endif

  /// @brief Factory method to build a TextSensor entity for a given Manager
  /// This is installed (see Register::register_platform) by yaml generated code
  ///  when setting up this platform.
  /// @param manager the Manager instance to which this entity will be linked
  /// @param name the name of the entity
  /// @param object_id the object_id of the entity
  /// @return the newly created TextSensor->Register entity
  static Register *build_entity(Manager *manager, const REG_DEF *reg_def, const char *name);

  void set_mask(uint32_t mask) { this->mask_ = mask; }

  // interface esphome::switch_::Switch
  void set_restore_mode(esphome::switch_::SwitchRestoreMode restore_mode) {}

 protected:
  friend class Manager;

  BITMASK_DEF::bitmask_t raw_value_{BITMASK_DEF::VALUE_UNKNOWN};
  BITMASK_DEF::bitmask_t mask_{0x01};

  void link_disconnected_() override;
  void init_reg_def_() override;
  inline void parse_bitmask_(BITMASK_DEF::bitmask_t bitmask_value) override;
  inline void parse_enum_(ENUM_DEF::enum_t enum_value) override;

  // interface esphome::switch_::Switch
#if defined(VEDIRECT_USE_HEXFRAME)
  void write_state(bool state) override;
#else
  void write_state(bool state) override{};
#endif

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
