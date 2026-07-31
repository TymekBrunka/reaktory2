#include <Arena.hpp>
#include <cstdint>
#include <cstdlib>
#include <cstring>
namespace Allocators {

Arena::Arena(uint32_t page_size, _Float16 growth_factor) {
  this->page_size = page_size;
  this->growth_factor = growth_factor;
  offset = 0;
  memory = (unsigned char *)calloc(1, 4096);
  next = nullptr;
}

Arena::~Arena() {
  if (memory)
    free(memory);
  if (next)
    delete next;
}

Arena::Arena(Arena &&other) noexcept {
  growth_factor = other.growth_factor;
  page_size = other.page_size;
  offset = other.offset;
  memory = other.memory;
  next = other.next;
  larger_blobs = std::move(larger_blobs);
  other.memory = nullptr;
  other.next = nullptr;
}

Arena &Arena::operator=(Arena &&other) noexcept {
  if (this != &other) {
    growth_factor = other.growth_factor;
    page_size = other.page_size;
    offset = other.offset;
    memory = other.memory;
    next = other.next;
    larger_blobs = std::move(larger_blobs);
    other.memory = nullptr;
    other.next = nullptr;
  }
  return *this;
}

void *Arena::alloc(uint32_t size) {
  for (Arena *arena = this; arena != nullptr; arena = arena->next) {
    if (arena->get_page_free_size() <= size) {
      arena->offset += size;
      return memory + arena->offset - size;
    }
  }
  return nullptr;
}

void Arena::attempt_free(void *ptr, uint32_t size) {
  for (Arena *arena = this; arena != nullptr; arena = arena->next) {
    if (ptr >= arena->memory && (unsigned char *)ptr + size == (arena->memory + offset)) {
      memset(ptr, 0, size);
      arena->offset -= size;
    }
  }
}

} // namespace Allocators
