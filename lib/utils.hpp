#pragma once

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

/**
 * @brief Конвертирует тип, для которого определена операция ввода в std::string
 * @tparam T: тип
 * @param value: значение
 * @return std::string: выходная строка
 */
template <typename T>
inline std::string ToString(T value) {
  std::stringstream ss;
  ss << value;
  return ss.str();
}

bool IsFileExists(const std::string& file_name) {
  std::ifstream file(file_name.c_str());
  return file.good();
}

template <typename T>
int Sign(T x) {
  if (x > 0)
    return 1;
  else if (x < 0)
    return -1;
  else
    return 0;
}