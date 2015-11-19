#include "Token.h"

#include <algorithm>
#include <sstream>

Token::Token()
    : m_symbolTableEntry(0)
    , m_value("")
    , m_line(-1)
    , m_column(-1)
    , m_tokenType(TokenType::Invalid)
    , m_operatorType(OperatorType::Invalid)
    , m_numberType(NumberType::Invalid)
{
}

Token::Token(const std::string &value, int line, int column)
    : m_symbolTableEntry(0)
    , m_value(value)
    , m_line(line)
    , m_column(column)
    , m_tokenType(TokenType::Identifier)
    , m_operatorType(OperatorType::Invalid)
    , m_numberType(NumberType::Invalid)
{
}

Token::Token(const std::string &value, int line, int column, NumberType::Enum num)
    : m_symbolTableEntry(0)
    , m_value(value)
    , m_line(line)
    , m_column(column)
    , m_tokenType(TokenType::Number)
    , m_operatorType(OperatorType::Invalid)
    , m_numberType(num)
{
}

Token::Token(const std::string &value, int line, int column, TokenType::Enum type)
    : m_symbolTableEntry(0)
    , m_value(value)
    , m_line(line)
    , m_column(column)
    , m_tokenType(type)
    , m_operatorType(OperatorType::Invalid)
    , m_numberType(NumberType::Invalid)
{
}

Token::Token(const std::string &value, int line, int column, TokenType::Enum type, OperatorType::Enum oper)
    : m_symbolTableEntry(0)
    , m_value(value)
    , m_line(line)
    , m_column(column)
    , m_tokenType(type)
    , m_operatorType(oper)
    , m_numberType(NumberType::Invalid)
{
}

TokenType::Enum Token::tokenType() const
{
    return m_tokenType;
}

OperatorType::Enum Token::operatorType() const
{
    return m_operatorType;
}

NumberType::Enum Token::numberType() const
{
    return m_numberType;
}

SymbolTableEntry *Token::symbolTableEntry() const
{
    return m_symbolTableEntry;
}

void Token::setSymbolTableEntry(SymbolTableEntry *entry)
{
    m_symbolTableEntry = entry;
}

std::string Token::value() const
{
    std::string result = m_value;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);

    return result;
}

int Token::valueAsInt() const
{
    std::stringstream ss;

    int value = 0;
    ss << m_value;
    ss >> value;

    return value;
}

float Token::valueAsReal() const
{
    std::stringstream ss;

    float value = 0;
    ss << m_value;
    ss >> value;

    return value;
}

static std::string operToString(OperatorType::Enum oper)
{
    switch (oper) {
    case OperatorType::Equal:
        return "EQ";
    case OperatorType::Greater:
        return "GT";
    case OperatorType::GreaterOrEqual:
        return "GE";
    case OperatorType::Less:
        return "LT";
    case OperatorType::LessOrEqual:
        return "LE";
    case OperatorType::NotEqual:
        return "NE";
    case OperatorType::Add:
        return "PLUS";
    case OperatorType::Or:
        return "OR";
    case OperatorType::Subtract:
        return "MINUS";
    case OperatorType::And:
        return "AND";
    case OperatorType::Divide:
        return "DIVIDE";
    case OperatorType::IntegerDivide:
        return "DIV";
    case OperatorType::Modulus:
        return "MOD";
    case OperatorType::Multiply:
        return "MULTIPLY";
    default:
        return "<<<INVALID OPERATOR>>>";
    }
}

std::ostream &operator<<(std::ostream &stream, const Token &token)
{
    std::string str;

    switch (token.tokenType()) {
    case TokenType::Identifier:
        stream << "ID(" << token.value() << ")";
        break;

    case TokenType::Number:
        stream << "NUMBER(" << token.value() << ")";
        break;

    case TokenType::Begin:
    case TokenType::Do:
    case TokenType::Else:
    case TokenType::End:
    case TokenType::Function:
    case TokenType::If:
    case TokenType::Integer:
    case TokenType::Of:
    case TokenType::Procedure:
    case TokenType::Program:
    case TokenType::Real:
    case TokenType::Then:
    case TokenType::Var:
    case TokenType::While:
        str = token.value();
        std::transform(str.begin(), str.end(), str.begin(), toupper);
        stream << str;
        break;

    case TokenType::AddOp:
        stream << "ADDOP(" << operToString(token.operatorType()) << ")";
        break;

    case TokenType::MulOp:
        stream << "MULOP(" << operToString(token.operatorType()) << ")";
        break;

    case TokenType::RelOp:
        stream << "RELOP(" << operToString(token.operatorType()) << ")";
        break;

    case TokenType::Assign:
        stream << "ASSIGNOP";
        break;

    case TokenType::Not:
        stream << "NOT";
        break;

    case TokenType::Range:
        stream << "RANGE";
        break;

    case TokenType::LBracket:
        stream << "LBRACKET";
        break;

    case TokenType::RBracket:
        stream << "RBRACKET";
        break;

    case TokenType::Colon:
        stream << "COLON";
        break;

    case TokenType::Comma:
        stream << "COMMA";
        break;

    case TokenType::LParen:
        stream << "LPAREN";
        break;

    case TokenType::RParen:
        stream << "RPAREN";
        break;

    case TokenType::Period:
        stream << "DOT";
        break;

    case TokenType::Semicolon:
        stream << "SEMICOL";
        break;

    case TokenType::Eof:
        stream << "EOF";
        break;

    case TokenType::Error:
        stream << "ERROR";
        break;

    default:
        stream << "<<<INVALID TOKEN>>>";
    }

    return stream;
}
