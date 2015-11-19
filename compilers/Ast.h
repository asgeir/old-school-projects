#pragma once

#include "AstPrimitives.h"
#include "Token.h"

#include <vector>

class BasicBlock { public: virtual ~BasicBlock() {} };
class Function { public: virtual ~Function() {} };
class Value { public: virtual ~Value() {} };

#define NullValue 0

typedef std::vector<Value *> ValueList;

class Builder
{
public:
    virtual ~Builder() {}

    virtual BasicBlock *getInsertBlock() = 0;
    virtual void setInsertBlock(BasicBlock *bb) = 0;

    virtual Value *constValue(int value) = 0;
    virtual Value *constValue(float value) = 0;

    virtual BasicBlock *createBasicBlock(const char *name=0, Function *parent=0) = 0;

    virtual Value *createTempVariable(BasicBlock *parent, TypePtr type) = 0;
    virtual Value *createVariable(IdentifierPtr id, TypePtr type, BasicBlock *parent=0) = 0;

    virtual Function *createFunction(IdentifierPtr name, DeclarationsPtr arguments, TypePtr returnType) = 0;
    virtual Value *symbolTableLookup(IdentifierPtr id) = 0;

    virtual Value *createCall(Value *target, ValueList &params) = 0;

    virtual Value *createAdd(Value *lhs, Value *rhs) = 0;
    virtual Value *createAnd(Value *lhs, Value *rhs) = 0;
    virtual void   createAssign(Value *dest, Value *value) = 0;
    virtual Value *createDiv(Value *lhs, Value *rhs) = 0;
    virtual Value *createIDiv(Value *lhs, Value *rhs) = 0;
    virtual Value *createMod(Value *lhs, Value *rhs) = 0;
    virtual Value *createMul(Value *lhs, Value *rhs) = 0;
    virtual Value *createNeg(Value *value) = 0;
    virtual Value *createNot(Value *value) = 0;
    virtual Value *createOr(Value *lhs, Value *rhs) = 0;
    virtual Value *createSub(Value *lhs, Value *rhs) = 0;

    virtual Value *createCmpEq(Value *lhs, Value *rhs) = 0;
    virtual Value *createCmpGe(Value *lhs, Value *rhs) = 0;
    virtual Value *createCmpGt(Value *lhs, Value *rhs) = 0;
    virtual Value *createCmpLe(Value *lhs, Value *rhs) = 0;
    virtual Value *createCmpLt(Value *lhs, Value *rhs) = 0;
    virtual Value *createCmpNe(Value *lhs, Value *rhs) = 0;

    virtual void createBr(BasicBlock *dest) = 0;
    virtual void createBrCond(Value *cond, BasicBlock *True, BasicBlock *False) = 0;
    virtual void createReturn(Value *value) = 0;

    virtual BasicBlock *createBeq(Value *lhs, Value *rhs, BasicBlock *True);
    virtual BasicBlock *createBge(Value *lhs, Value *rhs, BasicBlock *True);
    virtual BasicBlock *createBgt(Value *lhs, Value *rhs, BasicBlock *True);
    virtual BasicBlock *createBle(Value *lhs, Value *rhs, BasicBlock *True);
    virtual BasicBlock *createBlt(Value *lhs, Value *rhs, BasicBlock *True);
    virtual BasicBlock *createBne(Value *lhs, Value *rhs, BasicBlock *True);
};

typedef boost::shared_ptr<Builder> BuilderPtr;

struct Expression
{
    ~Expression() {}
    virtual Value *codegen(BuilderPtr builder) = 0;

    OperatorType::Enum expressionType;
};

struct Expressions
{
    ValueList codegen(BuilderPtr builder);

    std::vector<ExpressionPtr> list;
};

struct AddOpExpression : public Expression
{
    virtual Value *codegen(BuilderPtr builder);

    bool lhsNegated;
    TermPtr lhs;
    OperatorType::Enum addOp;
    AddOpExpressionPtr rhs;
};

struct RelOpExpression : public Expression
{
    virtual Value *codegen(BuilderPtr builder);

    ExpressionPtr lhs;
    OperatorType::Enum relOp;
    ExpressionPtr rhs;
};

struct Factor
{
    Value *codegen(BuilderPtr builder);

    IdentifierPtr id;
    NumberPtr number;
    ExpressionPtr expression;
    FactorPtr negatedFactor;
};

struct Declaration
{
    Value *codegen(BuilderPtr builder);

    IdentifierPtr id;
    TypePtr type;
};

struct Declarations
{
    ValueList codegen(BuilderPtr builder);

    std::vector<DeclarationPtr> list;
};

struct Identifier
{
    Token id;
    ExpressionPtr index;
    ExpressionsPtr parameters;
};

struct Identifiers
{
    std::vector<IdentifierPtr> list;
};

struct Number
{
    Token value;
};

struct Program
{
    void codegen(BuilderPtr builder);

    IdentifierPtr programName;
    IdentifiersPtr inputOutput;
    DeclarationsPtr variables;
    SubprogramDeclarationsPtr functions;
    CompoundStatementPtr mainProgram;
};

struct Statement
{
    ~Statement() {}
    virtual void codegen(BuilderPtr builder) = 0;
};

struct Statements
{
    void codegen(BuilderPtr builder);

    std::vector<StatementPtr> list;
};

struct AssignmentOrCallStatement : public Statement
{
    void codegen(BuilderPtr builder);

    IdentifierPtr id;
    ExpressionPtr value;
};

struct CompoundStatement : public Statement
{
    void codegen(BuilderPtr builder);

    StatementsPtr statements;
};

struct IfStatement : public Statement
{
    void codegen(BuilderPtr builder);

    ExpressionPtr expression;
    StatementPtr thenPart;
    StatementPtr elsePart;
};

struct WhileStatement : public Statement
{
    void codegen(BuilderPtr builder);

    ExpressionPtr expression;
    StatementPtr doPart;
};

struct SubprogramDeclaration
{
    void codegen(BuilderPtr builder);

    bool isFunction;
    IdentifierPtr name;
    DeclarationsPtr arguments;
    TypePtr returnType;
    DeclarationsPtr declarations;
    CompoundStatementPtr body;
};

struct SubprogramDeclarations
{
    void codegen(BuilderPtr builder);

    std::vector<SubprogramDeclarationPtr> list;
};

struct Term
{
    Value *codegen(BuilderPtr builder);

    FactorPtr lhs;
    OperatorType::Enum mulOp;
    TermPtr rhs;
};

struct Type
{
    NumberType::Enum standardType;
    bool isArray;
    NumberPtr startsAt;
    NumberPtr endsAt;
};
