#pragma once

#include "esphome/core/entity_base.h"

#include "defines.h"
#include "ve_reg_frame.h"

#include <vector>

namespace esphome {
namespace m3_vedirect {

class Register {
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
  // build function for a plain base 'Entity' object for every specialized type
  // so that the Manager will always instantiate an object (which will have no
  // behavior other than working as a stub in this case).
  typedef Register *(*build_entity_func_t)(Manager *manager, const char *name, const char *object_id);
  enum Platform {
    BinarySensor,
    Number,
    Select,
    Sensor,
    Switch,
    TextSensor,
    Platform_COUNT,
  };
  static void register_platform(Platform platform, build_entity_func_t build_entity_func) {
    BUILD_ENTITY_FUNC[platform] = build_entity_func;
  }

  // Called by Manager initializer (should just be called once but no harm if multiple invocations)
  // This will 'fix' missing platforms registration by filling in with the most appropriate
  // build_entity_func_t for the case.
  static void update_platforms();

  const REG_DEF *get_reg_def() const { return this->reg_def_; }
  register_id_t get_register_id() const {
    return this->reg_def_ ? this->reg_def_->register_id : REG_DEF::REGISTER_UNDEFINED;
  }

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
  static build_entity_func_t BUILD_ENTITY_FUNC[Platform_COUNT];
  static Register *build_entity(Manager *manager, const char *name, const char *object_id) { return new Register(); }
  // Called by the actual platform implementation to setup its EntityBase properties
  static void dynamic_init_entity_(EntityBase *entity, const char *name, const char *object_id,
                                   const char *manager_name, const char *manager_id);

  const REG_DEF *reg_def_;
#if defined(VEDIRECT_USE_HEXFRAME) && defined(VEDIRECT_USE_TEXTFRAME)
  Register(parse_hex_func_t parse_hex_func = parse_hex_empty_, parse_text_func_t parse_text_func = parse_text_empty_)
      : reg_def_(nullptr), parse_hex_(parse_hex_func), parse_text_(parse_text_func) {}
#elif defined(VEDIRECT_USE_HEXFRAME)
  Register(parse_hex_func_t parse_hex_func = parse_hex_empty_) : reg_def_(nullptr), parse_hex_(parse_hex_func) {}
#elif defined(VEDIRECT_USE_TEXTFRAME)
  Register(parse_text_func_t parse_text_func = parse_text_empty_) : reg_def_(nullptr), parse_text_(parse_text_func) {}
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

  // Called by the manager to setup a RegisterDispatcher in order to cascade 'parse_hex' calls
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
};

/// @brief Mixin style specialization for entities that can write configuration data to
/// the VEDirect interface (Number, Select, Switch)
class WritableRegister {
 public:
  Manager *const manager;
  WritableRegister(Manager *manager) : manager(manager) {}
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
/// when needed so that it'll be able to dispatch frame data to multiple registers.
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