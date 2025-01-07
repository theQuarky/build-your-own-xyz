#pragma once
#include "../common/common_types.h"
#include <vector>

namespace core::utils {

/**
 * @brief String manipulation utilities
 */
class StringUtils {
public:
  /**
   * @brief Splits string by delimiter
   */
  static std::vector<String> split(const String &str, char delimiter);

  /**
   * @brief Joins strings with delimiter
   */
  static String join(const std::vector<String> &parts, const String &delimiter);

  /**
   * @brief Trims whitespace from both ends
   */
  static String trim(const String &str);

  /**
   * @brief Replaces all occurrences of `from` with `to`
   */
  static String replace(String str, const String &from, const String &to);

  /**
   * @brief Checks if string starts with prefix
   */
  static bool startsWith(const String &str, const String &prefix);

  /**
   * @brief Checks if string ends with suffix
   */
  static bool endsWith(const String &str, const String &suffix);
};

} // namespace core::utils