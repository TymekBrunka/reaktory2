#pragma once
#include <cstdint>
#include <string>
#include <vector>
namespace ipfd {

enum class opt : uint8_t {
  none = 0,
  // For file open, allow multiselect.
  multiselect = 0x1,
  // For file save, force overwrite and disable the confirmation dialog.
  force_overwrite = 0x2,
  // For folder select, force path to be the provided argument instead
  // of the last opened directory, which is the Microsoft-recommended,
  // user-friendly behaviour.
  force_path = 0x4,
};

enum class icon {
  info = 0,
  warning,
  error,
  question,
};

inline opt operator|(opt a, opt b) { return opt(uint8_t(a) | uint8_t(b)); }
inline bool operator&(opt a, opt b) { return bool(uint8_t(a) & uint8_t(b)); }

bool available();

std::vector<std::string>
open_file(std::string const &title, std::string const &default_path = "",
          std::vector<std::string> const &filters = {"All Files", "*"},
          opt options = opt::none);

std::string save_file(std::string const &title, std::string const &initial_path,
                      std::vector<std::string> filters = {"All Files", "*"},
                      opt option = opt::none);

std::string select_folder(std::string const &title,
                          std::string const &default_path = "",
                          opt option = opt::none);

void notify(std::string const &title, std::string const &text,
            icon icon = icon::info);

} // namespace ipfd
