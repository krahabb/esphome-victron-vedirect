#include "manager.h"
#include "register.h"

#include "esphome/core/log.h"

namespace esphome {
namespace m3_vedirect {

static const char TAG[] = "m3_vedirect.%s";

std::vector<Manager *> Manager::managers_;

const std::vector<Manager *> Manager::get_managers(const std::string &vedirect_id) {
  if (vedirect_id.empty()) {
    return {managers_.front()};
  } else if (vedirect_id == "*") {
    return managers_;
  } else {
    for (auto manager : managers_) {
      if (manager->vedirect_id_ == vedirect_id) {
        return {manager};
      }
    }
  }
  return {};
}

void Manager::setup() {
  Register::update_platforms();
  char *buf = new char[sizeof(TAG) + strlen(this->vedirect_id_)];
  sprintf(buf, TAG, this->vedirect_id_);
  this->logtag_ = buf;
  Manager::managers_.push_back(this);
}

void Manager::loop() {
  const uint32_t millis_ = millis();
#ifdef USE_SENSOR
  if (this->run_time_) {
    float run_time = millis_ / 1000;
    if (run_time != this->run_time_->raw_state)
      this->run_time_->publish_state(run_time);
  }
#endif
  auto available = this->available();
  if (available) {
    uint8_t frame_buf[256];
    if (available > sizeof(frame_buf))
      available = sizeof(frame_buf);
    if (this->read_array(frame_buf, available)) {
      this->millis_last_rx_ = millis_;
      this->decode(frame_buf, frame_buf + available);
    }
#if defined(VEDIRECT_USE_HEXFRAME)
    if (this->ping_timeout_ && ((millis_ - this->millis_last_ping_tx_) > this->ping_timeout_)) {
      this->send_hexframe(HexFrame_Command(HEXFRAME::COMMAND::Ping));
      this->millis_last_ping_tx_ = this->millis_last_hexframe_tx_;
    }
#endif
  } else {
    if (this->connected_ && ((millis_ - this->millis_last_rx_) > VEDIRECT_LINK_TIMEOUT_MILLIS)) {
      this->on_disconnected_();
    }
  }
}

void Manager::dump_config() { ESP_LOGCONFIG(this->logtag_, "VEDirect:"); }

void Manager::init_register(Register *hex_register, const REG_DEF *reg_def) {
  hex_register->reg_def_ = reg_def;
  hex_register->init_reg_def_();
  if (reg_def->register_id != REG_DEF::REGISTER_UNDEFINED) {
    auto result = this->hex_registers_.emplace(reg_def->register_id, hex_register);
    if (!result.second) {
      // register_id already present in our set so we must setup/update an RegisterDispatcher
      auto &existing_pair = *result.first;
      existing_pair.second = existing_pair.second->cascade_dispatcher_(hex_register);
    }
  }
}

void Manager::init_register(Register *entity, REG_DEF::TYPE register_type) {
  this->init_register(entity, &REG_DEF::DEFS[register_type]);
#if defined(VEDIRECT_USE_TEXTFRAME)
  auto text_def = TEXT_DEF::find_type(register_type);
  if (text_def)
    this->text_registers_.emplace(text_def->label, entity);
#endif
}

#if defined(VEDIRECT_USE_HEXFRAME)
void Manager::send_hexframe(const HexFrame &hexframe) {
  this->write_array((const uint8_t *) hexframe.encoded(), hexframe.encoded_size());
  this->millis_last_hexframe_tx_ = millis();
  ESP_LOGD(this->logtag_, "HEX FRAME: sent %s", hexframe.encoded());
}

void Manager::send_hexframe(const char *rawframe, bool addchecksum) {
  HexFrameT<VEDIRECT_HEXFRAME_MAX_SIZE> hexframe;
  if (HexFrame::DecodeResult::Valid == hexframe.decode(rawframe, addchecksum)) {
    this->send_hexframe(hexframe);
  } else {
    ESP_LOGE(this->logtag_, "HEX FRAME: wrong encoding on request to send %s", rawframe);
  }
}

void Manager::request_set(register_id_t register_id, const void *data, HEXFRAME::DATA_TYPE data_type,
                          request_callback_t callback, request_callback_param_t callback_param) {
  // Request(s) in our storage are re-used as far as they're expired (millis == 0)
  Request *request = nullptr;
  for (auto it : this->requests_) {
    if (!it->millis) {
      request = it;
      goto _setup_request;
    }
  }
  // When no cached Request structs are available we increase our
  // storage. This will never be reduced/compacted though, hoping it doesn't grow
  // too much due to a fast burst of requests coming in (a pending request expires
  // either when replied or after a VEDIRECT_COMMAND_TIMEOUT_MILLIS timeout).
  this->requests_.push_back(new Request());
  request = this->requests_.back();
  request->tag = std::to_string(this->requests_.size());

_setup_request:
  request->millis = millis();
  request->hex_frame.command_set(register_id, data, data_type);
  request->callback = callback;
  request->callback_param = callback_param;
  ++this->pending_requests_;
  this->send_hexframe(request->hex_frame);
  this->set_timeout(request->tag, VEDIRECT_COMMAND_TIMEOUT_MILLIS, [this, request]() {
    if (request->millis) {
      ESP_LOGD(this->logtag_, "HEX FRAME: timeout on request(tag=%s) %s", request->tag.c_str(),
               request->hex_frame.encoded());
      // This means the SET command wasn't (yet) replied so we just timeout it.
      request->callback(request->callback_param, nullptr);
      request->millis = 0;
      --this->pending_requests_;
    }
  });
}

#endif  // defined(VEDIRECT_USE_HEXFRAME)

#if defined(VEDIRECT_USE_TEXTFRAME)
void Manager::init_register(Register *entity, const char *label) {
  auto text_def = TEXT_DEF::find_label(label);
  if (text_def) {
    if (!entity->reg_def_) {
      // only set reg_def from our presets (if any) if the yaml generated code
      // didn't set a custom configuration
      auto reg_def = REG_DEF::find_type(text_def->register_type);
      if (reg_def)
        this->init_register(entity, reg_def);
    }
  }
  this->text_registers_.emplace(label, entity);
}
#endif  // defined(VEDIRECT_USE_TEXTFRAME)

void Manager::on_connected_() {
  ESP_LOGD(this->logtag_, "LINK: connected");
  this->connected_ = true;
#ifdef USE_BINARY_SENSOR
  if (auto link_connected = this->link_connected_) {
    link_connected->publish_state(true);
  }
#endif
#if defined(VEDIRECT_USE_HEXFRAME)
  if (this->auto_create_hex_entities_ || this->hex_registers_.size()) {
    this->send_hexframe(HexFrame_Command(HEXFRAME::COMMAND::Ping));
    this->millis_last_ping_tx_ = this->millis_last_hexframe_tx_;
  }
#endif
}

void Manager::on_disconnected_() {
  ESP_LOGD(this->logtag_, "LINK: disconnected");
  this->connected_ = false;
  this->reset();  // cleanup the frame handler
#ifdef USE_BINARY_SENSOR
  if (auto link_connected = this->link_connected_) {
    link_connected->publish_state(false);
  }
#endif
#if defined(VEDIRECT_USE_TEXTFRAME)
  for (auto &pair : this->text_registers_) {
    pair.second->link_disconnected_();
  }
#endif
  for (auto &pair : this->hex_registers_) {
    pair.second->link_disconnected_();
  }
}

const char *FRAME_ERRORS[] = {
    "checksum",      "coding",         "overflow",
#if defined(VEDIRECT_USE_TEXTFRAME)
    "NAME overflow", "VALUE overflow", "RECORD overflow",
#endif
};

#if defined(VEDIRECT_USE_HEXFRAME)
void Manager::requests_match_get_or_set_(const RxHexFrame &rx_hex_frame) {
  for (auto request : this->requests_) {
    if ((request->hex_frame.command() == rx_hex_frame.command()) &&
        (request->hex_frame.register_id() == rx_hex_frame.register_id())) {
      ESP_LOGD(this->logtag_, "HEX FRAME: received reply %s (flags: 0x%02X) for request %s", rx_hex_frame.encoded(),
               rx_hex_frame.flags(), request->hex_frame.encoded());
      request->callback(request->callback_param, &rx_hex_frame);
      request->millis = 0;
      --this->pending_requests_;
      return;
    }
  }
}

void Manager::on_frame_hex_(const RxHexFrame &hexframe) {
  ESP_LOGD(this->logtag_, "HEX FRAME: received %s", hexframe.encoded());

  if (!this->connected_)
    this->on_connected_();

  this->hexframe_callback_.call(hexframe);

#ifdef USE_TEXT_SENSOR
  if (this->rawhexframe_)
    this->rawhexframe_->publish_state(std::string(hexframe.encoded()));
#endif

  this->millis_last_hexframe_rx_ = this->millis_last_rx_;
  switch (hexframe.command()) {
    case HEXFRAME::COMMAND::Async:
      goto _forward_to_register;
    case HEXFRAME::COMMAND::Get:
    case HEXFRAME::COMMAND::Set:
#ifdef ESPHOME_LOG_HAS_ERROR
      if (hexframe.flags()) {
        ESP_LOGE(this->logtag_, "HEX FRAME: received flags 0x%02X", hexframe.flags());
      }
#endif
      if (this->pending_requests_) {
        this->requests_match_get_or_set_(hexframe);
      }
      goto _forward_to_register;
    case HEXFRAME::COMMAND::Error:
      ESP_LOGE(this->logtag_, "HEX FRAME: received error frame %s", hexframe.encoded());
      return;
  }
  return;

_forward_to_register:
  if (hexframe.data_size() > 0) {
    Register *hex_register = this->get_hex_register_(hexframe.register_id(), this->auto_create_hex_entities_);
    if (hex_register)
      hex_register->parse_hex(&hexframe);
  } else {
    ESP_LOGE(this->logtag_, "HEX FRAME: inconsistent size: %s", hexframe.encoded());
  }
}

void Manager::on_frame_hex_error_(Error error) { ESP_LOGE(this->logtag_, "HEX FRAME: %s", FRAME_ERRORS[error]); }
#endif  // #if defined(VEDIRECT_USE_HEXFRAME)
#if defined(VEDIRECT_USE_TEXTFRAME)
void Manager::on_frame_text_(TextRecord **text_records, uint8_t text_records_count) {
  ESP_LOGD(this->logtag_, "TEXT FRAME: processing");

  if (!this->connected_)
    this->on_connected_();

  this->millis_last_textframe_rx_ = this->millis_last_rx_;

#ifdef USE_TEXT_SENSOR
  if (auto rawtextframe = this->rawtextframe_) {
    std::string textframe_value;
    textframe_value.reserve(text_records_count * sizeof(FrameHandler::TextRecord));
    for (uint8_t i = 0; i < text_records_count; ++i) {
      const TextRecord *text_record = text_records[i];
      textframe_value.append(text_record->name);
      textframe_value.append(":");
      textframe_value.append(text_record->value);
      textframe_value.append(",");
    }
    if (rawtextframe->raw_state != textframe_value) {
      rawtextframe->publish_state(textframe_value);
    }
  }
#endif

  for (uint8_t i = 0; i < text_records_count; ++i) {
    const TextRecord *text_record = text_records[i];
    auto entity_iter = this->text_registers_.find(text_record->name);
    if (entity_iter == this->text_registers_.end()) {
      if (this->auto_create_text_entities_) {
        ESP_LOGD(this->logtag_, "Auto-Creating entity for VE.Direct text field: %s", text_record->name);
        Register *hex_register;
        const char *label;
        auto text_def = TEXT_DEF::find_label(text_record->name);
        if (text_def) {
          label = text_def->label;
          // check if we have an already defined matching hex register
          auto reg_def = REG_DEF::find_type(text_def->register_type);
          if (reg_def) {
            hex_register = this->get_hex_register_(reg_def->register_id, true);
          } else {
            hex_register = Register::BUILD_ENTITY_FUNC[Register::TextSensor](this, label, label);
          }
        } else {
          // We lack the definition for this TEXT RECORD so
          // we return a plain TextSensor entity.
          // We allocate a copy since the label param is 'volatile'
          label = strdup(text_record->name);
          hex_register = Register::BUILD_ENTITY_FUNC[Register::TextSensor](this, label, label);
        }
        this->text_registers_.emplace(label, hex_register);
        hex_register->parse_text(text_record->value);
      }
    } else {
      entity_iter->second->parse_text(text_record->value);
    }
  }
}

void Manager::on_frame_text_error_(Error error) { ESP_LOGE(this->logtag_, "TEXT FRAME: %s", FRAME_ERRORS[error]); }

#endif  // #if defined(VEDIRECT_USE_TEXTFRAME)

Register *Manager::get_hex_register_(register_id_t register_id, bool create) {
  auto entity_iter = this->hex_registers_.find(register_id);
  if (entity_iter == this->hex_registers_.end()) {
    if (create) {
      ESP_LOGD(this->logtag_, "Auto-Creating entity for VE.Direct hex register: %04X", (int) register_id);
      Register *hex_register;
      auto reg_def = REG_DEF::find_register_id(register_id);
      if (reg_def) {
        switch (reg_def->cls) {
          case REG_DEF::CLASS::NUMERIC:
            if (reg_def->access == REG_DEF::ACCESS::READ_ONLY) {
              hex_register = Register::BUILD_ENTITY_FUNC[Register::Sensor](this, reg_def->label, reg_def->label);
            } else {
              hex_register = Register::BUILD_ENTITY_FUNC[Register::Number](this, reg_def->label, reg_def->label);
            }
            break;
          case REG_DEF::CLASS::BOOLEAN:
            if (reg_def->access == REG_DEF::ACCESS::READ_ONLY) {
              hex_register = Register::BUILD_ENTITY_FUNC[Register::BinarySensor](this, reg_def->label, reg_def->label);
            } else {
              hex_register = Register::BUILD_ENTITY_FUNC[Register::Switch](this, reg_def->label, reg_def->label);
            }
            break;
          case REG_DEF::CLASS::ENUM:
            if (reg_def->access == REG_DEF::ACCESS::READ_ONLY) {
              hex_register = Register::BUILD_ENTITY_FUNC[Register::TextSensor](this, reg_def->label, reg_def->label);
            } else {
              hex_register = Register::BUILD_ENTITY_FUNC[Register::Select](this, reg_def->label, reg_def->label);
            }
            break;
          case REG_DEF::CLASS::BITMASK: {
            hex_register = Register::BUILD_ENTITY_FUNC[Register::TextSensor](this, reg_def->label, reg_def->label);
          } break;
          default:
            hex_register = Register::BUILD_ENTITY_FUNC[Register::TextSensor](this, reg_def->label, reg_def->label);
        }
      } else {
        // else build a raw text sensor
        char *object_id = new char[7];
        sprintf(object_id, "0x%04X", (int) register_id);
        char *name = new char[16];
        sprintf(name, "Register %s", object_id);
        hex_register = Register::BUILD_ENTITY_FUNC[Register::TextSensor](this, name, object_id);
        reg_def = new REG_DEF(register_id);
      }
      this->init_register(hex_register, reg_def);
      return hex_register;
    } else {
      return nullptr;
    }
  } else {
    return entity_iter->second;
  }
}

}  // namespace m3_vedirect
}  // namespace esphome
