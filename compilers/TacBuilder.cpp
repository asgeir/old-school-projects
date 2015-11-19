#include "TacBuilder.h"

#include <sstream>

static int g_labelUid = 0;

class TacBasicBlock : public BasicBlock
{
public:
    TacBasicBlock(const std::string &n, TacFunction *o);

    std::string name;
    TacFunction *owner;
    std::stringstream code;
};

class TacFunction : public Function
{
public:
    TacFunction(const std::string &n, DeclarationsPtr a, TypePtr r)
        : name(n), arguments(a) {}

    TacBasicBlockPtr createBasicBlock(const std::string &name);
    TacFunctionPtr createFunction(IdentifierPtr name, DeclarationsPtr arguments, TypePtr returnType);
    TacValuePtr createVariable(IdentifierPtr id, TypePtr type);
    TacValuePtr createTemporary(TypePtr type);
    TacBasicBlockPtr findBasicBlock(TacBasicBlock *bb);
    TacFunctionPtr findFunction(TacFunction *func);
    TacValuePtr lookupSymbol(IdentifierPtr id);
    std::string output();

    std::string name;
    DeclarationsPtr arguments;
    TacFunction *parent;
    std::vector<TacFunctionPtr> children;
    std::vector<TacBasicBlockPtr> blocks;
    std::vector<TacValuePtr> symbols;
};

class TacValue : public Value
{
public:
    TacValue() : id(), type(), isConstant(false), isFunction(false) {}

    virtual std::string value() const
    {
        return id->id.value();
    }

    IdentifierPtr id;
    TypePtr type;
    bool isConstant;
    bool isFunction;
};

class ConstIntTacValue : public TacValue
{
public:
    ConstIntTacValue(int v) : intValue(v) { isConstant = true; }

    virtual std::string value() const
    {
        std::stringstream ss;
        ss << intValue;

        return ss.str();
    }

    int intValue;
};

class ConstRealTacValue : public TacValue
{
public:
    ConstRealTacValue(float v) : realValue(v) { isConstant = true; }

    virtual std::string value() const
    {
        std::stringstream ss;
        ss << realValue;

        return ss.str();
    }

    float realValue;
};

class FunctionTacValue : public TacValue
{
public:
    FunctionTacValue(TacFunctionPtr f) : function(f)
    {
        isFunction = true;

        IdentifierPtr fid(new Identifier);
        fid->id = Token(f->name, -1, -1);

        id = fid;
    }

    TacFunctionPtr function;
};

TacBasicBlock::TacBasicBlock(const std::string &n, TacFunction *o)
    : name(n)
    , owner(o)
{
    if (n.empty()) {
        std::stringstream ss;

        ss << "__block__label__" << (++g_labelUid);
        name = ss.str();
    }
}

TacBasicBlockPtr TacFunction::createBasicBlock(const std::string &name)
{
    TacBasicBlockPtr block(new TacBasicBlock(name, this));

    blocks.push_back(block);

    return block;
}

TacFunctionPtr TacFunction::createFunction(IdentifierPtr name, DeclarationsPtr arguments, TypePtr returnType)
{
    TacFunctionPtr func(new TacFunction(name->id.value(), arguments, returnType));
    TacValuePtr funcValue(new FunctionTacValue(func));

    func->parent = this;

    children.push_back(func);
    symbols.push_back(funcValue);

    return func;
}

TacValuePtr TacFunction::createVariable(IdentifierPtr id, TypePtr type)
{
    TacValuePtr symbol(new TacValue);
    symbol->isConstant = false;
    symbol->id = id;
    symbol->type = type;

    symbols.push_back(symbol);

    return symbol;
}

TacValuePtr TacFunction::createTemporary(TypePtr type)
{
    std::stringstream name;
    name << "__temp__" << (++g_labelUid);

    IdentifierPtr id(new Identifier);
    id->id = Token(name.str(), -1, -1);

    return createVariable(id, type);
}

TacBasicBlockPtr TacFunction::findBasicBlock(TacBasicBlock *bb)
{
    for (std::vector<TacBasicBlockPtr>::iterator i = blocks.begin(); i != blocks.end(); ++i) {
        if (i->get() == bb) {
            return *i;
        }
    }

    for (std::vector<TacFunctionPtr>::iterator i = children.begin(); i != children.end(); ++i) {
        TacBasicBlockPtr ptr = (*i)->findBasicBlock(bb);
        if (ptr) {
            return ptr;
        }
    }

    return TacBasicBlockPtr();
}

