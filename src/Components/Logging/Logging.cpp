#include <Logging.hpp>
#include <iostream>
#include <thread>

#ifdef _WIN32
#include <process.h>
#else
#include <unistd.h>
#endif

namespace Log {

translations_map_t messages[2];

Logger *Logger::Global = nullptr;

Logger::Logger(uint32_t user_lang_, const std::vector<Callback> &callbacks_) {
  user_lang = user_lang_;
  callbacks = callbacks_;
#ifdef _WIN32
  pid = _getpid();
#else
  pid = getpid();
#endif
}

Logger::Logger(uint32_t user_lang_, std::vector<Callback> &&callbacks_) {
  user_lang = user_lang_;
  callbacks = callbacks_;
#ifdef _WIN32
  pid = _getpid();
#else
  pid = getpid();
#endif
}

void Logger::log(uint8_t tag, uintptr_t additional_data, const char *context,
                 translations_key_t message_idx, std::format_args &&args) {

  std::chrono::time_point<std::chrono::system_clock> now =
      std::chrono::system_clock::now();
  for (auto callback : callbacks) {
    if (callback.tag & tag)
      (*callback.write)(*this, now, user_lang, tag, message_idx, context, true,
                        &args, callback.data);
  }
}

void Logger::log_uform(uint8_t tag, uintptr_t additional_data,
                       const char *context, translations_key_t message_idx) {

  std::chrono::time_point<std::chrono::system_clock> now =
      std::chrono::system_clock::now();
  for (auto callback : callbacks) {
    if (callback.tag & tag)
      (*callback.write)(*this, now, user_lang, tag, message_idx, context, false,
                        nullptr, callback.data);
  }
}

Callback ConsoleLog_Callback = Callback{
    .tag = ACCEPT_DEFAULTS,
    .data = nullptr,
    .write = [](Logger &logger,
                std::chrono::time_point<std::chrono::system_clock> timestamp,
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
        log_level = "\x1b[35mDEBUG\x1b[0m";

      const char *severity = "LOW";
      if (tag & SEV_HIGH)
        severity = "\x1b[31mHIGH\x1b[0m";
      else if (tag & SEV_MED)
        severity = "\x1b[33mMEDIUM\x1b[0m";

      std::cerr << "[pid: " << logger.get_pid() << "][tid: "
                << std::hash<std::thread::id>{}(std::this_thread::get_id())
                << "][\x1b[33m" << timestamp << "\x1b[0m][\x1b[34m" << context
                << "\x1b[0m][" << log_level << "][" << severity << "] "
                << LOG_FMT(user_lang, message_idx, , *args, is_formatted)
                << "\n";
    }};

} // namespace Log
