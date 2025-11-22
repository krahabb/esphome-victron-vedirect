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

Register *Sensor::build_entity(Manager *manager, const REG_DEF *reg_def, const char *name) {
#if ESPHOME_VERSION_CODE >= VERSION_CODE(2025, 8, 0)
  if (App.get_sensors().size() >= ESPHOME_ENTITY_SENSOR_COUNT) {
    return Register::drop_platform(manager, Platform::Sensor);
  }
#endif
  auto entity = new Sensor(manager);
  manager->init_entity(entity, reg_def, name);
  App.register_sensor(entity);
#ifdef USE_API
  entity->add_on_state_callback([entity](float state) { api::global_api_server->on_sensor_update(entity, state); });
#endif
  return entity;
}

void Sensor::link_disconnected_() {
  if (this->has_state()) {
    this->publish_state(NAN);
    this->set_has_state(false);
  }
}

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
      this->text_scale_ = REG_DEF::SCALE_TO_SCALE[reg_def->text_scale];
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
  /* This is the handler used when no DATA_TYPE is specified.
    Since we don't expect the data format to change, we can use the actual frame data size to
    infer a somewhat reasonable data type. This will lack sign information though.
    Once the data type is inferred, we'll install the proper handler for the next frame
    so that we don't have to go through this process again.
  */
  Sensor *sensor = static_cast<Sensor *>(hex_register);
  switch (hex_frame->data_size()) {
    case 1:
      sensor->parse_hex_ = parse_hex_t_<uint8_t>;
      break;
    case 2:
      sensor->parse_hex_ = parse_hex_t_<uint16_t>;
      break;
    case 4:
      sensor->parse_hex_ = parse_hex_t_<uint32_t>;
      break;
    default:
      if (!std::isnan(sensor->raw_state)) {
        sensor->publish_state(NAN);
      }
      return;
  }
  sensor->parse_hex_(hex_register, hex_frame);
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