TacFunctionPtr TacFunction::findFunction(TacFunction *func)
{
    for (std::vector<TacFunctionPtr>::iterator i = children.begin(); i != children.end(); ++i) {
        if (func == i->get()) {
            return *i;
        }

        TacFunctionPtr ptr = (*i)->findFunction(func);
        if (ptr) {
            return ptr;
        }
    }

    return TacFunctionPtr();
}

TacValuePtr TacFunction::lookupSymbol(IdentifierPtr id)
{
    for (std::vector<TacValuePtr>::iterator i = symbols.begin(); i != symbols.end(); ++i)
    {
        if ((*i)->value() == id->id.value()) {
            return (*i);
        }
    }

    if (parent) {
        return parent->lookupSymbol(id);
    }

    return TacValuePtr();
}

std::string TacFunction::output()
{
    std::stringstream ss;

    if (!name.empty()) {
        ss << name << ":";
    }

    if (arguments) {
        for (std::vector<DeclarationPtr>::iterator i = arguments->list.begin(); i != arguments->list.end(); ++i) {
            ss << "\t\t" << "FPARAM" << "\t" << (*i)->id->id.value() << std::endl;
        }
    }

    for (std::vector<TacValuePtr>::iterator i = symbols.begin(); i != symbols.end(); ++i) {
        if ((*i)->isConstant || (*i)->isFunction) {
            continue;
        }

        ss << "\t\t" << "VAR" << "\t" << (*i)->value() << std::endl;
    }

    // redundant
    ss << "\t\t" << "GOTO" << "\t" << blocks.front()->name << std::endl;

    for (std::vector<TacFunctionPtr>::iterator i = children.begin(); i != children.end(); ++i)
    {
        ss << (*i)->output();
    }

    for (std::vector<TacBasicBlockPtr>::iterator i = blocks.begin(); i != blocks.end(); ++i) {
        if (!(*i)->name.empty()) {
            ss << (*i)->name << ":";
        }

        ss << (*i)->code.str() << std::endl;
    }

    return ss.str();
}

TacBuilder::TacBuilder()
    : m_programFunction(new TacFunction("", DeclarationsPtr(), TypePtr()))
    , m_insertBlock()
{
    TacValuePtr writelnPtr(new TacValue);

    IdentifierPtr writelnIdPtr(new Identifier);
    writelnIdPtr->id = Token("writeln", -1, -1);

    writelnPtr->id = writelnIdPtr;
    writelnPtr->isConstant = true;
    writelnPtr->isFunction = true;

    m_programFunction->symbols.push_back(writelnPtr);
}

std::string TacBuilder::output()
{
    return m_programFunction->output();
}

BasicBlock *TacBuilder::getInsertBlock()
{
    return m_insertBlock.get();
}

void TacBuilder::setInsertBlock(BasicBlock *bb)
{
    m_insertBlock = m_programFunction->findBasicBlock((TacBasicBlock *)bb);
}

Value *TacBuilder::constValue(int value)
{
    TacValuePtr constValue(new ConstIntTacValue(value));
    m_constValues.push_back(constValue);

    return constValue.get();
}

Value *TacBuilder::constValue(float value)
{
    TacValuePtr constValue(new ConstRealTacValue(value));
    m_constValues.push_back(constValue);

    return constValue.get();
}

BasicBlock *TacBuilder::createBasicBlock(const char *name, Function *parent)
{
    if (!m_insertBlock) {
        m_insertBlock = m_programFunction->createBasicBlock(name);
        return m_insertBlock.get();
    }

    if (parent == 0) {
        parent = m_insertBlock->owner;
    }

    if (name == 0) {
        name = "";
    }

    TacFunctionPtr owner = findFunction((TacFunction *)parent);

    return owner->createBasicBlock(name).get();
}

Value *TacBuilder::createTempVariable(BasicBlock *parent, TypePtr type)
{
    if (parent == 0) {
        parent = m_insertBlock.get();
    }

    TacFunction *owner = m_programFunction->findBasicBlock((TacBasicBlock *)parent)->owner;
    return owner->createTemporary(type).get();
}

