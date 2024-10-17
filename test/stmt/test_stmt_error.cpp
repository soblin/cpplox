#include <cpplox/interpreter.hpp>
#include <cpplox/parser.hpp>
#include <cpplox/tokenizer.hpp>

#include <gtest/gtest.h>

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

using TestSyntaxErrorKindParamT = std::pair<const std::string, lox::SyntaxErrorKind>;

class TestSyntaxErrorKind : public ::testing::TestWithParam<TestSyntaxErrorKindParamT>
{
};

TEST_P(TestSyntaxErrorKind, expr_statment_syntax_errors)
{
  const auto [source, kind] = GetParam();

  ASSERT_NO_FATAL_FAILURE(CheckParseTokensTest(source));
  const auto & tokens = ParseTokensTest(source);

  auto parser = lox::Parser(tokens);
  const auto parse_result = parser.program();
  EXPECT_EQ(lox::is_variant_v<lox::SyntaxError>(parse_result), true);
  const auto & err = lox::as_variant<lox::SyntaxError>(parse_result);
  EXPECT_EQ(err.kind, kind);
}

INSTANTIATE_TEST_SUITE_P(
  TestSyntaxErrorKindCases, TestSyntaxErrorKind,
  ::testing::Values(
    //
    TestSyntaxErrorKindParamT{
      R"(
(1 + 2) * ( 3 + 4)
)",
      lox::SyntaxErrorKind::StmtWithoutSemicolun},
    //
    TestSyntaxErrorKindParamT{
      R"(
print (1 + 2) * ( 3 + 4)
)",
      lox::SyntaxErrorKind::StmtWithoutSemicolun},
    //
    TestSyntaxErrorKindParamT{
      R"(
foo = (1 + 2) * ( 3 + 4)
)",
      lox::SyntaxErrorKind::StmtWithoutSemicolun},
    /**
     * Assginment tests
     */
    TestSyntaxErrorKindParamT{
      R"(
(b) = 3;
)",
      lox::SyntaxErrorKind::InvalidAssignmentTarget},
    //
    TestSyntaxErrorKindParamT{
      R"(
var a = (1+2;
)",
      lox::SyntaxErrorKind::UnmatchedParenError},
    //
    TestSyntaxErrorKindParamT{
      R"(
var a;
a = (1 + 2;
)",
      lox::SyntaxErrorKind::UnmatchedParenError},
    //
    TestSyntaxErrorKindParamT{
      R"(
var a
)",
      lox::SyntaxErrorKind::StmtWithoutSemicolun},
    //
    TestSyntaxErrorKindParamT{
      R"(
var 1 = 2 + 3;
)",
      lox::SyntaxErrorKind::MissingValidIdentifierDecl},
    //
    TestSyntaxErrorKindParamT{
      R"(
var a = 2 + 3
)",
      lox::SyntaxErrorKind::StmtWithoutSemicolun},
    /**
     * If statement tests
     */
    TestSyntaxErrorKindParamT{
      R"(
var a = "123";
var b = 10;
if {
  b = 100;
}
)",
      lox::SyntaxErrorKind::MissingIfConditon},
    //
    TestSyntaxErrorKindParamT{
      R"(
var a = "123";
var b = 10;
if (b == 10 {
  b = 100;
}
)",
      lox::SyntaxErrorKind::UnmatchedParenError},
    //
    TestSyntaxErrorKindParamT{
      R"(
var a = "123";
var b = 10;
if (a == "123");
)",
      lox::SyntaxErrorKind::MissingIfBody},
    //
    TestSyntaxErrorKindParamT{
      R"(
var a = "123";
var b = 10;
if (b == 10){
  b = 100
}
)",
      lox::SyntaxErrorKind::StmtWithoutSemicolun},
    //
    TestSyntaxErrorKindParamT{
      R"(
var a = "123";
var b = 10;
if (b == 10) {
  b = 100;
} else if(b == 100) {
  print b
}
)",
      lox::SyntaxErrorKind::StmtWithoutSemicolun},
    //
    TestSyntaxErrorKindParamT{
      R"(
var a = "123";
var b = 10;
if (b == 10) {
  b = 100;
} else if(b == 100) {
  print b;
} else
  print b;
)",
      lox::SyntaxErrorKind::UnmatchedBraceError},
    //
    TestSyntaxErrorKindParamT{
      R"(
var a = "123";
var b = 10;
if (b == 10) {
  b = 100;
} else if(b == 100) {
  print b;
} else {
  print b
}
)",
      lox::SyntaxErrorKind::StmtWithoutSemicolun},
    //
    TestSyntaxErrorKindParamT{
      R"(
var a = "123";
var b = 10;
if (c = "12345"; c != "12345"){
  b = 100;
} else if (var d = 12345; c != "12345") {
  b = d;
} else if (var e = 123456; d != 12345) {
  b = b + d + e;
} else {
  b = 54321;
}
)",
      lox::SyntaxErrorKind::UnmatchedParenError},
    //
    TestSyntaxErrorKindParamT{
      R"(
var a = "123";
var b = 10;
if (var c == "12345"; c != "12345"){
  b = 100;
} else if (var d = 12345; c != "12345") {
  b = d;
} else if (var e = 123456; d != 12345) {
  b = b + d + e;
} else {
  b = 54321;
}
)",
      lox::SyntaxErrorKind::StmtWithoutSemicolun},
    /**
     * While statement tests
     */
    TestSyntaxErrorKindParamT{
      R"(
var a = 0;
var b = 0;
var c = 0;
while (a < 10 {
   c = c + 100;
   while(b < 10) {
      c = c + 10;
      b = b + 1;
   }
   a = a + 1;
}
)",
      lox::SyntaxErrorKind::UnmatchedParenError},
    //
    TestSyntaxErrorKindParamT{
      R"(
var a = 0;
var b = 0;
var c = 0;
while (a < 10) {
   c = c + 100;
   while(b < 10) {
      c = c + 10;
      b = b + 1;
   // missing }
   a = a + 1;
}
)",
      lox::SyntaxErrorKind::UnmatchedBraceError},
    //
    TestSyntaxErrorKindParamT{
      R"(
var a = 0;
var b = 0;
var c = 0;
while a < 10 {
   c = c + 100;
   while(b < 10) {
      c = c + 10;
      b = b + 1;
   }
   a = a + 1;
}
)",
      lox::SyntaxErrorKind::MissingWhileConditon},
    //
    TestSyntaxErrorKindParamT{
      R"(
var a = 0;
var b = 0;
var c = 0;
while (a < 10)
  print a;
)",
      lox::SyntaxErrorKind::MissingWhileBody},
    /**
     * For statement tests
     */
    TestSyntaxErrorKindParamT{
      R"(
var a = 0;
for var b = 0; b < 10; b = b + 1 {
  a = a + 1;
}
)",
      lox::SyntaxErrorKind::MissingForCondition},
    //
    TestSyntaxErrorKindParamT{
      R"(
var a = 0;
for (var b = (1+2; b < 10; b = b + 1) {
  a = a + 1;
}
)",
      lox::SyntaxErrorKind::UnmatchedParenError},
    TestSyntaxErrorKindParamT{
      R"(
var a = 0;
for ((1+2; b < 10; b = b + 1) {
  a = a + 1;
}
)",
      lox::SyntaxErrorKind::UnmatchedParenError},
    //
    TestSyntaxErrorKindParamT{
      R"(
var a = 0;
for (var b = 1+2; b < 10; b = (b + 1) {
  a = a + 1;
}
)",
      lox::SyntaxErrorKind::UnmatchedParenError},
    //
    TestSyntaxErrorKindParamT{
      R"(
var a = 0;
for (var b = 1+2; b < 10; b = b + 1)
  a = a + 1;
)",
      lox::SyntaxErrorKind::MissingForBody},
    //
    TestSyntaxErrorKindParamT{
      R"(
var a = 0;
for (var b = 1+2; b < 10; b = b + 1) {
  a = a + 1
}
)",
      lox::SyntaxErrorKind::StmtWithoutSemicolun},
    //
    TestSyntaxErrorKindParamT{
      R"(
var a = 0;
for (var b = 1+2; b < 10; b = b + 1) {
  break
}
)",
      lox::SyntaxErrorKind::StmtWithoutSemicolun},
    //
    TestSyntaxErrorKindParamT{
      R"(
var a = 0;
for (var b = 1+2; b < 10; b = b + 1) {
  continue
}
)",
      lox::SyntaxErrorKind::StmtWithoutSemicolun},
    //
    TestSyntaxErrorKindParamT{
      R"(
fun foo {
  print a;
}
)",
      lox::SyntaxErrorKind::MissingFuncParameterDecl},
    //
    TestSyntaxErrorKindParamT{
      R"(
fun foo(a, b)
  print a;
)",
      lox::SyntaxErrorKind::MissingFuncBodyDecl},
    //
    TestSyntaxErrorKindParamT{
      R"(
fun foo(a, b) {
  print a
}
)",
      lox::SyntaxErrorKind::StmtWithoutSemicolun}));

