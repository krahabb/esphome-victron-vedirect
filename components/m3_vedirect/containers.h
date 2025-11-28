#pragma once

#include <cstddef>
#include <cmath>
#include <string>

namespace esphome {
namespace m3_vedirect {

// fix wrong static_assert behavior in uninstantiated templates
// https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2023/p2593r1.html
// This would actually be needed only for ESP8266 toolchain (xtensa 10.3.0)
// while ESP32 toolchain seem to behave correctly (as of 2025-11-28).
template<typename T> struct _assert_false : std::false_type {};
#define TEMPLATE_MEMBER_NEED_OVERRIDE(_Targ, msg) \
  static_assert(_assert_false<_Targ>::value, msg " must be implemented by derived classes.")

// Basic functors used by our Map implementation.
/// @brief Hash (default) functor for numeric integral keys.
template<typename TKey> struct hash_default {
  constexpr size_t operator()(TKey key) const { return key; }
};
/// @brief Compare (default) functor for numeric integral keys.
template<typename TKey> struct compare_default {
  constexpr int operator()(TKey key_low, TKey key_high) const { return key_high - key_low; }
};

/// @brief A bucket for holding registers with the same hash (see TinyMap).
/// This abstract class provides the interface for a bucket implementation.
template<typename TKey, typename TValue, typename TBucket> class Bucket {
 public:
  using bucket_type = TBucket;
  using value_type = TValue;

  /// @brief prototype/default allocator for Bucket objects.
  /// @tparam TKey
  /// @tparam TValue
  /// @tparam TBucket
  class Allocator {
   public:
    using bucket_type = TBucket;

    bucket_type *alloc(TKey key, TValue value, bucket_type *next) const { return new bucket_type(key, value, next); }
    void dealloc(bucket_type *bucket) const { delete bucket; }

   protected:
    // Helper for specialized allocators
    void init_bucket_(bucket_type *bucket, TKey key, bucket_type *next) const { bucket->init_(key, next); }
  };
  friend class Allocator;
  using allocator_type = Allocator;

  TKey bucket_key() const { return this->bucket_key_; }
  value_type bucket_value() { TEMPLATE_MEMBER_NEED_OVERRIDE(TKey, "bucket_value"); }
  const value_type bucket_value() const { TEMPLATE_MEMBER_NEED_OVERRIDE(TKey, "bucket_value"); }
  bucket_type *bucket_next() const { return this->bucket_next_; }

  /// @brief Placeholder implementation for debug dumping
  std::string bucket_dump() const requires std::is_integral_v<TKey> { return std::to_string(this->bucket_key_); }

  std::string bucket_dump() const requires std::is_convertible_v<TKey, std::string> {
    return std::string(this->bucket_key_);
  }

 protected : Bucket() = default;
  Bucket(TKey key, TValue value, bucket_type *next) { TEMPLATE_MEMBER_NEED_OVERRIDE(TKey, "Bucket constructor"); };
  Bucket(TKey key, bucket_type *next) : bucket_key_(key), bucket_next_(next) {}

 private:
  template<size_t, typename, typename, typename, typename, typename, typename> friend class TinyMap;

  void init_(TKey key, bucket_type *next) {
    this->bucket_key_ = key;
    this->bucket_next_ = next;
  }

  TKey bucket_key_;
  bucket_type *bucket_next_;
};

/// @brief A Bucket implementation where the mapped object (TValue) is itself the bucket.
/// This way, TinyMap can avoid allocating separate bucket objects.
/// This needs to be used as a base class for actual TValue implementation.
/// @tparam TKey
/// @tparam TValue
template<typename TKey, typename TValue> class ValueBucket : public Bucket<TKey, TValue *, TValue> {
 public:
  using bucket_base_type = Bucket<TKey, TValue *, TValue>;
  using bucket_type = TValue;
  using value_type = TValue *;

  class Allocator : public bucket_base_type::Allocator {
   public:
    bucket_type *alloc(TKey key, TValue *value, bucket_type *next) const {
      // TValue itself inherits from Bucket so there's no need
      // to add any memory overhead for the bucket structure.
      this->init_bucket_(value, key, next);
      return value;
    }
    void dealloc(bucket_type *bucket) const {
      // nothing to do since we don't allocate anything dynamically
    }
  };
  using allocator_type = Allocator;

  value_type bucket_value() { return static_cast<TValue *>(this); }
  const value_type bucket_value() const { return static_cast<const TValue *>(this); }
};

/// @brief A (simple) Bucket implementation where the mapped object (TValue) is contained in the bucket.
/// This implementation only supports copy semantics, so TValue must be copyable and the class just implements
/// storage for it.
template<typename TKey, typename TValue> class SimpleBucket : public Bucket<TKey, TValue, SimpleBucket<TKey, TValue>> {
 public:
  using bucket_base_type = Bucket<TKey, TValue, SimpleBucket<TKey, TValue>>;
  using bucket_type = SimpleBucket<TKey, TValue>;
  using value_type = TValue;

  class Allocator : public bucket_base_type::Allocator {
   public:
    bucket_type *alloc(TKey key, TValue value, bucket_type *next) const { return new bucket_type(key, value, next); }
    void dealloc(bucket_type *bucket) const { delete bucket; }
  };
  using allocator_type = Allocator;

  value_type bucket_value() { return this->value_; }
  const value_type bucket_value() const { return this->value_; }

 protected:
  value_type value_;

  SimpleBucket(TKey key, TValue value, bucket_type *next) : bucket_base_type(key, next), value_(value) {}
};

/// @brief An optimized container for the set of HEX registers managed by the Manager.
/// This basically tries to split the lookup process through a fast hash map lookup and
/// a short linear search in case of collisions.
/// The number of buckets must be a power of 2 and is template defined so that
/// it can be adjusted at compile time. A value between 16 and 256 should be
/// reasonable with 256 being a bit memory hungry (1KB of RAM for the bucket array).
/// but will provide the fastest lookups with very few collisions. (considering HEX
/// registers have great variability on low nibble/byte)
template<size_t MAP_SIZE, typename TKey, typename TValue, typename TBucket = SimpleBucket<TKey, TValue>,
         typename THash = hash_default<TKey>, typename TCompare = compare_default<TKey>,
         typename TBucketAllocator = TBucket::allocator_type>
class TinyMap {
 public:
  using bucket_type = TBucket;
  using bucket_allocator_type = TBucketAllocator;
  using hash_type = THash;
  using compare_type = TCompare;
  using tinymap_type = TinyMap<MAP_SIZE, TKey, TValue, TBucket, THash, TCompare, TBucketAllocator>;
  static constexpr size_t map_size = MAP_SIZE;