Value *TacBuilder::createVariable(IdentifierPtr id, TypePtr type, BasicBlock *parent)
{
    if (parent == 0) {
        parent = m_insertBlock.get();
    }

    TacFunction *owner = m_programFunction->findBasicBlock((TacBasicBlock *)parent)->owner;

    return owner->createVariable(id, type).get();
}

Function *TacBuilder::createFunction(IdentifierPtr name, DeclarationsPtr arguments, TypePtr returnType)
{
    if (!m_insertBlock) {
        return m_programFunction->createFunction(name, arguments, returnType).get() ;
    }

    return m_insertBlock->owner->createFunction(name, arguments, returnType).get();
}

Value *TacBuilder::symbolTableLookup(IdentifierPtr id)
{
    return m_insertBlock->owner->lookupSymbol(id).get();
}

Value *TacBuilder::createCall(Value *target, ValueList &params)
{
    TacValuePtr symbol = m_insertBlock->owner->lookupSymbol(((TacValue *)target)->id);

    for (ValueList::iterator i = params.begin(); i != params.end(); ++i) {
        m_insertBlock->code << "\t\t" << "APARAM" << "\t" << ((TacValue *)(*i))->value() << std::endl;
    }

    m_insertBlock->code << "\t\t" << "CALL" << "\t" << symbol->value() << std::endl;

    if (symbol->isFunction) {
        return symbol.get();
    }

    return 0;
}

Value *TacBuilder::createAdd(Value *lhs, Value *rhs)
{
    TacValuePtr result = m_insertBlock->owner->createTemporary(TypePtr());

    TacValue *lhsValue = (TacValue *)lhs;
    TacValue *rhsValue = (TacValue *)rhs;
    m_insertBlock->code << "\t\t" << "ADD" << "\t" << lhsValue->value() << "\t" << rhsValue->value() << "\t" << result->value() << std::endl;

    return result.get();
}

Value *TacBuilder::createAnd(Value *lhs, Value *rhs)
{
    TacValuePtr result = m_insertBlock->owner->createTemporary(TypePtr());

    TacValue *lhsValue = (TacValue *)lhs;
    TacValue *rhsValue = (TacValue *)rhs;
    m_insertBlock->code << "\t\t" << "AND" << "\t" << lhsValue->value() << "\t" << rhsValue->value() << "\t" << result->value() << std::endl;

    return result.get();
}

void TacBuilder::createAssign(Value *dest, Value *value)
{
    TacValue *destValue = (TacValue *)dest;
    TacValue *valueValue = (TacValue *)value;
    m_insertBlock->code << "\t\t" << "ASSIGN" << "\t" << valueValue->value() << "\t" << destValue->value() << std::endl;
}

Value *TacBuilder::createDiv(Value *lhs, Value *rhs)
{
    TacValuePtr result = m_insertBlock->owner->createTemporary(TypePtr());

    TacValue *lhsValue = (TacValue *)lhs;
    TacValue *rhsValue = (TacValue *)rhs;
    m_insertBlock->code << "\t\t" << "DIVIDE" << "\t" << lhsValue->value() << "\t" << rhsValue->value() << "\t" << result->value() << std::endl;

    return result.get();
}

Value *TacBuilder::createIDiv(Value *lhs, Value *rhs)
{
    TacValuePtr result = m_insertBlock->owner->createTemporary(TypePtr());

    TacValue *lhsValue = (TacValue *)lhs;
    TacValue *rhsValue = (TacValue *)rhs;
    m_insertBlock->code << "\t\t" << "DIV" << "\t" << lhsValue->value() << "\t" << rhsValue->value() << "\t" << result->value() << std::endl;

    return result.get();
}

Value *TacBuilder::createMod(Value *lhs, Value *rhs)
{
    TacValuePtr result = m_insertBlock->owner->createTemporary(TypePtr());

    TacValue *lhsValue = (TacValue *)lhs;
    TacValue *rhsValue = (TacValue *)rhs;
    m_insertBlock->code << "\t\t" << "MOD" << "\t" << lhsValue->value() << "\t" << rhsValue->value() << "\t" << result->value() << std::endl;

    return result.get();
}

Value *TacBuilder::createMul(Value *lhs, Value *rhs)
{
    TacValuePtr result = m_insertBlock->owner->createTemporary(TypePtr());

    TacValue *lhsValue = (TacValue *)lhs;
    TacValue *rhsValue = (TacValue *)rhs;
    m_insertBlock->code << "\t\t" << "MULT" << "\t" << lhsValue->value() << "\t" << rhsValue->value() << "\t" << result->value() << std::endl;

    return result.get();
}

