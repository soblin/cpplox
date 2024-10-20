#include <cpplox/interpreter.hpp>
#include <cpplox/parser.hpp>
#include <cpplox/tokenizer.hpp>

#include <gtest/gtest.h>

#include <cmath>

void CheckParseTokensTest(const std::string & source)
{
  auto tokenizer = lox::Tokenizer(source);
  const auto result = tokenizer.take_tokens();
  EXPECT_EQ(lox::is_variant_v<lox::Tokens>(result), true);
}

lox::Tokens ParseTokensTest(const std::string & source)
{
  auto tokenizer = lox::Tokenizer(source);
  const auto result = tokenizer.take_tokens();
  const auto & tokens = lox::as_variant<lox::Tokens>(result);
  return tokens;
}

void CheckParseProgramTest(const std::string & source)
{
  const auto & tokens = ParseTokensTest(source);
  auto parser = lox::Parser(tokens);
  const auto parse_result = parser.program();
  EXPECT_EQ(lox::is_variant_v<lox::Program>(parse_result), true);
}

std::pair<lox::Program, lox::Tokens> ParseProgramTest(const std::string & source)
{
  const auto & tokens = ParseTokensTest(source);
  auto parser = lox::Parser(tokens);
  const auto parse_result = parser.program();
  return {lox::as_variant<lox::Program>(parse_result), tokens};
}

template <typename T>
void test_as_t(const std::optional<lox::Value> & value_opt, const T & test)
{
  EXPECT_EQ(value_opt.has_value(), true);
  EXPECT_EQ(lox::is_variant_v<T>(value_opt.value()), true);
  EXPECT_EQ(lox::as_variant<T>(value_opt.value()), test);
}

template <>
void test_as_t<double>(const std::optional<lox::Value> & value_opt, const double & test)
{
  EXPECT_EQ(value_opt.has_value(), true);
  EXPECT_EQ(lox::is_variant_v<double>(value_opt.value()), true);
  EXPECT_FLOAT_EQ(lox::as_variant<double>(value_opt.value()), test);
}

void test_as(
  const std::optional<lox::Value> & value_opt, const lox::Value & test, const size_t type_index)
{
  if (type_index == lox::helper::Nil_Index) {
    ASSERT_NO_FATAL_FAILURE(test_as_t<lox::Nil>(value_opt, lox::as_variant<lox::Nil>(test)));
  } else if (type_index == lox::helper::bool_Index) {
    ASSERT_NO_FATAL_FAILURE(test_as_t<bool>(value_opt, lox::as_variant<bool>(test)));
  } else if (type_index == lox::helper::long_Index) {
    ASSERT_NO_FATAL_FAILURE(test_as_t<int64_t>(value_opt, lox::as_variant<int64_t>(test)));
  } else if (type_index == lox::helper::double_Index) {
    ASSERT_NO_FATAL_FAILURE(test_as_t<double>(value_opt, lox::as_variant<double>(test)));
  } else if (type_index == lox::helper::str_Index) {
    ASSERT_NO_FATAL_FAILURE(test_as_t<std::string>(value_opt, lox::as_variant<std::string>(test)));
  }
}

using TestStmtVariableSideEffectParam = std::tuple<
  const char *, std::vector<std::tuple<
                  size_t,     //<! variable token index
                  size_t,     //<! type index
                  lox::Value  //<! expected value
                  >>>;

class TestStmtVariableSideEffect : public ::testing::TestWithParam<TestStmtVariableSideEffectParam>
{
};

TEST_P(TestStmtVariableSideEffect, expr_var_side_effect)
{
  const auto [source_str, targets] = GetParam();
  const auto source = std::string(source_str);
  ASSERT_NO_FATAL_FAILURE(CheckParseProgramTest(source));
  const auto [program, tokens] = ParseProgramTest(source);

  lox::Interpreter interpreter{};
  const auto err = interpreter.execute(program);
  EXPECT_EQ(err.has_value(), false);
  for (const auto & [var_index, type_index, test] : targets) {
    const auto var_opt = interpreter.get_variable(tokens[var_index]);
    ASSERT_NO_FATAL_FAILURE(test_as(var_opt, test, type_index));
  }
}

