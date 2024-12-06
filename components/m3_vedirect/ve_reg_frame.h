#pragma once
#include "ve_reg.h"
#include <string>
#include <cstring>  // need memcpy

namespace m3_ve_reg {

// VEDIRECT_USE_HEXFRAME and VEDIRECT_USE_TEXTFRAME can be used to enable/disable
// entire support for parsing of the respective type of frame. This is used
// mainly to optimize FrameHandler but could also be used in dependant code to
// conditionally include features directly related to this.
#if !defined(VEDIRECT_USE_HEXFRAME) && !defined(VEDIRECT_USE_TEXTFRAME)
// enable both by default if nothing was choosen
#define VEDIRECT_USE_HEXFRAME
#define VEDIRECT_USE_TEXTFRAME
#endif

#if defined(VEDIRECT_USE_HEXFRAME)
// Fix a (reasonable) limit to the maximum size (in raw frame bytes) of an incoming
// HEX frame so that we abort pumping data into memory when something is likely wrong
// This is needed since our FrameHandler statically allocates the receiving HexFrame
// buffer in order to optimize memory management and performance
#ifndef VEDIRECT_HEXFRAME_MAX_SIZE
#define VEDIRECT_HEXFRAME_MAX_SIZE 64
#endif
#endif  // defined(VEDIRECT_USE_HEXFRAME)

#if defined(VEDIRECT_USE_TEXTFRAME)
#define VEDIRECT_NAME_LEN 9
#define VEDIRECT_VALUE_LEN 33
#define VEDIRECT_RECORDS_COUNT 22
#endif  // defined(VEDIRECT_USE_TEXTFRAME)

#if defined(VEDIRECT_USE_HEXFRAME)
/// @brief  Helper class to manage HEX frames. It allows building an internal
/// binary representation and encoding/decoding
/// to the HEX format suitable for serial communication.
/// HexFrame works as a base implementation and needs to be overrided with
/// the correct (pre-reserved) storage implementation to avoid usual container dynamic reallocations
struct HexFrame {
 public:
  enum DecodeResult : int8_t {
    /// @brief Decoder status ok, need more data to complete the frame
    Continue = 0,
    /// @brief Hex frame decoding completed: checksum ok
    Valid = 1,
    /// @brief Need to call 'init' before decoding
    InitError = -1,
    /// @brief Buffer too long to be stored in actual HexFrame storage
    Overflow = -2,
    /// @brief Hex frame decoding completed: checksum not ok
    ChecksumError = -3,
    /// @brief Unexpected encoding in the buffer, decoding aborted
    CodingError = -4,
    /// @brief Special case for CodingError when the input stream is 'null' terminated
    Terminated = -5,
  };

  // Generic (raw) data accessors
  inline const HEXFRAME *record() const { return (HEXFRAME *) this->rawframe_begin_; }
  inline HEXFRAME *record() { return (HEXFRAME *) this->rawframe_begin_; }
  inline const uint8_t *begin() const { return this->rawframe_begin_; }
  inline const uint8_t *end() const { return this->rawframe_end_; }
  inline int capacity() const { return end_of_storage() - this->rawframe_begin_; }
  inline int size() const { return this->rawframe_end_ - this->rawframe_begin_; }

  inline const char *encoded() {
    if (this->needs_encoding_())
      this->encode_();
    return encoded_begin_;
  }
  inline const char *encoded() const { return encoded_begin_; }
  inline const char *encoded_end() {
    if (this->needs_encoding_())
      this->encode_();
    return encoded_end_;
  }
  inline const char *encoded_end() const { return encoded_end_; }
  inline int encoded_size() {
    if (this->needs_encoding_())
      this->encode_();
    return encoded_end_ - encoded_begin_;
  }
  inline int encoded_size() const { return encoded_end_ - encoded_begin_; }

