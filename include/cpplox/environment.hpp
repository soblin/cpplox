#pragma once
#include <cpplox/expression.hpp>

#include <boost/unordered/unordered_flat_map.hpp>

#include <string>
namespace lox
{
inline namespace environment
{
class Environment
{
public:
  Environment() = default;
  auto define(const std::string & var_name, const Value & var_value) -> void
  {
    values[var_name] = var_value;
  }

private:
  boost::unordered_flat_map<std::string, Value> values;
};

}  // namespace environment
}  // namespace lox
