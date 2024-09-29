#include <cpplox/tokenizer.hpp>
#include <cpplox/variant.hpp>

#include <gtest/gtest.h>

TEST(Tokenizer, is_at_end)
{
  {
    const std::string source = "";
    auto tokenizer = lox::Tokenizer(source);
    EXPECT_EQ(tokenizer.is_at_end(), true);
  }

  {
    const std::string source = "abc";
    auto tokenizer = lox::Tokenizer(source);
    EXPECT_EQ(tokenizer.is_at_end(), false);
  }
}

TEST(Tokenizer, scan_left_right_paren)
{
  const std::string source = "()";
  auto tokenizer = lox::Tokenizer(source);
  const auto result = tokenizer.take_tokens();
  EXPECT_EQ(lox::is_variant_v<lox::Tokens>(result), true);

  const auto & tokens = lox::as_variant<lox::Tokens>(result);
  EXPECT_EQ(tokens.size(), 2);

  EXPECT_EQ(tokens[0].type, lox::TokenType::LeftParen);
  EXPECT_EQ(tokens[0].lexeme, "(");

  EXPECT_EQ(tokens[1].type, lox::TokenType::RightParen);
  EXPECT_EQ(tokens[1].lexeme, ")");
}

TEST(Tokenizer, scan_left_right_brace)
{
  const std::string source = "({})";
  auto tokenizer = lox::Tokenizer(source);
  const auto result = tokenizer.take_tokens();
  EXPECT_EQ(lox::is_variant_v<lox::Tokens>(result), true);

  const auto & tokens = lox::as_variant<lox::Tokens>(result);
  EXPECT_EQ(tokens.size(), 4);

  EXPECT_EQ(tokens[1].type, lox::TokenType::LeftBrace);
  EXPECT_EQ(tokens[1].lexeme, "{");

  EXPECT_EQ(tokens[2].type, lox::TokenType::RightBrace);
  EXPECT_EQ(tokens[2].lexeme, "}");
}

TEST(Tokenizer, scan_comma_dot_with_space)
{
  const std::string source = "({, , .})";
  // ==> '(', '{', ',', ',', '.', '}', ')'
  auto tokenizer = lox::Tokenizer(source);
  const auto result = tokenizer.take_tokens();
  EXPECT_EQ(lox::is_variant_v<lox::Tokens>(result), true);

  const auto & tokens = lox::as_variant<lox::Tokens>(result);
  EXPECT_EQ(tokens.size(), 7);

  EXPECT_EQ(tokens[2].type, lox::TokenType::Comma);
  EXPECT_EQ(tokens[2].lexeme, ",");

  EXPECT_EQ(tokens[3].type, lox::TokenType::Comma);
  EXPECT_EQ(tokens[3].lexeme, ",");

  EXPECT_EQ(tokens[4].type, lox::TokenType::Dot);
  EXPECT_EQ(tokens[4].lexeme, ".");
}

TEST(Tokenizer, scan_semicolun_comment)
{
  const std::string source =
    R"(+;
// this is comment,
/
;
)";
  auto tokenizer = lox::Tokenizer(source);
  const auto result = tokenizer.take_tokens();
  EXPECT_EQ(lox::is_variant_v<lox::Tokens>(result), true);

  const auto & tokens = lox::as_variant<lox::Tokens>(result);
  EXPECT_EQ(tokens.size(), 4);

  EXPECT_EQ(tokens[0].type, lox::TokenType::Plus);
  EXPECT_EQ(tokens[0].lexeme, "+");
  EXPECT_EQ(tokens[0].line->number, 1);
  EXPECT_EQ(tokens[0].get_lexical_column(), 1);

  EXPECT_EQ(tokens[1].type, lox::TokenType::Semicolun);
  EXPECT_EQ(tokens[1].lexeme, ";");
  EXPECT_EQ(tokens[1].line->number, 1);
  EXPECT_EQ(tokens[1].get_lexical_column(), 2);

  EXPECT_EQ(tokens[2].type, lox::TokenType::Slash);
  EXPECT_EQ(tokens[2].lexeme, "/");
  EXPECT_EQ(tokens[2].line->number, 3);
  EXPECT_EQ(tokens[2].get_lexical_column(), 1);

  EXPECT_EQ(tokens[3].type, lox::TokenType::Semicolun);
  EXPECT_EQ(tokens[3].lexeme, ";");
  EXPECT_EQ(tokens[3].line->number, 4);
  EXPECT_EQ(tokens[3].get_lexical_column(), 1);
}

