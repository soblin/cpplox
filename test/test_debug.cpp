#include <cpplox/error.hpp>
#include <cpplox/interpreter.hpp>
#include <cpplox/parser.hpp>
#include <cpplox/statement.hpp>
#include <cpplox/tokenizer.hpp>
#include <cpplox/variant.hpp>

#include <gtest/gtest.h>

int main(int argc, char ** argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
