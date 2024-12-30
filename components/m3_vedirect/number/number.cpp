#include "number.h"
#include "esphome/core/application.h"
#ifdef USE_API
#include "esphome/components/api/api_server.h"
#endif

#include "../manager.h"

namespace esphome {
namespace m3_vedirect {

Register *Number::build_entity(Manager *manager, const char *name, const char *object_id) {
  auto entity = new Number(manager);
  Register::dynamic_init_entity_(entity, name, object_id, manager->get_vedirect_name(), manager->get_vedirect_id());
  App.register_number(entity);
#ifdef USE_API
  if (api::global_api_server)
    entity->add_on_state_callback([entity](float state) { api::global_api_server->on_number_update(entity, state); });
#endif
  return entity;
}

void Number::link_disconnected_() { this->publish_state(NAN); }

void Number::init_reg_def_() {
  auto reg_def = this->reg_def_;
  switch (reg_def->cls) {
    case REG_DEF::CLASS::NUMERIC:
      this->hex_scale_ = REG_DEF::SCALE_TO_SCALE[reg_def->scale];
      this->traits.set_unit_of_measurement(REG_DEF::UNITS[reg_def->unit]);
      this->traits.set_device_class(UNIT_TO_DEVICE_CLASS[reg_def->unit]);
      this->traits.set_step(this->hex_scale_);
#if defined(VEDIRECT_USE_HEXFRAME)
      switch (reg_def->unit) {
        case REG_DEF::UNIT::KELVIN:
          // special treatment for 'temperature' registers which are expected to carry un16 kelvin degrees
          this->parse_hex_ = parse_hex_kelvin_;
          break;
        default:
          this->parse_hex_ = DATA_TYPE_TO_PARSE_HEX_FUNC_[reg_def->data_type];
      }
#endif
      break;
    default:
      break;
  }
}

#if defined(VEDIRECT_USE_HEXFRAME)
void Number::control(float value) {
  // Assuming 'value' is not out of range of the underlying data type, this code
  // should work for both signed/unsigned quantities
  int native_value = value / this->hex_scale_;
  this->manager->request_set(this->reg_def_->register_id, &native_value, this->reg_def_->data_type, request_callback_,
                             this);
};

void Number::request_callback_(void *callback_param, const RxHexFrame *hex_frame) {
  Number *_number = reinterpret_cast<Number *>(callback_param);
  if (!hex_frame || (hex_frame && hex_frame->flags())) {
    // Error or timeout..resend actual state since it looks like HA esphome does optimistic
    // updates in it's HA entity instance...
    _number->publish_state(_number->state);
  } else {
    // Invalidate our state so that the subsequent dispatching/parsing goes through
    // an effective publish_state. This is needed (again) since the frontend already
    // optimistically updated the entity to the new value but even in case of success,
    // the device might 'force' a different setting if the request was for an unsupported
    // value
    _number->state = NAN;
  }
}

void Number::parse_hex_default_(Register *hex_register, const RxHexFrame *hex_frame) {
  Number *number = static_cast<Number *>(hex_register);
  float value;
  switch (hex_frame->data_size()) {
    case 1:
      value = hex_frame->data_t<uint8_t>() * number->hex_scale_;
      break;
    case 2:
      // it might be signed though
      value = hex_frame->data_t<uint16_t>() * number->hex_scale_;
      break;
    case 4:
      value = hex_frame->data_t<uint32_t>() * number->hex_scale_;
      break;
    default:
      if (!std::isnan(number->state)) {
        number->publish_state(NAN);
      }
      return;
  }
  if (number->state != value) {
    number->publish_state(value);
  }
}

void Number::parse_hex_kelvin_(Register *hex_register, const RxHexFrame *hex_frame) {
  Number *number = static_cast<Number *>(hex_register);
  uint16_t raw_value = hex_frame->data_t<uint16_t>();
  if (raw_value == HEXFRAME::DATA_UNKNOWN<uint16_t>()) {
    if (!std::isnan(number->state)) {
      number->publish_state(NAN);
    }
  } else {
    // hoping the operands are int-promoted and the result is an int
    float value = (raw_value - 27316) * number->hex_scale_;
    if (number->state != value) {
      number->publish_state(value);
    }
  }
}

template<typename T> void Number::parse_hex_t_(Register *hex_register, const RxHexFrame *hex_frame) {
  static_assert(RxHexFrame::ALLOCATED_DATA_SIZE >= 4, "HexFrame storage might lead to access overflow");
  Number *number = static_cast<Number *>(hex_register);
  T raw_value = hex_frame->data_t<T>();
  if (raw_value == HEXFRAME::DATA_UNKNOWN<T>()) {
    if (!std::isnan(number->state)) {
      number->publish_state(NAN);
    }
  } else {
    float value = raw_value * number->hex_scale_;
    if (number->state != value) {
      number->publish_state(value);
    }
  }
}

const Number::parse_hex_func_t Number::DATA_TYPE_TO_PARSE_HEX_FUNC_[REG_DEF::DATA_TYPE::_COUNT] = {
    Number::parse_hex_default_,     Number::parse_hex_t_<uint8_t>, Number::parse_hex_t_<uint16_t>,
    Number::parse_hex_t_<uint32_t>, Number::parse_hex_t_<int8_t>,  Number::parse_hex_t_<int16_t>,
    Number::parse_hex_t_<int32_t>,
};
#endif  //  defined(VEDIRECT_USE_HEXFRAME)

}  // namespace m3_vedirect
}  // namespace esphome
