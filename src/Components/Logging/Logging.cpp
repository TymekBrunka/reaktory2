#include <Logging.hpp>
#include <iostream>
namespace Log {

translations_map_t messages[2];

Logger *Logger::Global = nullptr;

void Logger::log(uint8_t tag, uintptr_t additional_data, const char *context,
                 translations_key_t message_idx, std::format_args &&args) {

  std::chrono::time_point<std::chrono::system_clock> now =
      std::chrono::system_clock::now();
  for (auto callback : callbacks) {
    if (callback.tag & tag)
      (*callback.write)(now, user_lang, tag, message_idx, context, true, &args,
                        callback.data);
  }
}

void Logger::log_uform(uint8_t tag, uintptr_t additional_data,
                       const char *context, translations_key_t message_idx) {

  std::chrono::time_point<std::chrono::system_clock> now =
      std::chrono::system_clock::now();
  for (auto callback : callbacks) {
    if (callback.tag & tag)
      (*callback.write)(now, user_lang, tag, message_idx, context, false,
                        nullptr, callback.data);
  }
}

Callback ConsoleLog_Callback = Callback{
    .tag = ACCEPT_DEFAULTS,
    .data = nullptr,
    .write = [](std::chrono::time_point<std::chrono::system_clock> timestamp,
                uint32_t user_lang, uint8_t tag, translations_key_t message_idx,
                const char *context, bool is_formatted, std::format_args *args,
                void *data) {
      const char *log_level = "INFO";
      if (tag & ERROR)
        log_level = "\x1b[31mERROR\x1b[0m";
      else if (tag & WARNING)
        log_level = "\x1b[33mWARNING\x1b[0m";
      else if (tag & VERBOSE)
        log_level = "\x1b[39mVERBOSE\x1b[0m";
      else if (tag & DEBUG)
        log_level = "\x1b[38mDEBUG\x1b[0m";

      const char *severity = "LOW";
      if (tag & SEV_HIGH)
        severity = "\x1b[31mHIGH\x1b[0m";
      else if (tag & SEV_MED)
        severity = "\x1b[33mMEDIUM\x1b[0m";

      std::cerr << "[\x1b[33m" << timestamp << "\x1b[0m][" << context << "]["
                << log_level << "][" << severity << "] "
                << LOG_FMT(user_lang, message_idx, , *args, is_formatted)
                << "\n";
    }};

} // namespace Log