  // Shortcut accessors for general HEX frames data.
  // Beware these are generally not 'safe' and heavily depends
  // on frame type
  uint8_t command() const { return *this->begin(); }
  group_id_t group_id() const { return *(uint8_t *) (this->begin() + 1); }
  register_id_t register_id() const { return *(uint16_t *) (this->begin() + 1); }
  uint8_t flags() const { return *(this->begin() + 3); }
  const uint8_t *data_begin() const { return this->begin() + 4; }
  const uint8_t *data_end() const { return this->end(); }
  int data_size() const { return this->size() - 4; }
  // FRAGILE: unchecked cast to data type: be careful for buffer overflow
  template<typename T> T data_t() const { return *(T *) this->data_begin(); }
  // size-checked cast to unsigned 32
  uint32_t safe_data_u32() const {
    switch (this->data_size()) {
      case 1:
        return this->data_t<uint8_t>();
      case 2:
        return this->data_t<uint16_t>();
      case 3:
        return this->data_t<uint32_t>() & 0x00FFFFFF;
      case 4:
        return this->data_t<uint32_t>();
      default:
        return 0;  // FRAGILE: invalid value though...
    }
  }
  const char *data_str() const { return (const char *) this->data_begin(); }
  /// @brief Safely extracts the 'raw' payload (i.e. the data past the register id and flags)
  /// prepending '0x' to the returned string
  bool data_to_hex(std::string &hexdata) const;
  /// @brief Safely extracts the 'raw' payload (i.e. the data past the register id and flags)
  /// prepending '0x' to the returned string. Ensure buf_size is at least 3 for the checks to work
  bool data_to_hex(char *buf, size_t buf_size) const;

  /// @brief Decode an HEXFRAME starting from a plain (encoded) HEX stream.
  /// This can be used to decode raw HEX streams and/or to add the checksum
  /// to partially encoded streams. When the input 'hexdigits' terminates on '\n'
  /// the decoder assumes the checksum is included in the unput stream and ignores
  /// 'addchecksum'
  /// @param hexdigits
  /// @param addchecksum
  /// @return
  DecodeResult decode(const char *hexdigits, bool addchecksum);

  // Helper 'encoder' methods: beware these will not check storage access
  // so that the caller has to be sure the allocated data size is enough to hold
  // the 'pumped in' data.

  /// @brief Builds a plain command frame payload
  /// @param command
  inline void command(HEXFRAME::COMMAND command) {
    this->rawframe_begin_[0] = command;
    this->rawframe_end_ = this->rawframe_begin_ + 1;
    this->encode_();
  }
  /// @brief Builds a command GET register frame
  /// @param register_id
  inline void command_get(register_id_t register_id) {
    auto record = this->record();
    record->command = HEXFRAME::COMMAND::Get;
    record->register_id = register_id;
    record->flags = 0;
    this->rawframe_end_ = this->rawframe_begin_ + 4;
    this->encode_();
  }
  void command_set(register_id_t register_id, const void *data, size_t data_size) {
    auto record = this->record();
    record->command = HEXFRAME::COMMAND::Set;
    record->register_id = register_id;
    record->flags = 0;
    memcpy(record->data, data, data_size);
    this->rawframe_end_ = this->rawframe_begin_ + 4 + data_size;
    this->encode_();
  }
  void command_set(register_id_t register_id, const void *data, HEXFRAME::DATA_TYPE data_type) {
    this->command_set(register_id, data, HEXFRAME::DATA_TYPE_TO_SIZE[data_type]);
  }
  template<typename T> void command_set(register_id_t register_id, T data) {
    auto record = this->record();
    record->command = HEXFRAME::COMMAND::Set;
    record->register_id = register_id;
    record->flags = 0;
    *(T *) record->data = data;
    this->rawframe_end_ = this->rawframe_begin_ + 4 + sizeof(T);
    this->encode_();
  }

  inline void push_back(uint8_t data) {
    // warning: unchecked boundary
    *this->rawframe_end_++ = data;
  }

  // Interface(s) for custom storage implementation
  virtual const uint8_t *end_of_storage() const = 0;
  virtual const char *encoded_end_of_storage() const = 0;

 protected:
  friend class HexFrameDecoder;

  HexFrame(uint8_t *rawframe_ptr, char *encoded_ptr)
      : rawframe_begin_(rawframe_ptr),
        rawframe_end_(rawframe_ptr),
        encoded_begin_(encoded_ptr),
        encoded_end_(encoded_ptr) {}
  uint8_t *const rawframe_begin_;
  uint8_t *rawframe_end_;

  char *const encoded_begin_;
  char *encoded_end_;