  /// @brief An iterator for traversing the registers in the map.
  class iterator {
   public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = TBucket;
    using difference_type = std::ptrdiff_t;
    using pointer = value_type *;
    using reference = value_type &;
    using const_reference = value_type const &;

    iterator() : buckets_(nullptr), buckets_end_(nullptr), p_bucket_(nullptr) {}
    iterator(bucket_type **bucket_begin, bucket_type **bucket_end, bucket_type *p_bucket = nullptr)
        : buckets_(bucket_begin), buckets_end_(bucket_end), p_bucket_(p_bucket) {
      this->advance_to_valid_();
    }

    reference operator*() { return *this->p_bucket_; }
    const_reference operator*() const { return *this->p_bucket_; }
    pointer operator->() const { return this->p_bucket_; }

    // Prefix increment
    iterator &operator++() {
      if (this->p_bucket_) {
        this->p_bucket_ = this->p_bucket_->bucket_next();
        if (!this->p_bucket_)
          this->advance_to_valid_();
      }
      return *this;
    }

    friend bool operator==(const iterator &a, const iterator &b) { return a.p_bucket_ == b.p_bucket_; }
    friend bool operator!=(const iterator &a, const iterator &b) { return a.p_bucket_ != b.p_bucket_; }

    // our friendly interface to avoid instantianing an iterator::end() object
    bool is_end() const { return this->p_bucket_ == nullptr; }

   private:
    bucket_type **buckets_;
    bucket_type **buckets_end_;
    bucket_type *p_bucket_;

    void advance_to_valid_() {
      while (!this->p_bucket_ && (this->buckets_ < this->buckets_end_)) {
        this->p_bucket_ = *(this->buckets_++);
      }
    }
  };
  // We use a fixed size array of buckets to avoid dynamic memory allocation
  // and keep things simple. Each bucket contains a linked list of registers so that
  // we can handle collisions through a (supposedly) short linear search.

