#pragma once
#include "esphome/components/uart/uart.h"
#include "esphome/core/automation.h"
#include "esphome/core/component.h"
#include "esphome/core/entity_base.h"
#include "esphome/core/helpers.h"

#ifdef USE_BINARY_SENSOR
#include "esphome/components/binary_sensor/binary_sensor.h"
#endif
#ifdef USE_TEXT_SENSOR
#include "esphome/components/text_sensor/text_sensor.h"
#endif

#include "defines.h"
#include "ve_reg_frame.h"
#include "register.h"
#include "containers.h"

#include <string_view>
#include <string>
#include <cstring>
#include <vector>

namespace esphome {
namespace m3_vedirect {

// Optimized container(s) for our HEX/TEXT registers collections.
struct hash_textlabel {
  constexpr size_t operator()(const char *s) const {
    // validated on the actual set of VEDirect TEXT labels.
    // this looks like a good trade-off between speed, size, and collisions
    unsigned char hash = *s;
    while (auto _s = *++s) {
      hash = (hash << 1) + _s;
    }
    return hash;
  }
};

struct hash_register_id {
  constexpr size_t operator()(register_id_t id) const {
    // This hash will be masked inside TinyMap to fit MAP_SIZE.
    // The idea for this hash is to just use the low bits
    // (which prove to be the most selective) of the register id
    // up to the size of the map.
    return id;
  }
};

class HexRegistersMap : public TinyMap<VEDIRECT_HEXMAP_SIZE, register_id_t, Register *, Register, hash_register_id> {
 public:
  using base_type = TinyMap<VEDIRECT_HEXMAP_SIZE, register_id_t, Register *, Register, hash_register_id>;
};

struct compare_textlabel {
  constexpr int operator()(const char *key_low, const char *key_high) const { return strcmp(key_high, key_low); }
};

class TextRegistersMap : public TinyMap<VEDIRECT_TEXTMAP_SIZE, const char *, Register *,
                                        SimpleBucket<const char *, Register *>, hash_textlabel, compare_textlabel> {
 public:
  using base_type = TinyMap<VEDIRECT_TEXTMAP_SIZE, const char *, Register *, SimpleBucket<const char *, Register *>,
                            hash_textlabel, compare_textlabel>;
};

#define MANAGER_ENTITY_(type, name) \
 protected: \
  type *name##_{}; /* NOLINT */ \
\
 public: \
  void set_##name(type *name) { /* NOLINT */ \
    this->name##_ = name; \
  }

class Manager : public uart::UARTDevice, public Component, protected FrameHandler {
 public:
  enum Error : uint8_t {
    NONE = FrameHandler::Error::NONE,
    CHECKSUM = FrameHandler::Error::CHECKSUM,
    CODING = FrameHandler::Error::CODING,
    OVERFLOW = FrameHandler::Error::OVERFLOW,
#if defined(VEDIRECT_USE_TEXTFRAME)
    NAME_OVERFLOW = FrameHandler::Error::NAME_OVERFLOW,
    VALUE_OVERFLOW = FrameHandler::Error::VALUE_OVERFLOW,
    RECORD_OVERFLOW = FrameHandler::Error::RECORD_OVERFLOW,
#endif
    TIMEOUT = FrameHandler::Error::_COUNT,
    UNEXPECTED,  // unexpected response to a request
    REMOTE,      // remote replied with an error or unknown frame
    FLAGS,       // remote replied with error flags to a register request
    QUEUE_FULL,  // request queue is full
    _COUNT,
  };

  class StaticIterator {
   public:
    explicit StaticIterator(const std::string &vedirect_id_key);
    bool has_next() const { return this->current_ != nullptr; }
    Manager *next();

   protected:
    Manager *current_;
    Manager *next_;
  };

  typedef std::function<void(const HexFrame *, uint8_t)> request_callback_t;

// dedicated entities to manage component state/behavior
#ifdef USE_BINARY_SENSOR
  MANAGER_ENTITY_(binary_sensor::BinarySensor, link_connected)
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
  Manager() : next_(Manager::list_) { Manager::list_ = this; }

  void setup() override;
  void loop() override;
  void dump_config() override;

