#include "number.h"
#include "esphome/core/application.h"
#if ESPHOME_VERSION_CODE < VERSION_CODE(2025, 11, 0)
#ifdef USE_API
#include "esphome/components/api/api_server.h"
#endif
#endif

#include "../manager.h"

namespace esphome {
namespace m3_vedirect {

Register *Number::build_entity(Manager *manager, const REG_DEF *reg_def, const char *name) {
#if ESPHOME_VERSION_CODE >= VERSION_CODE(2025, 8, 0)
  if (App.get_numbers().size() >= ESPHOME_ENTITY_NUMBER_COUNT) {
    return Register::drop_platform(manager, Platform::Number);
  }
#endif
  auto entity = new Number(manager);
  manager->init_entity(entity, reg_def, name);
  App.register_number(entity);
#if ESPHOME_VERSION_CODE < VERSION_CODE(2025, 11, 0)
// See https://github.com/esphome/esphome/pull/11772
#if defined(USE_API)
  entity->add_on_state_callback([entity](float state) { api::global_api_server->on_number_update(entity, state); });
#endif
#endif
  return entity;
}

void Number::link_disconnected_() {
  if (this->has_state()) {
    this->publish_state(NAN);
    this->set_has_state(false);
  }
}

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
  this->request_set_((int) std::roundf(value / this->hex_scale_), [this](const HexFrame *frame, uint8_t error) {
    if (error) {
      // Error or timeout..resend actual state since it looks like HA esphome does optimistic
      // updates in it's HA entity instance...
      this->publish_state(this->state);
    } else {
      // Invalidate our state so that the subsequent dispatching/parsing goes through
      // an effective publish_state. This is needed (again) since the frontend already
      // optimistically updated the entity to the new value but even in case of success,
      // the device might 'force' a different setting if the request was for an unsupported
      // value
      this->state = NAN;
    }
  });
};

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