template <typename T>
class TestRuntimeErrorKind : public ::testing::TestWithParam<const char *>
{
public:
  using Type = T;
};

using TestTypeError = TestRuntimeErrorKind<lox::TypeError>;

TEST_P(TestTypeError, expr_statment_runtime_errors)
{
  const auto & source = std::string(GetParam());

  ASSERT_NO_FATAL_FAILURE(CheckParseProgramTest(source));
  const auto [program, _] = ParseProgramTest(source);

  lox::Interpreter interpreter{};
  const auto exec = interpreter.execute(program);
  EXPECT_EQ(exec.has_value(), true);
  EXPECT_EQ(lox::is_variant_v<typename std::decay_t<Type>>(exec.value()), true);
}

using TestUndefinedVariableError = TestRuntimeErrorKind<lox::UndefinedVariableError>;

TEST_P(TestUndefinedVariableError, expr_statment_runtime_errors)
{
  const auto & source = std::string(GetParam());

  ASSERT_NO_FATAL_FAILURE(CheckParseProgramTest(source));
  const auto [program, _] = ParseProgramTest(source);

  lox::Interpreter interpreter{};
  const auto exec = interpreter.execute(program);
  EXPECT_EQ(exec.has_value(), true);
  EXPECT_EQ(lox::is_variant_v<typename std::decay_t<Type>>(exec.value()), true);
}

