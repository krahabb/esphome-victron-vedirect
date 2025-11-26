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

  // Hack the base SelectTraits to allow direct non-const access
  // to the options vector, so that we can update/mantain it as needed.
  // Since we maintain our 'options' list in ENUM_DEF, we need to
  // override the base class member to fill it with our data.
  // We can't however avoid a bit of data duplication since the array is maintained
  // both in ENUM_DEF and in SelectTraits::options. Since 2025.11.0 the FixedVector
  // implementation avoids storing std::string in its copy and this is better than nothing
  // so we're only duplicating an array of pointers. Still, preferrable, would be to have a
  // 'data provider' interface in SelectTraits to avoid duplicating data at all.
#if ESPHOME_VERSION_CODE >= VERSION_CODE(2025, 11, 0)
  // See https://github.com/esphome/esphome/pull/11772
  typedef FixedVector<const char *> options_type;
#else
  typedef std::vector<std::string> options_type;
#endif
  /// @brief Our custom SelectTraits implementation overrides accessor to allow dynamic
  /// options modifications
  class Traits : public esphome::select::SelectTraits {
   protected:
    friend class Select;
    inline options_type &options() { return this->options_; }
  };
  /// @brief A fake SelectTraits-like implementation that discards options configuration during component setup
  /// This will be used through member hiding to avoid initial setup of the base class 'traits' member
  class FakeTraits {
   public:
    void set_options(const std::initializer_list<const char *> &options) {}
    void set_options(const options_type &options) {}
  };
  FakeTraits traits;

 protected:
  friend class Manager;
  ENUM_DEF::enum_t enum_value_{ENUM_DEF::VALUE_UNKNOWN};

  inline Traits &traits_() { return reinterpret_cast<Traits &>(this->esphome::select::Select::traits); }

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

  // optimized publish_state bypassing index checks since we're mantaining our
  // own 'options' source of truth in enum_def_
  void publish_enum_(ENUM_DEF::enum_t enum_value);
  void publish_index_(size_t index);
  void publish_unknown_();
};

}  // namespace m3_vedirect
}  // namespace esphome