  static_assert((MAP_SIZE > 0) && (MAP_SIZE & (MAP_SIZE - 1)) == 0, "TinyMap: MAP_SIZE must be a power of 2");

  inline static size_t hash(TKey key) {
    THash hasher;
    return hasher(key) & (MAP_SIZE - 1);
  }
  inline static int compare(TKey key_low, TKey key_high) {
    TCompare comparer;
    return comparer(key_low, key_high);
  }

  TinyMap() = default;
  ~TinyMap() { clear(); }

  size_t size() const { return this->size_; }
  bool empty() const { return this->size_ == 0; }

  iterator begin() { return iterator(&this->buckets_[0], &this->buckets_[MAP_SIZE]); }
  iterator end() { return iterator(); }
  const iterator cbegin() const { return iterator(&this->buckets_[0], &this->buckets_[MAP_SIZE]); }
  const iterator cend() const { return iterator(); }

  void clear() {
    for (size_t i = 0; i < MAP_SIZE; ++i) {
      bucket_type *bucket = this->buckets_[i];
      while (bucket) {
        bucket_type *next = bucket->bucket_next();
        this->bucket_allocator_.dealloc(bucket);
        bucket = next;
      }
      this->buckets_[i] = nullptr;
    }
    this->size_ = 0;
  }

  bucket_type *find(TKey key) const {
    for (auto bucket = this->buckets_[hash(key)]; bucket; bucket = bucket->bucket_next_) {
      auto cmp = compare(key, bucket->bucket_key_);
      if (cmp == 0) {
        return bucket;
      } else if (cmp > 0) {
        return nullptr;
      }
    }
    return nullptr;
  }

  void insert(TKey key, TValue value) {
    bucket_type **bucket = &this->buckets_[hash(key)];
    for (; *bucket; bucket = &(*bucket)->bucket_next_) {
      if (compare(key, (*bucket)->bucket_key_) >= 0)
        break;
    }
    *bucket = this->bucket_allocator_.alloc(key, value, *bucket);
    ++this->size_;
  }

  // statistics/debugging
  bool bucket_empty(size_t bucket_index) const {
    if (bucket_index < MAP_SIZE) {
      return this->buckets_[bucket_index] == nullptr;
    }
    return true;
  }

  std::string bucket_dump(size_t bucket_index) const {
    std::string result;
    if (bucket_index < MAP_SIZE) {
      result += "Bucket[" + std::to_string(bucket_index) + "] -> ";
      for (bucket_type *bucket = this->buckets_[bucket_index]; bucket; bucket = bucket->bucket_next_) {
        result += "[" + bucket->bucket_dump() + "]";
      }
    }
    return result;
  }

  struct stats {
    size_t num_buckets{0};
    size_t num_elements{0};
    size_t max_chain_length{0};
    float load_factor{0.0f};
    float fill_factor{0.0f};  // number of used buckets in the main array
    float load_average{0.0f};
    float load_variance{0.0f};
    float load_stddev{0.0f};
  };

  stats get_stats() const {
    stats result;
    result.num_buckets = MAP_SIZE;
    result.num_elements = this->size_;
    result.load_factor = static_cast<float>(this->size_) / static_cast<float>(MAP_SIZE);
    size_t used_buckets = 0;
    float total_chain_energy = 0.0f;
    for (size_t i = 0; i < MAP_SIZE; ++i) {
      size_t chain_length = 0;
      for (bucket_type *bucket = this->buckets_[i]; bucket; bucket = bucket->bucket_next_) {
        ++chain_length;
      }
      if (chain_length) {
        ++used_buckets;
        if (chain_length > result.max_chain_length) {
          result.max_chain_length = chain_length;
        }
        total_chain_energy += chain_length * chain_length;
      }
    }
    if (used_buckets) {
      result.fill_factor = static_cast<float>(used_buckets) / static_cast<float>(MAP_SIZE);
      result.load_average = static_cast<float>(this->size_) / static_cast<float>(used_buckets);
      result.load_variance =
          total_chain_energy / static_cast<float>(used_buckets) - result.load_average * result.load_average;
      result.load_stddev = std::sqrt(result.load_variance);
    }
    return result;
  }

 private:
  bucket_type *buckets_[MAP_SIZE] = {};
  size_t size_{0};
  bucket_allocator_type bucket_allocator_{};
};

}  // namespace m3_vedirect
}  // namespace esphome
