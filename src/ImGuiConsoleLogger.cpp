#include <App.hpp>
#include <FileUtils.hpp>
#include <Logging.hpp>
#include <chrono>
#include <format>

Log::write_fun App::log_imgui_writer =
    [](Log::Logger &logger,
       std::chrono::time_point<std::chrono::system_clock> timestamp,
       uint32_t user_lang, uint8_t tag, Log::translations_key_t message_idx,
       const char *context, bool is_formatted, std::format_args *args,
       void *data) {
      LogFileWriterData *self = (LogFileWriterData *)data;

      LogMessage log{
          .tag = tag,
          .context = context,
          .timestamp = std::format("{}", timestamp),
      };

      log.message = LOG_FMT(user_lang, message_idx, Log::, *args, is_formatted);
      self->logs.push_back(std::move(log));
    };
