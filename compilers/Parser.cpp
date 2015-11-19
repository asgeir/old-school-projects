#include "Parser.h"
#include "Ast.h"
#include "Lexer.h"

#include <algorithm>
#include <iostream>

namespace ErrorCodes
{
    enum Enum
    {
        Recovering = -1,
        NoError = 0,

        ExpectedAssignment,
        ExpectedColon,
        ExpectedDo,
        ExpectedBegin,
        ExpectedElse,
        ExpectedEnd,
        ExpectedIdentifier,
        ExpectedLBracket,
        ExpectedLParen,
        ExpectedNumber,
        ExpectedOf,
        ExpectedPeriod,
        ExpectedProgram,
        ExpectedRange,
        ExpectedRBracket,
        ExpectedRParen,
        ExpectedSemicolon,
        ExpectedStandardType,
        ExpectedStatement,
        ExpectedSubprogramHead,
        ExpectedThen,
    };
};

static std::string codeToMessage(int errorCode);

Parser::Parser()
    : m_errorCode(ErrorCodes::NoError)
    , m_errorCount(0)
{
}

bool Parser::error() const
{
    return m_errorCode != ErrorCodes::NoError;
}

int Parser::errorCount() const
{
    return m_errorCount;
}

ProgramPtr Parser::parse(boost::shared_ptr<Lexer> lexer)
{
    m_lexer = lexer;
    m_curToken = m_lexer->nextToken();
    return parseProgram();
}

bool Parser::match(TokenType::Enum tokenType)
{
    bool match = m_curToken.tokenType() == tokenType;
    if (match) {
        m_curToken = m_lexer->nextToken();
    }

    return match;
}

void Parser::panic(const TokenType::Enum synchronizingTokens[], int numElements)
{
    const TokenType::Enum *begin = synchronizingTokens;
    const TokenType::Enum *end = synchronizingTokens + numElements;

    while (std::find(begin, end, m_curToken.tokenType()) == end) {
        m_curToken = m_lexer->nextToken();

        if (m_curToken.tokenType() == TokenType::Eof) {
            // always sync on eof
            break;
        }
    }

    m_errorCode = ErrorCodes::Recovering;
}

void Parser::reportError(int errorCode)
{
    m_errorCode = errorCode;

    std::cerr << "ERROR in line " << m_curToken.line() << ": ";
    if (m_curToken.tokenType() == TokenType::Error) {
        std::cerr << "Illegal character";
    } else {
        std::cerr << codeToMessage(errorCode);
    }
    std::cerr << std::endl;
    std::cerr << "\t" << m_lexer->line() << std::endl;

    std::cerr << "\t";
    for (int i = 1; i < m_curToken.column(); ++i) {
        std::cerr << " ";
    }

    std::cerr << "^";
    for (int i = 0; i < m_curToken.value().size() - 1; ++i) {
        std::cerr << "~";
    }

    std::cerr << std::endl;

    ++m_errorCount;
}

DeclarationsPtr Parser::parseArgumentList()
{
    IdentifiersPtr identifiers = parseIdentifierList();
    if (m_errorCode > ErrorCodes::NoError) {
        return DeclarationsPtr();
    }

    if (!match(TokenType::Colon)) {
        reportError(ErrorCodes::ExpectedColon);
        return DeclarationsPtr();
    }

    TypePtr type = parseType();
    if (m_errorCode > ErrorCodes::NoError) {
        return DeclarationsPtr();
    }

    DeclarationsPtr declarations(new Declarations);
    for (std::vector<IdentifierPtr>::const_iterator i = identifiers->list.begin(); i != identifiers->list.end(); ++i) {
        DeclarationPtr declaration(new Declaration);
        declaration->id = *i;
        declaration->type = type;

        declarations->list.push_back(declaration);
    }

    DeclarationsPtr rest = parseArgumentList_r();
    if (rest) {
        declarations->list.insert(declarations->list.end(), rest->list.begin(), rest->list.end());
    }

    return declarations;
}

DeclarationsPtr Parser::parseArgumentList_r()
{
    if (match(TokenType::Semicolon)) {
        return parseArgumentList();
    }

    return DeclarationsPtr();
}

