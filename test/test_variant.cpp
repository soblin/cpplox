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
}

int main(int argc, char ** argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