INSTANTIATE_TEST_SUITE_P(
  TestStmtVariableSideEffect, TestStmtVariableSideEffect,
  ::testing::Values(
    /**
     * Assignment
     */
    TestStmtVariableSideEffectParam{
      R"(
var a = (1 + 2) * (3 + 45.6);
var b = a;
var c = a * b;
var d = 123.456;
d = c;
)",
      {
        {
          1,                          //<! "a"
          lox::helper::double_Index,  //<! double
          (1 + 2) * (3 + 45.6)        //<! test
        },
        {
          16,                         //<! "b"
          lox::helper::double_Index,  //<! double
          (1 + 2) * (3 + 45.6)        //<! test
        },
        {
          21,                                //<! "c"
          lox::helper::double_Index,         //<! double
          std::pow((1 + 2) * (3 + 45.6), 2)  //<! test
        },
        {
          28,                                //<! "d"
          lox::helper::double_Index,         //<! double
          std::pow((1 + 2) * (3 + 45.6), 2)  //<! test
        },
      }},
    //
    TestStmtVariableSideEffectParam{
      R"(
var a;
)",
      {{
        1,                       //<! "a"
        lox::helper::Nil_Index,  //<! Nil
        lox::Nil{}               //<! test
      }}},
    /**
     * If statement
     */
    TestStmtVariableSideEffectParam{
      R"(
var a = "123";
var b = 10;
if (a == "123"){
  b = 100;
}
)",
      {{
        6,                        //<! "b"
        lox::helper::long_Index,  //<! long
        100                       //<! test
      }}},                        //
    //
    TestStmtVariableSideEffectParam{
      R"(
var a = "123";
var b = 10;
if (a == "1234"){
  b = 100;
} else {
  b = 1000;
}
)",
      {{
        6,                        //<! "b"
        lox::helper::long_Index,  //<! long
        1000                      //<! test
      }}},                        //
    //
    TestStmtVariableSideEffectParam{
      R"(
var a = "123";
var b = 10;
if (a == "1234"){
  b = 100;
} else if (a == "123") {
  b = 200;
} else {
  b = 300;
}
)",
      {{
        6,                        //<! "b"
        lox::helper::long_Index,  //<! long
        200                       //<! test
      }}},                        //
    //
    TestStmtVariableSideEffectParam{
      R"(
var a = "123";
var b = 10;
if (a == "1234"){
  b = 100;
} else if (a == "123") {
  b = 200;
}

print a;
print b;
)",
      {{
        6,                        //<! "b"
        lox::helper::long_Index,  //<! long
        200                       //<! test
      }}},                        //
    //
    TestStmtVariableSideEffectParam{
      R"(
var a = "123";
var b = 10;
if (a == "1234"){
  b = 100;
} else if (a == "12345") {
  b = 200;
}

print a;
print b;
)",
      {{
        6,                        //<! "b"
        lox::helper::long_Index,  //<! long
        10                        //<! test
      }}},                        //
    //
    TestStmtVariableSideEffectParam{
      R"(
var a = "123";
var b = 10;
if (a == "1234"){
  b = 100;
} else if (a == "12345") {
  b = 200;
} else {
  b = 300;
}
)",
      {{
        6,                        //<! "b"
        lox::helper::long_Index,  //<! long
        300                       //<! test
      }}},                        //
    //
    TestStmtVariableSideEffectParam{
      R"(
var a = "123";
var b = 10;
if (var c = "12345"; c == "12345"){
  b = 100;
} else if (var d = 12345; c == "12345") {
  b = d;
} else {
  b = 300;
}
)",
      {{
        6,                        //<! "b"
        lox::helper::long_Index,  //<! long
        100                       //<! test
      }}},                        //
    //
    TestStmtVariableSideEffectParam{
      R"(
var a = "123";
var b = 10;
if (var c = "12345"; c != "12345"){
  b = 100;
} else if (var d = 12345; c == "12345") {
  b = d;
} else {
  b = 300;
}
)",
      {{
        6,                        //<! "b"
        lox::helper::long_Index,  //<! long
        12345                     //<! test
      }}},                        //
    //
    TestStmtVariableSideEffectParam{
      R"(
var a = "123";
var b = 10;
if (var c = "12345"; c != "12345"){
  b = 100;
} else if (var d = 12345; c != "12345") {
  b = d;
} else if (var e = 123456; d == 12345) {
  b = b + d + e;
} else {
// do nothing
}
)",
      {{
        6,                        //<! "b"
        lox::helper::long_Index,  //<! long
        10 + 12345 + 123456       //<! test
      }}},                        //
    //
    TestStmtVariableSideEffectParam{
      R"(
var a = "123";
var b = 10;
if (var c = "12345"; c != "12345"){
  b = 100;
} else if (var d = 12345; c != "12345") {
  b = d;
} else if (var e = 123456; d != 12345) {
  b = b + d + e;
} else {
  b = 54321;
}
)",
      {{
        6,                        //<! "b"
        lox::helper::long_Index,  //<! long
        54321                     //<! test
      }}},                        //
    /**
     * logical and/or, test side-effect of short-circuit
     */
    //
    TestStmtVariableSideEffectParam{
      R"(
var a = "123";
var b = 10;
var c = "before";
b = (c == "before") and (c = "after") == "after";
)",
      {{
         6,                        //<! "b"
         lox::helper::bool_Index,  //<! bool
         true                      //<! test
       },
       {
         11,                      //<! "c"
         lox::helper::str_Index,  //<! string
         "after",                 //<! test
       }}},                       //
    //
    TestStmtVariableSideEffectParam{
      R"(
var a = "123";
var b = 10;
var c = "before";
b = (c == "before") or (c = "after") == "after";
)",
      {{
         6,                        //<! "b"
         lox::helper::bool_Index,  //<! bool
         true                      //<! test
       },
       {
         11,                      //<! "c"
         lox::helper::str_Index,  //<! string
         "before"                 //<! test
       }}},                       //
    //
    TestStmtVariableSideEffectParam{
      R"(
var a = "123";
var b = 10;
var c = "before";
b = (c == "before2") and (c = "after") == "after";
)",
      {{
         6,                        //<! "b"
         lox::helper::bool_Index,  //<! bool
         false                     //<! test
       },
       {
         11,                      //<! "c"
         lox::helper::str_Index,  //<! string
         "before"                 //<! test
       }}},                       //
    //
    TestStmtVariableSideEffectParam{
      R"(
var a = "123";
var b = 10;
var c = "before";
b = (c == "before2") or (c = "after") == "after";
)",
      {{
         6,                        //<! "b"
         lox::helper::bool_Index,  //<! bool
         true                      //<! test
       },
       {
         11,                      //<! "c"
         lox::helper::str_Index,  //<! string
         "after"                  //<! test
       }}},                       //
    //
    TestStmtVariableSideEffectParam{
      R"(
var a = 123;
var b = 10;
var c = "before";
b = (a == 100 + 23) and (c == "be" + "fore");
)",
      {{
        6,                        //<! "b"
        lox::helper::bool_Index,  //<! bool
        true                      //<! test
      }}},                        //
    /**
     * For/While statement
     */
    //
    TestStmtVariableSideEffectParam{
      R"(
var a = 0;
var b = 0;
var c = 0;
while (a < 10) {
   c = c + 100;
   while(b < 10) {
      c = c + 10;
      b = b + 1;
   }
   a = a + 1;
}
)",
      {{
        11,                       //<! "c"
        lox::helper::long_Index,  //<! long
        1100                      //<! test
      }}},                        //
    //
    TestStmtVariableSideEffectParam{
      R"(
var a = 0;
var b = 0;
var c = 0;
while (a < 10) {
   c = c + 100;
   while(b < 10) {
      c = c + 10;
      b = b + 1;
   }
   a = a + 1;
}
)",
      {{
         1,                        //<! "a"
         lox::helper::long_Index,  //<! long
         10                        //<! test
       },
       {
         6,                        //<! "b"
         lox::helper::long_Index,  //<! long
         10                        //<! test
       },
       {
         11,                       //<! "c"
         lox::helper::long_Index,  //<! long
         1100                      //<! test
       }}},                        //
    //
    TestStmtVariableSideEffectParam{
      R"(
var a = -10;
var b = -10;
var c = 0;
for (a = 0; a < 10; a = a + 1) {
   c = c + 10;
   for (b = 0; b < 10; b = b + 1) {
      c = c + 20;
   }
}
)",
      {{
         1,                        //<! "a"
         lox::helper::long_Index,  //<! long
         10                        //<! test
       },
       {
         7,                        //<! "b"
         lox::helper::long_Index,  //<! long
         10                        //<! test
       },
       {
         13,                       //<! "c"
         lox::helper::long_Index,  //<! long
         (10 + (10 * 20)) * 10     //<! test
       }}},                        //
    //
    TestStmtVariableSideEffectParam{
      R"(
var a = -10;
var b = -10;
var c = 0;
for (a = 0; a < 10; a = a + 1) {
   c = c + 10;
   for (var d = 0; d < 10; d = d + 1) {
      c = c + 20;
   }
}
)",
      {{
         1,                        //<! "a"
         lox::helper::long_Index,  //<! long
         10                        //<! test
       },
       {
         13,                       //<! "c"
         lox::helper::long_Index,  //<! long
         (10 + (10 * 20)) * 10     //<! test
       }}},                        //
    //
    TestStmtVariableSideEffectParam{
      R"(
var a = 0;
var b = 0;
var c = 0;
for (; a < 10; a = a + 1) {
   c = c + 10;
   for (; b < 10; b = b + 1) {
      c = c + 20;
   }
   // NOTE: here, already b == 10; from the next iteration of a==1; c+=20 is not executed anymore!
}
)",
      {{
         1,                        //<! "a"
         lox::helper::long_Index,  //<! long
         10                        //<! test
       },
       {
         6,                        //<! "b"
         lox::helper::long_Index,  //<! long
         10                        //<! test
       },
       {
         11,                       //<! "c"
         lox::helper::long_Index,  //<! long
         10 * 20 + 10 * 10         //<! test
       }}},                        //
    //
    TestStmtVariableSideEffectParam{
      R"(
var a = 0;
var b = 0;
var c = 0;
for (; a < 10;) {
   c = c + 10;
   for (; b < 10;) {
      c = c + 20;
      b = b + 1;
   }
   // NOTE: here, already b == 10; from the next iteration of a==1; c+=20 is not executed anymore!
   a = a + 1;
}
)",
      {{
         1,                        //<! "a"
         lox::helper::long_Index,  //<! long
         10                        //<! test
       },
       {
         6,                        //<! "b"
         lox::helper::long_Index,  //<! long
         10                        //<! test
       },
       {
         11,                       //<! "c"
         lox::helper::long_Index,  //<! long
         10 * 20 + 10 * 10         //<! test
       }}},                        //
    //
    TestStmtVariableSideEffectParam{
      R"(
var a = 0;
var b = 0;
for (;;) {
  a = a + 1;
  if (a > 10) {
    break;
    b = 10;
  } else {
    continue;
  }
}
)",
      {{
         1,                        //<! "a"
         lox::helper::long_Index,  //<! long
         11                        //<! test
       },
       {
         6,                        //<! "b"
         lox::helper::long_Index,  //<! long
         0                         //<! test
       }}},                        //
    //
    TestStmtVariableSideEffectParam{
      R"(
var a = 0;
var b = 0;
for (;;) {
  a = a + 1;
  if (a > 10) {
    break;
  } else {
    continue;
    b = 10;
  }
}
)",
      {{
         1,                        //<! "a"
         lox::helper::long_Index,  //<! long
         11                        //<! test
       },
       {
         6,                        //<! "b"
         lox::helper::long_Index,  //<! long
         0                         //<! test
       }}},                        //
    //
    TestStmtVariableSideEffectParam{
      R"(
var a = 0;
var b = 0;
var c = 0;
for (;;) {
  a = a + 1;
  if (a > 10) {
    break;
  }
  for (;;) {
    if (b > 10) {
      break;
    }
    b = b + 1;
  }
  c = c + 1;
}
)",
      {{
         1,                        //<! "a"
         lox::helper::long_Index,  //<! long
         11                        //<! test
       },
       {
         6,                        //<! "b"
         lox::helper::long_Index,  //<! long
         11                        //<! test
       },
       {
         11,                       //<! "c"
         lox::helper::long_Index,  //<! long
         10                        //<! test
       }}},                        //
    //
    TestStmtVariableSideEffectParam{
      R"(
var a = 0;
var b = 0;
var c = 0;
while (true) {
  a = a + 1;
  if (a > 10) {
    break;
  }
  while (true) {
    if (b > 10) {
      break;
    }
    b = b + 1;
  }
  c = c + 1;
}
)",
      {{
         1,                        //<! "a"
         lox::helper::long_Index,  //<! long
         11                        //<! test
       },
       {
         6,                        //<! "b"
         lox::helper::long_Index,  //<! long
         11                        //<! test
       },
       {
         11,                       //<! "c"
         lox::helper::long_Index,  //<! long
         10                        //<! test
       }}},                        //
    //
    TestStmtVariableSideEffectParam{
      R"(
var a = 0;
var b = 0;
var c = 0;
while (true) {
  a = a + 1;
  if (a > 10) {
    break;
  }
  while (true) {
    if (b > 10) {
      break;
    } else {
      b = b + 1;
      continue;
    }
    b = b + 10;
  }
  c = c + 1;
}
)",
      {{
         1,                        //<! "a"
         lox::helper::long_Index,  //<! long
         11                        //<! test
       },
       {
         6,                        //<! "b"
         lox::helper::long_Index,  //<! long
         11                        //<! test
       },
       {
         11,                       //<! "c"
         lox::helper::long_Index,  //<! long
         10                        //<! test
       }}}));