void Parser::parseArguments(SubprogramDeclarationPtr sub)
{
    if (match(TokenType::LParen)) {
        DeclarationsPtr arguments = parseArgumentList();

        if (!match(TokenType::RParen)) {
            reportError(ErrorCodes::ExpectedRParen);
            return;
        }

        sub->arguments = arguments;
        return;
    }
}

static const TokenType::Enum compoundStatementFollow[] = {
    TokenType::Eof,
    TokenType::Period,
    TokenType::Semicolon,
};
static const int compoundStatementFollowSize = 3;

CompoundStatementPtr Parser::parseCompoundStatement()
{
    if (!match(TokenType::Begin)) {
        reportError(ErrorCodes::ExpectedBegin);

        panic(compoundStatementFollow, compoundStatementFollowSize);
        return CompoundStatementPtr();
    }

    StatementsPtr statements = parseOptionalStatements();
    if (m_errorCode && m_errorCode != ErrorCodes::ExpectedStatement) {
        panic(compoundStatementFollow, compoundStatementFollowSize);
        return CompoundStatementPtr();
    }

    m_errorCode = ErrorCodes::NoError;

    if (!match(TokenType::End)) {
        reportError(ErrorCodes::ExpectedEnd);

        panic(compoundStatementFollow, compoundStatementFollowSize);
        return CompoundStatementPtr();
    }

    CompoundStatementPtr compoundStatement(new CompoundStatement);
    compoundStatement->statements = statements;

    return compoundStatement;
}

static const TokenType::Enum declarationsFollow[] = {
    TokenType::Var,
    TokenType::Procedure,
    TokenType::Function,
    TokenType::Begin,
    TokenType::Eof
};
static const int declarationsFollowSize = 5;

DeclarationsPtr Parser::parseDeclarations()
{
    if (!match(TokenType::Var)) {
        return DeclarationsPtr();
    }

    IdentifiersPtr identifiers = parseIdentifierList();
    if (m_errorCode > ErrorCodes::NoError) {
        panic(declarationsFollow, declarationsFollowSize);
        return DeclarationsPtr();
    }

    if (!match(TokenType::Colon)) {
        reportError(ErrorCodes::ExpectedColon);

        panic(declarationsFollow, declarationsFollowSize);
        return DeclarationsPtr();
    }

    TypePtr type = parseType();
    if (m_errorCode > ErrorCodes::NoError) {
        panic(declarationsFollow, declarationsFollowSize);
        return DeclarationsPtr();
    }

    if (!match(TokenType::Semicolon)) {
        reportError(ErrorCodes::ExpectedSemicolon);

        panic(declarationsFollow, declarationsFollowSize);
        return DeclarationsPtr();
    }

    DeclarationsPtr declarations(new Declarations);
    for (std::vector<IdentifierPtr>::const_iterator i = identifiers->list.begin(); i != identifiers->list.end(); ++i) {
        DeclarationPtr declaration(new Declaration);
        declaration->id = *i;
        declaration->type = type;

        declarations->list.push_back(declaration);
    }

    DeclarationsPtr rest = parseDeclarations();
    if (rest) {
        declarations->list.insert(declarations->list.end(), rest->list.begin(), rest->list.end());
    }

    return declarations;
}

ExpressionPtr Parser::parseExpression()
{
    RelOpExpressionPtr expr(new RelOpExpression);

    expr->lhs = parseSimpleExpression();
    parseExpression_p(expr);

    if (expr->rhs) {
        return expr;
    }

    return expr->lhs;
}

void Parser::parseExpression_p(RelOpExpressionPtr expr)
{
    Token op = m_curToken;
    if (match(TokenType::RelOp)) {
        expr->relOp = op.operatorType();
        expr->rhs = parseSimpleExpression();
        return;
    }

    // is empty
    return;
}

