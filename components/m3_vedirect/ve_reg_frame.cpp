#include "ve_reg_frame.h"
#include <string.h>

namespace m3_ve_reg {

#if defined(VEDIRECT_USE_HEXFRAME)

const char HEX_DIGITS_MAP[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

bool HexFrame::data_to_hex(std::string &hexdata) const {
  int encoded_size = this->encoded_end_ - this->encoded_begin_;
  if (encoded_size > 11) {
    hexdata = "0x";
    hexdata.append(this->encoded_begin_ + 8, this->encoded_end_ - 3);
    return true;
  }
  return false;
}

bool HexFrame::data_to_hex(char *buf, size_t buf_size) const {
  int encoded_size = this->encoded_end_ - this->encoded_begin_ - 11;
  if (encoded_size > 0) {
    if ((encoded_size + 2) >= buf_size)
      encoded_size = buf_size - 3;
    buf[0] = '0';
    buf[1] = 'x';
    memcpy(buf + 2, this->encoded_begin_ + 8, encoded_size);
    buf[encoded_size + 2] = 0;
    return true;
  }
  return false;
}

HexFrame::DecodeResult HexFrame::decode(const char *hexdigits, bool addchecksum) {
  HexFrameDecoder decoder;
  decoder.init(this);

  for (;;) {
    switch (auto result = decoder.decode(*hexdigits++)) {
      case DecodeResult::Continue:
        continue;
      case DecodeResult::Terminated:
        if (addchecksum) {
          *this->encoded_end_++ = HEX_DIGITS_MAP[decoder.get_checksum() >> 4];
          *this->encoded_end_++ = HEX_DIGITS_MAP[decoder.get_checksum() & 0x0F];
        }
        *this->encoded_end_++ = '\n';
        *this->encoded_end_ = 0;
        return DecodeResult::Valid;
      default:
        return result;
    }
  }
}

void HexFrame::command(HEXFRAME::COMMAND command, register_id_t register_id, const void *data, size_t data_size) {
  auto record = this->record();
  record->command = command;
  record->register_id = register_id;
  record->flags = 0;
  if (data) {
    memcpy(record->data, data, data_size);
    this->rawframe_end_ = this->rawframe_begin_ + 4 + data_size;
  } else {
    this->rawframe_end_ = this->rawframe_begin_ + 4;
  }
  this->encode_();
}

void HexFrame::encode_() {
  this->encoded_end_ = this->encoded_begin_;
  int hexframe_size = this->size();
  if (hexframe_size > 0) {
    int encoded_size = 1 + 1 + (hexframe_size - 1) * 2 + 2 + 2;
    if (encoded_size <= (this->encoded_end_of_storage() - this->encoded_begin_)) {
      *this->encoded_end_++ = ':';
      const uint8_t *data = this->begin();
      uint8_t checksum = 0x55 - *data;
      *this->encoded_end_++ = HEX_DIGITS_MAP[(*data) & 0x0F];
      while (++data < this->end()) {
        *this->encoded_end_++ = HEX_DIGITS_MAP[(*data) >> 4];
        *this->encoded_end_++ = HEX_DIGITS_MAP[(*data) & 0x0F];
        checksum -= *data;
      }
      *this->encoded_end_++ = HEX_DIGITS_MAP[checksum >> 4];
      *this->encoded_end_++ = HEX_DIGITS_MAP[checksum & 0x0F];
      *this->encoded_end_++ = '\n';
    }
  }
  *this->rawframe_end_ = 0;
  *this->encoded_end_ = 0;
}

#endif  // defined(VEDIRECT_USE_HEXFRAME)

#if defined(VEDIRECT_USE_HEXFRAME) && defined(VEDIRECT_USE_TEXTFRAME)
void FrameHandler::decode(uint8_t *data_begin, uint8_t *data_end) {
  uint8_t data;
handle_state:
  switch (this->frame_state_) {
    case State::Name:
    handle_state_name:
      while (data_begin < data_end) {
        data = *data_begin++;
        switch (data) {
          case '\t':  // end of name
            this->text_checksum_ += '\t';
            *this->text_record_write_ = 0;
            if (strcmp(this->text_record_->name, "Checksum")) {
              this->frame_text_value_start_();
              this->frame_state_ = State::Value;
              goto handle_state_value;
            } else {  // the Checksum record indicates a EOF
              this->frame_state_ = State::Checksum;
              goto handle_state_checksum;
            }
          case ':':  // HEX FRAME
            this->frame_hex_start_();
            goto handle_state_hex;
          default:
            this->text_checksum_ += data;
            *this->text_record_write_ = data;
            if (++this->text_record_write_ >= this->text_record_write_end_) {
              this->on_frame_text_error_(Error::NAME_OVERFLOW);
              this->frame_state_ = State::Idle;
              goto handle_state_idle;
            }
        }
      }
      break;
    case State::Value:
    handle_state_value:
      while (data_begin < data_end) {
        data = *data_begin++;
        switch (data) {
          case '\n':  // start of next record
            this->text_checksum_ += '\n';
            *this->text_record_write_ = 0;
            if (++this->text_records_count_ >= VEDIRECT_RECORDS_COUNT) {
              this->on_frame_text_error_(Error::RECORD_OVERFLOW);
              this->frame_state_ = State::Idle;
              goto handle_state_idle;
            }
            this->frame_text_name_start_();
            this->frame_state_ = State::Name;
            goto handle_state_name;
          case '\r':  // pre-start of next record
            this->text_checksum_ += '\r';
            break;
          case ':':  // HEX FRAME
            this->frame_hex_start_();
            goto handle_state_hex;
          default:
            this->text_checksum_ += data;
            *this->text_record_write_ = data;
            if (++this->text_record_write_ >= this->text_record_write_end_) {
              this->on_frame_text_error_(Error::VALUE_OVERFLOW);
              this->frame_state_ = State::Idle;
              goto handle_state_idle;
            }
        }
      }
      break;
    case State::Idle:
    handle_state_idle:
      while (data_begin < data_end) {
        switch (*data_begin++) {
          case ':':  // HEX FRAME
            this->frame_hex_start_();
            goto handle_state_hex;
          case '\n':  // start of TEXT FRAME
            this->text_checksum_ += '\n';
            this->text_records_count_ = 0;
            this->frame_text_name_start_();
            this->frame_state_ = State::Name;
            goto handle_state_name;
          case '\r':  // pre-start of TEXT FRAME
            this->text_checksum_ = '\r';
          default:
            break;
        }
      }
      break;
    case State::Hex:
    handle_state_hex:
      while (data_begin < data_end) {
        switch (this->hexframe_decoder_.decode(*data_begin++)) {
          case HexFrameDecoder::Result::Continue:
            break;
          case HexFrameDecoder::Result::Valid:
            this->on_frame_hex_(this->hexframe_);
            this->frame_state_ = this->frame_state_backup_;
            goto handle_state;
          case HexFrameDecoder::Result::ChecksumError:
            this->on_frame_hex_error_(Error::CHECKSUM);
            this->frame_state_ = this->frame_state_backup_;
            goto handle_state;
          case HexFrameDecoder::Result::Overflow:
            this->on_frame_hex_error_(Error::OVERFLOW);
            this->frame_state_ = State::Idle;
            goto handle_state_idle;
          default:
            // case HexFrameDecoder::Result::CodingError:
            // case HexFrameDecoder::Result::Terminated:
            this->on_frame_hex_error_(Error::CODING);
            this->frame_state_ = State::Idle;
            goto handle_state_idle;
        }
      }
      break;
    case State::Checksum:
    handle_state_checksum:
      if (data_begin < data_end) {
        if ((uint8_t) (this->text_checksum_ + *data_begin++)) {
          this->on_frame_text_error_(Error::CHECKSUM);
        } else {
          this->on_frame_text_(this->text_records_, this->text_records_count_);
        }
        this->frame_state_ = State::Idle;
        goto handle_state_idle;
      }
      break;
  }
}
#elif defined(VEDIRECT_USE_HEXFRAME)
void FrameHandler::decode(uint8_t *data_begin, uint8_t *data_end) {
  uint8_t data;
  switch (this->frame_state_) {
    case State::Idle:
    handle_state_idle:
      while (data_begin < data_end) {
        switch (*data_begin++) {
          case ':':  // HEX FRAME
            this->frame_hex_start_();
            goto handle_state_hex;
          default:
            break;
        }
      }
      break;
    case State::Hex:
    handle_state_hex:
      while (data_begin < data_end) {
        switch (this->hexframe_decoder_.decode(*data_begin++)) {
          case HexFrameDecoder::Result::Continue:
            break;
          case HexFrameDecoder::Result::Valid:
            this->on_frame_hex_(this->hexframe_);
            this->frame_state_ = State::Idle;
            goto handle_state_idle;
          case HexFrameDecoder::Result::ChecksumError:
            this->on_frame_hex_error_(Error::CHECKSUM);
            this->frame_state_ = State::Idle;
            goto handle_state_idle;
          case HexFrameDecoder::Result::Overflow:
            this->on_frame_hex_error_(Error::OVERFLOW);
            this->frame_state_ = State::Idle;
            goto handle_state_idle;
          default:
            // case HexFrameDecoder::Result::CodingError:
            // case HexFrameDecoder::Result::Terminated:
            this->on_frame_hex_error_(Error::CODING);
            this->frame_state_ = State::Idle;
            goto handle_state_idle;
        }
      }
      break;
  }
}
#elif defined(VEDIRECT_USE_TEXTFRAME)
void FrameHandler::decode(uint8_t *data_begin, uint8_t *data_end) {
  uint8_t data;
handle_state:
  switch (this->frame_state_) {
    case State::Name:
    handle_state_name:
      while (data_begin < data_end) {
        data = *data_begin++;
        switch (data) {
          case '\t':  // end of name
            this->text_checksum_ += '\t';
            *this->text_record_write_ = 0;
            if (strcmp(this->text_record_->name, "Checksum")) {
              this->frame_text_value_start_();
              this->frame_state_ = State::Value;
              goto handle_state_value;
            } else {  // the Checksum record indicates a EOF
              this->frame_state_ = State::Checksum;
              goto handle_state_checksum;
            }
          case ':':  // HEX FRAME
            this->frame_hex_start_();
            goto handle_state_hex;
          default:
            this->text_checksum_ += data;
            *this->text_record_write_ = data;
            if (++this->text_record_write_ >= this->text_record_write_end_) {
              this->on_frame_text_error_(Error::NAME_OVERFLOW);
              this->frame_state_ = State::Idle;
              goto handle_state_idle;
            }
        }
      }
      break;
    case State::Value:
    handle_state_value:
      while (data_begin < data_end) {
        data = *data_begin++;
        switch (data) {
          case '\n':  // start of next record
            this->text_checksum_ += '\n';
            *this->text_record_write_ = 0;
            if (++this->text_records_count_ >= VEDIRECT_RECORDS_COUNT) {
              this->on_frame_text_error_(Error::RECORD_OVERFLOW);
              this->frame_state_ = State::Idle;
              goto handle_state_idle;
            }
            this->frame_text_name_start_();
            this->frame_state_ = State::Name;
            goto handle_state_name;
          case '\r':  // pre-start of next record
            this->text_checksum_ += '\r';
            break;
          case ':':  // HEX FRAME
            this->frame_hex_start_();
            goto handle_state_hex;
          default:
            this->text_checksum_ += data;
            *this->text_record_write_ = data;
            if (++this->text_record_write_ >= this->text_record_write_end_) {
              this->on_frame_text_error_(Error::VALUE_OVERFLOW);
              this->frame_state_ = State::Idle;
              goto handle_state_idle;
            }
        }
      }
      break;
    case State::Idle:
    handle_state_idle:
      while (data_begin < data_end) {
        switch (*data_begin++) {
          case ':':  // HEX FRAME
            this->frame_hex_start_();
            goto handle_state_hex;
          case '\n':  // start of TEXT FRAME
            this->text_checksum_ += '\n';
            this->text_records_count_ = 0;
            this->frame_text_name_start_();
            this->frame_state_ = State::Name;
            goto handle_state_name;
          case '\r':  // pre-start of TEXT FRAME
            this->text_checksum_ = '\r';
          default:
            break;
        }
      }
      break;
    case State::Hex:
    handle_state_hex:
      while (data_begin < data_end) {
        switch (*data_begin++) {
          case '\n':
            this->frame_state_ = this->frame_state_backup_;
            goto handle_state;
        }
      }
      break;
    case State::Checksum:
    handle_state_checksum:
      if (data_begin < data_end) {
        if ((uint8_t) (this->text_checksum_ + *data_begin++)) {
          this->on_frame_text_error_(Error::CHECKSUM);
        } else {
          this->on_frame_text_(this->text_records_, this->text_records_count_);
        }
        this->frame_state_ = State::Idle;
        goto handle_state_idle;
      }
      break;
  }
}
#endif
/*
static_assert(sizeof(HexFrame::Record) == 8, "HexFrame::Record size failure");
static_assert(sizeof(HexFrame) == 20, "HexFrame size failure = ");
static_assert(sizeof(HexFrame_Get) == 40, "HexFrame_Get size failure = ");

auto _a = HexFrame_Set(0, 1);
static_assert(sizeof(_a) == 52, "HexFrame_Set size failure = ");
*/

}  // namespace m3_ve_reg
