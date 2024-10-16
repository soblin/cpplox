#pragma once
#include <cstddef>
#include <cstdint>
#include <limits>

namespace lox
{

inline namespace system
{

static constexpr size_t max_recursion_limit = std::numeric_limits<uint16_t>::max() - 1;
static constexpr size_t max_argument_size = std::numeric_limits<uint8_t>::max() - 1;

}  // namespace system
}  // namespace lox