TEST(Tokenizer, scan_plus_minus_star_slash)
{
  const std::string source = "+ - * /";
  auto tokenizer = lox::Tokenizer(source);
  const auto result = tokenizer.take_tokens();
  EXPECT_EQ(lox::is_variant_v<lox::Tokens>(result), true);

  const auto & tokens = lox::as_variant<lox::Tokens>(result);
  EXPECT_EQ(tokens.size(), 4);

  EXPECT_EQ(tokens[0].type, lox::TokenType::Plus);
  EXPECT_EQ(tokens[0].lexeme, "+");
  EXPECT_EQ(tokens[0].line->number, 1);
  EXPECT_EQ(tokens[0].get_lexical_column(), 1);

  EXPECT_EQ(tokens[1].type, lox::TokenType::Minus);
  EXPECT_EQ(tokens[1].lexeme, "-");
  EXPECT_EQ(tokens[1].line->number, 1);
  EXPECT_EQ(tokens[1].get_lexical_column(), 3);

  EXPECT_EQ(tokens[2].type, lox::TokenType::Star);
  EXPECT_EQ(tokens[2].lexeme, "*");
  EXPECT_EQ(tokens[2].line->number, 1);
  EXPECT_EQ(tokens[2].get_lexical_column(), 5);

  EXPECT_EQ(tokens[3].type, lox::TokenType::Slash);
  EXPECT_EQ(tokens[3].lexeme, "/");
  EXPECT_EQ(tokens[3].line->number, 1);
  EXPECT_EQ(tokens[3].get_lexical_column(), 7);
}

TEST(Tokenizer, scan_compare)
{
  const std::string source = "!, =, !=, ==, <, >, <=, >=";
  auto tokenizer = lox::Tokenizer(source);
  const auto result = tokenizer.take_tokens();
  EXPECT_EQ(lox::is_variant_v<lox::Tokens>(result), true);

  const auto & tokens = lox::as_variant<lox::Tokens>(result);
  EXPECT_EQ(tokens.size(), 15);

  EXPECT_EQ(tokens[0].type, lox::TokenType::Bang);
  EXPECT_EQ(tokens[0].lexeme, "!");
  EXPECT_EQ(tokens[0].line->number, 1);
  EXPECT_EQ(tokens[0].get_lexical_column(), 1);

  EXPECT_EQ(tokens[2].type, lox::TokenType::Equal);
  EXPECT_EQ(tokens[2].lexeme, "=");
  EXPECT_EQ(tokens[2].line->number, 1);
  EXPECT_EQ(tokens[2].get_lexical_column(), 4);

  EXPECT_EQ(tokens[4].type, lox::TokenType::BangEqual);
  EXPECT_EQ(tokens[4].lexeme, "!=");
  EXPECT_EQ(tokens[4].line->number, 1);
  EXPECT_EQ(tokens[4].get_lexical_column(), 7);

  EXPECT_EQ(tokens[6].type, lox::TokenType::EqualEqual);
  EXPECT_EQ(tokens[6].lexeme, "==");
  EXPECT_EQ(tokens[6].line->number, 1);
  EXPECT_EQ(tokens[6].get_lexical_column(), 11);

  EXPECT_EQ(tokens[8].type, lox::TokenType::Less);
  EXPECT_EQ(tokens[8].lexeme, "<");
  EXPECT_EQ(tokens[8].line->number, 1);
  EXPECT_EQ(tokens[8].get_lexical_column(), 15);

  EXPECT_EQ(tokens[10].type, lox::TokenType::Greater);
  EXPECT_EQ(tokens[10].lexeme, ">");
  EXPECT_EQ(tokens[10].line->number, 1);
  EXPECT_EQ(tokens[10].get_lexical_column(), 18);

  EXPECT_EQ(tokens[12].type, lox::TokenType::LessEqual);
  EXPECT_EQ(tokens[12].lexeme, "<=");
  EXPECT_EQ(tokens[12].line->number, 1);
  EXPECT_EQ(tokens[12].get_lexical_column(), 21);

  EXPECT_EQ(tokens[14].type, lox::TokenType::GreaterEqual);
  EXPECT_EQ(tokens[14].lexeme, ">=");
  EXPECT_EQ(tokens[14].line->number, 1);
  EXPECT_EQ(tokens[14].get_lexical_column(), 25);
}