ExpressionsPtr Parser::parseExpressionList()
{
    if (m_curToken.tokenType() != TokenType::Identifier &&
        m_curToken.tokenType() != TokenType::AddOp &&
        m_curToken.tokenType() != TokenType::Number &&
        m_curToken.tokenType() != TokenType::LParen &&
        m_curToken.tokenType() != TokenType::Not) {
        return ExpressionsPtr();
    }

    ExpressionsPtr expressions(new Expressions);

    ExpressionPtr curExpression = parseExpression();
    if (m_errorCode > ErrorCodes::NoError) {
        return ExpressionsPtr();
    }

    expressions->list.push_back(curExpression);

    ExpressionsPtr rest = parseExpressionList_r();
    if (rest) {
        expressions->list.insert(expressions->list.end(), rest->list.begin(), rest->list.end());
    }

    return expressions;
}

ExpressionsPtr Parser::parseExpressionList_r()
{
    if (match(TokenType::Comma)) {
        return parseExpressionList();
    }

    return ExpressionsPtr();
}

static const TokenType::Enum factorFollow[] = {
    TokenType::Do,
    TokenType::End,
    TokenType::Eof,
    TokenType::MulOp,
    TokenType::Period,
    TokenType::Semicolon,
    TokenType::RelOp,
    TokenType::RParen,
};
static const int factorFollowSize = 7;

FactorPtr Parser::parseFactor()
{
    FactorPtr factor(new Factor);

    if (m_curToken.tokenType() == TokenType::Number) {
        factor->number = parseNumber();
        if (m_errorCode > ErrorCodes::NoError) {
            panic(factorFollow, factorFollowSize);
            return FactorPtr();
        }

        return factor;
    } else if (m_curToken.tokenType() == TokenType::LParen) {
        match(TokenType::LParen);

        ExpressionPtr expr = parseExpression();
        if (m_errorCode > ErrorCodes::NoError) {
            panic(factorFollow, factorFollowSize);
            return FactorPtr();
        }

        if (!match(TokenType::RParen)) {
            reportError(ErrorCodes::ExpectedRParen);

            panic(factorFollow, factorFollowSize);
            return FactorPtr();
        }

        factor->expression = expr;
        return factor;
    } else if (m_curToken.tokenType() == TokenType::Not) {
        match(TokenType::Not);

        factor->negatedFactor = parseFactor();
        if (m_errorCode > ErrorCodes::NoError) {
            panic(factorFollow, factorFollowSize);
            return FactorPtr();
        }

        return factor;
    }

    IdentifierPtr identifier = parseIdentifier();
    if (m_errorCode > ErrorCodes::NoError) {
        panic(factorFollow, factorFollowSize);
        return FactorPtr();
    }

    parseFactor_p(identifier);
    if (m_errorCode > ErrorCodes::NoError) {
        panic(factorFollow, factorFollowSize);
        return FactorPtr();
    }

    factor->id = identifier;
    return factor;
}

void Parser::parseFactor_p(IdentifierPtr id)
{
    if (m_curToken.tokenType() == TokenType::LBracket) {
        id->index = parseOptionalIndex();
        return;
    } else if (match(TokenType::LParen)) {
        id->parameters = parseExpressionList();
        if (m_errorCode > ErrorCodes::NoError) {
            panic(factorFollow, factorFollowSize);
            return;
        }

        if (!match(TokenType::RParen)) {
            reportError(ErrorCodes::ExpectedRParen);

            panic(factorFollow, factorFollowSize);
            return;
        }

        if (!id->parameters) {
            // empty param list
            ExpressionsPtr tmp(new Expressions);
            id->parameters = tmp;
        }

        return;
    }

    // is empty
}

IdentifierPtr Parser::parseIdentifier()
{
    Token curIdentifier = m_curToken;
    if (!match(TokenType::Identifier)) {
        reportError(ErrorCodes::ExpectedIdentifier);
        return IdentifierPtr();
    }

    IdentifierPtr id(new Identifier);
    id->id = curIdentifier;

    return id;
}

static const TokenType::Enum identifierListFollow[] = {
    TokenType::Colon,
    TokenType::Semicolon,
    TokenType::RParen,
};
static const int identifierListFollowSize = 3;

