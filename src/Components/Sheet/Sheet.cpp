#include "XLCellValue.hpp"
#include "XLDocument.hpp"
#include "XLSheet.hpp"
#include "XLStyles.hpp"
#include <OpenXLSX.hpp>
#include <Sheet.hpp>
#include <iostream>
#include <string>
#include <vector>

// // automated with python, reference:
// //
// https://c-rex.net/samples/ooxml/e1/Part4/OOXML_P4_DOCX_numFmt_topic_ID0EHDH6.html
// static uint32_t internal_nrfmt_ids_with_date_or_time[] = {
//     14, 15, 16, 17, 18, 19, 20, 21, 22, 38, 40, 45, 46, 47, 27, 28, 29,
//     30, 31, 32, 33, 34, 35, 36, 50, 51, 52, 53, 54, 55, 56, 57, 58, 81};

struct datetime_identity {
  bool is_datetime = false;
  uint32_t id = 0;
};

static bool is_datetime(OpenXLSX::XLDocument &doc,
                        std::vector<datetime_identity> &datetime_ids,
                        const OpenXLSX::XLCell &cell) {
  uint32_t id = doc.styles()
                    .cellFormats()
                    .cellFormatByIndex(cell.cellFormat())
                    .numberFormatId();

  // compare against ms internal number format id's
  if ((id >= 14 && id <= 22) || (id >= 27 && id <= 36) || id == 38 ||
      id == 40 || (id >= 45 && id <= 47) || (id >= 50 && id <= 58) || id == 81)
    return true;

  bool is_custom_fmt = false;
  OpenXLSX::XLStyleIndex stylindex = 0;
  const auto &numFmts = doc.styles().numberFormats();
  for (size_t i = 0; i < numFmts.count(); i++)
    if (id == numFmts.numberFormatByIndex((OpenXLSX::XLStyleIndex)i)
                  .numberFormatId()) {
      is_custom_fmt = true;
      stylindex = (OpenXLSX::XLStyleIndex)i;
      break;
    }

  if (!is_custom_fmt)
    return false;

  for (datetime_identity numFmtId : datetime_ids) {
    if (id == numFmtId.id)
      return numFmtId.is_datetime;
  }

  const std::string &code = numFmts.numberFormatByIndex(stylindex).formatCode();
  for (char c : code) {
    if (c == 'y' || c == 'm' || c == 'd' || c == 'h' || c == 's' || c == 'e') {
      datetime_ids.push_back(datetime_identity{true, id});
      return true;
    }
  }

  datetime_ids.push_back(datetime_identity{false, id});
  return false;
}

void ReadXLSXfile(const std::filesystem::path &path_) {
  OpenXLSX::XLDocument doc;
  doc.open(path_.string());
  std::vector<datetime_identity> datetime_ids;

  std::cerr << "=== document has " << doc.styles().numberFormats().count()
            << " number formats.\n";

  for (size_t i = 0; i < doc.styles().numberFormats().count(); i++) {
    const auto &format = doc.styles().numberFormats().numberFormatByIndex(
        (OpenXLSX::XLStyleIndex)i);
    std::cerr << format.formatCode() << "\n";
  }

  std::cerr << "\n";
  std::cerr
      << "--- " << doc.workbook().worksheetCount()
      << " worksheets to load (we're going to use only 1st one anyway)---\n";
  OpenXLSX::XLWorksheet sheet = doc.workbook().worksheet(1);
  int width = sheet.columnCount(), height = sheet.rowCount();
  std::cerr << "   \n";
  std::cerr << "   size: " << width << "x" << height << "\n";

  // for (const auto &row : sheet.rows(height)) {
  //   std::cerr << "   ";
  //   for (const auto &cell : row.cells(width)) {
  //     std::cerr << cell.value().typeAsString() << " | ";
  //   }
  //   std::cerr << "\n";
  // }

  for (const auto &row : sheet.rows(height)) {
    std::cerr << "   ";
    for (const auto &cell : row.cells(width)) {
      if (cell.value().type() == OpenXLSX::XLValueType::Float ||
          cell.value().type() == OpenXLSX::XLValueType::Integer)
        std::cerr << cell.value().getString() << "\\"
                  << is_datetime(doc, datetime_ids, cell) << " | ";
      else {
        std::cerr << cell.value().getString() << " | ";
      }
    }
    std::cerr << "\n";
  }

  // find first header cell (assuming there's nothing before the table start)
  int startr = 0;
  int startc = 0;
  bool foundstart = false;
  for (const auto &row : sheet.rows(height)) {
    startr++;
    if (!row.cellCount())
      continue;

    startc = 0;
    for (const auto &cell : row.cells(width)) {
      startc++;
      if (cell.value().type() == OpenXLSX::XLValueType::String) {
        foundstart = true;
        break;
      }
    }
    if (foundstart)
      break;
  }

  if (!foundstart)
    return;

  int lasthidx = startc - 1;
  std::vector<std::string> headers;
  for (const auto &header : sheet.row(startr).cells(startc, width)) {
    if (header.value().type() != OpenXLSX::XLValueType::String)
      break;

    lasthidx++;
    headers.push_back(header.value().getString());
  }

  // for (const auto &row : sheet.rows(startr + 1, height)) {
  //   std::cerr << "   ";
  //   for (const auto &cell : row.cells(startc, lasthidx)) {
  //     std::cerr << cell.value() << " | ";
  //     // std::string a = cell.value().getString();
  //   }
  //   std::cerr << "\n";
  //   if (!row.cellCount())
  //     break;
  // }
  std::cerr << "------\n";

  doc.close();
}
