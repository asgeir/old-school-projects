#include "Lexer.h"

static const int kLineSize = 1024;

Lexer::Lexer(std::istream *in, std::ostream *out)
    : yyFlexLexer(in, out)
    , m_column(1)
    , m_line("")
{
    m_line.reserve(kLineSize);
}

void Lexer::ignore()
{
    std::string tokenValue = rawValue();

    m_line += tokenValue;
    m_column += YYLeng();
}

std::string Lexer::rawValue() const
{
    return std::string(YYText(), YYLeng());
}

void Lexer::resetLine()
{
    m_column = 1;
    m_line.clear();
}

const std::string &Lexer::line() const
{
    return m_line;
}

const Token &Lexer::nextToken()
{
    yylex();
    return m_token;
}

const Token &Lexer::token() const
{
    return m_token;
}

int Lexer::addOp(OperatorType::Enum type)
{
    m_token = Token(rawValue(), lineno(), m_column, TokenType::AddOp, type);
    m_line += rawValue();
    m_column += YYLeng();
    return TokenType::AddOp;
}

int Lexer::eof()
{
    m_token = Token(rawValue(), lineno(), m_column, TokenType::Eof);
    return TokenType::Eof;
}

int Lexer::error()
{
    m_token = Token(rawValue(), lineno(), m_column, TokenType::Error);
    m_line += rawValue();
    return TokenType::Error;
}

int Lexer::identifier()
{
    m_token = Token(rawValue(), lineno(), m_column);
    m_line += rawValue();
    m_column += YYLeng();
    return TokenType::Identifier;
}

int Lexer::keyword(TokenType::Enum type)
{
    m_token = Token(rawValue(), lineno(), m_column, type);
    m_line += rawValue();
    m_column += YYLeng();
    return type;
}

int Lexer::mulOp(OperatorType::Enum type)
{
    m_token = Token(rawValue(), lineno(), m_column, TokenType::MulOp, type);
    m_line += rawValue();
    m_column += YYLeng();
    return TokenType::MulOp;
}

int Lexer::number(NumberType::Enum number)
{
    m_token = Token(rawValue(), lineno(), m_column, number);
    m_line += rawValue();
    m_column += YYLeng();
    return number;
}

int Lexer::oper(TokenType::Enum type)
{
    m_token = Token(rawValue(), lineno(), m_column, type);
    m_line += rawValue();
    m_column += YYLeng();
    return type;
}

int Lexer::punctuation(TokenType::Enum type)
{
    m_token = Token(rawValue(), lineno(), m_column, type);
    m_line += rawValue();
    m_column += YYLeng();
    return type;
}

int Lexer::relOp(OperatorType::Enum type)
{
    m_token = Token(rawValue(), lineno(), m_column, TokenType::RelOp, type);
    m_line += rawValue();
    m_column += YYLeng();
    return TokenType::RelOp;
}
