#pragma once
#include <ctime>
#include <filesystem>
#include <expected>

struct NullCell {
  bool dummy;
};

struct ErrorCell {
  std::string msg;
};

typedef std::variant<NullCell, ErrorCell, std::string, int64_t, double, bool,
                     tm>
    CellValue;

struct SheetRow {
  size_t offset;
  tm time;
};

struct Sheet {
  int width = 0;
  int height = 0;
  std::vector<std::string> headers;
  std::vector<CellValue> data;
  std::vector<SheetRow> rows;

  static void printcell2console(CellValue *cell);
};

std::expected<Sheet, bool> ReadXLSXfile(const std::filesystem::path &path_);
