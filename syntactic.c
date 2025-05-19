#include <stdio.h>
#include <stdlib.h>
#include "syntactic.h"
#include "symbols.h"
#include "types.h"

Symbol *crtStruct = NULL; // Current struct being defined
Symbol *crtFunc = NULL;   // Current function being defined
extern int crtDepth;      // Current block nesting depth
extern Token *crtTk;
Token *consumedTk = NULL;

void addVar(Token *tkName, Type *t)
{
    Symbol *s;

    if (crtStruct)
    {
        if (findSymbol(&crtStruct->members, tkName->text))
            tkerr(crtTk, "symbol redefinition: %s", tkName->text);
        s = addSymbol(&crtStruct->members, tkName->text, CLS_VAR);
    }
    else if (crtFunc)
    {
        s = findSymbol(&symbols, tkName->text);
        if (s && s->depth == crtDepth)
            tkerr(crtTk, "symbol redefinition: %s", tkName->text);
        s = addSymbol(&symbols, tkName->text, CLS_VAR);
        s->mem = MEM_LOCAL;
    }
    else
    {
        if (findSymbol(&symbols, tkName->text))
            tkerr(crtTk, "symbol redefinition: %s", tkName->text);
        s = addSymbol(&symbols, tkName->text, CLS_VAR);
        s->mem = MEM_GLOBAL;
    }

    s->type = *t;
}

int consume(int code)
{
    if (crtTk->code == code)
    {
        consumedTk = crtTk;
        crtTk = crtTk->next;
        return 1; // success
    }
    return 0; // no match
}

int unit()
{
    while (1)
    {
        if (declStruct())
        {
            continue;
        }
        if (declFunc())
        {
            continue;
        }
        if (declVar())
        {
            continue;
        }
        break;
    }
    if (!consume(END))
        tkerr(crtTk, "missing END");
    return 1;
}

int declStruct()
{
    if (!consume(STRUCT))
        return 0;
    if (!consume(ID))
        tkerr(crtTk, "missing struct name");
    Token *tkName = consumedTk;

    if (!consume(LACC))
        tkerr(crtTk, "missing { after struct name");

    if (crtFunc || crtStruct)
        tkerr(crtTk, "struct declared in non-global scope");

    if (findSymbol(&symbols, tkName->text))
        tkerr(crtTk, "symbol redefinition: %s", tkName->text);

    crtStruct = addSymbol(&symbols, tkName->text, CLS_STRUCT);
    initSymbols(&crtStruct->members);
    while (declVar())
    {
    }

    if (!consume(RACC))
        tkerr(crtTk, "missing } after struct body");
    if (!consume(SEMICOLON))
        tkerr(crtTk, "missing ; after struct declaration");

    crtStruct = NULL;
    return 1;
}

int declVar()
{
    Type t;

    if (!typeBase(&t))
        return 0;

    if (!consume(ID))
        tkerr(crtTk, "missing variable name");
    Token *tkName = consumedTk;

    if (!arrayDecl(&t))
        t.nElements = -1;

    addVar(tkName, &t);

    while (consume(COMMA))
    {
        if (!consume(ID))
            tkerr(crtTk, "missing variable name after ,");
        tkName = consumedTk;
        if (!arrayDecl(&t))
            t.nElements = -1;
        addVar(tkName, &t);
    }

    if (!consume(SEMICOLON))
        tkerr(crtTk, "missing ; after variable declaration");
    return 1;
}

int funcArg()
{
    Type t;
    if (!typeBase(&t))
        return 0;
    if (!consume(ID))
        tkerr(crtTk, "missing argument name");

    Token *tkName = consumedTk;

    if (!arrayDecl(&t))
        t.nElements = -1;

    Symbol *s = addSymbol(&symbols, tkName->text, CLS_VAR);
    s->mem = MEM_ARG;
    s->type = t;

    Symbol *arg = addSymbol(&crtFunc->args, tkName->text, CLS_VAR);
    arg->mem = MEM_ARG;
    arg->type = t;

    return 1;
}

