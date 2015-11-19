#include "Ast.h"

BasicBlock *Builder::createBeq(Value *lhs, Value *rhs, BasicBlock *True)
{
    BasicBlock *False = createBasicBlock(0);
    Value *cmp = createCmpEq(lhs, rhs);
    createBrCond(cmp, True, False);
    return False;
}

BasicBlock *Builder::createBge(Value *lhs, Value *rhs, BasicBlock *True)
{
    BasicBlock *False = createBasicBlock(0);
    Value *cmp = createCmpGe(lhs, rhs);
    createBrCond(cmp, True, False);
    return False;
}

BasicBlock *Builder::createBgt(Value *lhs, Value *rhs, BasicBlock *True)
{
    BasicBlock *False = createBasicBlock(0);
    Value *cmp = createCmpGt(lhs, rhs);
    createBrCond(cmp, True, False);
    return False;
}

BasicBlock *Builder::createBle(Value *lhs, Value *rhs, BasicBlock *True)
{
    BasicBlock *False = createBasicBlock(0);
    Value *cmp = createCmpLe(lhs, rhs);
    createBrCond(cmp, True, False);
    return False;
}

BasicBlock *Builder::createBlt(Value *lhs, Value *rhs, BasicBlock *True)
{
    BasicBlock *False = createBasicBlock(0);
    Value *cmp = createCmpLt(lhs, rhs);
    createBrCond(cmp, True, False);
    return False;
}

BasicBlock *Builder::createBne(Value *lhs, Value *rhs, BasicBlock *True)
{
    BasicBlock *False = createBasicBlock(0);
    Value *cmp = createCmpNe(lhs, rhs);
    createBrCond(cmp, True, False);
    return False;
}

ValueList Expressions::codegen(BuilderPtr builder)
{
    ValueList values;

    for (std::vector<ExpressionPtr>::const_iterator i = list.begin(); i != list.end(); ++i) {
        values.push_back((*i)->codegen(builder));
    }

    return values;
}

Value *AddOpExpression::codegen(BuilderPtr builder)
{
    Value *lhsValue = lhs->codegen(builder);
    if (lhsNegated) {
        lhsValue = builder->createNeg(lhsValue);
    }

    Value *result = lhsValue;

    if (rhs) {
        Value *rhsValue = rhs->codegen(builder);

        if (addOp == OperatorType::Add) {
            result = builder->createAdd(lhsValue, rhsValue);
        } else if (addOp == OperatorType::Or) {
            result = builder->createOr(lhsValue, rhsValue);
        } else if (addOp == OperatorType::Subtract) {
            result = builder->createSub(lhsValue, rhsValue);
        } else {
            result = 0;
        }
    }

    return result;
}

Value *RelOpExpression::codegen(BuilderPtr builder)
{
    Value *lhsValue = lhs->codegen(builder);
    Value *rhsValue = rhs->codegen(builder);

    switch (relOp) {
    case OperatorType::Equal:
        return builder->createCmpEq(lhsValue, rhsValue);
        break;
    case OperatorType::GreaterOrEqual:
        return builder->createCmpGe(lhsValue, rhsValue);
        break;
    case OperatorType::Greater:
        return builder->createCmpGt(lhsValue, rhsValue);
        break;
    case OperatorType::LessOrEqual:
        return builder->createCmpLe(lhsValue, rhsValue);
        break;
    case OperatorType::Less:
        return builder->createCmpLt(lhsValue, rhsValue);
        break;
    case OperatorType::NotEqual:
        return builder->createCmpNe(lhsValue, rhsValue);
        break;
    default:
        return 0;
    }

    return 0;
}

Value *Factor::codegen(BuilderPtr builder)
{
    if (number) {
        if (number->value.isInteger()) {
            return builder->constValue(number->value.valueAsInt());
        } else {
            return builder->constValue(number->value.valueAsReal());
        }
    }

    if (negatedFactor) {
        return builder->createNot(negatedFactor->codegen(builder));
    }

    if (expression) {
        return expression->codegen(builder);
    }

    Value *idValue = builder->symbolTableLookup(id);

    if (id->parameters) {
        ValueList paramValues = id->parameters->codegen(builder);
        return builder->createCall(idValue, paramValues);
    }

    return idValue;
}

