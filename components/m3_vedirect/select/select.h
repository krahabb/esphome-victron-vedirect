#pragma once
#include "esphome/components/select/select.h"

#include "../register.h"

namespace esphome {
namespace m3_vedirect {

class Select final : public WritableRegister, public esphome::select::Select {
 public:
#if defined(VEDIRECT_USE_HEXFRAME) && defined(VEDIRECT_USE_TEXTFRAME)
  Select(Manager *manager) : WritableRegister(manager, parse_hex_default_, parse_text_default_) {}
#elif defined(VEDIRECT_USE_HEXFRAME)
  Select(Manager *manager) : WritableRegister(manager, parse_hex_default_) {}
#elif defined(VEDIRECT_USE_TEXTFRAME)
  Select(Manager *manager) : WritableRegister(manager, parse_text_default_) {}
#endif

  /// @brief Factory method to build a Select entity for a given Manager
  /// This is installed (see Register::register_platform) by yaml generated code
  ///  when setting up this platform.
  /// @param manager the Manager instance to which this entity will be linked
  /// @param name the name of the entity
  /// @return the newly created Select entity
  static Register *build_entity(Manager *manager, const REG_DEF *reg_def, const char *name);

 protected:
  friend class Manager;
  ENUM_DEF::enum_t enum_value_{ENUM_DEF::VALUE_UNKNOWN};

  void link_disconnected_() override;
  void init_reg_def_() override;
  inline void parse_enum_(ENUM_DEF::enum_t enum_value) override;
  inline void parse_string_(const char *string_value) override;

// interface esphome::select::Select
#if defined(VEDIRECT_USE_HEXFRAME)
  void control(size_t index) override;
  void control(const std::string &value) override;
  void control_enum_(const ENUM_DEF::LOOKUP_DEF *lookup_def);
#else
  void control(const std::string &value) override {}
#endif

#if defined(VEDIRECT_USE_HEXFRAME)
  static void parse_hex_default_(Register *hexregister, const RxHexFrame *hexframe);
  static void parse_hex_enum_(Register *hexregister, const RxHexFrame *hexframe);
#endif
#if defined(VEDIRECT_USE_TEXTFRAME)
  static void parse_text_default_(Register *hex_register, const char *text_value);
  static void parse_text_enum_(Register *hex_register, const char *text_value);
#endif

#if ESPHOME_VERSION_CODE >= VERSION_CODE(2025, 11, 0)
  // See https://github.com/esphome/esphome/pull/11772
  typedef FixedVector<const char *> options_type;
#else
  typedef std::vector<std::string> options_type;
#endif
  // Hack the basic SelectTraits to allow dynamic management
  // of options from our Select entity without always copying/moving
  class SelectTraits : public esphome::select::SelectTraits {
   public:
    inline options_type &options() { return this->options_; }
  };
  inline SelectTraits &traits_() { return reinterpret_cast<SelectTraits &>(this->traits); }

  // optimized publish_state bypassing index checks since we're mantaining our
  // own 'options' source of truth in enum_def_
  void publish_enum_(ENUM_DEF::enum_t enum_value);
  void publish_index_(size_t index);
  void publish_unknown_();
};

}  // namespace m3_vedirect
}  // namespace esphome