Value *TacBuilder::createNeg(Value *value)
{
    TacValuePtr result = m_insertBlock->owner->createTemporary(TypePtr());

    TacValue *valueValue = (TacValue *)value;
    m_insertBlock->code << "\t\t" << "UMINUS" << "\t" << valueValue->value() << "\t" << result->value() << std::endl;

    return result.get();
}

Value *TacBuilder::createNot(Value *value)
{
    TacValuePtr result = m_insertBlock->owner->createTemporary(TypePtr());

    TacValue *valueValue = (TacValue *)value;
    m_insertBlock->code << "\t\t" << "NOT" << "\t" << valueValue->value() << "\t" << valueValue->value() << "\t" << result->value() << std::endl;

    return result.get();
}

Value *TacBuilder::createOr(Value *lhs, Value *rhs)
{
    TacValuePtr result = m_insertBlock->owner->createTemporary(TypePtr());

    TacValue *lhsValue = (TacValue *)lhs;
    TacValue *rhsValue = (TacValue *)rhs;
    m_insertBlock->code << "\t\t" << "OR" << "\t" << lhsValue->value() << "\t" << rhsValue->value() << "\t" << result->value() << std::endl;

    return result.get();
}

Value *TacBuilder::createSub(Value *lhs, Value *rhs)
{
    TacValuePtr result = m_insertBlock->owner->createTemporary(TypePtr());

    TacValue *lhsValue = (TacValue *)lhs;
    TacValue *rhsValue = (TacValue *)rhs;
    m_insertBlock->code << "\t\t" << "SUB" << "\t" << lhsValue->value() << "\t" << rhsValue->value() << "\t" << result->value() << std::endl;

    return result.get();
}

Value *TacBuilder::createCmpEq(Value *lhs, Value *rhs)
{
    TacValuePtr result = m_insertBlock->owner->createTemporary(TypePtr());
    std::string trueLabel = createLabel();
    std::string doneLabel = createLabel();

    TacValue *lhsValue = (TacValue *)lhs;
    TacValue *rhsValue = (TacValue *)rhs;
    m_insertBlock->code << "\t\t" << "EQ" << "\t" << lhsValue->value() << "\t" << rhsValue->value() << "\t" << trueLabel << std::endl;
    m_insertBlock->code << "\t\t" << "ASSIGN" << "\t" << "0" << "\t" << result->value() << std::endl;
    m_insertBlock->code << "\t\t" << "GOTO" << "\t" << doneLabel << std::endl;
    m_insertBlock->code << trueLabel << ":";
    m_insertBlock->code << "\t\t" << "ASSIGN" << "\t" << "1" << "\t" << result->value() << std::endl;
    m_insertBlock->code << doneLabel << ":";

    return result.get();
}

Value *TacBuilder::createCmpGe(Value *lhs, Value *rhs)
{
    TacValuePtr result = m_insertBlock->owner->createTemporary(TypePtr());
    std::string trueLabel = createLabel();
    std::string doneLabel = createLabel();

    TacValue *lhsValue = (TacValue *)lhs;
    TacValue *rhsValue = (TacValue *)rhs;
    m_insertBlock->code << "\t\t" << "GE" << "\t" << lhsValue->value() << "\t" << rhsValue->value() << "\t" << trueLabel << std::endl;
    m_insertBlock->code << "\t\t" << "ASSIGN" << "\t" << "0" << "\t" << result->value() << std::endl;
    m_insertBlock->code << "\t\t" << "GOTO" << "\t" << doneLabel << std::endl;
    m_insertBlock->code << trueLabel << ":";
    m_insertBlock->code << "\t\t" << "ASSIGN" << "\t" << "1" << "\t" << result->value() << std::endl;
    m_insertBlock->code << doneLabel << ":";

    return result.get();
}

