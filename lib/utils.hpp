#pragma once

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

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

inline bool IsFileExists(const std::string& file_name) {
  std::ifstream file(file_name.c_str());
  return file.good();
}

template <typename T>
inline int Sign(T x) {
  if (x > 0)
    return 1;
  else if (x < 0)
    return -1;
  else
    return 0;
}

inline void PrintVector(const std::vector<int>& vec) {
  printf("[");
  for (size_t i = 0; i < vec.size(); i++) printf("%d ", vec[i]);
  printf("]");
}

template <typename T>
inline T Sum(const std::vector<T>& vec) {
  T res = 0;
  for (size_t i = 0; i < vec.size(); i++) res += vec[i];
  return res;
}