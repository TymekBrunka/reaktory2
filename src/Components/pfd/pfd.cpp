#include <pfd.hpp>
#include <portable-file-dialogs.h>
namespace ipfd {

bool available() { return pfd::settings::available(); }

std::vector<std::string> open_file(std::string const &title,
                                   std::string const &default_path,
                                   std::vector<std::string> const &filters,
                                   opt options) {

  return pfd::open_file(title, default_path, filters, (pfd::opt)options)
      .result();
}

std::string save_file(std::string const &title, std::string const &initial_path,
                      std::vector<std::string> filters, opt option) {

  return pfd::save_file(title, initial_path, filters, (pfd::opt)option)
      .result();
}

std::string select_folder(std::string const &title,
                          std::string const &default_path, opt option) {

  return pfd::select_folder(title, default_path, (pfd::opt)option).result();
}

static pfd::icon icon2icon(icon icon_) {
  switch (icon_) {
  case icon::info:
    return pfd::icon::info;
  case icon::warning:
    return pfd::icon::warning;
  case icon::error:
    return pfd::icon::error;
  case icon::question:
    return pfd::icon::question;
  default:
    return pfd::icon::info;
  }
}

void notify(std::string const &title, std::string const &text, icon icon) {
  pfd::notify(title, text, icon2icon(icon));
}

} // namespace ipfd