int declFunc()
{
    Token *startTk = crtTk;
    Type t;

    if (typeBase(&t))
    {
        if (consume(MUL))
            t.nElements = 0;
        else
            t.nElements = -1;
    }
    else if (consume(VOID))
    {
        t.typeBase = TB_VOID;
        t.nElements = -1;
    }
    else
    {
        crtTk = startTk;
        return 0;
    }

    if (!consume(ID))
    {
        crtTk = startTk;
        return 0;
    }

    Token *tkName = consumedTk;

    if (!consume(LPAR))
    {
        crtTk = startTk;
        return 0;
    }

    if (crtStruct || crtFunc)
        tkerr(crtTk, "function declared in non-global scope");

    if (findSymbol(&symbols, tkName->text))
        tkerr(crtTk, "symbol redefinition: %s", tkName->text);

    crtFunc = addSymbol(&symbols, tkName->text, CLS_FUNC);
    initSymbols(&crtFunc->args);
    crtFunc->type = t;
    crtDepth++;

    if (funcArg())
    {
        while (consume(COMMA))
        {
            if (!funcArg())
                tkerr(crtTk, "missing function argument");
        }
    }

    if (!consume(RPAR))
        tkerr(crtTk, "missing ) in function declaration");

    crtDepth--;

    if (!stmCompound())
        tkerr(crtTk, "missing function body");

    deleteSymbolsAfter(&symbols, crtFunc);
    crtFunc = NULL;
    return 1;
}

int typeName(Type *t)
{
    int typeBaseVal;
    if (!typeBase(&typeBaseVal))
        return 0;
    int nElements = -1;
    if (consume(LBRACKET))
    {
        if (!consume(CT_INT))
            tkerr(crtTk, "missing array size");
        nElements = consumedTk->i;
        if (!consume(RBRACKET))
            tkerr(crtTk, "missing ]");
    }
    *t = createType(typeBaseVal, nElements);
    return 1;
}

int typeBase(int *typeBase)
{
    if (consume(INT))
    {
        *typeBase = TB_INT;
        return 1;
    }
    if (consume(DOUBLE))
    {
        *typeBase = TB_DOUBLE;
        return 1;
    }
    if (consume(CHAR))
    {
        *typeBase = TB_CHAR;
        return 1;
    }
    if (consume(STRUCT))
    {
        if (!consume(ID))
            tkerr(crtTk, "missing struct name");
        Symbol *s = findSymbol(&symbols, consumedTk->text);
        if (!s || s->cls != CLS_STRUCT)
            tkerr(crtTk, "undefined struct");
        *typeBase = TB_STRUCT;
        return 1;
    }
    return 0;
}

int arrayDecl(Type *ret)
{
    if (consume(LBRACKET))
    {
        if (!expr(ret))
        {
            // just ignore for now, as per original spec
        }
        ret->nElements = 0; // Do not compute real size
        if (!consume(RBRACKET))
            tkerr(crtTk, "missing ] in array declaration");
        return 1;
    }
    return 0;
}

int expr(RetVal *rv)
{
    return exprAssign(rv);
}

int exprAssign(RetVal *rv)
{
    Token *startTk = crtTk;
    RetVal rvl, rvr;

    if (exprUnary(&rvl))
    {
        if (consume(ASSIGN))
        {
            if (!rvl.isLVal)
                tkerr(crtTk, "cannot assign to a non-lvalue");
            if (!exprAssign(&rvr))
                tkerr(crtTk, "invalid right operand");
            cast(&rvl.type, &rvr.type);
            *rv = rvl;
            rv->isLVal = 0;
            rv->isCtVal = 0;
            return 1;
        }
        crtTk = startTk;
    }
    return exprOr(rv);
}