TEST(Tokenizer, scan_skip)
{
  const std::string source = "+ - \r\t\n * /";
  auto tokenizer = lox::Tokenizer(source);
  const auto result = tokenizer.take_tokens();
  EXPECT_EQ(lox::is_variant_v<lox::Tokens>(result), true);

  const auto & tokens = lox::as_variant<lox::Tokens>(result);
  EXPECT_EQ(tokens.size(), 4);

  EXPECT_EQ(tokens[0].type, lox::TokenType::Plus);
  EXPECT_EQ(tokens[0].lexeme, "+");
  EXPECT_EQ(tokens[0].line->number, 1);
  EXPECT_EQ(tokens[0].get_lexical_column(), 1);

  EXPECT_EQ(tokens[1].type, lox::TokenType::Minus);
  EXPECT_EQ(tokens[1].lexeme, "-");
  EXPECT_EQ(tokens[1].line->number, 1);
  EXPECT_EQ(tokens[1].get_lexical_column(), 3);

  EXPECT_EQ(tokens[2].type, lox::TokenType::Star);
  EXPECT_EQ(tokens[2].lexeme, "*");
  EXPECT_EQ(tokens[2].line->number, 2);
  EXPECT_EQ(tokens[2].get_lexical_column(), 2);

  EXPECT_EQ(tokens[3].type, lox::TokenType::Slash);
  EXPECT_EQ(tokens[3].lexeme, "/");
  EXPECT_EQ(tokens[3].line->number, 2);
  EXPECT_EQ(tokens[3].get_lexical_column(), 4);
}

TEST(Tokenizer, scan_string)
{
  const std::string source = R"(= "xyz";
= "This is a very long comment 1.
This is a very long comment 2.
This is a very long comment 3.")";
  auto tokenizer = lox::Tokenizer(source);
  const auto result = tokenizer.take_tokens();
  EXPECT_EQ(lox::is_variant_v<lox::Tokens>(result), true);
  const auto & tokens = lox::as_variant<lox::Tokens>(result);
  EXPECT_EQ(tokens.size(), 5);

  EXPECT_EQ(tokens[0].type, lox::TokenType::Equal);
  EXPECT_EQ(tokens[0].lexeme, "=");
  EXPECT_EQ(tokens[0].line->number, 1);
  EXPECT_EQ(tokens[0].get_lexical_column(), 1);

  EXPECT_EQ(tokens[1].type, lox::TokenType::String);
  EXPECT_EQ(tokens[1].lexeme, R"(xyz)");
  EXPECT_EQ(tokens[1].line->number, 1);
  // NOTE: for string, I decided to define the start position from the content
  EXPECT_EQ(tokens[1].get_lexical_column(), 4);

  EXPECT_EQ(tokens[2].type, lox::TokenType::Semicolun);
  EXPECT_EQ(tokens[2].lexeme, ";");
  EXPECT_EQ(tokens[2].line->number, 1);
  EXPECT_EQ(tokens[2].get_lexical_column(), 8);

  EXPECT_EQ(tokens[3].type, lox::TokenType::Equal);
  EXPECT_EQ(tokens[3].lexeme, "=");
  EXPECT_EQ(tokens[3].line->number, 2);
  EXPECT_EQ(tokens[3].get_lexical_column(), 1);

  EXPECT_EQ(tokens[4].type, lox::TokenType::String);
  EXPECT_EQ(tokens[4].lexeme, R"(This is a very long comment 1.
This is a very long comment 2.
This is a very long comment 3.)");
  EXPECT_EQ(tokens[4].line->number, 4);
}

