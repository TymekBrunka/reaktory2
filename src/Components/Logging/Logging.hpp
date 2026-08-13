#pragma once
#include <core.hpp>
#include <cstdint>
#include <format>
#include <string_view>
namespace Log {

enum LOG_LANG {
  LANG_PL,
  LANG_EN,
};

extern std::string_view messages_pl[];
extern std::string_view messages_en[];

// using string = std::basic_string<char, std::char_traits<char>,
//                                  Allocators::RPmallocator<char>>;

enum LOG_TAG {
  INFO = 1u << 0,
  DEBUG = 1u << 1,
  VERBOSE = 1u << 2,
  WARNING = 1u << 3,
  ERROR = 1u << 4,

  SEV_LOW = 1u << 5,
  SEV_MED = 1u << 6,
  SEV_HIGH = 1u << 7,

  DEFAULT = INFO | SEV_LOW,
  ACCEPT_DEFAULTS = 255u & ~VERBOSE,
  ACCEPT_ALL = 255u,
};

// struct Message {
//   LOG_TAG tag;
//   uintptr_t additional_data;
//   const char *context;
//   std::string message;
// };

struct Callback {
  uint8_t tag = DEFAULT;
  void *data;
  void (*write)(std::chrono::time_point<std::chrono::system_clock> timestamp,
                uint32_t user_lang, uint8_t tag, uint32_t message_idx,
                const char *context, bool is_formatted, std::format_args *args,
                void *data);
};

class Logger {
private:
  uint32_t user_lang = 0;
  std::vector<Callback> callbacks;

public:
  Logger() = default;
  ~Logger() = default;

  static Logger *Global;

  inline Logger(uint32_t user_lang_, const std::vector<Callback> &callbacks_)
      : user_lang(user_lang_), callbacks(callbacks_) {};

  inline Logger(uint32_t user_lang_, std::vector<Callback> &&callbacks_)
      : user_lang(user_lang_), callbacks(callbacks_) {};

  void log(uint8_t tag, uintptr_t additional_data, const char *context,
           uint32_t message_idx, std::format_args &&args);

  void log_uform(uint8_t tag, uintptr_t additional_data, const char *context,
                 uint32_t message_idx);
};

inline void log(uint8_t tag, uintptr_t additional_data, const char *context,
                uint32_t message_idx, std::format_args &&args) {

  Logger::Global->log(tag, additional_data, context, message_idx,
                      std::move(args));
}

inline void log_uform(uint8_t tag, uintptr_t additional_data,
                      const char *context, uint32_t message_idx) {

  Logger::Global->log_uform(tag, additional_data, context, message_idx);
}

#define LOG_MSG(lang, idx, namespac)                                           \
  (((lang) == namespac LANG_PL ? namespac messages_pl                          \
                               : namespac messages_en)[(idx)])

#define LOG_FMT(lang, idx, namespac, args, do_fmt)                             \
  ((do_fmt) ? std::vformat(LOG_MSG(lang, idx, namespac), (args))              \
            : LOG_MSG(lang, idx, namespac))

extern Callback ConsoleLog_Callback;

} // namespace Log
