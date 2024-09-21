#include <cpplox/error.hpp>
#include <cpplox/variant.hpp>

namespace lox
{

inline namespace error
{

auto format_parse_error(const ParseError & error) -> std::string
{
  return std::visit(
    visit_variant{
      [](const InvalidCharacterError & e) {
        return "InvalidCharacterError at line " + std::to_string(e.line) + ", " + e.character;
      },
      [](const NonTerminatedStringError & e) {
        return "NonTerminatedStringError at line " + std::to_string(e.line) + ", " + e.str;
      },
      [](const NonTerminatedNumberError & e) {
        return "NonTerminatedNumberError at line " + std::to_string(e.line) + ", " + e.str;
      },
      [](const InvalidNumberError & e) {
        return "InvalidNumberError at line " + std::to_string(e.line) + ", " + e.str;
      },
      [](const InvalidIdentifierError & e) {
        return "InvalidIdentifierError at line " + std::to_string(e.line) + ", " + e.str;
      },
    },
    error);
}
}  // namespace error
}  // namespace lox