using TestMaxLoopError = TestRuntimeErrorKind<lox::MaxLoopError>;

TEST_P(TestMaxLoopError, expr_statment_runtime_errors)
{
  const auto & source = std::string(GetParam());

  ASSERT_NO_FATAL_FAILURE(CheckParseProgramTest(source));
  const auto [program, _] = ParseProgramTest(source);

  lox::Interpreter interpreter{};
  const auto exec = interpreter.execute(program);
  EXPECT_EQ(exec.has_value(), true);
  EXPECT_EQ(lox::is_variant_v<typename std::decay_t<Type>>(exec.value()), true);
}

using TestNotInvocableError = TestRuntimeErrorKind<lox::NotInvocableError>;

TEST_P(TestNotInvocableError, test_not_invocable)
{
  const auto & source = std::string(GetParam());

  ASSERT_NO_FATAL_FAILURE(CheckParseProgramTest(source));
  const auto [program, _] = ParseProgramTest(source);

  lox::Interpreter interpreter{};
  const auto exec = interpreter.execute(program);
  EXPECT_EQ(exec.has_value(), true);
  EXPECT_EQ(lox::is_variant_v<typename std::decay_t<Type>>(exec.value()), true);
}

using TestNoReturnFromFunction = TestRuntimeErrorKind<lox::NoReturnFromFunction>;

TEST_P(TestNoReturnFromFunction, test_no_return_value)
{
  const auto & source = std::string(GetParam());

  ASSERT_NO_FATAL_FAILURE(CheckParseProgramTest(source));
  const auto [program, _] = ParseProgramTest(source);

  lox::Interpreter interpreter{};
  const auto exec = interpreter.execute(program);
  EXPECT_EQ(exec.has_value(), true);
  EXPECT_EQ(lox::is_variant_v<typename std::decay_t<Type>>(exec.value()), true);
}

