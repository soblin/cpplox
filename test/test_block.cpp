#include <cpplox/error.hpp>
#include <cpplox/interpreter.hpp>
#include <cpplox/parser.hpp>
#include <cpplox/statement.hpp>
#include <cpplox/tokenizer.hpp>
#include <cpplox/variant.hpp>

#include <boost/variant.hpp>

#include <gtest/gtest.h>

TEST(Block, global_variable)
{
  const std::string source1 = R"(
// first body
var a = "global a";
var b = "global b";
var c = "global c";

// first block
{
   var a = "inner1_a";
   var b = "inner1_b";
   var c = "inner1_c";
   // second block
   {
      var a = "inner2_a";
      var b = "inner2_b";
      var c = "inner2_c";
   }
   c  = "inner1_c_after";
   // end second block
}
// end first block
)";
  auto tokenizer = lox::Tokenizer(source1);
  const auto result = tokenizer.take_tokens();
  EXPECT_EQ(lox::is_variant_v<lox::Tokens>(result), true);
  const auto & tokens = lox::as_variant<lox::Tokens>(result);

  auto parser = lox::Parser(tokens);
  const auto parse_result = parser.program();
  EXPECT_EQ(lox::is_variant_v<lox::Program>(parse_result), true);
  const auto & program = lox::as_variant<lox::Program>(parse_result);

  EXPECT_EQ(program.size(), 4);

  // final result
  lox::Interpreter interpreter{};
  interpreter.execute(program);
  const auto a_opt = interpreter.get_variable(tokens[1]);
  EXPECT_EQ(a_opt.has_value(), true);
  EXPECT_EQ(lox::is_variant_v<std::string>(a_opt.value()), true);
  EXPECT_EQ(lox::as_variant<std::string>(a_opt.value()), "global a");
  const auto b_opt = interpreter.get_variable(tokens[6]);
  EXPECT_EQ(b_opt.has_value(), true);
  EXPECT_EQ(lox::is_variant_v<std::string>(b_opt.value()), true);
  EXPECT_EQ(lox::as_variant<std::string>(b_opt.value()), "global b");
  const auto c_opt = interpreter.get_variable(tokens[11]);
  EXPECT_EQ(c_opt.has_value(), true);
  EXPECT_EQ(lox::is_variant_v<std::string>(c_opt.value()), true);
  EXPECT_EQ(lox::as_variant<std::string>(c_opt.value()), "global c");

  // first block
  {
    lox::Interpreter interpreter_first;
    const auto & first_block_decl = program[3];
    EXPECT_EQ(first_block_decl.which(), 1 /* Stmt */);
    const auto & first_block_stmt = boost::get<lox::Stmt>(first_block_decl);
    EXPECT_EQ(first_block_stmt.which(), 2 /* Block */);
    const auto & first_block = boost::get<lox::Block>(first_block_stmt);
    const auto & first_block_program = first_block.declarations;
    EXPECT_EQ(first_block_program.size(), 5);
    interpreter_first.execute(first_block_program);
    const auto a_opt = interpreter_first.get_variable(tokens[1]);
    EXPECT_EQ(a_opt.has_value(), true);
    EXPECT_EQ(lox::is_variant_v<std::string>(a_opt.value()), true);
    EXPECT_EQ(lox::as_variant<std::string>(a_opt.value()), "inner1_a");
    const auto b_opt = interpreter_first.get_variable(tokens[6]);
    EXPECT_EQ(b_opt.has_value(), true);
    EXPECT_EQ(lox::is_variant_v<std::string>(b_opt.value()), true);
    EXPECT_EQ(lox::as_variant<std::string>(b_opt.value()), "inner1_b");
    const auto c_opt = interpreter_first.get_variable(tokens[11]);
    EXPECT_EQ(c_opt.has_value(), true);
    EXPECT_EQ(lox::is_variant_v<std::string>(c_opt.value()), true);
    EXPECT_EQ(lox::as_variant<std::string>(c_opt.value()), "inner1_c_after");
    // second block
    {
      lox::Interpreter interpreter_second;
      const auto & second_block_decl = first_block_program[3];
      EXPECT_EQ(second_block_decl.which(), 1 /* Stmt */);
      const auto & second_block_stmt = boost::get<lox::Stmt>(second_block_decl);
      EXPECT_EQ(second_block_stmt.which(), 2 /* Block */);
      const auto & second_block = boost::get<lox::Block>(second_block_stmt);
      const auto & second_block_program = second_block.declarations;
      interpreter_second.execute(second_block_program);
      const auto a_opt = interpreter_second.get_variable(tokens[1]);
      EXPECT_EQ(a_opt.has_value(), true);
      EXPECT_EQ(lox::is_variant_v<std::string>(a_opt.value()), true);
      EXPECT_EQ(lox::as_variant<std::string>(a_opt.value()), "inner2_a");
      const auto b_opt = interpreter_second.get_variable(tokens[6]);
      EXPECT_EQ(b_opt.has_value(), true);
      EXPECT_EQ(lox::is_variant_v<std::string>(b_opt.value()), true);
      EXPECT_EQ(lox::as_variant<std::string>(b_opt.value()), "inner2_b");
      const auto c_opt = interpreter_second.get_variable(tokens[11]);
      EXPECT_EQ(c_opt.has_value(), true);
      EXPECT_EQ(lox::is_variant_v<std::string>(c_opt.value()), true);
      EXPECT_EQ(lox::as_variant<std::string>(c_opt.value()), "inner2_c");
    }
  }
}