using TestFunctionValueParam = std::tuple<
  const char *, std::vector<std::tuple<
                  size_t,     //<! variable token index
                  size_t,     //<! type index
                  lox::Value  //<! expected value
                  >>>;

class TestFunctionValue : public ::testing::TestWithParam<TestFunctionValueParam>
{
};

TEST_P(TestFunctionValue, function_value)
{
  const auto [source_str, targets] = GetParam();
  const auto source = std::string(source_str);
  ASSERT_NO_FATAL_FAILURE(CheckParseProgramTest(source));
  const auto [program, tokens] = ParseProgramTest(source);

  lox::Interpreter interpreter{};
  const auto err = interpreter.execute(program);
  EXPECT_EQ(err.has_value(), false);
  for (const auto & [var_index, type_index, test] : targets) {
    const auto var_opt = interpreter.get_variable(tokens[var_index]);
    ASSERT_NO_FATAL_FAILURE(test_as(var_opt, test, type_index));
  }
}

INSTANTIATE_TEST_SUITE_P(
  TestFunctionValue, TestFunctionValue,
  ::testing::Values(
    /**
     * function
     */
    TestFunctionValueParam{
      R"(
var glob_a = 0;
var glob_b = 0;
var glob_c = 0;
fun foo(a, b, c) {
   glob_a = a;
   glob_b = b;
   glob_c = c;
   return;
}

foo(10, 20, 30);
)",
      {{
         1,                        //<! "a"
         lox::helper::long_Index,  //<! long
         10                        //<! test
       },
       {
         6,                        //<! "b"
         lox::helper::long_Index,  //<! long
         20                        //<! test
       },
       {
         11,                       //<! "c"
         lox::helper::long_Index,  //<! long
         30                        //<! test
       }}},
    //
    TestFunctionValueParam{
      R"(
var a = 0;

fun fib(a) {
  if (a == 0) {
    return 0;
  } else if (a == 1) {
    return 1;
  } else {
    return fib(a-1) + fib(a - 2);
  }
}

a = fib(10);
)",
      {{
        1,                        //<! "a"
        lox::helper::long_Index,  //<! long
        55                        //<! test
      }}},
    //
    TestFunctionValueParam{
      R"(
var a = 0;

fun fib(a) {
  if (a == 0) {
    return 0;
  }
  if (a == 1) {
    return 1;
  } else {
    return fib(a-1) + fib(a - 2);
  }
}

a = fib(10);
)",
      {{
        1,                        //<! "a"
        lox::helper::long_Index,  //<! long
        55                        //<! test
      }}},
    //
    TestFunctionValueParam{
      R"(
var a = 0;

fun fib(a) {
  if (a == 0) {
    return 0;
  }
  if (a == 1) {
    return 1;
  }
  return fib(a-1) + fib(a - 2);
}

a = fib(10);
)",
      {{
        1,                        //<! "a"
        lox::helper::long_Index,  //<! long
        55                        //<! test
      }}},
    //
    TestFunctionValueParam{
      R"(
var a = 0;

fun sum(a) {
  var ret = 0;
  for(var i = 0; i <= a; i = i + 1) {
    ret = ret + i;
  }
  return ret;
}

a = sum(10);
)",
      {{
        1,                        //<! "a"
        lox::helper::long_Index,  //<! long
        55                        //<! test
      }}},
    //
    TestFunctionValueParam{
      R"(
var a = 0;

fun sum(a) {
  var ret = 0;
  for(var i = 0;;) {
    if (i > a) {
      return ret;
    }
    ret = ret + i;
    i = i + 1;
  }
  return ret;
}

a = sum(10);
)",
      {{
        1,                        //<! "a"
        lox::helper::long_Index,  //<! long
        55                        //<! test
      }}},
    //
    TestFunctionValueParam{
      R"(
var a = 0;

fun foo() {
  a = 10;
  return;
}

foo();
)",
      {{
        1,                        //<! "a"
        lox::helper::long_Index,  //<! long
        10                        //<! test
      }}}));

