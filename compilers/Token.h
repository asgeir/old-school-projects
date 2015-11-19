#pragma once

#include <string>

struct TokenType
{
    enum Enum
    {
        Identifier,
        Number,

        Keywords = 0x1000,
        Array,
        Begin,
        Do,
        Else,
        End,
        Function,
        If,
        Integer,
        Of,
        Procedure,
        Program,
        Real,
        Then,
        Var,
        While,

        Operators = 0x2000,
        Assign,
        AddOp,
        MulOp,
        Not,
        Range,
        RelOp,

        Punctuation = 0x4000,
        LBracket,
        RBracket,
        Colon,
        Comma,
        LParen,
        RParen,
        Period,
        Semicolon,

        Eof = 0xf000,
        Error,
        Invalid = 0xffff
    };
};

struct OperatorType
{
    enum Enum
    {
        Equal,
        Greater,
        GreaterOrEqual,
        Less,
        LessOrEqual,
        NotEqual,

        AddOp = 0x1000,
        Add,
        Or,
        Subtract,

        MulOp = 0x2000,
        And,
        Divide,
        IntegerDivide,
        Modulus,
        Multiply,

        Invalid = 0xffff
    };
};

struct NumberType
{
    enum Enum
    {
        Integer,
        Real,

        Invalid = 0xff
    };
};

class SymbolTableEntry;

class Token
{
public:
    Token();
    Token(const std::string &value, int line, int column);
    Token(const std::string &value, int line, int column, NumberType::Enum num);
    Token(const std::string &value, int line, int column, TokenType::Enum type);
    Token(const std::string &value, int line, int column, TokenType::Enum type, OperatorType::Enum oper);

    TokenType::Enum tokenType() const;
    OperatorType::Enum operatorType() const;
    NumberType::Enum numberType() const;

    SymbolTableEntry *symbolTableEntry() const;
    void setSymbolTableEntry(SymbolTableEntry *entry);

    bool isIdentifier() const;
    bool isInteger() const { return m_numberType == NumberType::Integer; }
    bool isReal() const;
    bool isKeyword() const;
    bool isOperator() const;

    std::string value() const;
    int valueAsInt() const;
    float valueAsReal() const;

    int line() const { return m_line; }
    int column() const { return m_column; }

private:
    SymbolTableEntry *m_symbolTableEntry;
    std::string m_value;
    int m_line;
    int m_column;
    TokenType::Enum m_tokenType;
    OperatorType::Enum m_operatorType;
    NumberType::Enum m_numberType;
};

std::ostream &operator<<(std::ostream &stream, const Token &token);