int exprOr(RetVal *rv)
{
    RetVal rvl, rvr;
    if (!exprAnd(&rvl))
        return 0;
    *rv = rvl;
    while (consume(OR))
    {
        if (!exprAnd(&rvr))
            tkerr(crtTk, "invalid operand for ||");
        if (!isArithmeticType(&rv->type) || !isArithmeticType(&rvr.type))
            tkerr(crtTk, "invalid types for ||");
        rv->type = createType(TB_INT, -1);
        rv->isLVal = 0;
        rv->isCtVal = 0;
    }
    return 1;
}
int exprAnd(RetVal *rv)
{
    RetVal rvl, rvr;
    if (!exprEq(&rvl))
        return 0;
    *rv = rvl;
    while (consume(AND))
    {
        if (!exprEq(&rvr))
            tkerr(crtTk, "invalid operand for &&");
        if (!isArithmeticType(&rv->type) || !isArithmeticType(&rvr.type))
            tkerr(crtTk, "invalid types for &&");
        rv->type = createType(TB_INT, -1);
        rv->isLVal = 0;
        rv->isCtVal = 0;
    }
    return 1;
}

int exprEq(RetVal *rv)
{
    RetVal rvl, rvr;
    if (!exprRel(&rvl))
        return 0;
    *rv = rvl;
    while (consume(EQUAL) || consume(NOTEQ))
    {
        int op = consumedTk->code;
        if (!exprRel(&rvr))
            tkerr(crtTk, "missing operand after == or !=");
        cast(&rvl.type, &rvr.type);
        cast(&rvr.type, &rvl.type);
        rv->type = createType(TB_INT, -1);
        rv->isLVal = 0;
        rv->isCtVal = 0;
    }
    return 1;
}

int exprRel(RetVal *rv)
{
    RetVal rvl, rvr;
    if (!exprAdd(&rvl))
        return 0;
    *rv = rvl;
    while (consume(LESS) || consume(LESSEQ) || consume(GREATER) || consume(GREATEREQ))
    {
        if (!exprAdd(&rvr))
            tkerr(crtTk, "missing operand for relational op");
        if (!isArithmeticType(&rvl.type) || !isArithmeticType(&rvr.type))
            tkerr(crtTk, "invalid types for relational op");
        rv->type = createType(TB_INT, -1);
        rv->isLVal = 0;
        rv->isCtVal = 0;
    }
    return 1;
}

int exprAdd(RetVal *rv)
{
    RetVal rvl, rvr;
    if (!exprMul(&rvl))
        return 0;
    *rv = rvl;
    while (consume(ADD) || consume(SUB))
    {
        if (!exprMul(&rvr))
            tkerr(crtTk, "missing operand for + or -");
        if (!isArithmeticType(&rv->type) || !isArithmeticType(&rvr.type))
            tkerr(crtTk, "invalid types for + or -");
        rv->type = getArithType(&rv->type, &rvr.type);
        rv->isLVal = 0;
        rv->isCtVal = 0;
    }
    return 1;
}

int exprMul(RetVal *rv)
{
    RetVal rvl, rvr;
    if (!exprCast(&rvl))
        return 0;
    *rv = rvl;
    while (consume(MUL) || consume(DIV))
    {
        if (!exprCast(&rvr))
            tkerr(crtTk, "missing operand for * or /");
        if (!isArithmeticType(&rv->type) || !isArithmeticType(&rvr.type))
            tkerr(crtTk, "invalid types for * or /");
        rv->type = getArithType(&rv->type, &rvr.type);
        rv->isLVal = 0;
        rv->isCtVal = 0;
    }
    return 1;
}

