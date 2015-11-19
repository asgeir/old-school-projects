#pragma once

#include <boost/shared_ptr.hpp>

struct AddOpExpression;
struct Arguments;
struct CompoundStatement;
struct Declaration;
struct Declarations;
struct Expression;
struct Expressions;
struct Factor;
struct Identifier;
struct Identifiers;
struct Lexer;
struct Number;
struct Program;
struct RelOpExpression;
struct Statement;
struct Statements;
struct AssignmentOrCallStatement;
struct IfStatement;
struct WhileStatement;
struct SubprogramDeclaration;
struct SubprogramDeclarations;
struct Term;
struct Type;

typedef boost::shared_ptr<AddOpExpression> AddOpExpressionPtr;
typedef boost::shared_ptr<Arguments> ArgumentsPtr;
typedef boost::shared_ptr<CompoundStatement> CompoundStatementPtr;
typedef boost::shared_ptr<Declaration> DeclarationPtr;
typedef boost::shared_ptr<Declarations> DeclarationsPtr;
typedef boost::shared_ptr<Expression> ExpressionPtr;
typedef boost::shared_ptr<Expressions> ExpressionsPtr;
typedef boost::shared_ptr<Factor> FactorPtr;
typedef boost::shared_ptr<Identifier> IdentifierPtr;
typedef boost::shared_ptr<Identifiers> IdentifiersPtr;
typedef boost::shared_ptr<Lexer> LexerPtr;
typedef boost::shared_ptr<Number> NumberPtr;
typedef boost::shared_ptr<Program> ProgramPtr;
typedef boost::shared_ptr<RelOpExpression> RelOpExpressionPtr;
typedef boost::shared_ptr<Statement> StatementPtr;
typedef boost::shared_ptr<Statements> StatementsPtr;
typedef boost::shared_ptr<AssignmentOrCallStatement> AssignmentOrCallStatementPtr;
typedef boost::shared_ptr<IfStatement> IfStatementPtr;
typedef boost::shared_ptr<WhileStatement> WhileStatementPtr;
typedef boost::shared_ptr<SubprogramDeclaration> SubprogramDeclarationPtr;
typedef boost::shared_ptr<SubprogramDeclarations> SubprogramDeclarationsPtr;
typedef boost::shared_ptr<Term> TermPtr;
typedef boost::shared_ptr<Type> TypePtr;