IdentifiersPtr Parser::parseIdentifierList()
{
    IdentifierPtr curIdentifier = parseIdentifier();
    if (m_errorCode > ErrorCodes::NoError) {
        panic(identifierListFollow, identifierListFollowSize);
        return IdentifiersPtr();
    }

    IdentifiersPtr identifiers(new Identifiers);
    identifiers->list.push_back(curIdentifier);

    IdentifiersPtr rest = parseIdentifierList_r();
    if (rest) {
        identifiers->list.insert(identifiers->list.end(), rest->list.begin(), rest->list.end());
    }

    return identifiers;
}

IdentifiersPtr Parser::parseIdentifierList_r()
{
    if (match(TokenType::Comma)) {
        return parseIdentifierList();
    }

    return IdentifiersPtr();
}

NumberPtr Parser::parseNumber()
{
    Token numberValue = m_curToken;
    if (!match(TokenType::Number)) {
        reportError(ErrorCodes::ExpectedNumber);
        return NumberPtr();
    }

    NumberPtr number(new Number);
    number->value = numberValue;

    return number;
}

static const TokenType::Enum optionalIndexFollow[] = {
    TokenType::Assign,
    TokenType::End,
    TokenType::Eof,
    TokenType::MulOp,
    TokenType::Period,
    TokenType::Semicolon,
};
static const int optionalIndexFollowSize = 6;

ExpressionPtr Parser::parseOptionalIndex()
{
    if (match(TokenType::LBracket)) {
        ExpressionPtr indexExpression = parseExpression();
        if (m_errorCode > ErrorCodes::NoError) {
            panic(optionalIndexFollow, optionalIndexFollowSize);
            return ExpressionPtr();
        }

        if (!match(TokenType::RBracket)) {
            reportError(ErrorCodes::ExpectedRBracket);

            panic(optionalIndexFollow, optionalIndexFollowSize);
            return ExpressionPtr();
        }

        return indexExpression;
    }

    return ExpressionPtr();
}

ExpressionsPtr Parser::parseOptionalParameters()
{
    if (match(TokenType::LParen)) {
        ExpressionsPtr parameters = parseExpressionList();
        if (m_errorCode > ErrorCodes::NoError) {
            return ExpressionsPtr();
        }

        if (!match(TokenType::RParen)) {
            reportError(ErrorCodes::ExpectedRParen);
            return ExpressionsPtr();
        }

        if (!parameters) {
            // empty parameter list
            ExpressionsPtr tmp(new Expressions);
            parameters = tmp;
        }

        return parameters;
    }

    return ExpressionsPtr();
}

static const TokenType::Enum optionalStatementsFollow[] = {
    TokenType::End,
    TokenType::Eof,
    TokenType::Period,
    TokenType::Semicolon,
};
static const int optionalStatementsFollowSize = 4;

StatementsPtr Parser::parseOptionalStatements()
{
    if (m_curToken.tokenType() != TokenType::Identifier &&
        m_curToken.tokenType() != TokenType::Begin &&
        m_curToken.tokenType() != TokenType::If &&
        m_curToken.tokenType() != TokenType::While) {
        return StatementsPtr();
    }

    StatementsPtr statements = parseStatementList();
    if (m_errorCode > ErrorCodes::NoError) {
        panic(optionalStatementsFollow, optionalStatementsFollowSize);
        return StatementsPtr();
    }

    return statements;
}

static const TokenType::Enum programFollow[] = {
    TokenType::Eof,
};
static const int programFollowSize = 1;

