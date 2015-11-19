#pragma once

#include "AstPrimitives.h"
#include "Token.h"

#include <boost/shared_ptr.hpp>

class Parser
{
public:
    Parser();

    bool error() const;
    int errorCount() const;

    ProgramPtr parse(boost::shared_ptr<Lexer> lexer);

private:
    bool match(TokenType::Enum tokenType);
    void panic(const TokenType::Enum synchronizingTokens[], int numElements);
    void reportError(int errorCode);

    DeclarationsPtr parseArgumentList();
    DeclarationsPtr parseArgumentList_r();
    void parseArguments(SubprogramDeclarationPtr sub);
    CompoundStatementPtr parseCompoundStatement();
    DeclarationsPtr parseDeclarations();
    ExpressionPtr parseExpression();
    void parseExpression_p(RelOpExpressionPtr expr);
    ExpressionsPtr parseExpressionList();
    ExpressionsPtr parseExpressionList_r();
    FactorPtr parseFactor();
    void parseFactor_p(IdentifierPtr id);
    IdentifierPtr parseIdentifier();
    IdentifiersPtr parseIdentifierList();
    IdentifiersPtr parseIdentifierList_r();
    NumberPtr parseNumber();
    ExpressionPtr parseOptionalIndex();
    ExpressionsPtr parseOptionalParameters();
    StatementsPtr parseOptionalStatements();
    ProgramPtr parseProgram();
    ExpressionPtr parseSimpleExpression();
    void parseSimpleExpression_r(AddOpExpressionPtr expr);
    TypePtr parseStandardType();
    StatementPtr parseStatement();
    void parseStatement_p(AssignmentOrCallStatementPtr statement);
    StatementsPtr parseStatementList();
    StatementsPtr parseStatementList_r();
    SubprogramDeclarationPtr parseSubprogramDeclaration();
    SubprogramDeclarationsPtr parseSubprogramDeclarations();
    void parseSubprogramHead(SubprogramDeclarationPtr sub);
    TermPtr parseTerm();
    void parseTerm_r(TermPtr term);
    TypePtr parseType();

    boost::shared_ptr<Lexer> m_lexer;
    int m_errorCode;
    int m_errorCount;
    Token m_curToken;
};
