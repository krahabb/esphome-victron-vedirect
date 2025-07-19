#include "select.h"
#include "esphome/core/application.h"
#ifdef USE_API
#include "esphome/components/api/api_server.h"
#endif

#include "../manager.h"

#include <cinttypes>

namespace esphome {
namespace m3_vedirect {

#ifdef ESPHOME_LOG_HAS_DEBUG
static const char *const TAG = "m3_vedirect.select";
#endif

Register *Select::build_entity(Manager *manager, const char *name, const char *object_id) {
  auto entity = new Select(manager);
  Register::dynamic_init_entity_(entity, name, object_id, manager->get_vedirect_name(), manager->get_vedirect_id());
  App.register_select(entity);
#ifdef USE_API
  if (api::global_api_server)
    entity->add_on_state_callback([entity](const std::string &state, size_t index) {
      api::global_api_server->on_select_update(entity, state, index);
    });
#endif
  return entity;
}

void Select::link_disconnected_() {
  this->enum_value_ = ENUM_DEF::VALUE_UNKNOWN;
  this->publish_state_("unknown", -1);
}

void Select::init_reg_def_() {
  switch (this->reg_def_->cls) {
    case REG_DEF::CLASS::ENUM:
      for (auto &lookup_def : this->reg_def_->enum_def->LOOKUPS) {
        this->traits_().options().push_back(std::string(lookup_def.label));
      }
#if defined(VEDIRECT_USE_HEXFRAME)
      this->parse_hex_ = parse_hex_enum_;
#endif
#if defined(VEDIRECT_USE_TEXTFRAME)
      this->parse_text_ = parse_text_enum_;
#endif
      break;
    default:
      break;
  }
}

void Select::parse_enum_(ENUM_DEF::enum_t enum_value) {
  if (this->enum_value_ != enum_value) {
    this->enum_value_ = enum_value;
    this->publish_enum_(enum_value);
  }
}

void Select::parse_string_(const char *string_value) {
  if (strcmp(this->state.c_str(), string_value)) {
    auto &options = this->traits_().options();
    auto value = std::string(string_value);
    auto it = std::find(options.begin(), options.end(), value);
    auto index = std::distance(options.begin(), it);
    if (it == options.end()) {
      options.push_back(value);
    }
    this->publish_state_(value, index);
  }
}

#if defined(VEDIRECT_USE_HEXFRAME)
void Select::control(const std::string &value) {
  // TODO: are we 100% sure enum_def is defined ? check yaml init code
  auto lookup_def = this->reg_def_->enum_def->lookup_value(value.c_str());
  if (lookup_def) {
    this->manager->request(HEXFRAME::COMMAND::Set, this->reg_def_->register_id, &lookup_def->value,
                           this->reg_def_->data_type, request_callback_, this);
  }
}

void Select::request_callback_(void *callback_param, const RxHexFrame *hex_frame) {
  Select *_select = reinterpret_cast<Select *>(callback_param);
  if (!hex_frame || (hex_frame && hex_frame->flags())) {
    // Error or timeout..resend actual state since it looks like HA esphome does optimistic
    // updates in it's HA entity instance...
    if (_select->enum_value_ != ENUM_DEF::VALUE_UNKNOWN) {
      _select->publish_enum_(_select->enum_value_);
    }
  } else {
    // Invalidate our state so that the subsequent dispatching/parsing goes through
    // an effective publish_state. This is needed (again) since the frontend already
    // optimistically updated the entity to the new value but even in case of success,
    // the device might 'force' a different setting if the request was for an unsupported
    // ENUM
    _select->enum_value_ = ENUM_DEF::VALUE_UNKNOWN;
  }
}

void Select::parse_hex_default_(Register *hex_register, const RxHexFrame *hex_frame) {
  char hex_value[RxHexFrame::ALLOCATED_ENCODED_SIZE];
  if (hex_frame->data_to_hex(hex_value, RxHexFrame::ALLOCATED_ENCODED_SIZE)) {
    static_cast<Select *>(hex_register)->parse_string_(hex_value);
  }
}

void Select::parse_hex_enum_(Register *hex_register, const RxHexFrame *hex_frame) {
  static_assert(RxHexFrame::ALLOCATED_DATA_SIZE >= 1, "HexFrame storage might lead to access overflow");
  static_cast<Select *>(hex_register)->parse_enum_(hex_frame->data_t<ENUM_DEF::enum_t>());
}
#endif  // defined(VEDIRECT_USE_HEXFRAME)

#if defined(VEDIRECT_USE_TEXTFRAME)
void Select::parse_text_default_(Register *hex_register, const char *text_value) {
  static_cast<Select *>(hex_register)->parse_string_(text_value);
}

void Select::parse_text_enum_(Register *hex_register, const char *text_value) {
  char *endptr;
  ENUM_DEF::enum_t enum_value = strtoumax(text_value, &endptr, 0);
  if (*endptr == 0)
    static_cast<Select *>(hex_register)->parse_enum_(enum_value);
}
#endif  // defined(VEDIRECT_USE_TEXTFRAME)

void Select::publish_enum_(ENUM_DEF::enum_t enum_value) {
  // the select::traits implementation is so bad...
  // it would be nice to have a data provider interface though but
  // this is it and we'd rather not patch the official esphome core.
  // Here we'll try to mantain sync between our enum_def and the select::options array
  auto &options = this->traits_().options();
  auto enum_def = this->reg_def_->enum_def;
  auto lookup_result = enum_def->get_lookup(enum_value);
  if (options.size() != enum_def->LOOKUPS.size()) {
    ESP_LOGD(TAG, "'%s': Rebuilding options (prev size %zu, new size %zu)", this->get_name().c_str(), options.size(),
             enum_def->LOOKUPS.size());
    options.clear();
    for (auto &lookup_def : enum_def->LOOKUPS) {
      options.push_back(std::string(lookup_def.label));
    }
  }
  this->publish_state_(lookup_result.index);
}

void Select::publish_state_(const std::string &state, size_t index) {
  // our custom publish_state doesn't really care if the index is correct or not since esphome
  // discards it anyway when broadcasting through api
  this->set_has_state(true);
  this->state = state;
  ESP_LOGD(TAG, "'%s': Sending state %s (index %zu)", this->get_name().c_str(), state.c_str(), index);
  this->state_callback_.call(state, index);
}

}  // namespace m3_vedirect
}  // namespace esphome