ProgramPtr Parser::parseProgram()
{
    if (!match(TokenType::Program)) {
        reportError(ErrorCodes::ExpectedProgram);

        panic(programFollow, programFollowSize);
        return ProgramPtr();
    }

    IdentifierPtr programName = parseIdentifier();
    if (m_errorCode > ErrorCodes::NoError) {
        panic(programFollow, programFollowSize);
        return ProgramPtr();
    }

    if (!match(TokenType::LParen)) {
        reportError(ErrorCodes::ExpectedLParen);

        panic(programFollow, programFollowSize);
        return ProgramPtr();
    }

    IdentifiersPtr inputOutput = parseIdentifierList();
    if (m_errorCode > ErrorCodes::NoError) {
        panic(programFollow, programFollowSize);
        return ProgramPtr();
    }

    if (!match(TokenType::RParen)) {
        reportError(ErrorCodes::ExpectedRParen);

        panic(programFollow, programFollowSize);
        return ProgramPtr();
    }

    if (!match(TokenType::Semicolon)) {
        reportError(ErrorCodes::ExpectedSemicolon);

        panic(programFollow, programFollowSize);
        return ProgramPtr();
    }

    DeclarationsPtr globalVariables = parseDeclarations();
    if (m_errorCode > ErrorCodes::NoError) {
        panic(programFollow, programFollowSize);
        return ProgramPtr();
    }

    SubprogramDeclarationsPtr functions = parseSubprogramDeclarations();
    if (m_errorCode > ErrorCodes::NoError) {
        panic(programFollow, programFollowSize);
        return ProgramPtr();
    }

    CompoundStatementPtr mainProgram = parseCompoundStatement();
    if (m_errorCode > ErrorCodes::NoError) {
        panic(programFollow, programFollowSize);
        return ProgramPtr();
    }

    if (!match(TokenType::Period)) {
        reportError(ErrorCodes::ExpectedPeriod);

        panic(programFollow, programFollowSize);
        return ProgramPtr();
    }

    ProgramPtr program(new Program);
    program->programName = programName;
    program->inputOutput = inputOutput;
    program->variables = globalVariables;
    program->functions = functions;
    program->mainProgram = mainProgram;

    return program;
}

static const TokenType::Enum simpleExpressionFollow[] = {
    TokenType::End,
    TokenType::Eof,
    TokenType::Do,
    TokenType::Period,
    TokenType::Semicolon,
    TokenType::RelOp,
    TokenType::RParen,
};
static const int simpleExpressionFollowSize = 6;

ExpressionPtr Parser::parseSimpleExpression()
{
    AddOpExpressionPtr expr(new AddOpExpression);

    Token sign = m_curToken;
    if (sign.operatorType() != OperatorType::Or && match(TokenType::AddOp)) {
        if (sign.operatorType() == OperatorType::Subtract) {
            expr->lhsNegated = true;
        } else {
            expr->lhsNegated = false;
        }
    }

    expr->lhs = parseTerm();
    if (m_errorCode > ErrorCodes::NoError) {
        panic(simpleExpressionFollow, simpleExpressionFollowSize);
        return ExpressionPtr();
    }

    parseSimpleExpression_r(expr);
    if (m_errorCode > ErrorCodes::NoError) {
        panic(simpleExpressionFollow, simpleExpressionFollowSize);
        return ExpressionPtr();
    }

    return expr;
}

void Parser::parseSimpleExpression_r(AddOpExpressionPtr expr)
{
    Token op = m_curToken;
    if (match(TokenType::AddOp)) {
        expr->addOp = op.operatorType();

        AddOpExpressionPtr rhs(new AddOpExpression);
        rhs->lhs = parseTerm();
        if (m_errorCode > ErrorCodes::NoError) {
            return;
        }

        expr->rhs = rhs;

        parseSimpleExpression_r(rhs);
        return;
    }

    return;
}

TypePtr Parser::parseStandardType()
{
    TypePtr type(new Type);

    if (m_curToken.tokenType() == TokenType::Integer) {
        match(TokenType::Integer);

        type->standardType = NumberType::Integer;

        return type;
    } else if (m_curToken.tokenType() == TokenType::Real) {
        match(TokenType::Real);

        type->standardType = NumberType::Real;

        return type;
    }

    reportError(ErrorCodes::ExpectedStandardType);
    return TypePtr();
}

static const TokenType::Enum statementFollow[] = {
    TokenType::End,
    TokenType::Eof,
    TokenType::Period,
    TokenType::Semicolon,
};
static const int statementFollowSize = 4;