TEST(Statement, if_statement)
{
  {
    const std::string source = R"(
(1 + 2) * ( 3 + 4);
)";

    ASSERT_NO_FATAL_FAILURE(CheckParseProgramTest(source));
    const auto [program, _] = ParseProgramTest(source);

    EXPECT_EQ(lox::is_variant_v<lox::Stmt>(program[0]), true);
    const auto & stmt = lox::as_variant<lox::Stmt>(program[0]);
    EXPECT_EQ(lox::is_variant_v<lox::ExprStmt>(stmt), true);
    const auto & stmt1 = lox::as_variant<lox::ExprStmt>(stmt);

    auto interpreter = lox::Interpreter{};
    const auto & eval_opt = interpreter.evaluate_expr(stmt1.expression);
    EXPECT_EQ(lox::is_variant_v<lox::Value>(eval_opt), true);
    const auto & eval = lox::as_variant<lox::Value>(eval_opt);
    EXPECT_EQ(lox::is_variant_v<int64_t>(eval), true);
    EXPECT_EQ(lox::as_variant<int64_t>(eval), (1 + 2) * (3 + 4));
  }
  {
    const std::string source = R"(
print (1 + 2) * ( 3 + 4);
)";
    ASSERT_NO_FATAL_FAILURE(CheckParseProgramTest(source));
    const auto [program, _] = ParseProgramTest(source);

    EXPECT_EQ(lox::is_variant_v<lox::Stmt>(program[0]), true);
    const auto & stmt = lox::as_variant<lox::Stmt>(program[0]);
    EXPECT_EQ(lox::is_variant_v<lox::PrintStmt>(stmt), true);
    const auto & stmt1 = lox::as_variant<lox::PrintStmt>(stmt);

    auto interpreter = lox::Interpreter{};
    const auto & eval_opt = interpreter.evaluate_expr(stmt1.expression);
    EXPECT_EQ(lox::is_variant_v<lox::Value>(eval_opt), true);
    const auto & eval = lox::as_variant<lox::Value>(eval_opt);
    EXPECT_EQ(lox::is_variant_v<int64_t>(eval), true);
    EXPECT_EQ(lox::as_variant<int64_t>(eval), (1 + 2) * (3 + 4));
  }
  {
    const std::string source = R"(
var a = (1 + 2) * ( 3 + 4);
)";
    ASSERT_NO_FATAL_FAILURE(CheckParseProgramTest(source));
    const auto [program, _] = ParseProgramTest(source);

    EXPECT_EQ(lox::is_variant_v<lox::VarDecl>(program[0]), true);
    const auto & stmt1 = lox::as_variant<lox::VarDecl>(program[0]);

    auto interpreter = lox::Interpreter{};
    const auto & eval_opt = interpreter.evaluate_expr(stmt1.initializer.value());
    EXPECT_EQ(lox::is_variant_v<lox::Value>(eval_opt), true);
    const auto & eval = lox::as_variant<lox::Value>(eval_opt);
    EXPECT_EQ(lox::is_variant_v<int64_t>(eval), true);
    EXPECT_EQ(lox::as_variant<int64_t>(eval), (1 + 2) * (3 + 4));
  }
}

int main(int argc, char ** argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
