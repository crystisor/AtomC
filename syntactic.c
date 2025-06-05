#include <stdio.h>
#include <stdlib.h>
#include "syntactic.h"
#include <string.h>

Symbol *crtStruct = NULL; // Current struct being defined
Symbol *crtFunc = NULL;   // Current function being defined
extern int crtDepth;      // Current block nesting depth
extern Token *crtTk;
Token *consumedTk = NULL;
extern Token *lookAhead;

int isTypeName()
{
    Token *start = crtTk;
    Type dummy;
    int result = typeName(&dummy);
    crtTk = start;
    return result;
}

void addVar(Token *tkName, Type *t)
{
    Symbol *s;

    if (t->typeBase == TB_VOID)
        tkerr(tkName, "variable '%s' cannot be of type void", tkName->text);
    if (t->nElements >= 0 && t->typeBase == TB_VOID)
        tkerr(tkName, "array cannot have void elements");
    if (t->nElements >= 0 && t->nElements == 0)
        tkerr(tkName, "array size must be positive");

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
        lookAhead = crtTk;
        return 1; // success
    }
    return 0; // no match
}

int unit()
{
    while (1)
    {
        Token *startTk = crtTk;

        Type t;
        if (typeName(&t) || consume(VOID))
        {
            if (!consume(ID))
                tkerr(crtTk, "missing identifier");

            if (lookAhead && lookAhead->code == LPAR)
            {
                crtTk = startTk;
                if (!declFunc())
                    tkerr(crtTk, "invalid function declaration");
            }
            else
            {
                crtTk = startTk;
                if (!declVar())
                    tkerr(crtTk, "invalid variable declaration");
            }
            continue;
        }

        if (declStruct())
            continue;

        break;
    }

    if (!consume(END))
        tkerr(crtTk, "missing END");

    return 1;
}

int declStruct()
{

    // Token *startTk = crtTk;
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
    // Type baseType;
    Type varType;

    if (!typeName(&varType))
        return 0;

    if (!consume(ID))
        tkerr(crtTk, "missing variable name");
    Token *tkName = consumedTk;

    // Type varType = baseType;
    arrayDecl(&varType);

    addVar(tkName, &varType);

    if (consume(ASSIGN))
    {
        RetVal rv;
        if (!expr(&rv))
            tkerr(crtTk, "invalid initializer");

        if (!canCast(&rv.type, &varType))
            tkerr(crtTk, "cannot cast initializer to declared type");
    }

    while (consume(COMMA))
    {
        if (!consume(ID))
            tkerr(crtTk, "missing variable name after ,");
        tkName = consumedTk;

        // varType = baseType;  // RESET to base type
        if (!arrayDecl(&varType))
            varType.nElements = -1;

        addVar(tkName, &varType);
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
    }
    else
    {
        return 0;
    }

    if (!consume(ID))
    {
        crtTk = startTk;
        return 0;
    }

    Token *tkName = consumedTk;

    if (!consume(LPAR))
        tkerr(crtTk, "missing ( in function declaration");
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

    crtDepth--; // parameters done

    if (!stmCompound())
    {
        tkerr(crtTk, "missing function body");
    }

    deleteSymbolsAfter(&symbols, crtFunc);
    crtFunc = NULL;
    return 1;
}

int typeName(Type *ret)
{
    if (!typeBase(ret)) // Fill in base type (e.g., int, double, etc.)
        return 0;

    ret->nPtr = 0;       // init
    while (consume(MUL)) // Consume '*' tokens
        ret->nPtr++;
    if (!arrayDecl(ret))
    {
        ret->nElements = -1; // No array declaration
    }

    return 1;
}

