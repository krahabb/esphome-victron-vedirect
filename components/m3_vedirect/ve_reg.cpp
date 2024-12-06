#include "ve_reg.h"

namespace m3_ve_reg {

const uint8_t HEXFRAME::DATA_TYPE_TO_SIZE[] = {0, 1, 2, 4, 1, 2, 4};

const HEXFRAME::get_data_int_func_t HEXFRAME::GET_DATA_AS_INT[DATA_TYPE::_COUNT] = {
    HEXFRAME::get_data_t<int, uint8_t>,   // STRING = 0 (actually to be better defined)
    HEXFRAME::get_data_t<int, uint8_t>,   // U8 = 1,
    HEXFRAME::get_data_t<int, uint16_t>,  // U16 = 2,
    HEXFRAME::get_data_t<int, uint32_t>,  // U32 = 3,
    HEXFRAME::get_data_t<int, int8_t>,    // I8 = 4,
    HEXFRAME::get_data_t<int, int16_t>,   // I16 = 5,
    HEXFRAME::get_data_t<int, int32_t>,   // I32 = 6,
};
}  // namespace m3_ve_reg