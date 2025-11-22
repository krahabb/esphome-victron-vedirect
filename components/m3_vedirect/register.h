#pragma once

#include "defines.h"
#include "ve_reg_frame.h"
#include "containers.h"

#include <functional>
#include <vector>

namespace esphome {
namespace m3_vedirect {

/// @brief Base class for all VEDirect registers/entities.
class Register : public ValueBucket<register_id_t, Register> {
 public:
  friend class Manager;
  friend class RegisterDispatcher;

  // Setup a dynamic registration system for the purpose of dynamically
  // creating entities in the Manager should it be configured so.
  // In order to build a specific entity type, it's compilation unit must be
  // added to the build and this is done in yaml by explicitly setting
  // the corresponding platform entry. This allows to completely disable
  // some (or all) of the platforms and save some code size by
  // not including the unneeded compilation units. When a platform is added
  // through yaml generation, the generator will also add a registration
  // for the corresponding entity 'build_entity' function so that the Manager
  // will be able to instantiate the correct entity. By default, we setup a
  // build function for a plain base 'Register' object for every specialized type
  // so that the Manager will always instantiate an object (which will have no
  // behavior other than working as a stub in this case).
  typedef Register *(*build_entity_func_t)(Manager *manager, const REG_DEF *reg_def, const char *name);
  enum Platform {
    BinarySensor,
    Number,
    Select,
    Sensor,
    Switch,
    TextSensor,
    Platform_COUNT,
  };

 private:
  /// @brief Maps a platform to a factory function for building an entity of that platform.
  /// If a platform is not registered, we will try to
  /// substitute it with the most appropriate available platform
  static build_entity_func_t BUILD_ENTITY_FUNC[Platform_COUNT];
  /// @brief Maps a 'register definition' (through REG_DEF::CLASS and REG_DEF::ACCESS)
  /// to the most appropriate Platform factory function.
  static const Platform REGDEF_FACTORY_MAP[REG_DEF::CLASS::CLASS_COUNT][REG_DEF::ACCESS::ACCESS_COUNT];

  /// @brief Builds an entity based off the provided register definition
  /// using the most appropriate platform available.
  /// This is used by the Manager when dynamically creating entities
  /// based off the register definitions (REG_DEF::DEFS).
  /// @param reg_def: the register definition to be used for building the entity
  /// @return a new entity of the most appropriate platform or a plain Register.
  static Register *auto_create(Manager *manager, const REG_DEF *reg_def);

 public:
  /// @brief Default build function for a plain Register object.
  /// This (or a more specific platform version) is used to build a specific entity/register
  /// when the corresponding platform is requested when dynamically creating entities (see
  /// Manager::get_register).
  static Register *build_entity(Manager *manager, const REG_DEF *reg_def, const char *name) { return new Register(); }

  const REG_DEF *get_reg_def() const { return this->reg_def_; }

#if defined(VEDIRECT_USE_HEXFRAME)
  typedef FrameHandler::RxHexFrame RxHexFrame;
  typedef void (*parse_hex_func_t)(Register *hex_register, const RxHexFrame *hexframe);
  inline void parse_hex(const RxHexFrame *hexframe) { this->parse_hex_(this, hexframe); }
#endif
#if defined(VEDIRECT_USE_TEXTFRAME)
  typedef void (*parse_text_func_t)(Register *hex_register, const char *text_value);
  inline void parse_text(const char *text_value) { this->parse_text_(this, text_value); }
#endif

 protected:
  const REG_DEF *reg_def_{nullptr};

#if defined(VEDIRECT_USE_HEXFRAME) && defined(VEDIRECT_USE_TEXTFRAME)
  Register(parse_hex_func_t parse_hex_func = parse_hex_empty_, parse_text_func_t parse_text_func = parse_text_empty_)
      : parse_hex_(parse_hex_func), parse_text_(parse_text_func) {}
#elif defined(VEDIRECT_USE_HEXFRAME)
  Register(parse_hex_func_t parse_hex_func = parse_hex_empty_) : parse_hex_(parse_hex_func) {}
#elif defined(VEDIRECT_USE_TEXTFRAME)
  Register(parse_text_func_t parse_text_func = parse_text_empty_) : parse_text_(parse_text_func) {}
#endif

  // called by the Manager when VEDirect timeouts (we'll send 'unknown' to APIServer)
  virtual void link_disconnected_(){};
  /// @brief Preset entity properties based off our REG_DEF. This is being called
  /// automatically by components methods when a proper definition is available.
  /// @param reg_def: the proper register definition if available
  virtual void init_reg_def_(){};

  // These work as the effective datatype parser and are invoked by both
  // the parse_hex_... and the parse_text_... handlers
  virtual void parse_bitmask_(BITMASK_DEF::bitmask_t bitmask_value){};
  virtual void parse_enum_(ENUM_DEF::enum_t enum_value){};
  virtual void parse_string_(const char *string_value){};

