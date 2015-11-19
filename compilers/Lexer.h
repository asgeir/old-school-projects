#pragma once

#include "Token.h"

#include <FlexLexer.h>

class Lexer : public yyFlexLexer
{
public:
    Lexer(std::istream *in = 0, std::ostream *out = 0);

    virtual int yylex();

    const std::string &line() const;
    const Token &nextToken();
    const Token &token() const;

private:
    void ignore();
    std::string rawValue() const;
    void resetLine();

    int addOp(OperatorType::Enum type);
    int eof();
    int error();
    int identifier();
    int keyword(TokenType::Enum type);
    int mulOp(OperatorType::Enum type);
    int number(NumberType::Enum number);
    int oper(TokenType::Enum type);
    int punctuation(TokenType::Enum type);
    int relOp(OperatorType::Enum type);

    Token m_token;
    int m_column;
    std::string m_line;
};