StatementPtr Parser::parseStatement()
{
    if (m_curToken.tokenType() == TokenType::Begin) {
        return parseCompoundStatement();
    } else if (match(TokenType::If)) {
        ExpressionPtr expression = parseExpression();
        if (m_errorCode > ErrorCodes::NoError) {
            panic(statementFollow, statementFollowSize);
            return StatementPtr();
        }

        if (!match(TokenType::Then)) {
            reportError(ErrorCodes::ExpectedThen);

            panic(statementFollow, statementFollowSize);
            return StatementPtr();
        }

        StatementPtr thenStatement = parseStatement();
        if (m_errorCode > ErrorCodes::NoError) {
            panic(statementFollow, statementFollowSize);
            return StatementPtr();
        }

        if (!match(TokenType::Else)) {
            reportError(ErrorCodes::ExpectedElse);

            panic(statementFollow, statementFollowSize);
            return StatementPtr();
        }

        StatementPtr elseStatement = parseStatement();
        if (m_errorCode > ErrorCodes::NoError) {
            panic(statementFollow, statementFollowSize);
            return StatementPtr();
        }

        IfStatementPtr statement(new IfStatement);
        statement->expression = expression;
        statement->thenPart = thenStatement;
        statement->elsePart = elseStatement;

        return statement;
    } else if (match(TokenType::While)) {
        ExpressionPtr expression = parseExpression();
        if (m_errorCode > ErrorCodes::NoError) {
            panic(statementFollow, statementFollowSize);
            return StatementPtr();
        }

        if (!match(TokenType::Do)) {
            reportError(ErrorCodes::ExpectedDo);

            panic(statementFollow, statementFollowSize);
            return StatementPtr();
        }

        StatementPtr doPart = parseStatement();
        if (m_errorCode > ErrorCodes::NoError) {
            panic(statementFollow, statementFollowSize);
            return StatementPtr();
        }

        WhileStatementPtr whileStatement(new WhileStatement);
        whileStatement->expression = expression;
        whileStatement->doPart = doPart;

        return whileStatement;
    }

    IdentifierPtr identifier = parseIdentifier();
    if (m_errorCode > ErrorCodes::NoError) {
        panic(statementFollow, statementFollowSize);
        return StatementPtr();
    }

    AssignmentOrCallStatementPtr statement(new AssignmentOrCallStatement);
    statement->id = identifier;

    parseStatement_p(statement);
    if (m_errorCode > ErrorCodes::NoError) {
        panic(statementFollow, statementFollowSize);
        return StatementPtr();
    }

    return statement;
}

void Parser::parseStatement_p(AssignmentOrCallStatementPtr statement)
{
    ExpressionsPtr parameters = parseOptionalParameters();
    if (parameters) {
        statement->id->parameters = parameters;
        return;
    }

    ExpressionPtr index = parseOptionalIndex();
    statement->id->index = index;

    if (match(TokenType::Assign)) {
        ExpressionPtr value = parseExpression();
        if (m_errorCode > ErrorCodes::NoError) {
            panic(statementFollow, statementFollowSize);
            return;
        }

        statement->value = value;
    }
}

static const TokenType::Enum statementListFollow[] = {
    TokenType::End,
    TokenType::Eof,
    TokenType::Period,
    TokenType::Semicolon,
};
static const int statementListFollowSize = 4;

StatementsPtr Parser::parseStatementList()
{
    StatementPtr statement = parseStatement();
    if (m_errorCode > ErrorCodes::NoError) {
        panic(statementListFollow, statementListFollowSize);
        return StatementsPtr();
    }

    StatementsPtr statements(new Statements);
    statements->list.push_back(statement);

    // concat this to the result
    StatementsPtr rest = parseStatementList_r();
    if (rest) {
        statements->list.insert(statements->list.end(), rest->list.begin(), rest->list.end());
    }

    return statements;
}

StatementsPtr Parser::parseStatementList_r()
{
    if (match(TokenType::Semicolon)) {
        return parseStatementList();
    }

    return StatementsPtr();
}

static const TokenType::Enum subprogramDeclarationFollow[] = {
    TokenType::Begin,
    TokenType::Eof,
    TokenType::Period,
    TokenType::Semicolon,
};
static const int subprogramDeclarationFollowSize = 4;

