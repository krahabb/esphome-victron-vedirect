#include "sensor.h"
#include "esphome/core/application.h"
#ifdef USE_API
#include "esphome/components/api/api_server.h"
#endif

#include "../manager.h"

namespace esphome {
namespace m3_vedirect {

const sensor::StateClass Sensor::UNIT_TO_STATE_CLASS[REG_DEF::UNIT::UNIT_COUNT] = {
    sensor::StateClass::STATE_CLASS_NONE,        sensor::StateClass::STATE_CLASS_MEASUREMENT,
    sensor::StateClass::STATE_CLASS_MEASUREMENT, sensor::StateClass::STATE_CLASS_MEASUREMENT,
    sensor::StateClass::STATE_CLASS_MEASUREMENT, sensor::StateClass::STATE_CLASS_MEASUREMENT,
    sensor::StateClass::STATE_CLASS_TOTAL,       sensor::StateClass::STATE_CLASS_TOTAL_INCREASING,
    sensor::StateClass::STATE_CLASS_MEASUREMENT, sensor::StateClass::STATE_CLASS_MEASUREMENT,
    sensor::StateClass::STATE_CLASS_MEASUREMENT, sensor::StateClass::STATE_CLASS_MEASUREMENT,
    sensor::StateClass::STATE_CLASS_MEASUREMENT, sensor::StateClass::STATE_CLASS_MEASUREMENT,
};
const uint8_t Sensor::SCALE_TO_DIGITS[REG_DEF::SCALE::SCALE_COUNT] = {
    0,  // S_1,
    1,  // S_0_1,
    2,  // S_0_01,
    3,  // S_0_001,
    2,  // S_0_25,
};

Register *Sensor::build_entity(Manager *manager, const char *name, const char *object_id) {
  auto entity = new Sensor(manager);
  Register::dynamic_init_entity_(entity, name, object_id, manager->get_vedirect_name(), manager->get_vedirect_id());
  App.register_sensor(entity);
#ifdef USE_API
  if (api::global_api_server)
    entity->add_on_state_callback([entity](float state) { api::global_api_server->on_sensor_update(entity, state); });
#endif
  return entity;
}

void Sensor::link_disconnected_() { this->publish_state(NAN); }

void Sensor::init_reg_def_() {
  auto reg_def = this->reg_def_;
#if defined(VEDIRECT_USE_HEXFRAME)
  this->parse_hex_ = DATA_TYPE_TO_PARSE_HEX_FUNC_[reg_def->data_type];
#endif
  switch (reg_def->cls) {
    case REG_DEF::CLASS::NUMERIC:
      this->set_unit_of_measurement(REG_DEF::UNITS[reg_def->unit]);
      this->set_device_class(UNIT_TO_DEVICE_CLASS[reg_def->unit]);
      this->set_state_class(UNIT_TO_STATE_CLASS[reg_def->unit]);
      this->set_accuracy_decimals(SCALE_TO_DIGITS[reg_def->scale]);
      this->hex_scale_ = REG_DEF::SCALE_TO_SCALE[reg_def->scale];
      this->text_scale_ = REG_DEF::SCALE_TO_SCALE[reg_def_->text_scale];
#if defined(VEDIRECT_USE_HEXFRAME)
      switch (reg_def->unit) {
        case REG_DEF::UNIT::KELVIN:
          // special treatment for 'temperature' registers which are expected to carry un16 kelvin degrees
          this->parse_hex_ = parse_hex_kelvin_;
          break;
        default:
          break;
      }
#endif
      break;
    default:
      break;
  }
}

#if defined(VEDIRECT_USE_HEXFRAME)
void Sensor::parse_hex_default_(Register *hex_register, const RxHexFrame *hex_frame) {
  Sensor *sensor = static_cast<Sensor *>(hex_register);
  float value;
  switch (hex_frame->data_size()) {
    case 1:
      value = hex_frame->data_t<uint8_t>() * sensor->hex_scale_;
      break;
    case 2:
      // it might be signed though
      value = hex_frame->data_t<uint16_t>() * sensor->hex_scale_;
      break;
    case 4:
      value = hex_frame->data_t<uint32_t>() * sensor->hex_scale_;
      break;
    default:
      if (!std::isnan(sensor->raw_state)) {
        sensor->publish_state(NAN);
      }
      return;
  }
  if (sensor->raw_state != value) {
    sensor->publish_state(value);
  }
}

void Sensor::parse_hex_kelvin_(Register *hex_register, const RxHexFrame *hex_frame) {
  Sensor *sensor = static_cast<Sensor *>(hex_register);
  uint16_t raw_value = hex_frame->data_t<uint16_t>();
  if (raw_value == HEXFRAME::DATA_UNKNOWN<uint16_t>()) {
    if (!std::isnan(sensor->raw_state)) {
      sensor->publish_state(NAN);
    }
  } else {
    // hoping the operands are int-promoted and the result is an int
    float value = (raw_value - 27316) * sensor->hex_scale_;
    if (sensor->raw_state != value) {
      sensor->publish_state(value);
    }
  }
}

template<typename T> void Sensor::parse_hex_t_(Register *hex_register, const RxHexFrame *hex_frame) {
  static_assert(RxHexFrame::ALLOCATED_DATA_SIZE >= 4, "HexFrame storage might lead to access overflow");
  Sensor *sensor = static_cast<Sensor *>(hex_register);
  T raw_value = hex_frame->data_t<T>();
  if (raw_value == HEXFRAME::DATA_UNKNOWN<T>()) {
    if (!std::isnan(sensor->raw_state)) {
      sensor->publish_state(NAN);
    }
  } else {
    float value = raw_value * sensor->hex_scale_;
    if (sensor->raw_state != value) {
      sensor->publish_state(value);
    }
  }
}

const Sensor::parse_hex_func_t Sensor::DATA_TYPE_TO_PARSE_HEX_FUNC_[REG_DEF::DATA_TYPE::_COUNT] = {
    Sensor::parse_hex_default_,     Sensor::parse_hex_t_<uint8_t>, Sensor::parse_hex_t_<uint16_t>,
    Sensor::parse_hex_t_<uint32_t>, Sensor::parse_hex_t_<int8_t>,  Sensor::parse_hex_t_<int16_t>,
    Sensor::parse_hex_t_<int32_t>,
};
#endif  // defined(VEDIRECT_USE_HEXFRAME)

#if defined(VEDIRECT_USE_TEXTFRAME)
void Sensor::parse_text_default_(Register *hex_register, const char *text_value) {
  Sensor *sensor = static_cast<Sensor *>(hex_register);
  char *endptr;
  float value = strtof(text_value, &endptr) * sensor->text_scale_;
  if (*endptr) {
    // failed conversion
    if (!std::isnan(sensor->raw_state)) {
      sensor->publish_state(NAN);
    }
  } else {
    if (sensor->raw_state != value) {
      sensor->publish_state(value);
    }
  }
}
#endif  // defined(VEDIRECT_USE_TEXTFRAME)

}  // namespace m3_vedirect
}  // namespace esphome
