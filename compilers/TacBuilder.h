#pragma once

#include "Ast.h"

#include <boost/shared_ptr.hpp>

#include <vector>

class TacBasicBlock;
class TacFunction;
class TacValue;

typedef boost::shared_ptr<TacBasicBlock> TacBasicBlockPtr;
typedef boost::shared_ptr<TacFunction> TacFunctionPtr;
typedef boost::shared_ptr<TacValue> TacValuePtr;

class TacBuilder : public Builder
{
public:
    TacBuilder();

    std::string output();

    virtual BasicBlock *getInsertBlock();
    virtual void setInsertBlock(BasicBlock *bb);

    virtual Value *constValue(int value);
    virtual Value *constValue(float value);

    virtual BasicBlock *createBasicBlock(const char *name=0, Function *parent=0);

    virtual Value *createTempVariable(BasicBlock *parent, TypePtr type);
    virtual Value *createVariable(IdentifierPtr id, TypePtr type, BasicBlock *parent=0);

    virtual Function *createFunction(IdentifierPtr name, DeclarationsPtr arguments, TypePtr returnType);
    virtual Value *symbolTableLookup(IdentifierPtr id);

    virtual Value *createCall(Value *target, ValueList &params);

    virtual Value *createAdd(Value *lhs, Value *rhs);
    virtual Value *createAnd(Value *lhs, Value *rhs);
    virtual void   createAssign(Value *dest, Value *value);
    virtual Value *createDiv(Value *lhs, Value *rhs);
    virtual Value *createIDiv(Value *lhs, Value *rhs);
    virtual Value *createMod(Value *lhs, Value *rhs);
    virtual Value *createMul(Value *lhs, Value *rhs);
    virtual Value *createNeg(Value *value);
    virtual Value *createNot(Value *value);
    virtual Value *createOr(Value *lhs, Value *rhs);
    virtual Value *createSub(Value *lhs, Value *rhs);

    virtual Value *createCmpEq(Value *lhs, Value *rhs);
    virtual Value *createCmpGe(Value *lhs, Value *rhs);
    virtual Value *createCmpGt(Value *lhs, Value *rhs);
    virtual Value *createCmpLe(Value *lhs, Value *rhs);
    virtual Value *createCmpLt(Value *lhs, Value *rhs);
    virtual Value *createCmpNe(Value *lhs, Value *rhs);

    virtual void createBr(BasicBlock *dest);
    virtual void createBrCond(Value *cond, BasicBlock *True, BasicBlock *False);
    virtual void createReturn(Value *value);

    virtual BasicBlock *createBeq(Value *lhs, Value *rhs, BasicBlock *True);
    virtual BasicBlock *createBge(Value *lhs, Value *rhs, BasicBlock *True);
    virtual BasicBlock *createBgt(Value *lhs, Value *rhs, BasicBlock *True);
    virtual BasicBlock *createBle(Value *lhs, Value *rhs, BasicBlock *True);
    virtual BasicBlock *createBlt(Value *lhs, Value *rhs, BasicBlock *True);
    virtual BasicBlock *createBne(Value *lhs, Value *rhs, BasicBlock *True);

private:
    std::string createLabel();
    TacFunctionPtr findFunction(TacFunction *function);

    TacFunctionPtr m_programFunction;
    TacBasicBlockPtr m_insertBlock;

    std::vector<TacValuePtr> m_constValues;
};