int typeBase(Type *ret)
{
    if (consume(INT))
    {
        ret->typeBase = TB_INT;
        return 1;
    }
    if (consume(DOUBLE))
    {
        ret->typeBase = TB_DOUBLE;
        return 1;
    }
    if (consume(CHAR))
    {
        ret->typeBase = TB_CHAR;
        return 1;
    }
    if (consume(FLOAT))
    {
        ret->typeBase = TB_FLOAT;
        return 1;
    }
    if (consume(STRUCT))
    {
        if (!consume(ID))
            tkerr(crtTk, "missing identifier after struct");
        Token *tkName = consumedTk;
        Symbol *s = findSymbol(&symbols, tkName->text);
        if (!s)
            tkerr(crtTk, "undefined symbol: %s", tkName->text);
        if (s->cls != CLS_STRUCT)
            tkerr(crtTk, "%s is not a struct", tkName->text);
        ret->typeBase = TB_STRUCT;
        ret->s = s;
        return 1;
    }
    return 0;
}

int arrayDecl(Type *ret)
{
    if (consume(LBRACKET)) {
        RetVal rvSize;
        if (!expr(&rvSize))
            tkerr(crtTk, "invalid expression for array size");

        if (!rvSize.isCtVal || rvSize.type.typeBase != TB_INT || rvSize.type.nElements != -1)
            tkerr(crtTk, "array size must be an integer constant scalar");

        if ((ret->nElements = rvSize.ctVal.i) <= 0)
            tkerr(crtTk, "array size must be positive");

        if (!consume(RBRACKET))
            tkerr(crtTk, "missing ] in array declaration");

        return 1;
    }

    ret->nElements = -1;
    return 0;
}


int expr(RetVal *rv)
{
    return exprAssign(rv);
}

int exprAssign(RetVal *rv)
{
    Token *startTk = crtTk;

    RetVal rvl;
    if (exprPostfix(&rvl))
    {
        if (consume(ASSIGN))
        {
            RetVal rvr;
            if (!exprAssign(&rvr))
                tkerr(crtTk, "missing right side of assignment");

            if (!rvl.isLVal)
                tkerr(crtTk, "left operand is not a lvalue");

            if (!rvr.wasCast && !canCast(&rvr.type, &rvl.type))
            {
                tkerr(crtTk, "cannot implicitly convert %s to %s",
                      typeToString(&rvr.type), typeToString(&rvl.type));
            }

            cast(&rvl.type, &rvr.type);
            *rv = rvl;
            return 1;
        }
        crtTk = startTk;
    }

    return exprOr(rv);
}

int exprOr(RetVal *rv)
{
    RetVal r1, r2;
    if (!exprAnd(&r1))
        return 0;
    while (consume(OR))
    {
        if (!exprAnd(&r2))
            tkerr(crtTk, "missing right operand after ||");
        r1.type = createType(TB_INT, -1);
        r1.isLVal = 0;
        r1.isCtVal = 0;
    }
    *rv = r1;
    return 1;
}

int exprAnd(RetVal *rv)
{
    RetVal r1, r2;
    if (!exprEq(&r1))
        return 0;
    while (consume(AND))
    {
        if (!exprEq(&r2))
            tkerr(crtTk, "missing right operand after &&");
        r1.type = createType(TB_INT, -1);
        r1.isLVal = 0;
        r1.isCtVal = 0;
    }
    *rv = r1;
    return 1;
}

int exprEq(RetVal *rv)
{
    RetVal r1, r2;
    if (!exprRel(&r1))
        return 0;
    while (consume(EQUAL) || consume(NOTEQ))
    {
        if (!exprRel(&r2))
            tkerr(crtTk, "missing right operand after == or !=");
        r1.type = createType(TB_INT, -1);
        r1.isLVal = 0;
        r1.isCtVal = 0;
    }
    *rv = r1;
    return 1;
}

int exprRel(RetVal *rv)
{
    RetVal r1, r2;
    if (!exprAdd(&r1))
        return 0;
    while (consume(LESS) || consume(LESSEQ) || consume(GREATER) || consume(GREATEREQ))
    {
        if (!exprAdd(&r2))
            tkerr(crtTk, "missing right operand after relational operator");
        r1.type = createType(TB_INT, -1);
        r1.isLVal = 0;
        r1.isCtVal = 0;
    }
    *rv = r1;
    return 1;
}

