#pragma once
#include "esphome/components/uart/uart.h"
#include "esphome/core/automation.h"
#include "esphome/core/component.h"
#include "esphome/core/entity_base.h"

#ifdef USE_BINARY_SENSOR
#include "esphome/components/binary_sensor/binary_sensor.h"
#endif
#ifdef USE_SENSOR
#include "esphome/components/sensor/sensor.h"
#endif
#ifdef USE_TEXT_SENSOR
#include "esphome/components/text_sensor/text_sensor.h"
#endif

#include "defines.h"
#include "ve_reg_frame.h"

#include <unordered_map>
#include <string_view>
#include <string>
#include <vector>

namespace esphome {
namespace m3_vedirect {

#define MANAGER_ENTITY_(type, name) \
 protected: \
  type *name##_{}; /* NOLINT */ \
\
 public: \
  void set_##name(type *name) { /* NOLINT */ \
    this->name##_ = name; \
  }

class Manager : public uart::UARTDevice, public Component, protected FrameHandler {
// dedicated entities to manage component state/behavior
#ifdef USE_BINARY_SENSOR
  MANAGER_ENTITY_(binary_sensor::BinarySensor, link_connected)
#endif
#ifdef USE_SENSOR
  MANAGER_ENTITY_(sensor::Sensor, run_time)
#endif
#ifdef USE_TEXT_SENSOR
#if defined(VEDIRECT_USE_HEXFRAME)
  MANAGER_ENTITY_(text_sensor::TextSensor, rawhexframe)
#endif
#if defined(VEDIRECT_USE_TEXTFRAME)
  MANAGER_ENTITY_(text_sensor::TextSensor, rawtextframe)
#endif
#endif

 public:
  static const std::vector<Manager *> get_managers(const std::string &vedirect_id);

  void setup() override;
  void loop() override;
  void dump_config() override;

  const char *get_vedirect_id() { return this->vedirect_id_; }
  void set_vedirect_id(const char *vedirect_id) { this->vedirect_id_ = vedirect_id; }
  const char *get_vedirect_name() { return this->vedirect_name_; }
  void set_vedirect_name(const char *vedirect_name) { this->vedirect_name_ = vedirect_name; }

  /// @brief Initialize and link the hex_register into the Manager dispatcher system
  /// @param hex_register : the register to be initialized/linked
  /// @param reg_def : the register descriptor definition
  void init_register(Register *hex_register, const REG_DEF *reg_def);
  /// @brief Configure this entity based off our registers grammar (REG_DEF::DEFS).
  /// This method is part of the public interface called by yaml generated code
  /// @param register_type the TYPE enum from our pre-defined registers set
  void init_register(Register *entity, REG_DEF::TYPE register_type);

#if defined(VEDIRECT_USE_HEXFRAME)
  void set_auto_create_hex_entities(bool value) { this->auto_create_hex_entities_ = value; }
  void set_ping_timeout(uint32_t seconds) { this->ping_timeout_ = seconds * 1000; }

  // send_xxx api: sends an HEX frame without transaction control (send and forget)

  void send_hexframe(const HexFrame &hexframe);
  void send_hexframe(const char *rawframe, bool addchecksum = true);
  void send_hexframe(const std::string &rawframe, bool addchecksum = true) {
    this->send_hexframe(rawframe.c_str(), addchecksum);
  }
  void send_command(HEXFRAME::COMMAND command) { this->send_hexframe(HexFrame_Command(command)); }
  void send_register_get(register_id_t register_id) { this->send_hexframe(HexFrame_Get(register_id)); }
  void send_register_set(register_id_t register_id, const void *data, HEXFRAME::DATA_TYPE data_type) {
    this->send_hexframe(HexFrame_Set(register_id, data, data_type));
  }
  template<typename T> void send_register_set(register_id_t register_id, T data) {
    this->send_hexframe(HexFrame_Set(register_id, data));
  }

  // request_xxx api: sends an HEX frame and manage the reply forwarding back the result
  // typedefs for HEX frames requests/callbacks
  typedef void *request_callback_param_t;
  typedef void (*request_callback_t)(request_callback_param_t callback_param, const RxHexFrame *hex_frame);

  void request(HEXFRAME::COMMAND command, register_id_t register_id, const void *data, HEXFRAME::DATA_TYPE data_type,
               request_callback_t callback, request_callback_param_t callback_param);

  void add_on_frame_callback(std::function<void(const HexFrame &)> callback) {
    this->hexframe_callback_.add(std::move(callback));
  }
  class HexFrameTrigger : public Trigger<const HexFrame &> {
   public:
    explicit HexFrameTrigger(Manager *vedirect) {
      vedirect->add_on_frame_callback([this](const HexFrame &hexframe) { this->trigger(hexframe); });
    }
  };

  template<typename... Ts> class BaseAction : public Action<Ts...> {
   public:
    TEMPLATABLE_VALUE(std::string, vedirect_id)
  };

