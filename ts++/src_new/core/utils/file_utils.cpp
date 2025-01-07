#include "file_utils.h"
#include <fstream>
#include <sstream>

namespace core::utils {

std::optional<String> FileUtils::readFile(const String &path) {
  std::ifstream file(path);
  if (!file)
    return std::nullopt;

  std::stringstream buffer;
  buffer << file.rdbuf();
  return buffer.str();
}

bool FileUtils::writeFile(const String &path, const String &content) {
  std::ofstream file(path);
  if (!file)
    return false;

  file << content;
  return file.good();
}

bool FileUtils::exists(const String &path) {
  return std::filesystem::exists(path);
}

String FileUtils::getExtension(const String &path) {
  auto pos = path.find_last_of('.');
  return pos == String::npos ? "" : path.substr(pos + 1);
}

bool FileUtils::createDirectories(const String &path) {
  return std::filesystem::create_directories(path);
}

} // namespace core::utils