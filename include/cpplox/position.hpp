#pragma once

#include <optional>
#include <string>

namespace lox
{

inline namespace position
{

struct Line
{
  Line(
    const std::optional<std::string> & filename, const size_t number, const size_t start_index,
    const size_t end_index)
  : filename(filename), number(number), start_index(start_index), end_index(end_index)
  {
  }

  const std::optional<std::string> filename;  //!< this value is null in REPL
  const size_t number;                        //!< the line number(starting from 1, not index)
  const size_t start_index;  //!< the index of cursor position for the beginning of this line on the
                             //!< program string
  const size_t end_index;    //!< the index of cursor position for the end of this line on the
                             //!< program string
};
}  // namespace position
}  // namespace lox