  inline void invalidate_encoding_() { this->encoded_end_ = this->encoded_begin_; }
  inline bool needs_encoding_() const { return this->encoded_end_ == this->encoded_begin_; }
  void encode_();
};

/// @brief Provides a static storage implementation for HexFrame
template<std::size_t HF_DATA_SIZE> struct HexFrameT : public HexFrame {
  // allocated (maximum) size of the 'raw' payload excluding command and checksum
  static constexpr size_t ALLOCATED_DATA_SIZE = HF_DATA_SIZE;
  // allocated (maximum) size of the hex encoded whole frame
  static constexpr size_t ALLOCATED_ENCODED_SIZE = 1 + 1 + HF_DATA_SIZE * 2 + 2 + 1 + 1;

  HexFrameT() : HexFrame(this->rawframe_, this->encoded_) {}

  const uint8_t *end_of_storage() const override { return this->rawframe_ + sizeof(this->rawframe_); }
  const char *encoded_end_of_storage() const override { return this->encoded_ + sizeof(this->encoded_); }

 protected:
  // raw buffer: contains COMMAND + DATA + TERMINATOR(0)
  uint8_t rawframe_[1 + HF_DATA_SIZE + 1];

  // buffer for decoded/encoded hex digits:
  // :[COMMAND][DATAHIGH][DATALOW][CHECKSUMHIGH][CHECKSUMLOW]\n\0
  char encoded_[ALLOCATED_ENCODED_SIZE]{":"};
};

/// @brief Helper for plain 'command' frames (no payload)
struct HexFrame_Command : public HexFrameT<0> {
 public:
  HexFrame_Command(HEXFRAME::COMMAND command) { this->command(command); }
};

/// @brief Helper for plain 'GET register' frames
struct HexFrame_Get : public HexFrameT<3> {
 public:
  HexFrame_Get(register_id_t register_id) { this->command_get(register_id); }
};

/// @brief Helper for plain 'SET register' frames
struct HexFrame_Set : public HexFrameT<7> {
 public:
  HexFrame_Set(register_id_t register_id, const void *data, HEXFRAME::DATA_TYPE data_type) {
    this->command_set(register_id, data, data_type);
  }
  template<typename T> HexFrame_Set(register_id_t register_id, T data) { this->command_set(register_id, data); }
};

class HexFrameDecoder {
 public:
  typedef HexFrame::DecodeResult Result;

  void init(HexFrame *hexframe) {
    this->checksum_ = 0x55;
    this->hinibble_ = false;
    this->hexframe_ = hexframe;
    this->rawframe_end_of_storage_ = hexframe->end_of_storage();
    hexframe->rawframe_end_ = hexframe->rawframe_begin_;
    *hexframe->rawframe_end_ = 0;
    hexframe->encoded_end_ = hexframe->encoded_begin_ + 1;
    *hexframe->encoded_end_ = 0;
  }

  /// @brief Parse an incoming byte stream of HEX digits and builds
  /// the hex frame attached to this parser through 'init'.
  /// Once any of the termination states is reached (error or frame end)
  /// the decoder needs to be reinitialized through 'init'.
  /// Termination is directly detected in the input stream by parsing
  /// an '\n' (newline) or '\0' (string termination). If the stream
  /// is not properly terminated an overflow will be detected once the
  /// input stream fills the hexframe buffers.
  /// The decoding is optimized to parse both VEDirect incoming byte stream
  /// which usually starts on ':' and ends on '\n'. When this is the case,
  /// the opening ':' must be eated by the external frame handler.
  /// The other use-case is when we want to build an HexFrame starting from an already
  /// encoded (hex) string so that we can eventually add the checksum. In this scenario,
  /// the stream might (should) end on '\0' and we could then easily add the checksum
  /// if needed. See 'HexFrame::decode'
  /// While parsing, the input hexdigit is also accumulated into the 'encoded_'
  /// buffer of the HexFrame so that it could come handy for later prints or so
  /// without the need to re-encode the frame itself.
  /// @param hexdigit
  /// @return the parsing status 'Result' after each iteration
  Result decode(char hexdigit) {
    Result result;
    auto hexframe = this->hexframe_;
    if (!hexframe)
      return Result::InitError;
    if (hexframe->rawframe_end_ >= this->rawframe_end_of_storage_) {
      result = Result::Overflow;
      goto decode_exit;
    }
    if ((hexdigit >= '0') && (hexdigit <= '9')) {
      *hexframe->encoded_end_++ = hexdigit;
      hexdigit -= '0';
    } else if ((hexdigit >= 'A') && (hexdigit <= 'F')) {
      *hexframe->encoded_end_++ = hexdigit;
      hexdigit -= 55;
    } else if (hexdigit == '\n') {
      *hexframe->encoded_end_++ = '\n';
      if (this->hinibble_) {
        // frame alignment ok
        if (this->checksum_) {
          result = Result::ChecksumError;
        } else {
          result = Result::Valid;
          // pops out the checksum from the rawdata
          --hexframe->rawframe_end_;
        }
      } else {
        result = Result::CodingError;
      }
      goto decode_exit;
    } else {
      result = ((hexdigit == 0) && this->hinibble_) ? Result::Terminated : Result::CodingError;
      goto decode_exit;
    }

    if (this->hinibble_) {
      this->hinibble_ = false;
      *hexframe->rawframe_end_ = hexdigit << 4;
    } else {
      this->hinibble_ = true;
      *hexframe->rawframe_end_ |= hexdigit;
      this->checksum_ -= *hexframe->rawframe_end_++;
    }
    return Result::Continue;

  decode_exit:
    *hexframe->rawframe_end_ = 0;
    *hexframe->encoded_end_ = 0;
    this->hexframe_ = nullptr;
    return result;
  }

