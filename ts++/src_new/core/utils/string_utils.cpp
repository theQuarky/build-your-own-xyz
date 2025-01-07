#include "string_utils.h"
#include <algorithm>
#include <sstream>

namespace core::utils {

std::vector<String> StringUtils::split(const String &str, char delimiter) {
  std::vector<String> tokens;
  std::istringstream stream(str);
  String token;

  while (std::getline(stream, token, delimiter)) {
    tokens.push_back(token);
  }
  return tokens;
}

String StringUtils::join(const std::vector<String> &parts,
                         const String &delimiter) {
  if (parts.empty())
    return "";

  std::ostringstream result;
  result << parts[0];

  for (size_t i = 1; i < parts.size(); ++i) {
    result << delimiter << parts[i];
  }
  return result.str();
}

String StringUtils::trim(const String &str) {
  size_t first = str.find_first_not_of(" \t\n\r");
  if (first == String::npos)
    return "";

  size_t last = str.find_last_not_of(" \t\n\r");
  return str.substr(first, last - first + 1);
}

String StringUtils::replace(String str, const String &from, const String &to) {
  size_t pos = 0;
  while ((pos = str.find(from, pos)) != String::npos) {
    str.replace(pos, from.length(), to);
    pos += to.length();
  }
  return str;
}

bool StringUtils::startsWith(const String &str, const String &prefix) {
  return str.length() >= prefix.length() &&
         str.compare(0, prefix.length(), prefix) == 0;
}

bool StringUtils::endsWith(const String &str, const String &suffix) {
  return str.length() >= suffix.length() &&
         str.compare(str.length() - suffix.length(), suffix.length(), suffix) ==
             0;
}

} // namespace core::utils