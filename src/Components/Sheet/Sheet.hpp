#pragma once
#include <filesystem>
#include <ctime>

struct NullCell {
  bool dummy;
};

struct DateTime {
  tm time;
  double serial;
};

// struct CellValue {
//
// };

struct Sheet {
  std::vector<std::string> headers;
  std::vector<
};

void ReadXLSXfile(const std::filesystem::path &path_);