int exprCast(RetVal *rv)
{
    Token *startTk = crtTk;
    if (consume(LPAR))
    {
        Type dstType;
        if (typeName(&dstType))
        {
            if (!consume(RPAR))
                tkerr(crtTk, "missing ) after type cast");
            RetVal src;
            if (!exprCast(&src))
                tkerr(crtTk, "invalid cast operand");
            cast(&dstType, &src.type);
            rv->type = dstType;
            rv->isLVal = 0;
            rv->isCtVal = 0;
            return 1;
        }
         crtTk = startTk; // rollback LPAR
    }
    return exprUnary(rv);
}

int exprUnary(RetVal *rv)
{
    if (consume(SUB))
    {
        if (!exprUnary(rv))
            tkerr(crtTk, "invalid operand after -");
        if (!isArithmeticType(&rv->type))
            tkerr(crtTk, "invalid type for unary -");
        rv->isLVal = 0;
        rv->isCtVal = 0;
        return 1;
    }
    return exprPostfix(rv);
}

int exprPostfix(RetVal *rv)
{
    if (!exprPrimary(rv))
        return 0;
    while (1)
    {
        if (consume(LBRACKET))
        {
            RetVal idx;
            if (!expr(&idx))
                tkerr(crtTk, "invalid index expression");
            if (!isArrayType(&rv->type))
                tkerr(crtTk, "subscripting non-array");
            if (!isArithmeticType(&idx.type))
                tkerr(crtTk, "index must be arithmetic");
            rv->type = createType(rv->type.typeBase, -1);
            rv->isLVal = 1;
            if (!consume(RBRACKET))
                tkerr(crtTk, "missing ]");
        }
        else if (consume(DOT))
        {
            if (!isStructType(&rv->type))
                tkerr(crtTk, "field access on non-struct");
            if (!consume(ID))
                tkerr(crtTk, "missing field name");
            // Assuming struct fields are handled via symbol tables...
            // Add field lookup logic here if needed
            rv->isLVal = 1;
        }
        else
            break;
    }
    return 1;
}

int exprPrimary(RetVal *rv)
{
    if (consume(ID))
    {
        Symbol *s = findSymbol(&symbols, consumedTk->text);
        if (!s)
            tkerr(crtTk, "undefined identifier");
        rv->type = s->type;
        rv->isLVal = (s->cls == CLS_VAR);
        rv->isCtVal = 0;
        if (consume(LPAR))
        {
            if (s->cls != CLS_FUNC)
                tkerr(crtTk, "not a function");

            Symbol *param = s->args.begin;
            Symbols *args = &s->args;
            int argCount = args->end - args->begin; // number of arguments
            int i = 0;

            if (!consume(RPAR))
            {
                RetVal arg;
                if (!expr(&arg))
                    tkerr(crtTk, "missing argument");

                if (i >= argCount)
                    tkerr(crtTk, "too many arguments");

                cast(&args->begin[i]->type, &arg.type);
                i++;

                while (consume(COMMA))
                {
                    if (!expr(&arg))
                        tkerr(crtTk, "missing argument");

                    if (i >= argCount)
                        tkerr(crtTk, "too many arguments");

                    cast(&args->begin[i]->type, &arg.type);
                    i++;
                }

                if (!consume(RPAR))
                    tkerr(crtTk, "missing )");

                if (i < argCount)
                    tkerr(crtTk, "too few arguments");
            }
            rv->type = s->type;
            rv->isLVal = 0;
            rv->isCtVal = 0;
        }
        return 1;
    }
    if (consume(CT_INT))
    {
        rv->type = createType(TB_INT, -1);
        rv->isLVal = 0;
        rv->isCtVal = 1;
        rv->ctVal.i = consumedTk->i;
        return 1;
    }
    if (consume(CT_REAL))
    {
        rv->type = createType(TB_DOUBLE, -1);
        rv->isLVal = 0;
        rv->isCtVal = 1;
        rv->ctVal.d = consumedTk->r;
        return 1;
    }
    if (consume(CT_CHAR))
    {
        rv->type = createType(TB_CHAR, -1);
        rv->isLVal = 0;
        rv->isCtVal = 1;
        rv->ctVal.i = consumedTk->i;
        return 1;
    }
    if (consume(CT_STRING))
    {
        rv->type = createType(TB_CHAR, 0);
        rv->isLVal = 0;
        rv->isCtVal = 1;
        rv->ctVal.str = consumedTk->text;
        return 1;
    }
    if (consume(LPAR))
    {
        if (!expr(rv))
            tkerr(crtTk, "invalid expression");
        if (!consume(RPAR))
            tkerr(crtTk, "missing )");
        return 1;
    }
    return 0;
}