Value *Declaration::codegen(BuilderPtr builder)
{
    return builder->createVariable(id, type);
}

ValueList Declarations::codegen(BuilderPtr builder)
{
    ValueList values;

    for (std::vector<DeclarationPtr>::const_iterator i = list.begin(); i != list.end(); ++i) {
        values.push_back((*i)->codegen(builder));
    }

    return values;
}

void Program::codegen(BuilderPtr builder)
{
    BasicBlock *mainFunction = builder->createBasicBlock(programName->id.value().c_str());

    if (variables) {
        variables->codegen(builder);
    }
    //builder->createBr(mainFunction);

    if (functions) {
        functions->codegen(builder);
    }

    builder->setInsertBlock(mainFunction);
    mainProgram->codegen(builder);
}

void AssignmentOrCallStatement::codegen(BuilderPtr builder)
{
    if (value) {
        builder->createAssign(builder->symbolTableLookup(id), value->codegen(builder));
        return;
    }

    ValueList parameters;
    if (id->parameters) {
        parameters = id->parameters->codegen(builder);
    }

    builder->createCall(builder->symbolTableLookup(id), parameters);
}

void Statements::codegen(BuilderPtr builder)
{
    for (std::vector<StatementPtr>::const_iterator i = list.begin(); i != list.end(); ++i) {
        (*i)->codegen(builder);
    }
}

void CompoundStatement::codegen(BuilderPtr builder)
{
    if (statements) {
        statements->codegen(builder);
    }
}

void IfStatement::codegen(BuilderPtr builder)
{
    BasicBlock *thenBlock = builder->createBasicBlock();
    BasicBlock *elseBlock = builder->createBne(expression->codegen(builder), builder->constValue(0), thenBlock);
    BasicBlock *doneBlock = builder->createBasicBlock();

    BasicBlock *oldBlock = builder->getInsertBlock();

    builder->setInsertBlock(thenBlock);
    thenPart->codegen(builder);
    builder->createBr(doneBlock);

    builder->setInsertBlock(elseBlock);
    elsePart->codegen(builder);
    builder->createBr(doneBlock);

    builder->setInsertBlock(doneBlock);
}

void WhileStatement::codegen(BuilderPtr builder)
{
    BasicBlock *evaluation = builder->createBasicBlock();
    builder->createBr(evaluation);

    BasicBlock *body = builder->createBasicBlock();
    builder->setInsertBlock(body);

    doPart->codegen(builder);
    builder->createBr(evaluation);

    builder->setInsertBlock(evaluation);
    BasicBlock *tail = builder->createBne(expression->codegen(builder), builder->constValue(0), body);

    builder->setInsertBlock(tail);
}

void SubprogramDeclaration::codegen(BuilderPtr builder)
{
    BasicBlock *oldBlock = builder->getInsertBlock();

    Function *function = builder->createFunction(name, arguments, returnType);

    BasicBlock *functionBlock = builder->createBasicBlock(0, function);
    builder->setInsertBlock(functionBlock);

    if (declarations) {
        declarations->codegen(builder);
    }

    body->codegen(builder);

    Value *returnValue;
    if (isFunction) {
        returnValue = builder->symbolTableLookup(name);
    }
    builder->createReturn(returnValue);

    builder->setInsertBlock(oldBlock);
}

void SubprogramDeclarations::codegen(BuilderPtr builder)
{
    for (std::vector<SubprogramDeclarationPtr>::const_iterator i = list.begin(); i != list.end(); ++i) {
        (*i)->codegen(builder);
    }
}

Value *Term::codegen(BuilderPtr builder)
{
    if (!rhs) {
        return lhs->codegen(builder);
    }

    Value *lhsValue = lhs->codegen(builder);
    Value *rhsValue = rhs->codegen(builder);

    switch (mulOp) {
    case (OperatorType::And):
        return builder->createAnd(lhsValue, rhsValue);

    case (OperatorType::Divide):
        return builder->createDiv(lhsValue, rhsValue);

    case (OperatorType::IntegerDivide):
        return builder->createIDiv(lhsValue, rhsValue);

    case (OperatorType::Modulus):
        return builder->createMod(lhsValue, rhsValue);

    case (OperatorType::Multiply):
        return builder->createMul(lhsValue, rhsValue);

    default:
        return 0;
    }

    return 0;
}