int exprAdd(RetVal *rv)
{
    RetVal r1, r2;
    if (!exprMul(&r1))
        return 0;
    while (consume(ADD) || consume(SUB))
    {
        if (!exprMul(&r2))
            tkerr(crtTk, "missing operand after + or -");
        printf("\n%d %d\n", r1.type.nElements, r2.type.nElements);
        r1.type = getArithType(&r1.type, &r2.type);
        r1.isLVal = 0;
        r1.isCtVal = 0;
    }
    *rv = r1;
    return 1;
}

int exprMul(RetVal *rv)
{
    RetVal r1, r2;
    if (!exprCast(&r1))
        return 0;
    while (consume(MUL) || consume(DIV))
    {
        if (!exprCast(&r2))
            tkerr(crtTk, "missing operand after * or /");
        r1.type = getArithType(&r1.type, &r2.type);
        r1.isLVal = 0;
        r1.isCtVal = 0;
    }
    *rv = r1;
    return 1;
}

int exprCast(RetVal *rv)
{
    Token *startTk = crtTk;

    if (consume(LPAR))
    {
        if (isTypeName()) // Lookahead to confirm it's a cast
        {
            Type t;
            typeName(&t);

            if (!consume(RPAR))
                tkerr(crtTk, "missing ) in cast expression");

            RetVal rvInner;
            if (!exprCast(&rvInner))
                tkerr(crtTk, "missing expression after cast");

            if (!canCast(&rvInner.type, &t))
                tkerr(crtTk, "invalid cast");

            cast(&t, &rvInner.type);
            *rv = (RetVal){.type = t, .isLVal = 0, .isCtVal = rvInner.isCtVal, .wasCast = 1};
            return 1;
        }

        crtTk = startTk;
    }

    return exprPrimary(rv);
}

int exprUnary(RetVal *rv)
{
    if (consume(SUB))
    {
        RetVal operand;
        if (!exprUnary(&operand))
            tkerr(crtTk, "missing operand after '-'");

        if (operand.type.typeBase != TB_INT && operand.type.typeBase != TB_DOUBLE)
            tkerr(crtTk, "invalid type for unary '-'");

        operand.isLVal = 0;
        *rv = operand;
        return 1;
    }

    if (consume(NOT))
    {
        RetVal operand;
        if (!exprUnary(&operand))
            tkerr(crtTk, "missing operand after '!'");

        if (operand.type.typeBase != TB_INT)
            tkerr(crtTk, "invalid type for '!': expected int");

        operand.isLVal = 0;
        *rv = operand;
        return 1;
    }

    if (consume(AND))
    {
        RetVal operand;
        if (!exprUnary(&operand))
            tkerr(crtTk, "missing operand after '&'");

        if (!operand.isLVal)
            tkerr(crtTk, "cannot take address of rvalue");

        Type t = operand.type;
        t.nPtr++;
        operand.type = t;
        operand.isLVal = 0;
        operand.isCtVal = 0;
        *rv = operand;
        return 1;
    }

    if (consume(MUL))
    {
        RetVal operand;
        if (!exprUnary(&operand))
            tkerr(crtTk, "missing operand after '*'");

        if (operand.type.nPtr < 1)
            tkerr(crtTk, "cannot dereference non-pointer");

        Type t = operand.type;
        t.nPtr--;
        operand.type = t;
        operand.isLVal = 1;
        operand.isCtVal = 0;
        *rv = operand;
        return 1;
    }

    return exprCast(rv);
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
                tkerr(crtTk, "missing expression inside []");

            if (!consume(RBRACKET))
                tkerr(crtTk, "missing ]");

            if (rv->type.nElements < 0)
                tkerr(crtTk, "subscripted value is not an array");

            if (idx.type.nElements >= 0 || idx.type.typeBase == TB_STRUCT)
                tkerr(crtTk, "array index must be scalar");

            rv->type.nElements = -1; 

            rv->isLVal = 1;
            rv->isCtVal = 0;
            return 1;
        }
        else
        {
            break;
        }
    }

    return 1;
}