  template<typename... Ts> class Action_send_hexframe : public BaseAction<Ts...> {
   public:
    TEMPLATABLE_VALUE(std::string, data)

    void play(Ts... x) {
      for (auto manager : Manager::get_managers(this->vedirect_id_.value(x...)))
        manager->send_hexframe(this->data_.value(x...));
    }
  };
  template<typename... Ts> class Action_send_command : public BaseAction<Ts...> {
   public:
    TEMPLATABLE_VALUE(uint8_t, command)
    TEMPLATABLE_VALUE(register_id_t, register_id)
    TEMPLATABLE_VALUE(uint32_t, data)
    TEMPLATABLE_VALUE(uint8_t, data_size)

    void play(Ts... x) {
      for (auto manager : Manager::get_managers(this->vedirect_id_.value(x...))) {
        HEXFRAME::COMMAND command = (HEXFRAME::COMMAND) this->command_.value(x...);
        switch (command) {
          case HEXFRAME::COMMAND::Get:
            manager->send_register_get(this->register_id_.value(x...));
            break;
          case HEXFRAME::COMMAND::Set:
            switch (this->data_size_.value(x...)) {
              case 1:
                manager->send_register_set(this->register_id_.value(x...), (uint8_t) this->data_.value(x...));
                break;
              case 2:
                manager->send_register_set(this->register_id_.value(x...), (uint16_t) this->data_.value(x...));
                break;
              default:
                manager->send_register_set(this->register_id_.value(x...), this->data_.value(x...));
                break;
            }
            break;
          default:
            manager->send_command(command);
        }
      }
    }
  };
#endif  //  defined(VEDIRECT_USE_HEXFRAME)

#if defined(VEDIRECT_USE_TEXTFRAME)
  void set_auto_create_text_entities(bool value) { this->auto_create_text_entities_ = value; }
  /// @brief Binds the entity to a TEXT FRAME field label so that text frame parsing
  /// will be automatically routed. This method is part of the public interface
  /// called by yaml generated code
  /// @param label the name of the TEXT FRAME record to bind
  void init_register(Register *entity, const char *label);
#endif  // defined(VEDIRECT_USE_TEXTFRAME)

 protected:
  static std::vector<Manager *> managers_;
  // component config
  const char *logtag_;
  const char *vedirect_id_{nullptr};
  const char *vedirect_name_{nullptr};

  // component state
  bool connected_{false};
  uint32_t millis_last_rx_{0};
  uint32_t millis_last_frame_rx_{0};

  inline void on_connected_();
  inline void on_disconnected_();

// override FrameHandler
#if defined(VEDIRECT_USE_HEXFRAME)
  bool auto_create_hex_entities_{false};
  uint32_t ping_timeout_{VEDIRECT_PING_TIMEOUT_MILLIS};

  uint32_t millis_last_ping_tx_{0};

  friend class HexFrameTrigger;
  CallbackManager<void(const HexFrame &)> hexframe_callback_;

  /// @brief Context class for a request for an HEX command/request with transaction management
  /// so that the response (either succesfull or not) could be tracked and processed accordingly.
  struct Request {
   public:
    std::string tag;
    HexFrameT<7> hex_frame{};
    uint32_t millis{0};
    request_callback_t callback{};
    request_callback_param_t callback_param{};
  };

  std::vector<Request *> requests_;
  uint32_t pending_requests_{0};
  /// @brief Cancel all pending requests without invoking callbacks (link disconnected)
  void requests_cancel_();
  void requests_match_get_or_set_(const RxHexFrame &hexframe);

  /// @brief Context class for polling HEX registers on connection
  struct PollingContext {
    std::vector<Register *> registers;
    std::vector<Register *>::const_iterator current;
    PollingContext(Manager *manager);
    static void advance_(void *callback_param, const RxHexFrame *);
  };
  friend class PollingContext;
  PollingContext *polling_context_{nullptr};

  void on_frame_hex_(const RxHexFrame &hexframe) override;
  void on_frame_hex_error_(Error error) override;
#endif

#if defined(VEDIRECT_USE_TEXTFRAME)
  bool auto_create_text_entities_{true};

  // Map entities/registers by TEXT frame record names so that we can forward
  // data while parsing.
  typedef std::unordered_map<const char *, Register *, cstring_hash, cstring_eq> text_registers_t;
  text_registers_t text_registers_;
  void emplace_text_register_(const char *label, Register *_register);

  void on_frame_text_(TextRecord **text_records, uint8_t text_records_count) override;
  void on_frame_text_error_(Error error) override;
#endif

  // These structures, should only be conditionally compiled with VEDIRECT_USE_HEXFRAME
  // but are also helping auto-creation of TEXT registers so we'll leave them.
  typedef std::unordered_map<register_id_t, Register *> hex_registers_t;
  hex_registers_t hex_registers_;

  Register *get_hex_register_(register_id_t register_id, bool create);
};

}  // namespace m3_vedirect
}  // namespace esphome
