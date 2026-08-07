#pragma once
#include "concurrentqueue.h"
#include <RPmallocator.hpp>
#include <core.hpp>
namespace Log {

using string = std::basic_string<char, std::char_traits<char>,
                                 Allocators::RPmallocator<char>>;

enum LOG_TAG {
  LOG_INFO = 1u << 0,
  LOG_DEBUG = 1u << 1,
  LOG_VERBOSE = 1u << 2,
  LOG_WARNING = 1u << 3,
  LOG_ERROR = 1u << 4,

  LOG_SEVERITY_LOW = 1u << 5,
  LOG_SEVERITY_MEDIUM = 1u << 6,
  LOG_SEVERITY_HIGH = 1u << 7,

  LOG_DEFAULT_TAG = LOG_INFO | LOG_SEVERITY_LOW
};

struct Message {
  LOG_TAG tag;
  const char *context;
  string message;
};

class MessageQueue {
  moodycamel::ConcurrentQueue<Message> messages;
};

} // namespace Log