TEST(Tokenizer, scan_number)
{
  const std::string source = R"(= 123;
= 123.456;
= 0.123;)";
  auto tokenizer = lox::Tokenizer(source);
  const auto result = tokenizer.take_tokens();
  EXPECT_EQ(lox::is_variant_v<lox::Tokens>(result), true);

  const auto & tokens = lox::as_variant<lox::Tokens>(result);
  EXPECT_EQ(tokens.size(), 9);

  EXPECT_EQ(tokens[1].type, lox::TokenType::Number);
  EXPECT_EQ(tokens[1].lexeme, "123");
  EXPECT_EQ(tokens[1].line->number, 1);
  EXPECT_EQ(tokens[1].get_lexical_column(), 3);

  EXPECT_EQ(tokens[4].type, lox::TokenType::Number);
  EXPECT_EQ(tokens[4].lexeme, "123.456");
  EXPECT_EQ(tokens[4].line->number, 2);
  EXPECT_EQ(tokens[4].get_lexical_column(), 3);

  EXPECT_EQ(tokens[7].type, lox::TokenType::Number);
  EXPECT_EQ(tokens[7].lexeme, "0.123");
  EXPECT_EQ(tokens[7].line->number, 3);
  EXPECT_EQ(tokens[7].get_lexical_column(), 3);
}

TEST(Tokenizer, scan_invalid_number2)
{
  const std::string source = R"(a = 12.a56;)";
  auto tokenizer = lox::Tokenizer(source);
  const auto result = tokenizer.take_tokens();
  EXPECT_EQ(lox::is_variant_v<lox::SyntaxError>(result), true);

  const auto & parse_error = lox::as_variant<lox::SyntaxError>(result);
  EXPECT_EQ(parse_error.kind, lox::SyntaxErrorKind::InvalidNumberError);
  EXPECT_EQ(parse_error.line->number, 1);
  EXPECT_EQ(parse_error.get_lexical_column(), 5 /* 1-start column number */);
}

TEST(Tokenizer, scan_invalid_number1)
{
  {
    const std::string source = R"(= 123;
=   12abbbbb.456;)";
    auto tokenizer = lox::Tokenizer(source);
    const auto result = tokenizer.take_tokens();
    EXPECT_EQ(lox::is_variant_v<lox::SyntaxError>(result), true);

    const auto & parse_error = lox::as_variant<lox::SyntaxError>(result);
    EXPECT_EQ(parse_error.kind, lox::SyntaxErrorKind::InvalidNumberError);
    EXPECT_EQ(parse_error.line->number, 2);
    EXPECT_EQ(parse_error.get_lexical_column(), 5);
  }
  {
    const std::string source = R"(= 123;
=   12abbbbb.456)";
    auto tokenizer = lox::Tokenizer(source);
    const auto result = tokenizer.take_tokens();
    EXPECT_EQ(lox::is_variant_v<lox::SyntaxError>(result), true);

    const auto & parse_error = lox::as_variant<lox::SyntaxError>(result);
    EXPECT_EQ(parse_error.kind, lox::SyntaxErrorKind::InvalidNumberError);
    EXPECT_EQ(parse_error.line->number, 2);
    EXPECT_EQ(parse_error.get_lexical_column(), 5);
  }
  {
    const std::string source = R"(= 123a)";
    auto tokenizer = lox::Tokenizer(source);
    const auto result = tokenizer.take_tokens();
    EXPECT_EQ(lox::is_variant_v<lox::SyntaxError>(result), true);

    const auto & parse_error = lox::as_variant<lox::SyntaxError>(result);
    EXPECT_EQ(parse_error.kind, lox::SyntaxErrorKind::InvalidNumberError);
    EXPECT_EQ(parse_error.line->number, 1);
    EXPECT_EQ(parse_error.get_lexical_column(), 3);
  }
}

