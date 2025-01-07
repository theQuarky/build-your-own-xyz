#pragma once
#include <cstdint>
#include <memory>
#include <string>
#include <variant>

namespace core {

/**
 * Fixed-width unsigned integer types
 * Guarantees consistent sizes across different platforms
 */
using u8 = std::uint8_t;   // 8-bit unsigned integer
using u16 = std::uint16_t; // 16-bit unsigned integer
using u32 = std::uint32_t; // 32-bit unsigned integer
using u64 = std::uint64_t; // 64-bit unsigned integer

/**
 * Fixed-width signed integer types
 * Guarantees consistent sizes across different platforms
 */
using i8 = std::int8_t;   // 8-bit signed integer
using i16 = std::int16_t; // 16-bit signed integer
using i32 = std::int32_t; // 32-bit signed integer
using i64 = std::int64_t; // 64-bit signed integer

/**
 * Floating point types
 * f32: Single precision floating point
 * f64: Double precision floating point
 */
using f32 = float;
using f64 = double;

/**
 * String types
 * String: Owned string type
 * StringView: Non-owning string reference
 */
using String = std::string;
using StringView = std::string_view;

/**
 * @struct SourceLocation
 * @brief Tracks the location of a token or error in source code
 *
 * Used for error reporting and debugging to identify exact locations
 * in source files where issues occur.
 */
struct SourceLocation {
  String filename; // Name of the source file
  u32 line;        // Line number (1-based)
  u32 column;      // Column number (1-based)

  /**
   * @brief Constructs a new source location
   * @param f Filename
   * @param l Line number
   * @param c Column number
   */
  SourceLocation(String f, u32 l, u32 c)
      : filename(std::move(f)), line(l), column(c) {}
};

/**
 * @class Result
 * @brief Generic result type for operations that can fail
 *
 * Inspired by Rust's Result type, provides a way to handle operations
 * that can either succeed with a value or fail with an error.
 *
 * @tparam T The type of the success value
 * @tparam E The type of the error value
 */
template <typename T, typename E> class Result {
  std::variant<T, E> data; // Holds either success value or error

public:
  /**
   * @brief Constructs a successful result
   * @param value The success value
   */
  Result(T value) : data(std::move(value)) {}

  /**
   * @brief Constructs an error result
   * @param error The error value
   */
  Result(E error) : data(std::move(error)) {}

  /**
   * @brief Checks if the result is successful
   * @return true if contains success value, false if contains error
   */
  bool isOk() const { return data.index() == 0; }

  /**
   * @brief Checks if the result is an error
   * @return true if contains error, false if contains success value
   */
  bool isError() const { return data.index() == 1; }

  /**
   * @brief Gets the success value
   * @return Reference to the contained value
   * @warning Undefined behavior if result is error
   */
  T &unwrap() { return std::get<T>(data); }

  /**
   * @brief Gets the error value
   * @return Reference to the contained error
   * @warning Undefined behavior if result is success
   */
  E &getError() { return std::get<E>(data); }
};

} // namespace core