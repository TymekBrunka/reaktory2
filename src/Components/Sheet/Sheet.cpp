#include "XLCellValue.hpp"
#include "XLDateTime.hpp"
#include "XLDocument.hpp"
#include "XLSheet.hpp"
#include "XLStyles.hpp"
#include "XLXmlParserForwardDeclarations.hpp"
#include <OpenXLSX.hpp>
#include <Sheet.hpp>
#include <ctime>
#include <iostream>
#include <string>
#include <vector>

// // automated with python, reference:
// //
// https://c-rex.net/samples/ooxml/e1/Part4/OOXML_P4_DOCX_numFmt_topic_ID0EHDH6.html
// static uint32_t internal_nrfmt_ids_with_date_or_time[] = {
//     14, 15, 16, 17, 18, 19, 20, 21, 22, 38, 40, 45, 46, 47, 27, 28, 29,
//     30, 31, 32, 33, 34, 35, 36, 50, 51, 52, 53, 54, 55, 56, 57, 58, 81};

void Sheet::printcell2console(CellValue *cell) {
  if (const NullCell *value = std::get_if<NullCell>(cell)) {
    std::cerr << "...";
  } else if (const ErrorCell *value = std::get_if<ErrorCell>(cell)) {
    std::cerr << "(Error: " << value->msg << ")";
  } else if (const std::string *value = std::get_if<std::string>(cell)) {
    std::cerr << value->c_str();
  } else if (const int64_t *value = std::get_if<int64_t>(cell)) {
    std::cerr << (int)*value;
  } else if (const double *value = std::get_if<double>(cell)) {
    std::cerr << *value;
  } else if (const bool *value = std::get_if<bool>(cell)) {
    std::cerr << (*value ? "true" : "false");
  } else if (const tm *value = std::get_if<tm>(cell)) {
    static char dtbuf[50] = {0};
    if (value->tm_mday == 0)
      strftime(dtbuf, sizeof(dtbuf), "%H:%M:%S", value);
    else
      strftime(dtbuf, sizeof(dtbuf), "%Y-%m-%d %H:%M:%S", value);
    std::cerr << dtbuf;
  }
}

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

std::expected<Sheet, bool> ReadXLSXfile(const std::filesystem::path &path_, int sheetidx) {

  OpenXLSX::XLDocument doc;
  doc.open(path_.string());
  std::vector<datetime_identity> datetime_ids;
  Sheet sheet{.idx = sheetidx};

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

  OpenXLSX::XLWorksheet wbk = doc.workbook().worksheet(1);
  for (int i = 2; wbk.visibility() != OpenXLSX::XLSheetState::Visible; i++)
    wbk = std::move(doc.workbook().worksheet(i));
  int width = wbk.columnCount(), height = wbk.rowCount();
  std::cerr << "   \n";
  std::cerr << "   size: " << width << "x" << height << "\n";

  // find first header cell (assuming there's nothing before the table start)
  int startr = 0;
  int startc = 0;
  bool foundstart = false;
  for (const auto &row : wbk.rows(height)) {
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

  if (!foundstart) {
    doc.close();
    return std::unexpected(false);
  }

  int lasthidx = startc - 1;
  for (const auto &header : wbk.row(startr).cells(startc, width)) {
    if (header.value().type() != OpenXLSX::XLValueType::String)
      break;

    lasthidx++;
    sheet.headers.push_back(header.value().getString());
  }

  std::cerr << "start: (" << startc << ", " << startr
            << ") end.x = " << lasthidx << "\n";

  sheet.width = lasthidx - startc;
  if (sheet.width <= 0) {
    doc.close();
    return std::unexpected(false);
  }

  for (auto &row : wbk.rows(startr + 1, height)) {
    if (!row.cellCount())
      break;

    bool skiprow = true;

    const auto &dtcell = row.findCell(startc);
    if (!dtcell.empty() &&
        dtcell.value().type() == OpenXLSX::XLValueType::Float &&
        is_datetime(doc, datetime_ids, dtcell)) {
      double dt = dtcell.value().get<double>();
      tm d = OpenXLSX::XLDateTime(dt <= 1 ? dt + 1 : dt).tm();
      if (dt > 1) {
        sheet.rows.push_back(SheetRow{.offset = sheet.data.size()});
        sheet.rows[sheet.rows.size() - 1].time = d;
        skiprow = false;
      }
    }

    if (skiprow)
      continue;

    for (const auto &cell : row.cells(startc + 1, lasthidx)) {

      auto type = cell.value().type();

      CellValue cv;
      switch (type) {
      case OpenXLSX::XLValueType::Empty:
        cv = NullCell{};
        break;
      case OpenXLSX::XLValueType::Error:
        cv = ErrorCell{.msg = std::move(cell.value().get<std::string>())};
        break;
      case OpenXLSX::XLValueType::String:
        cv = std::move(cell.value().get<std::string>());
        break;
      case OpenXLSX::XLValueType::Integer:
        cv = cell.value().get<int64_t>();
        break;
      case OpenXLSX::XLValueType::Float:
        if (is_datetime(doc, datetime_ids, cell)) {
          double dt = cell.value().get<double>();
          tm d = OpenXLSX::XLDateTime(dt <= 1 ? dt + 1 : dt).tm();
          if (dt <= 1)
            d.tm_mday = 0;
          cv = d;
          break;
        } else {
          cv = cell.value().get<double>();
          break;
        }
        break;
      case OpenXLSX::XLValueType::Boolean:
        cv = cell.value().get<bool>();
        break;
      }

      sheet.data.push_back(cv);
    }
    sheet.height++;
  }

  // for (int h = 0; h < sheet.height; h++) {
  //   static char dtbuf[50] = {0};
  //   tm *value = &sheet.rows[h].time;
  //   strftime(dtbuf, sizeof(dtbuf), "%Y-%m-%d %H:%M:%S", value);
  //   std::cerr << dtbuf << " | ";
  //
  //   for (int w = 0; w < sheet.width; w++) {
  //     Sheet::printcell2console(&sheet.data[sheet.rows[h].offset + w]);
  //     std::cerr << " | ";
  //   }
  //   std::cerr << "\n";
  // }
  std::cerr << "------\n";

  doc.close();
  return sheet;
}
