#pragma once
#include <Arena.hpp>
#include <string>
namespace Log {
using Arena = Allocators::Arena;

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
  const char* context;
  std::basic_string<char, char, Allocators::ArenaAllocator<char>> message;
};

class MessageQueue {
  Arena *public_data;
  Arena *private_data;
  std::vector<Message> public_queue;
  std::vector<Message> private_queue;
};

} // namespace Log