int stm()
{
    Token *startTk = crtTk;

    if (stmCompound())
        return 1;

    if (consume(IF))
    {
        if (!consume(LPAR))
            tkerr(crtTk, "missing ( after if");

        RetVal rvCond;
        if (!expr(&rvCond))
            tkerr(crtTk, "invalid expression in if");
        if (!isScalarType(&rvCond.type))
            tkerr(crtTk, "the condition of if must be a scalar value");

        if (!consume(RPAR))
            tkerr(crtTk, "missing ) after if expression");
        if (!stm())
            tkerr(crtTk, "missing statement after if");

        if (consume(ELSE))
        {
            if (!stm())
                tkerr(crtTk, "missing statement after else");
        }
        return 1;
    }

    if (consume(WHILE))
    {
        if (!consume(LPAR))
            tkerr(crtTk, "missing ( after while");

        RetVal rvCond;
        if (!expr(&rvCond))
            tkerr(crtTk, "invalid expression in while");
        if (!isScalarType(&rvCond.type))
            tkerr(crtTk, "the condition of while must be a scalar value");

        if (!consume(RPAR))
            tkerr(crtTk, "missing ) after while expression");
        if (!stm())
            tkerr(crtTk, "missing statement after while");
        return 1;
    }

    if (consume(FOR))
    {
        if (!consume(LPAR))
            tkerr(crtTk, "missing ( after for");

        expr(0); // optional init expression
        if (!consume(SEMICOLON))
            tkerr(crtTk, "missing ; after for init expression");

        RetVal rvCond;
        expr(&rvCond); // optional condition
        if (rvCond.type.typeBase != 0 && !isScalarType(&rvCond.type))
            tkerr(crtTk, "the condition of for must be a scalar value");

        if (!consume(SEMICOLON))
            tkerr(crtTk, "missing ; after for condition");

        expr(0); // optional increment
        if (!consume(RPAR))
            tkerr(crtTk, "missing ) after for increment");

        if (!stm())
            tkerr(crtTk, "missing statement after for");
        return 1;
    }

    if (consume(BREAK))
    {
        if (!consume(SEMICOLON))
            tkerr(crtTk, "missing ; after break");
        return 1;
    }

    if (consume(RETURN))
    {
        RetVal rv;
        if (expr(&rv))
        {
            if (!crtFunc)
                tkerr(crtTk, "return statement not inside a function");
            cast(&crtFunc->type, &rv.type);
        }
        else
        {
            if (crtFunc && crtFunc->type.typeBase != TB_VOID)
                tkerr(crtTk, "missing return expression for non-void function");
        }
        if (!consume(SEMICOLON))
            tkerr(crtTk, "missing ; after return");
        return 1;
    }

    RetVal rv;
    expr(&rv); // optional
    if (!consume(SEMICOLON))
    {
        crtTk = startTk;
        return 0;
    }

    return 1;
}

int stmCompound()
{
    if (!consume(LACC))
        return 0;

    Symbol *start = symbols.end[-1];
    crtDepth++;

    while (1)
    {
        if (declVar())
        {
        }
        else if (stm())
        {
        }
        else
            break;
    }

    crtDepth--;
    deleteSymbolsAfter(&symbols, start);

    if (!consume(RACC))
        tkerr(crtTk, "missing } or syntax error");
    return 1;
}
