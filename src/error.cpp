#include <cpplox/error.hpp>
#include <cpplox/variant.hpp>

namespace lox
{

inline namespace error
{

auto format_parse_error(const SyntaxError & error) -> std::string
{
  const std::string desc = "";
  return desc + ": at line " + std::to_string(error.line) + ", column " +
         std::to_string(error.column);
}

}  // namespace error
}  // namespace lox
