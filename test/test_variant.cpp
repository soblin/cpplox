#include <cpplox/expression.hpp>
#include <cpplox/variant.hpp>

#include <gtest/gtest.h>

#include <cstdint>
#include <string>

TEST(Variant, is_variant_true)
{
  constexpr bool c = lox::detail::is_within_variant_v<char, std::variant<char, double, int>>;
  EXPECT_EQ(c, true);

  constexpr bool d = lox::detail::is_within_variant_v<char, std::variant<char, double, int>>;
  EXPECT_EQ(d, true);

  constexpr bool i = lox::detail::is_within_variant_v<char, std::variant<char, double, int>>;
  EXPECT_EQ(i, true);
}

TEST(Variant, is_variant_false)
{
  constexpr bool l = lox::detail::is_within_variant_v<uint64_t, std::variant<char, double, int>>;
  EXPECT_EQ(l, false);

  constexpr bool s = lox::detail::is_within_variant_v<size_t, std::variant<char, double, int>>;
  EXPECT_EQ(s, false);

  constexpr bool str =
    lox::detail::is_within_variant_v<std::string, std::variant<char, double, int>>;
  EXPECT_EQ(str, false);

  const auto b1 = lox::variant::experimental::is_within<int, std::variant<int, bool, std::string>>;
  EXPECT_EQ(b1, true);

  const auto b2 =
    lox::variant::experimental::is_within<double, std::variant<int, bool, std::string>>;
  EXPECT_EQ(b2, false);

  const auto b3 =
    lox::variant::experimental::is_within<int, boost::variant<int, bool, std::string>>;
  EXPECT_EQ(b3, true);

  const auto b4 =
    lox::variant::experimental::is_within<double, boost::variant<int, bool, std::string>>;
  EXPECT_EQ(b4, false);

  const auto b5 = lox::variant::experimental::is_within<lox::Variable, lox::Expr>;
  EXPECT_EQ(b5, true);

  const auto b6 = lox::variant::experimental::is_within<lox::Token, lox::Expr>;
  EXPECT_EQ(b6, false);

  // lox::variant::experimental::is_within<double, std::tuple<int, bool, std::string>>; this does
  // not compile
}

int main(int argc, char ** argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