SubprogramDeclarationPtr Parser::parseSubprogramDeclaration()
{
    SubprogramDeclarationPtr sub(new SubprogramDeclaration);

    parseSubprogramHead(sub);
    if (m_errorCode > ErrorCodes::NoError) {
        panic(subprogramDeclarationFollow, subprogramDeclarationFollowSize);
        return SubprogramDeclarationPtr();
    }

    DeclarationsPtr declarations = parseDeclarations();
    if (m_errorCode > ErrorCodes::NoError) {
        panic(subprogramDeclarationFollow, subprogramDeclarationFollowSize);
        return SubprogramDeclarationPtr();
    }

    sub->declarations = declarations;

    CompoundStatementPtr body = parseCompoundStatement();
    if (m_errorCode > ErrorCodes::NoError) {
        panic(subprogramDeclarationFollow, subprogramDeclarationFollowSize);
        return SubprogramDeclarationPtr();
    }

    sub->body = body;

    return sub;
}

static const TokenType::Enum subprogramDeclarationsFollow[] = {
    TokenType::Begin,
    TokenType::Eof,
    TokenType::Period,
};
static const int subprogramDeclarationsFollowSize = 3;

SubprogramDeclarationsPtr Parser::parseSubprogramDeclarations()
{
    if (m_curToken.tokenType() != TokenType::Function && m_curToken.tokenType() != TokenType::Procedure) {
        return SubprogramDeclarationsPtr();
    }

    SubprogramDeclarationPtr declaration = parseSubprogramDeclaration();
    if (m_errorCode <= ErrorCodes::NoError) {
        if (!match(TokenType::Semicolon)) {
            reportError(ErrorCodes::ExpectedSemicolon);

            panic(subprogramDeclarationsFollow, subprogramDeclarationsFollowSize);
            return SubprogramDeclarationsPtr();
        }

        SubprogramDeclarationsPtr subprograms(new SubprogramDeclarations);
        subprograms->list.push_back(declaration);

        SubprogramDeclarationsPtr rest = parseSubprogramDeclarations();
        if (rest) {
            subprograms->list.insert(subprograms->list.end(), rest->list.begin(), rest->list.end());
        }

        return subprograms;
    }

    return SubprogramDeclarationsPtr();
}

static const TokenType::Enum subprogramHeadFollow[] = {
    TokenType::Begin,
    TokenType::Eof,
    TokenType::Period,
    TokenType::Var,
};
static const int subprogramHeadFollowSize = 4;

void Parser::parseSubprogramHead(SubprogramDeclarationPtr sub)
{
    Token subprogramType = m_curToken;
    if (!match(TokenType::Function) && !match(TokenType::Procedure)) {
        reportError(ErrorCodes::ExpectedSubprogramHead);

        panic(subprogramHeadFollow, subprogramHeadFollowSize);
        return;
    }

    IdentifierPtr name = parseIdentifier();
    if (m_errorCode > ErrorCodes::NoError) {
        panic(subprogramHeadFollow, subprogramHeadFollowSize);
        return;
    }

    sub->name = name;

    parseArguments(sub);
    if (m_errorCode > ErrorCodes::NoError) {
        panic(subprogramHeadFollow, subprogramHeadFollowSize);
        return;
    }

    if (subprogramType.tokenType() == TokenType::Function) {
        sub->isFunction = true;

        if (!match(TokenType::Colon)) {
            reportError(ErrorCodes::ExpectedColon);

            panic(subprogramHeadFollow, subprogramHeadFollowSize);
            return;
        }

        TypePtr type = parseStandardType();
        if (m_errorCode > ErrorCodes::NoError) {
            panic(subprogramHeadFollow, subprogramHeadFollowSize);
            return;
        }

        sub->returnType = type;
    }

    if (!match(TokenType::Semicolon)) {
        reportError(ErrorCodes::ExpectedSemicolon);

        panic(subprogramHeadFollow, subprogramHeadFollowSize);
        return;
    }
}

static const TokenType::Enum termFollow[] = {
    TokenType::End,
    TokenType::Eof,
    TokenType::MulOp,
    TokenType::Period,
    TokenType::Semicolon,
    TokenType::RelOp,
    TokenType::RParen,
};
static const int termFollowSize = 7;

TermPtr Parser::parseTerm()
{
    TermPtr term(new Term);

    term->lhs = parseFactor();
    if (m_errorCode > ErrorCodes::NoError) {
        panic(termFollow, termFollowSize);
        return TermPtr();
    }

    // concat this to the term
    parseTerm_r(term);

    return term;
}

