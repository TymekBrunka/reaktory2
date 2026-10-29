#include "XLDocument.hpp"
#include "XLSheet.hpp"
#include <OpenXLSX.hpp>
#include <Sheet.hpp>
#include <iostream>

void ReadXLSXfile(const std::filesystem::path &path_) {
  OpenXLSX::XLDocument doc;
  doc.open(path_.string());

  std::cerr << "--- " << doc.workbook().worksheetCount()
            << " worksheets to load ---\n";
  for (const auto &name : doc.workbook().worksheetNames()) {
    OpenXLSX::XLWorksheet sheet = doc.workbook().worksheet(name);
    std::cerr << ">> " << name << "\n";
    int width = sheet.columnCount(), height = sheet.rowCount();
    std::cerr << "   \n";
    std::cerr << "   size: " << width << "x" << height << "\n";

    for (auto &row : sheet.rows(height)) {
      std::cerr << "   ";
      for (auto cell : row.cells(width)) {
        std::cerr << cell.value() << " | ";
      }
      std::cerr << "\n";
    }
    std::cerr << "------\n";
  }

  doc.close();
}