Value *TacBuilder::createCmpGt(Value *lhs, Value *rhs)
{
    TacValuePtr result = m_insertBlock->owner->createTemporary(TypePtr());
    std::string trueLabel = createLabel();
    std::string doneLabel = createLabel();

    TacValue *lhsValue = (TacValue *)lhs;
    TacValue *rhsValue = (TacValue *)rhs;
    m_insertBlock->code << "\t\t" << "GT" << "\t" << lhsValue->value() << "\t" << rhsValue->value() << "\t" << trueLabel << std::endl;
    m_insertBlock->code << "\t\t" << "ASSIGN" << "\t" << "0" << "\t" << result->value() << std::endl;
    m_insertBlock->code << "\t\t" << "GOTO" << "\t" << doneLabel << std::endl;
    m_insertBlock->code << trueLabel << ":";
    m_insertBlock->code << "\t\t" << "ASSIGN" << "\t" << "1" << "\t" << result->value() << std::endl;
    m_insertBlock->code << doneLabel << ":";

    return result.get();
}

Value *TacBuilder::createCmpLe(Value *lhs, Value *rhs)
{
    TacValuePtr result = m_insertBlock->owner->createTemporary(TypePtr());
    std::string trueLabel = createLabel();
    std::string doneLabel = createLabel();

    TacValue *lhsValue = (TacValue *)lhs;
    TacValue *rhsValue = (TacValue *)rhs;
    m_insertBlock->code << "\t\t" << "LE" << "\t" << lhsValue->value() << "\t" << rhsValue->value() << "\t" << trueLabel << std::endl;
    m_insertBlock->code << "\t\t" << "ASSIGN" << "\t" << "0" << "\t" << result->value() << std::endl;
    m_insertBlock->code << "\t\t" << "GOTO" << "\t" << doneLabel << std::endl;
    m_insertBlock->code << trueLabel << ":";
    m_insertBlock->code << "\t\t" << "ASSIGN" << "\t" << "1" << "\t" << result->value() << std::endl;
    m_insertBlock->code << doneLabel << ":";

    return result.get();
}

Value *TacBuilder::createCmpLt(Value *lhs, Value *rhs)
{
    TacValuePtr result = m_insertBlock->owner->createTemporary(TypePtr());
    std::string trueLabel = createLabel();
    std::string doneLabel = createLabel();

    TacValue *lhsValue = (TacValue *)lhs;
    TacValue *rhsValue = (TacValue *)rhs;
    m_insertBlock->code << "\t\t" << "LT" << "\t" << lhsValue->value() << "\t" << rhsValue->value() << "\t" << trueLabel << std::endl;
    m_insertBlock->code << "\t\t" << "ASSIGN" << "\t" << "0" << "\t" << result->value() << std::endl;
    m_insertBlock->code << "\t\t" << "GOTO" << "\t" << doneLabel << std::endl;
    m_insertBlock->code << trueLabel << ":";
    m_insertBlock->code << "\t\t" << "ASSIGN" << "\t" << "1" << "\t" << result->value() << std::endl;
    m_insertBlock->code << doneLabel << ":";

    return result.get();
}

Value *TacBuilder::createCmpNe(Value *lhs, Value *rhs)
{
    TacValuePtr result = m_insertBlock->owner->createTemporary(TypePtr());
    std::string trueLabel = createLabel();
    std::string doneLabel = createLabel();

    TacValue *lhsValue = (TacValue *)lhs;
    TacValue *rhsValue = (TacValue *)rhs;
    m_insertBlock->code << "\t\t" << "NE" << "\t" << lhsValue->value() << "\t" << rhsValue->value() << "\t" << trueLabel << std::endl;
    m_insertBlock->code << "\t\t" << "ASSIGN" << "\t" << "0" << "\t" << result->value() << std::endl;
    m_insertBlock->code << "\t\t" << "GOTO" << "\t" << doneLabel << std::endl;
    m_insertBlock->code << trueLabel << ":";
    m_insertBlock->code << "\t\t" << "ASSIGN" << "\t" << "1" << "\t" << result->value() << std::endl;
    m_insertBlock->code << doneLabel << ":";

    return result.get();
}

void TacBuilder::createBr(BasicBlock *dest)
{
    m_insertBlock->code << "\t\t" << "GOTO" << "\t" << ((TacBasicBlock *)dest)->name << std::endl;
}

void TacBuilder::createBrCond(Value *cond, BasicBlock *True, BasicBlock *False)
{
    m_insertBlock->code << "\t\t" << "EQ" << "\t" << "1" << "\t" << ((TacValue *)cond)->value() << ((TacBasicBlock *)True)->name << std::endl;
    m_insertBlock->code << "\t\t" << "GOTO" << "\t" << ((TacBasicBlock *)False)->name << std::endl;
}

void TacBuilder::createReturn(Value *value)
{
    m_insertBlock->code << "\t\t" << "RETURN" << std::endl;
}