  inline uint8_t get_checksum() { return this->checksum_; }

 protected:
  HexFrame *hexframe_{};
  bool hinibble_;
  uint8_t checksum_;
  const uint8_t *rawframe_end_of_storage_;
};

#endif  // defined(VEDIRECT_USE_HEXFRAME)

/** VEDirect frame handler:
 * This class needs to be overriden to get notification of relevant parsing events.
 * Feed in the raw data (from serial) to 'decode' and get events for
 * hex frame -> 'on_frame_hex_'
 * text frame -> 'on_frame_text_'
 * parsing errors (overflows, checksum, etc) -> on_frame_error_
 */
class FrameHandler {
 public:
  enum Error {
    CHECKSUM,
    CODING,
    OVERFLOW,
#if defined(VEDIRECT_USE_TEXTFRAME)
    NAME_OVERFLOW,
    VALUE_OVERFLOW,
    RECORD_OVERFLOW,
#endif
  };

  enum State {
    Idle,
    Hex,
#if defined(VEDIRECT_USE_TEXTFRAME)
    Name,
    Value,
    Checksum,
#endif
  };

#if defined(VEDIRECT_USE_HEXFRAME)
  typedef HexFrameT<VEDIRECT_HEXFRAME_MAX_SIZE> RxHexFrame;
#endif

#if defined(VEDIRECT_USE_TEXTFRAME)
  struct TextRecord {
    char name[VEDIRECT_NAME_LEN];
    char value[VEDIRECT_VALUE_LEN];
  };
#endif

  void reset() { this->frame_state_ = State::Idle; }
  void decode(uint8_t *data_begin, uint8_t *data_end);

 private:
  State frame_state_{State::Idle};

#if defined(VEDIRECT_USE_HEXFRAME)
  RxHexFrame hexframe_;
  HexFrameDecoder hexframe_decoder_;

  virtual void on_frame_hex_(const RxHexFrame &hexframe) {}
  virtual void on_frame_hex_error_(Error error) {}
#endif  // defined(VEDIRECT_USE_HEXFRAME)

#if defined(VEDIRECT_USE_TEXTFRAME)
  // Save current TEXT frame state if an HEX frame appears in the middle.
  // Current VEDirect docs say this is not the case anymore but older fw
  // could interlave HEX frames into TEXT frames.
  State frame_state_backup_;

  uint8_t text_checksum_;
  // This is a statically preallocated storage for incoming records
  // TextRecord(s) will be added 'on-demand' (and reused on every new frame parsing)
  // Having 'nullptr' means the record has not been allocated yet..simple as that.
  TextRecord *text_records_[VEDIRECT_RECORDS_COUNT]{};
  uint8_t text_records_count_;

  // buffered pointers to current text record parsing
  TextRecord *text_record_;
  char *text_record_write_;
  char *text_record_write_end_;
  inline void frame_text_name_start_() {
    TextRecord *text_record = this->text_records_[this->text_records_count_];
    if (!text_record)
      this->text_records_[this->text_records_count_] = text_record = new TextRecord;
    this->text_record_ = text_record;
    this->text_record_write_ = text_record->name;
    this->text_record_write_end_ = this->text_record_write_ + sizeof(this->text_record_->name);
  }
  inline void frame_text_value_start_() {
    // current text_record_ already in place since we were parsing the name
    this->text_record_write_ = this->text_record_->value;
    this->text_record_write_end_ = this->text_record_write_ + sizeof(this->text_record_->value);
  }
  virtual void on_frame_text_(TextRecord **text_records, uint8_t text_records_count) {}
  virtual void on_frame_text_error_(Error error) {}
#endif  // defined(VEDIRECT_USE_TEXTFRAME)

  inline void frame_hex_start_() {
#if defined(VEDIRECT_USE_HEXFRAME)
    this->hexframe_decoder_.init(&this->hexframe_);
#endif
#if defined(VEDIRECT_USE_TEXTFRAME)
    this->frame_state_backup_ = this->frame_state_;
#endif
    this->frame_state_ = State::Hex;
  }
};

}  // namespace m3_ve_reg