TEST(Block, global_variable_assign)
{
  const std::string source1 = R"(
// first body
var a = "global a";
var b = "global b";
var c = "global c";

// first block
{
   var a = "inner1_a";
   c = "inner1_c";
   // second block
   {
      var a = "inner2_a";
      var b = "inner2_b";
      c = "inner2_c";
   }
   // end second block
}
// end first block
)";
  auto tokenizer = lox::Tokenizer(source1);
  const auto result = tokenizer.take_tokens();
  EXPECT_EQ(lox::is_variant_v<lox::Tokens>(result), true);
  const auto & tokens = lox::as_variant<lox::Tokens>(result);

  auto parser = lox::Parser(tokens);
  const auto parse_result = parser.program();
  EXPECT_EQ(lox::is_variant_v<lox::Program>(parse_result), true);
  const auto & program = lox::as_variant<lox::Program>(parse_result);

  lox::Interpreter interpreter{};
  interpreter.execute(program);
  const auto a_opt = interpreter.get_variable(tokens[1]);
  EXPECT_EQ(a_opt.has_value(), true);
  EXPECT_EQ(lox::is_variant_v<std::string>(a_opt.value()), true);
  EXPECT_EQ(lox::as_variant<std::string>(a_opt.value()), "global a");
  const auto b_opt = interpreter.get_variable(tokens[6]);
  EXPECT_EQ(b_opt.has_value(), true);
  EXPECT_EQ(lox::is_variant_v<std::string>(b_opt.value()), true);
  EXPECT_EQ(lox::as_variant<std::string>(b_opt.value()), "global b");
  const auto c_opt = interpreter.get_variable(tokens[11]);
  EXPECT_EQ(c_opt.has_value(), true);
  EXPECT_EQ(lox::is_variant_v<std::string>(c_opt.value()), true);
  EXPECT_EQ(lox::as_variant<std::string>(c_opt.value()), "inner2_c");
}

TEST(Block, block_error)
{
  const std::string source1 = R"(
// first body
var a = "global a";
var b = "global b";
var c = "global c";

// first block
{
   var a = "inner1_a";
   c = "inner1_c";
)";
  auto tokenizer = lox::Tokenizer(source1);
  const auto result = tokenizer.take_tokens();
  EXPECT_EQ(lox::is_variant_v<lox::Tokens>(result), true);
  const auto & tokens = lox::as_variant<lox::Tokens>(result);

  auto parser = lox::Parser(tokens);
  const auto parse_result = parser.program();
  EXPECT_EQ(lox::is_variant_v<lox::SyntaxError>(parse_result), true);
  const auto & err = lox::as_variant<lox::SyntaxError>(parse_result);
  EXPECT_EQ(err.kind, lox::SyntaxErrorKind::UnmatchedBraceError);
}

int main(int argc, char ** argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