int exprPrimary(RetVal *rv)
{
    Token *startTk = crtTk;
    if (consume(ID))
    {
        Symbol *s = findSymbol(&symbols, consumedTk->text);
        if (!s)
            tkerr(crtTk, "undefined identifier: %s", consumedTk->text);
        memcpy(&rv->type, &s->type, sizeof(Type));
        rv->isLVal = (s->cls == CLS_VAR);
        rv->isCtVal = 0;

        if (s->cls == CLS_FUNC && consume(LPAR))
        {
            // Handle function arguments if needed
            if (!consume(RPAR))
            {
                while (1)
                {
                    RetVal arg;
                    if (!expr(&arg))
                        tkerr(crtTk, "invalid argument");
                    if (!consume(COMMA))
                        break;
                }
                if (!consume(RPAR))
                    tkerr(crtTk, "missing closing ) for function call");
            }

            rv->isLVal = 0; // Function result is not an LVal
        }

        return 1;
    }

    if (consume(CT_INT))
    {
        rv->type = createType(TB_INT, -1);
        rv->isCtVal = 1;
        rv->ctVal.i = consumedTk->i;
        rv->isLVal = 0;
        return 1;
    }

    if (consume(CT_REAL))
    {
        rv->type = createType(TB_DOUBLE, -1);
        rv->isCtVal = 1;
        rv->ctVal.d = consumedTk->r;
        rv->isLVal = 0;
        return 1;
    }

    if (consume(CT_CHAR))
    {
        rv->type = createType(TB_CHAR, -1);
        rv->isCtVal = 1;
        rv->ctVal.i = consumedTk->i;
        rv->isLVal = 0;
        return 1;
    }

    if (consume(CT_STRING))
    {
        rv->type = createType(TB_CHAR, 0);
        rv->isCtVal = 1;
        rv->ctVal.str = consumedTk->text;
        rv->isLVal = 0;
        return 1;
    }

    if (consume(LPAR))
    {
        if (!expr(rv))
            tkerr(crtTk, "invalid expression in parentheses");
        if (!consume(RPAR))
            tkerr(crtTk, "missing )");
        return 1;
    }

    crtTk = startTk;
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
        if (rvCond.type.nElements >= 0 || rvCond.type.typeBase == TB_STRUCT)
            tkerr(crtTk, "a condition must be of scalar type");

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
        if (rvCond.type.nElements >= 0 || rvCond.type.typeBase == TB_STRUCT)
            tkerr(crtTk, "a condition must be of scalar type");

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

        if (crtTk->code != SEMICOLON)
        {
            RetVal rv;
            expr(&rv);
        }
        if (!consume(SEMICOLON))
            tkerr(crtTk, "missing ; after for init expression");

        if (crtTk->code != SEMICOLON)
        {
            RetVal rvCond;
            if (!expr(&rvCond))
                tkerr(crtTk, "invalid expression in for condition");
            if (rvCond.type.nElements >= 0 || rvCond.type.typeBase == TB_STRUCT)
                tkerr(crtTk, "a condition must be of scalar type");
        }
        if (!consume(SEMICOLON))
            tkerr(crtTk, "missing ; after for condition");

        if (crtTk->code != RPAR)
        {
            RetVal rv;
            expr(&rv);
        }
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
        if (crtTk->code != SEMICOLON)
        {
            RetVal rv;
            if (!expr(&rv))
                tkerr(crtTk, "invalid expression in return");
            if (crtFunc->type.typeBase == TB_VOID)
                tkerr(crtTk, "void function should not return a value");

            cast(&crtFunc->type, &rv.type);
        }
        else
        {
            if (crtFunc->type.typeBase != TB_VOID)
                tkerr(crtTk, "non-void function must return a value");
        }

        if (!consume(SEMICOLON))
            tkerr(crtTk, "missing ; after return");
        return 1;
    }
    if (crtTk->code != SEMICOLON)
    {
        RetVal rv;
        expr(&rv);
    }
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

    Symbol *start = symbols.end[-1]; // mark where to clean up
    crtDepth++;

    while (1)
    {
        if (declVar())
        {
        }
        else
        {
            RetVal rv;
            if (stm(&rv))
            {
            }
            else
                break;
        }
    }

    crtDepth--;
    deleteSymbolsAfter(&symbols, start);

    if (!consume(RACC))
        tkerr(crtTk, "missing } or syntax error");

    return 1;
}