  // Called by the manager to setup a RegisterDispatcher in order to cascade 'parse_hex/parse_text' calls
  // when this Register is being added to the registered registers. The base implementation will
  // setup a new RegisterDispatcher cascading this and the provided 'hex_register' while the
  // RegisterDispatcher will just add it to it's existing list
  virtual Register *cascade_dispatcher_(Register *hex_register);

#if defined(VEDIRECT_USE_HEXFRAME)
  parse_hex_func_t parse_hex_;
  static void parse_hex_empty_(Register *hex_register, const RxHexFrame *hexframe) {}
#endif

#if defined(VEDIRECT_USE_TEXTFRAME)
  parse_text_func_t parse_text_;
  static void parse_text_empty_(Register *hex_register, const char *text_value) {}
#endif

  /// @brief Drops the platform 'build_entity' for the specified platform
  /// and returns a plain Register object instead. This is used when a platform 'build_entity'
  /// function detects we've run out of available space for entities registration.
  static Register *drop_platform(Manager *manager, Platform platform);
};

/// @brief Mixin style specialization for entities that can write configuration data to
/// the VEDirect interface (Number, Select, Switch)
class WritableRegister : public Register {
 public:
  Manager *const manager;

 protected:
#if defined(VEDIRECT_USE_HEXFRAME) && defined(VEDIRECT_USE_TEXTFRAME)
  WritableRegister(Manager *manager, parse_hex_func_t parse_hex_func = parse_hex_empty_,
                   parse_text_func_t parse_text_func = parse_text_empty_)
      : Register(parse_hex_func, parse_text_func), manager(manager) {}
#elif defined(VEDIRECT_USE_HEXFRAME)
  WritableRegister(Manager *manager, parse_hex_func_t parse_hex_func = parse_hex_empty_)
      : Register(parse_hex_func), manager(manager) {}
#elif defined(VEDIRECT_USE_TEXTFRAME)
  // We don't really need WritableRegister for TEXTFRAME only entities but let's keep the symmetry
  WritableRegister(Manager *manager, parse_text_func_t parse_text_func = parse_text_empty_)
      : Register(parse_text_func), manager(manager) {}
#endif

#if defined(VEDIRECT_USE_HEXFRAME)
  void request_set_(uint32_t value, std::function<void(const HexFrame *, uint8_t)> &&callback);
#endif
};

/// @brief Mixin style specialization for Number and Sensor entities.
class NumericRegister {
 public:
  static const char *UNIT_TO_DEVICE_CLASS[REG_DEF::UNIT::UNIT_COUNT];

 protected:
  float hex_scale_{1.};
};

/// @brief This class provides on-demand frame dispatching to multiple registers with the same
/// HEX address and/or TEXT label. This is installed in the Manager.hex_registers_ collection
/// when needed so that it'll be able to dispatch frame data to multiple registers with the same
/// HEX address and/or TEXT label.
/// After implementing TinyMap container this code might not be needed anymore.
class RegisterDispatcher final : public Register {
 public:
  friend class Register;
#if defined(VEDIRECT_USE_HEXFRAME) && defined(VEDIRECT_USE_TEXTFRAME)
  RegisterDispatcher(Register *_register) : Register(parse_hex_default_, parse_text_default_) {
    // We'll just use the first register's reg_def_ as our own.
    // Be careful since these could be quite different REG_DEFs except their register_id
    this->reg_def_ = _register->reg_def_;
    this->registers_.push_back(_register);
  }
#elif defined(VEDIRECT_USE_HEXFRAME)
  RegisterDispatcher(Register *_register) : Register(parse_hex_default_) {
    this->reg_def_ = _register->reg_def_;
    this->registers_.push_back(_register);
  }
#elif defined(VEDIRECT_USE_TEXTFRAME)
  RegisterDispatcher(Register *_register) : Register(parse_text_default_) {
    this->reg_def_ = _register->reg_def_;
    this->registers_.push_back(_register);
  }
#endif
 protected:
  std::vector<Register *> registers_;

  void link_disconnected_() override {
    for (auto _register : this->registers_) {
      _register->link_disconnected_();
    }
  }

  Register *cascade_dispatcher_(Register *_register) override {
    this->registers_.push_back(_register);
    return this;
  }

#if defined(VEDIRECT_USE_HEXFRAME)
  static void parse_hex_default_(Register *hex_register, const RxHexFrame *hex_frame) {
    for (auto _register : static_cast<RegisterDispatcher *>(hex_register)->registers_) {
      _register->parse_hex(hex_frame);
    }
  }
#endif
#if defined(VEDIRECT_USE_TEXTFRAME)
  static void parse_text_default_(Register *hex_register, const char *text_value) {
    for (auto _register : static_cast<RegisterDispatcher *>(hex_register)->registers_) {
      _register->parse_text(text_value);
    }
  }
#endif
};

}  // namespace m3_vedirect
}  // namespace esphome
