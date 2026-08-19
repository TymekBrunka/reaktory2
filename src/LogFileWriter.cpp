#include <App.hpp>
#include <FileUtils.hpp>
#include <Logging.hpp>
#include <chrono>

#ifdef _WIN32
#define SEP "\\"
#else
#define SEP "/"
#endif

Log::write_fun App::log_file_writer =
    [](std::chrono::time_point<std::chrono::system_clock> timestamp,
       uint32_t user_lang, uint8_t tag, Log::translations_key_t message_idx,
       const char *context, bool is_formatted, std::format_args *args,
       void *data) {
      const std::chrono::year_month_day today{
          std::chrono::floor<std::chrono::days>(timestamp)};

      LogFileWriterData *self = (LogFileWriterData *)data;

      if (self->last_day < today) {
        self->file.close();
        self->file_en.close();
        self->filepath = std::format("{}" SEP "logs" SEP "{}.log",
                                     FileUtils::APP_ROOT.string(), today);
        self->filepath_en = std::format("{}" SEP "logs" SEP "{}.en.log",
                                     FileUtils::APP_ROOT.string(), today);
        self->file.open(self->filepath, std::ios_base::app);
        self->file_en.open(self->filepath_en, std::ios_base::app);
      }

      const char *log_level = "INFO";
      if (tag & Log::ERROR)
        log_level = "ERROR";
      else if (tag & Log::WARNING)
        log_level = "WARNING";
      else if (tag & Log::VERBOSE)
        log_level = "VERBOSE";
      else if (tag & Log::DEBUG)
        log_level = "DEBUG";

      const char *severity = "LOW";
      if (tag & Log::SEV_HIGH)
        severity = "HIGH";
      else if (tag & Log::SEV_MED)
        severity = "MEDIUM";

      self->file << "[" << timestamp << "][" << context << "][" << log_level << "]["
           << severity << "] "
           << LOG_FMT(user_lang, message_idx, Log::, *args, is_formatted)
           << std::endl;

      self->file_en << "[" << timestamp << "][" << context << "][" << log_level << "]["
           << severity << "] "
           << LOG_FMT(Log::LANG_EN, message_idx, Log::, *args, is_formatted)
           << std::endl;
    };
