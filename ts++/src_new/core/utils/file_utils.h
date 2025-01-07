#pragma once
#include "../common/common_types.h"
#include <filesystem>
#include <optional>

namespace core::utils {

/**
 * @brief File utility functions
 */
class FileUtils {
public:
  /**
   * @brief Reads entire file content into string
   * @return File content or empty optional if failed
   */
  static std::optional<String> readFile(const String &path);

  /**
   * @brief Writes string content to file
   * @return True if successful
   */
  static bool writeFile(const String &path, const String &content);

  /**
   * @brief Checks if file exists and is readable
   */
  static bool exists(const String &path);

  /**
   * @brief Gets file extension
   */
  static String getExtension(const String &path);

  /**
   * @brief Creates directories recursively
   */
  static bool createDirectories(const String &path);
};

} // namespace core::utils