  // CONFIGURATION BEGIN
  const char *get_vedirect_id() { return this->vedirect_id_; }
  void set_vedirect_id(const char *vedirect_id) { this->vedirect_id_ = vedirect_id; }
  const char *get_vedirect_name() { return this->vedirect_name_; }
  void set_vedirect_name(const char *vedirect_name) { this->vedirect_name_ = vedirect_name; }
  /// @brief Initialize and link the hex_register into the Manager dispatcher system
  /// @param hex_register : the register to be initialized/linked
  /// @param reg_def : the register descriptor definition
  void init_register(Register *reg, const REG_DEF *reg_def);
  /// @brief Configure this entity based off our registers grammar (REG_DEF::DEFS).
  /// This method is part of the public interface called by yaml generated code
  /// @param register_type the TYPE enum from our pre-defined registers set
  void init_register(Register *reg, REG_DEF::TYPE register_type);

#if defined(VEDIRECT_USE_HEXFRAME)
  void set_auto_create_hex_entities(bool value) { this->auto_create_hex_entities_ = value; }
  void set_ping_timeout(uint32_t seconds) { this->ping_timeout_ = seconds * 1000; }

  void add_on_frame_callback(std::function<void(const HexFrame &)> &&callback) {
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

#if ESPHOME_VERSION_CODE >= VERSION_CODE(2025, 11, 0)
    // See https://github.com/esphome/esphome/pull/11704
    void play(const Ts &...x) override{
#else
    void play(Ts... x) {
#endif
        for (auto it = Manager::StaticIterator(this->vedirect_id_.value(x...)); it.has_next();) {
          it.next()->send_hexframe(this->data_.value(x...));
  }
}
};
template<typename... Ts> class Action_send_command : public BaseAction<Ts...> {
 public:
  TEMPLATABLE_VALUE(uint8_t, command)
  TEMPLATABLE_VALUE(register_id_t, register_id)
  TEMPLATABLE_VALUE(uint32_t, data)
  TEMPLATABLE_VALUE(uint8_t, data_size)

#if ESPHOME_VERSION_CODE >= VERSION_CODE(2025, 11, 0)
  // See https://github.com/esphome/esphome/pull/11704
  void play(const Ts &...x) override{
#else
    void play(Ts... x) {
#endif

      for (auto it = Manager::StaticIterator(this->vedirect_id_.value(x...)); it.has_next();) {
        HEXFRAME::COMMAND command = (HEXFRAME::COMMAND) this->command_.value(x...);
  switch (command) {
    case HEXFRAME::COMMAND::Get:
      it.next()->request_get(this->register_id_.value(x...));
      break;
    case HEXFRAME::COMMAND::Set:
      switch (this->data_size_.value(x...)) {
        case 1:
          it.next()->request_set(this->register_id_.value(x...), (uint8_t) this->data_.value(x...));
          break;
        case 2:
          it.next()->request_set(this->register_id_.value(x...), (uint16_t) this->data_.value(x...));
          break;
        default:
          it.next()->request_set(this->register_id_.value(x...), (uint32_t) this->data_.value(x...));
          break;
      }
      break;
    default:
      it.next()->request_command(command);
  }
}
}
}
;
#endif

#if defined(VEDIRECT_USE_TEXTFRAME)
void set_auto_create_text_entities(bool value) { this->auto_create_text_entities_ = value; }
/// @brief Binds the entity to a TEXT FRAME field label so that text frame parsing
/// will be automatically routed. This method is part of the public interface
/// called by yaml generated code
/// @param label the name of the TEXT FRAME record to bind
void init_register(Register *reg, const char *label);
#endif  // defined(VEDIRECT_USE_TEXTFRAME)
// CONFIGURATION END

const char *get_logtag() const { return this->logtag_; }
bool is_connected() const { return this->connected_; }

/// @brief Initialize an entity (Register) with the correct naming/id scheme
/// when dynamically created by the Manager.
void init_entity(EntityBase *entity, const REG_DEF *reg_def, const char *name);

bool has_multi_manager() { return Manager::list_->next_ != nullptr; }

#if defined(VEDIRECT_USE_HEXFRAME)
// The VEDirect port looks like not buffering enough incoming requests so that they'll
// be lost if we send too many requests too quickly. To mitigate this, we'll
// serialize the requests and wait for a response (or timeout) before sending
// the next one.
// This is a simple FIFO queue with a fixed maximum size.
//
// In order to allow maximum flexibility, 2 api(s) are provided:
// - 'send_xxx' api is provided in order to allow sending frames without
// transaction control (this will disrupt any ongoing transactions though)
// - 'request_xxx' api is provided in order to send a request and wait for a response
// before sending the next request in the queue (VEDIRECT_REQUEST_QUEUE_SIZE define
// can be used to set the queue depth - default is 5).

// send_xxx api: sends an HEX frame without transaction control (send and forget)
// This is fragile since it 'collides' with transaction managed requests
// so it should be used with care.
void send_hexframe(const HexFrame &hexframe);
void send_hexframe(const char *rawframe, bool addchecksum = true);
void send_hexframe(const std::string &rawframe, bool addchecksum = true) {
  this->send_hexframe(rawframe.c_str(), addchecksum);
}

/// @brief Send an HEX command/request with transaction management
/// @param command the HEX command to send
/// @param register_id the register id to use (ignored if command is not GET/SET)
/// @param data the data to use (ignored if command is not SET)
/// @param data_type the data type to use (ignored if command is not SET)
bool request(HEXFRAME::COMMAND command, register_id_t register_id = REG_DEF::REGISTER_UNDEFINED,
             const void *data = nullptr, size_t data_size = 0, request_callback_t &&callback = nullptr);
bool request_command(HEXFRAME::COMMAND command, request_callback_t &&callback = nullptr) {
  return this->request(command, REG_DEF::REGISTER_UNDEFINED, nullptr, 0, std::move(callback));
}
bool request_get(register_id_t register_id, request_callback_t &&callback = nullptr) {
  return this->request(HEXFRAME::COMMAND::Get, register_id, nullptr, 0, std::move(callback));
}
template<typename T> bool request_set(register_id_t register_id, T data, request_callback_t &&callback = nullptr) {
  return this->request(HEXFRAME::COMMAND::Set, register_id, &data, sizeof(T), std::move(callback));
}
bool request_set(register_id_t register_id, const void *data, size_t data_size,
                 request_callback_t &&callback = nullptr) {
  return this->request(HEXFRAME::COMMAND::Set, register_id, data, data_size, std::move(callback));
}

bool is_request_pending() const { return this->requests_read_; }
bool is_request_queue_full() const { return this->requests_read_ == this->requests_write_; }

bool is_polling() const { return !this->polling_registers_it_.is_end(); }
#endif  //  defined(VEDIRECT_USE_HEXFRAME)

protected:
// Keeps a linked list of all Manager instances
static Manager *list_;
Manager *next_;
// component config
const char *logtag_;
const char *vedirect_id_{nullptr};
const char *vedirect_name_{nullptr};

HexRegistersMap hex_registers_;

// component state
bool connected_{false};
int last_rx_{0};
int last_frame_rx_{0};

inline void on_connected_();
inline void on_disconnected_();

#if defined(VEDIRECT_USE_HEXFRAME)
bool auto_create_hex_entities_{false};
int ping_timeout_{VEDIRECT_PING_TIMEOUT_MILLIS};

int last_ping_tx_{0};

// @todo: move to a conditional compilation so we only add this code when actually used
friend class HexFrameTrigger;
CallbackManager<void(const HexFrame &)> hexframe_callback_;

/// @brief Context class for a request for an HEX command/request with transaction management
/// so that the response (either succesfull or not) could be tracked and processed accordingly.
struct Request : public HexFrameT<7> {
  request_callback_t callback;
  int timeout;
} requests_[VEDIRECT_REQUEST_QUEUE_SIZE];
Request *requests_read_{nullptr};
Request *requests_write_{requests_};
Request *const requests_last_{&requests_[VEDIRECT_REQUEST_QUEUE_SIZE - 1]};
void request_trigger_(Request *request);
void request_response_(Request *request, const HexFrame *response, Error error);

/// @brief Polling context for HEX registers on connection
HexRegistersMap::iterator polling_registers_it_;
void poll_next_register_();

void on_frame_hex_(const RxHexFrame &hexframe) override;
void on_frame_hex_error_(FrameHandler::Error error) override;
#endif

#if defined(VEDIRECT_USE_TEXTFRAME)
bool auto_create_text_entities_{true};

TextRegistersMap text_registers_;

void on_frame_text_(TextRecord **text_records, uint8_t text_records_count) override;
void on_frame_text_error_(FrameHandler::Error error) override;
#endif
}
;

}  // namespace m3_vedirect
}  // namespace esphome