TEST(Tokenizer, scan_identifier)
{
  const std::string source = R"(
a1 = 123;
_this = 123.456;
varFoo_1 = "abc";
matched = true;
)";
  auto tokenizer = lox::Tokenizer(source);
  const auto result = tokenizer.take_tokens();
  EXPECT_EQ(lox::is_variant_v<lox::Tokens>(result), true);

  const auto & tokens = lox::as_variant<lox::Tokens>(result);
  EXPECT_EQ(tokens.size(), 16);

  EXPECT_EQ(tokens[0].type, lox::TokenType::Identifier);
  EXPECT_EQ(tokens[0].lexeme, "a1");
  EXPECT_EQ(tokens[0].line->number, 2);
  EXPECT_EQ(tokens[0].get_lexical_column(), 1);

  EXPECT_EQ(tokens[1].type, lox::TokenType::Equal);
  EXPECT_EQ(tokens[1].lexeme, "=");
  EXPECT_EQ(tokens[1].line->number, 2);
  EXPECT_EQ(tokens[1].get_lexical_column(), 4);

  EXPECT_EQ(tokens[2].type, lox::TokenType::Number);
  EXPECT_EQ(tokens[2].lexeme, "123");
  EXPECT_EQ(tokens[2].line->number, 2);
  EXPECT_EQ(tokens[2].get_lexical_column(), 6);

  EXPECT_EQ(tokens[3].type, lox::TokenType::Semicolun);
  EXPECT_EQ(tokens[3].lexeme, ";");
  EXPECT_EQ(tokens[3].line->number, 2);
  EXPECT_EQ(tokens[3].get_lexical_column(), 9);

  EXPECT_EQ(tokens[4].type, lox::TokenType::Identifier);
  EXPECT_EQ(tokens[4].lexeme, "_this");
  EXPECT_EQ(tokens[4].line->number, 3);
  EXPECT_EQ(tokens[4].get_lexical_column(), 1);

  EXPECT_EQ(tokens[5].type, lox::TokenType::Equal);
  EXPECT_EQ(tokens[5].lexeme, "=");
  EXPECT_EQ(tokens[5].line->number, 3);
  EXPECT_EQ(tokens[5].get_lexical_column(), 7);

  EXPECT_EQ(tokens[6].type, lox::TokenType::Number);
  EXPECT_EQ(tokens[6].lexeme, "123.456");
  EXPECT_EQ(tokens[6].line->number, 3);
  EXPECT_EQ(tokens[6].get_lexical_column(), 9);

  EXPECT_EQ(tokens[7].type, lox::TokenType::Semicolun);
  EXPECT_EQ(tokens[7].lexeme, ";");
  EXPECT_EQ(tokens[7].line->number, 3);
  EXPECT_EQ(tokens[7].get_lexical_column(), 16);

  EXPECT_EQ(tokens[8].type, lox::TokenType::Identifier);
  EXPECT_EQ(tokens[8].lexeme, "varFoo_1");
  EXPECT_EQ(tokens[8].line->number, 4);
  EXPECT_EQ(tokens[8].get_lexical_column(), 1);

  EXPECT_EQ(tokens[9].type, lox::TokenType::Equal);
  EXPECT_EQ(tokens[9].lexeme, "=");
  EXPECT_EQ(tokens[9].line->number, 4);
  EXPECT_EQ(tokens[9].get_lexical_column(), 10);

  EXPECT_EQ(tokens[10].type, lox::TokenType::String);
  EXPECT_EQ(tokens[10].lexeme, "abc");
  EXPECT_EQ(tokens[10].line->number, 4);
  EXPECT_EQ(tokens[10].get_lexical_column(), 13);

  EXPECT_EQ(tokens[11].type, lox::TokenType::Semicolun);
  EXPECT_EQ(tokens[11].lexeme, ";");
  EXPECT_EQ(tokens[11].line->number, 4);
  EXPECT_EQ(tokens[11].get_lexical_column(), 17);

  EXPECT_EQ(tokens[12].type, lox::TokenType::Identifier);
  EXPECT_EQ(tokens[12].lexeme, "matched");
  EXPECT_EQ(tokens[12].line->number, 5);
  EXPECT_EQ(tokens[12].get_lexical_column(), 1);

  EXPECT_EQ(tokens[13].type, lox::TokenType::Equal);
  EXPECT_EQ(tokens[13].lexeme, "=");
  EXPECT_EQ(tokens[13].line->number, 5);
  EXPECT_EQ(tokens[13].get_lexical_column(), 9);

  EXPECT_EQ(tokens[14].type, lox::TokenType::True);
  EXPECT_EQ(tokens[14].lexeme, "true");
  EXPECT_EQ(tokens[14].line->number, 5);
  EXPECT_EQ(tokens[14].get_lexical_column(), 11);

  EXPECT_EQ(tokens[15].type, lox::TokenType::Semicolun);
  EXPECT_EQ(tokens[15].lexeme, ";");
  EXPECT_EQ(tokens[15].line->number, 5);
  EXPECT_EQ(tokens[15].get_lexical_column(), 15);
}