INSTANTIATE_TEST_SUITE_P(
  TestRuntimeErrorKindCases, TestUndefinedVariableError,
  ::testing::Values(
    //
    R"(
var a = (1 + 2) * (3 + 45.6);
var b = a;
var c = a * d;
)",
    //
    R"(
var a = "123";
var b = 10;
if (c == 100){
  b = 100;
}
)",
    //
    R"(
var a = "123";
var b = 10;
if (c == 100){
  b = 100;
}
)",
    //
    R"(
var a = "123";
var b = 10;
if (b == 100){
  b = 100;
} else if (c == 100) {
// do something;
}
)",
    //
    R"(
var a = "123";
var b = 10;
if (b == 100){
  b = 100;
} else if (b == 100) {
  // do something;
} else {
  print c;
}
)",
    //
    R"(
var a = 123;
var b = 10;
var c = "before";
b = (a == 100 + 23) and (c == be + "fore");
)",
    //
    R"(
var a = 0;
var b = 0;
var c = 0;
while (a < 10) {
   c = c + 100;
   while(d < 10) {
      c = c + 10;
      b = b + 1;
   }
   a = a + 1;
}
)",
    //
    R"(
var c = 0;
for (var a = 0; a < 10; a = a + 1) {
   c = c + 10;
   for (var b = 0; b < 10; b = b + 1) {
      c = c + d;
   }
}
)"));

INSTANTIATE_TEST_SUITE_P(
  TestTypeKindCases, TestTypeError,
  ::testing::Values(
    //
    R"(
var a = (1 + 2) * (3 + "str");
)",
    //
    R"(
var a;
a = (1 + 2) * (3 + "str");
)",
    //
    R"(
(1 + 2) * ( 3 + "str");
)",
    //
    R"(
var a = "123";
var b = 10;
if (b == 10){
  b = 100 + "123";
}
)",
    //
    R"(
var a = 123;
var b = 10;
var c = "before";
b = (a == 100 + "23") and (c == "be" + "fore");
)",
    //
    R"(
var a = 123;
var b = 10;
var c = "before";
b = (a == 100 + 23) and (a == 100 + "23");
)",
    //
    R"(
var c = 0;
for (var a = 0 + "0"; a < 10; a = a + 1) {
   c = c + 10;
   for (var b = 0; b < 10; b = b + 1) {
      c = c + 20;
   }
}
)",
    //
    R"(
var c = 0;
for (var a = 0; a < "10"; a = a + 1) {
   c = c + 10;
   for (var b = 0; b < 10; b = b + 1) {
      c = c + 20;
   }
}
)",
    //
    R"(
var c = 0;
for (var a = 0; a < 10; a = a + "1") {
   c = c + 10;
   for (var b = 0; b < 10; b = b + 1) {
      c = c + 20;
   }
}
)",
    //
    R"(
var a = 0;

fun fib(a) {
  return a + "a";
}

a = fib(10);
)",
    //
    R"(
var a = 0;

fun I(a) {
  return a;
}

a = I(10 + "10");
)"));

INSTANTIATE_TEST_SUITE_P(
  TestTypeKindCases, TestMaxLoopError,
  ::testing::Values(
    //
    R"(
for (var a = 1; a > 0; a = a + 1) {
  a = a + 1;
}
)",
    //
    R"(
var a = 1;
while (a > 0) {
  a = a + 1;
}
)"));

INSTANTIATE_TEST_SUITE_P(
  TestTypeKindCases, TestNotInvocableError,
  ::testing::Values(
    //
    R"(
fun foo(a, b, c){
  return a + b + c;
}

print foo(1,2,3)();
)"));

INSTANTIATE_TEST_SUITE_P(
  TestTypeKindCases, TestNoReturnFromFunction,
  ::testing::Values(
    //
    R"(
fun foo(a, b, c){
  print a;
  print b;
  print c;
}

print foo(1,2, 3);
)"));

int main(int argc, char ** argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