void Parser::parseTerm_r(TermPtr term)
{
    Token op = m_curToken;
    if (match(TokenType::MulOp)) {
        term->mulOp = op.operatorType();
        term->rhs = parseTerm();
    }
}

static const TokenType::Enum typeFollow[] = {
    TokenType::Begin,
    TokenType::Function,
    TokenType::Eof,
    TokenType::Procedure,
    TokenType::Semicolon,
};
static const int typeFollowSize = 5;

TypePtr Parser::parseType()
{
    if (match(TokenType::Array)) {
        if (!match(TokenType::LBracket)) {
            reportError(ErrorCodes::ExpectedLBracket);

            panic(typeFollow, typeFollowSize);
            return TypePtr();
        }

        NumberPtr from = parseNumber();
        if (m_errorCode > ErrorCodes::NoError) {
            panic(typeFollow, typeFollowSize);
            return TypePtr();
        }

        if (!match(TokenType::Range)) {
            reportError(ErrorCodes::ExpectedRange);

            panic(typeFollow, typeFollowSize);
            return TypePtr();
        }

        NumberPtr to = parseNumber();
        if (m_errorCode > ErrorCodes::NoError) {
            panic(typeFollow, typeFollowSize);
            return TypePtr();
        }

        if (!match(TokenType::RBracket)) {
            reportError(ErrorCodes::ExpectedRBracket);

            panic(typeFollow, typeFollowSize);
            return TypePtr();
        }

        if (!match(TokenType::Of)) {
            reportError(ErrorCodes::ExpectedOf);

            panic(typeFollow, typeFollowSize);
            return TypePtr();
        }

        TypePtr type = parseStandardType();
        if (m_errorCode > ErrorCodes::NoError) {
            panic(typeFollow, typeFollowSize);
            return TypePtr();
        }

        TypePtr arrayType(new Type);
        arrayType->isArray = true;
        arrayType->startsAt = from;
        arrayType->endsAt = to;
        arrayType->standardType = type->standardType;

        return arrayType;
    }

    return parseStandardType();
}

static std::string codeToMessage(int errorCode)
{
    switch (errorCode) {
    case ErrorCodes::NoError:
        return "";
        break;

    case ErrorCodes::ExpectedAssignment:
        return "Expected '='";
        break;

    case ErrorCodes::ExpectedColon:
        return "Expected ':'";
        break;

    case ErrorCodes::ExpectedDo:
        return "Expected 'do'";
        break;

    case ErrorCodes::ExpectedBegin:
        return "Expected 'begin'";
        break;

    case ErrorCodes::ExpectedElse:
        return "Expected 'else'";
        break;

    case ErrorCodes::ExpectedEnd:
        return "Expected 'end'";
        break;

    case ErrorCodes::ExpectedIdentifier:
        return "Expected identifier";
        break;

    case ErrorCodes::ExpectedLBracket:
        return "Expected '['";
        break;

    case ErrorCodes::ExpectedLParen:
        return "Expected '('";
        break;

    case ErrorCodes::ExpectedNumber:
        return "Expected number";
        break;

    case ErrorCodes::ExpectedOf:
        return "Expected 'of'";
        break;

    case ErrorCodes::ExpectedPeriod:
        return "Expected '.'";
        break;

    case ErrorCodes::ExpectedProgram:
        return "Expected 'program'";
        break;

    case ErrorCodes::ExpectedRange:
        return "Expected '..'";
        break;

    case ErrorCodes::ExpectedRBracket:
        return "Expected ']'";
        break;

    case ErrorCodes::ExpectedRParen:
        return "Expected ')'";
        break;

    case ErrorCodes::ExpectedSemicolon:
        return "Expected ';'";
        break;

    case ErrorCodes::ExpectedStandardType:
        return "Expected standard type";
        break;

    case ErrorCodes::ExpectedStatement:
        return "Expected statement";
        break;

    case ErrorCodes::ExpectedThen:
        return "Expected 'then'";
        break;

    default:
        return "***Unknown Error Code***";
    }

    return "***Not Reached***";
}