BasicBlock *TacBuilder::createBeq(Value *lhs, Value *rhs, BasicBlock *True)
{
    TacBasicBlock *falseBlock = (TacBasicBlock *)createBasicBlock();

    TacValue *lhsValue = (TacValue *)lhs;
    TacValue *rhsValue = (TacValue *)rhs;
    m_insertBlock->code << "\t\t" << "EQ" << "\t" << lhsValue->value() << "\t" << rhsValue->value() << "\t" << ((TacBasicBlock *)True)->name << std::endl;
    m_insertBlock->code << "\t\t" << "GOTO" << "\t" << falseBlock->name << std::endl;

    return falseBlock;
}

BasicBlock *TacBuilder::createBge(Value *lhs, Value *rhs, BasicBlock *True)
{
    TacBasicBlock *falseBlock = (TacBasicBlock *)createBasicBlock();

    TacValue *lhsValue = (TacValue *)lhs;
    TacValue *rhsValue = (TacValue *)rhs;
    m_insertBlock->code << "\t\t" << "GE" << "\t" << lhsValue->value() << "\t" << rhsValue->value() << "\t" << ((TacBasicBlock *)True)->name << std::endl;
    m_insertBlock->code << "\t\t" << "GOTO" << "\t" << falseBlock->name << std::endl;

    return falseBlock;
}

BasicBlock *TacBuilder::createBgt(Value *lhs, Value *rhs, BasicBlock *True)
{
    TacBasicBlock *falseBlock = (TacBasicBlock *)createBasicBlock();

    TacValue *lhsValue = (TacValue *)lhs;
    TacValue *rhsValue = (TacValue *)rhs;
    m_insertBlock->code << "\t\t" << "GT" << "\t" << lhsValue->value() << "\t" << rhsValue->value() << "\t" << ((TacBasicBlock *)True)->name << std::endl;
    m_insertBlock->code << "\t\t" << "GOTO" << "\t" << falseBlock->name << std::endl;

    return falseBlock;
}

BasicBlock *TacBuilder::createBle(Value *lhs, Value *rhs, BasicBlock *True)
{
    TacBasicBlock *falseBlock = (TacBasicBlock *)createBasicBlock();

    TacValue *lhsValue = (TacValue *)lhs;
    TacValue *rhsValue = (TacValue *)rhs;
    m_insertBlock->code << "\t\t" << "LE" << "\t" << lhsValue->value() << "\t" << rhsValue->value() << "\t" << ((TacBasicBlock *)True)->name << std::endl;
    m_insertBlock->code << "\t\t" << "GOTO" << "\t" << falseBlock->name << std::endl;

    return falseBlock;
}

BasicBlock *TacBuilder::createBlt(Value *lhs, Value *rhs, BasicBlock *True)
{
    TacBasicBlock *falseBlock = (TacBasicBlock *)createBasicBlock();

    TacValue *lhsValue = (TacValue *)lhs;
    TacValue *rhsValue = (TacValue *)rhs;
    m_insertBlock->code << "\t\t" << "LT" << "\t" << lhsValue->value() << "\t" << rhsValue->value() << "\t" << ((TacBasicBlock *)True)->name << std::endl;
    m_insertBlock->code << "\t\t" << "GOTO" << "\t" << falseBlock->name << std::endl;

    return falseBlock;
}

BasicBlock *TacBuilder::createBne(Value *lhs, Value *rhs, BasicBlock *True)
{
    TacBasicBlock *falseBlock = (TacBasicBlock *)createBasicBlock();

    TacValue *lhsValue = (TacValue *)lhs;
    TacValue *rhsValue = (TacValue *)rhs;
    m_insertBlock->code << "\t\t" << "NE" << "\t" << lhsValue->value() << "\t" << rhsValue->value() << "\t" << ((TacBasicBlock *)True)->name << std::endl;
    m_insertBlock->code << "\t\t" << "GOTO" << "\t" << falseBlock->name << std::endl;

    return falseBlock;
}

std::string TacBuilder::createLabel()
{
    std::stringstream ss;

    ss << "__temp__label__" << (++g_labelUid);

    return ss.str();
}

TacFunctionPtr TacBuilder::findFunction(TacFunction *function)
{
    if (function == m_programFunction.get()) {
        return m_programFunction;
    }

    return m_programFunction->findFunction(function);
}