TEST(Tokenizer, scan_keyword)
{
  const std::string source = R"(
cond1 = (true and true);
cond2 = (false or false);
cond3 = (cond1 and cond2);
)";
  auto tokenizer = lox::Tokenizer(source);
  const auto result = tokenizer.take_tokens();
  EXPECT_EQ(lox::is_variant_v<lox::Tokens>(result), true);

  const auto & tokens = lox::as_variant<lox::Tokens>(result);
  EXPECT_EQ(tokens.size(), 24);

  auto base = 0;
  EXPECT_EQ(tokens[base + 0].type, lox::TokenType::Identifier);
  EXPECT_EQ(tokens[base + 0].lexeme, "cond1");
  EXPECT_EQ(tokens[base + 0].line->number, 2);
  EXPECT_EQ(tokens[base + 0].get_lexical_column(), 1);

  EXPECT_EQ(tokens[base + 1].type, lox::TokenType::Equal);
  EXPECT_EQ(tokens[base + 1].lexeme, "=");
  EXPECT_EQ(tokens[base + 1].line->number, 2);
  EXPECT_EQ(tokens[base + 1].get_lexical_column(), 7);

  EXPECT_EQ(tokens[base + 2].type, lox::TokenType::LeftParen);
  EXPECT_EQ(tokens[base + 2].lexeme, "(");
  EXPECT_EQ(tokens[base + 2].line->number, 2);
  EXPECT_EQ(tokens[base + 2].get_lexical_column(), 9);

  EXPECT_EQ(tokens[base + 3].type, lox::TokenType::True);
  EXPECT_EQ(tokens[base + 3].lexeme, "true");
  EXPECT_EQ(tokens[base + 3].line->number, 2);
  EXPECT_EQ(tokens[base + 3].get_lexical_column(), 10);

  EXPECT_EQ(tokens[base + 4].type, lox::TokenType::And);
  EXPECT_EQ(tokens[base + 4].lexeme, "and");
  EXPECT_EQ(tokens[base + 4].line->number, 2);
  EXPECT_EQ(tokens[base + 4].get_lexical_column(), 15);

  EXPECT_EQ(tokens[base + 5].type, lox::TokenType::True);
  EXPECT_EQ(tokens[base + 5].lexeme, "true");
  EXPECT_EQ(tokens[base + 5].line->number, 2);
  EXPECT_EQ(tokens[base + 5].get_lexical_column(), 19);

  EXPECT_EQ(tokens[base + 6].type, lox::TokenType::RightParen);
  EXPECT_EQ(tokens[base + 6].lexeme, ")");
  EXPECT_EQ(tokens[base + 6].line->number, 2);
  EXPECT_EQ(tokens[base + 6].get_lexical_column(), 23);

  EXPECT_EQ(tokens[base + 7].type, lox::TokenType::Semicolun);
  EXPECT_EQ(tokens[base + 7].lexeme, ";");
  EXPECT_EQ(tokens[base + 7].line->number, 2);
  EXPECT_EQ(tokens[base + 7].get_lexical_column(), 24);

  base = 8;
  EXPECT_EQ(tokens[base + 0].type, lox::TokenType::Identifier);
  EXPECT_EQ(tokens[base + 0].lexeme, "cond2");
  EXPECT_EQ(tokens[base + 0].line->number, 3);
  EXPECT_EQ(tokens[base + 0].get_lexical_column(), 1);

  EXPECT_EQ(tokens[base + 1].type, lox::TokenType::Equal);
  EXPECT_EQ(tokens[base + 1].lexeme, "=");
  EXPECT_EQ(tokens[base + 1].line->number, 3);
  EXPECT_EQ(tokens[base + 1].get_lexical_column(), 7);

  EXPECT_EQ(tokens[base + 2].type, lox::TokenType::LeftParen);
  EXPECT_EQ(tokens[base + 2].lexeme, "(");
  EXPECT_EQ(tokens[base + 2].line->number, 3);
  EXPECT_EQ(tokens[base + 2].get_lexical_column(), 9);

  EXPECT_EQ(tokens[base + 3].type, lox::TokenType::False);
  EXPECT_EQ(tokens[base + 3].lexeme, "false");
  EXPECT_EQ(tokens[base + 3].line->number, 3);
  EXPECT_EQ(tokens[base + 3].get_lexical_column(), 10);

  EXPECT_EQ(tokens[base + 4].type, lox::TokenType::Or);
  EXPECT_EQ(tokens[base + 4].lexeme, "or");
  EXPECT_EQ(tokens[base + 4].line->number, 3);
  EXPECT_EQ(tokens[base + 4].get_lexical_column(), 16);

  EXPECT_EQ(tokens[base + 5].type, lox::TokenType::False);
  EXPECT_EQ(tokens[base + 5].lexeme, "false");
  EXPECT_EQ(tokens[base + 5].line->number, 3);
  EXPECT_EQ(tokens[base + 5].get_lexical_column(), 19);

  EXPECT_EQ(tokens[base + 6].type, lox::TokenType::RightParen);
  EXPECT_EQ(tokens[base + 6].lexeme, ")");
  EXPECT_EQ(tokens[base + 6].line->number, 3);
  EXPECT_EQ(tokens[base + 6].get_lexical_column(), 24);

  EXPECT_EQ(tokens[base + 7].type, lox::TokenType::Semicolun);
  EXPECT_EQ(tokens[base + 7].lexeme, ";");
  EXPECT_EQ(tokens[base + 7].line->number, 3);
  EXPECT_EQ(tokens[base + 7].get_lexical_column(), 25);

  base = 16;
  EXPECT_EQ(tokens[base + 0].type, lox::TokenType::Identifier);
  EXPECT_EQ(tokens[base + 0].lexeme, "cond3");
  EXPECT_EQ(tokens[base + 0].line->number, 4);
  EXPECT_EQ(tokens[base + 0].get_lexical_column(), 1);

  EXPECT_EQ(tokens[base + 1].type, lox::TokenType::Equal);
  EXPECT_EQ(tokens[base + 1].lexeme, "=");
  EXPECT_EQ(tokens[base + 1].line->number, 4);
  EXPECT_EQ(tokens[base + 1].get_lexical_column(), 7);

  EXPECT_EQ(tokens[base + 2].type, lox::TokenType::LeftParen);
  EXPECT_EQ(tokens[base + 2].lexeme, "(");
  EXPECT_EQ(tokens[base + 2].line->number, 4);
  EXPECT_EQ(tokens[base + 2].get_lexical_column(), 9);

  EXPECT_EQ(tokens[base + 3].type, lox::TokenType::Identifier);
  EXPECT_EQ(tokens[base + 3].lexeme, "cond1");
  EXPECT_EQ(tokens[base + 3].line->number, 4);
  EXPECT_EQ(tokens[base + 3].get_lexical_column(), 10);

  EXPECT_EQ(tokens[base + 4].type, lox::TokenType::And);
  EXPECT_EQ(tokens[base + 4].lexeme, "and");
  EXPECT_EQ(tokens[base + 4].line->number, 4);
  EXPECT_EQ(tokens[base + 4].get_lexical_column(), 16);

  EXPECT_EQ(tokens[base + 5].type, lox::TokenType::Identifier);
  EXPECT_EQ(tokens[base + 5].lexeme, "cond2");
  EXPECT_EQ(tokens[base + 5].line->number, 4);
  EXPECT_EQ(tokens[base + 5].get_lexical_column(), 20);

  EXPECT_EQ(tokens[base + 6].type, lox::TokenType::RightParen);
  EXPECT_EQ(tokens[base + 6].lexeme, ")");
  EXPECT_EQ(tokens[base + 6].line->number, 4);
  EXPECT_EQ(tokens[base + 6].get_lexical_column(), 25);

  EXPECT_EQ(tokens[base + 7].type, lox::TokenType::Semicolun);
  EXPECT_EQ(tokens[base + 7].lexeme, ";");
  EXPECT_EQ(tokens[base + 7].line->number, 4);
  EXPECT_EQ(tokens[base + 7].get_lexical_column(), 26);
}

int main(int argc, char ** argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
