#include <stdio.h>
#include <stdlib.h>
#include "syntactic.h"
#include "symbols.h"


Symbol *crtStruct = NULL;  // Current struct being defined
Symbol *crtFunc = NULL;    // Current function being defined
extern int crtDepth;          // Current block nesting depth
extern Token *crtTk;
Token *consumedTk = NULL;

void addVar(Token *tkName, Type *t)
{
    Symbol *s;

    if (crtStruct) {
        if (findSymbol(&crtStruct->members, tkName->text))
            tkerr(crtTk, "symbol redefinition: %s", tkName->text);
        s = addSymbol(&crtStruct->members, tkName->text, CLS_VAR);
    } else if (crtFunc) {
        s = findSymbol(&symbols, tkName->text);
        if (s && s->depth == crtDepth)
            tkerr(crtTk, "symbol redefinition: %s", tkName->text);
        s = addSymbol(&symbols, tkName->text, CLS_VAR);
        s->mem = MEM_LOCAL;
    } else {
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
            continue;
        if (declFunc())
            continue;
        if (declVar())
            continue;
        break;
    }
    if (!consume(END))
        tkerr(crtTk, "missing END");
    return 1;
}

int declStruct() {
    Token *startTk = crtTk;
    if (!consume(STRUCT)) return 0;

    if (!consume(ID)) tkerr(crtTk, "missing struct name");
    Token *tkName = consumedTk;

    if (!consume(LACC)) tkerr(crtTk, "missing { after struct name");

    if (crtFunc || crtStruct)
        tkerr(crtTk, "struct declared in non-global scope");

    if (findSymbol(&symbols, tkName->text))
        tkerr(crtTk, "symbol redefinition: %s", tkName->text);

    crtStruct = addSymbol(&symbols, tkName->text, CLS_STRUCT);
    initSymbols(&crtStruct->members);

    while (declVar()) { }

    if (!consume(RACC)) tkerr(crtTk, "missing } after struct body");
    if (!consume(SEMICOLON)) tkerr(crtTk, "missing ; after struct declaration");

    crtStruct = NULL;
    return 1;
}

int declVar() {
    Token *startTk = crtTk;
    Type t;

    if (!typeBase(&t)) return 0;

    if (!consume(ID)) tkerr(crtTk, "missing variable name");
    Token *tkName = consumedTk;

    if (!arrayDecl(&t)) t.nElements = -1;

    addVar(tkName, &t);

    while (consume(COMMA)) {
        if (!consume(ID)) tkerr(crtTk, "missing variable name after ,");
        tkName = consumedTk;
        if (!arrayDecl(&t)) t.nElements = -1;
        addVar(tkName, &t);
    }

    if (!consume(SEMICOLON)) tkerr(crtTk, "missing ; after variable declaration");
    return 1;
}


int funcArg() {
    Type t;
    if (!typeBase(&t)) return 0;
    if (!consume(ID)) tkerr(crtTk, "missing argument name");
    Token *tkName = consumedTk;
    if (!arrayDecl(&t)) t.nElements = -1;

    Symbol *s = addSymbol(&symbols, tkName->text, CLS_VAR);
    s->mem = MEM_ARG;
    s->type = t;

    Symbol *arg = addSymbol(&crtFunc->args, tkName->text, CLS_VAR);
    arg->mem = MEM_ARG;
    arg->type = t;
    return 1;
}


int declFunc() {
    Token *startTk = crtTk;
    Type t;

    if (typeBase(&t)) {
        if (consume(MUL)) t.nElements = 0;
        else t.nElements = -1;
    } else if (consume(VOID)) {
        t.typeBase = TB_VOID;
    } else {
        return 0;
    }

    if (!consume(ID)) {
        crtTk = startTk;
        return 0;
    }

    Token *tkName = consumedTk;

    if (!consume(LPAR)) tkerr(crtTk, "missing ( in function declaration");

    if (crtStruct || crtFunc)
        tkerr(crtTk, "function declared in non-global scope");

    if (findSymbol(&symbols, tkName->text))
        tkerr(crtTk, "symbol redefinition: %s", tkName->text);

    crtFunc = addSymbol(&symbols, tkName->text, CLS_FUNC);
    initSymbols(&crtFunc->args);
    crtFunc->type = t;
    crtDepth++;

    if (funcArg()) {
        while (consume(COMMA)) {
            if (!funcArg()) tkerr(crtTk, "missing function argument");
        }
    }

    if (!consume(RPAR)) tkerr(crtTk, "missing ) in function declaration");

    crtDepth--; // parameters done

    if (!stmCompound())
        tkerr(crtTk, "missing function body");

    deleteSymbolsAfter(&symbols, crtFunc);
    crtFunc = NULL;
    return 1;
}


int typeName(Type *ret)
{
    if (!typeBase(ret))  // Fill in base type (e.g., int, double, etc.)
        return 0;

    if (!arrayDecl(ret)) {
        ret->nElements = -1;  // No array declaration
    }

    return 1;
}


int typeBase(Type *ret) {
    if (consume(INT)) { ret->typeBase = TB_INT; return 1; }
    if (consume(DOUBLE)) { ret->typeBase = TB_DOUBLE; return 1; }
    if (consume(CHAR)) { ret->typeBase = TB_CHAR; return 1; }
    if (consume(STRUCT)) {
        if (!consume(ID)) tkerr(crtTk, "missing identifier after struct");
        Token *tkName = consumedTk;
        Symbol *s = findSymbol(&symbols, tkName->text);
        if (!s) tkerr(crtTk, "undefined symbol: %s", tkName->text);
        if (s->cls != CLS_STRUCT) tkerr(crtTk, "%s is not a struct", tkName->text);
        ret->typeBase = TB_STRUCT;
        ret->s = s;
        return 1;
    }
    return 0;
}


int arrayDecl(Type *ret) {
    if (consume(LBRACKET)) {
        if (!expr()) {
            // just ignore for now, as per original spec
        }
        ret->nElements = 0;  // Do not compute real size
        if (!consume(RBRACKET)) tkerr(crtTk, "missing ] in array declaration");
        return 1;
    }
    return 0;
}


int expr()
{
    return exprAssign();
}

int exprAssign() {
    Token *startTk = crtTk;
    if (exprUnary()) {
        if (consume(ASSIGN)) {
            if (!exprAssign()) tkerr(crtTk, "missing right side of assignment");
            return 1;
        }
        crtTk = startTk; // rollback if no assignment found
    }
    crtTk = startTk;
    return exprOr();
}

int exprOr() {
    if (!exprAnd()) return 0;
    while (consume(OR)) {
        if (!exprAnd()) tkerr(crtTk, "missing right operand after ||");
    }
    return 1;
}

int exprAnd() {
    if (!exprEq()) return 0;
    while (consume(AND)) {
        if (!exprEq()) tkerr(crtTk, "missing right operand after &&");
    }
    return 1;
}

int exprEq() {
    if (!exprRel()) return 0;
    while (consume(EQUAL) || consume(NOTEQ)) {
        if (!exprRel()) tkerr(crtTk, "missing right operand after == or !=");
    }
    return 1;
}

int exprRel() {
    if (!exprAdd()) return 0;
    while (consume(LESS) || consume(LESSEQ) || consume(GREATER) || consume(GREATEREQ)) {
        if (!exprAdd()) tkerr(crtTk, "missing right operand after relational operator");
    }
    return 1;
}

int exprAdd() {
    if (!exprMul()) return 0;
    while (consume(ADD) || consume(SUB)) {
        if (!exprMul()) tkerr(crtTk, "missing operand after + or -");
    }
    return 1;
}

int exprMul() {
    if (!exprCast()) return 0;
    while (consume(MUL) || consume(DIV)) {
        if (!exprCast()) tkerr(crtTk, "missing operand after * or /");
    }
    return 1;
}

int exprCast() {
    Token *startTk = crtTk;

    if (consume(LPAR)) {
        Type t;
        if (typeName(&t)) {
            if (!consume(RPAR)) tkerr(crtTk, "missing ) in cast expression");
            if (!exprCast()) tkerr(crtTk, "missing expression after cast");
            return 1;
        }
        crtTk = startTk; // rollback if it's not a type cast
    }

    return exprUnary();
}


int exprUnary() {
    if (consume(SUB) || consume(NOT)) {
        if (!exprUnary()) tkerr(crtTk, "missing operand after unary operator");
        return 1;
    }
    return exprPostfix();
}

int exprPostfix() {
    Token *startTk = crtTk;
    if (!exprPrimary()) return 0;

    for (;;) {
        if (consume(LBRACKET)) {
            if (!expr()) tkerr(crtTk, "missing expression inside []");
            if (!consume(RBRACKET)) tkerr(crtTk, "missing ]");
        } else if (consume(DOT)) {
            if (!consume(ID)) tkerr(crtTk, "missing field name after .");
        } else {
            break;
        }
    }

    return 1;
}

int exprPrimary() {
    Token *startTk = crtTk;

    if (consume(ID)) {
        if (consume(LPAR)) {
            if (expr()) {
                while (consume(COMMA)) {
                    if (!expr()) tkerr(crtTk, "missing expression after comma");
                }
            }
            if (!consume(RPAR)) tkerr(crtTk, "missing ) after function call");
        }
        return 1;
    }

    if (consume(CT_INT) || consume(CT_REAL) || consume(CT_CHAR) || consume(CT_STRING)) {
        return 1;
    }

    if (consume(LPAR)) {
        if (!expr()) tkerr(crtTk, "invalid expression in parentheses");
        if (!consume(RPAR)) tkerr(crtTk, "missing )");
        return 1;
    }

    crtTk = startTk;
    return 0;
}


int stm() {
    Token *startTk = crtTk;

    if (stmCompound())
        return 1;

    if (consume(IF)) {
        if (!consume(LPAR)) tkerr(crtTk, "missing ( after if");
        if (!expr()) tkerr(crtTk, "invalid expression in if");
        if (!consume(RPAR)) tkerr(crtTk, "missing ) after if expression");
        if (!stm()) tkerr(crtTk, "missing statement after if");

        if (consume(ELSE)) {
            if (!stm()) tkerr(crtTk, "missing statement after else");
        }
        return 1;
    }

    if (consume(WHILE)) {
        if (!consume(LPAR)) tkerr(crtTk, "missing ( after while");
        if (!expr()) tkerr(crtTk, "invalid expression in while");
        if (!consume(RPAR)) tkerr(crtTk, "missing ) after while expression");
        if (!stm()) tkerr(crtTk, "missing statement after while");
        return 1;
    }

    if (consume(FOR)) {
        if (!consume(LPAR)) tkerr(crtTk, "missing ( after for");

        // Optional init expression
        expr();
        if (!consume(SEMICOLON)) tkerr(crtTk, "missing ; after for init expression");

        // Optional condition expression
        expr();
        if (!consume(SEMICOLON)) tkerr(crtTk, "missing ; after for condition");

        // Optional increment expression
        expr();
        if (!consume(RPAR)) tkerr(crtTk, "missing ) after for increment");

        if (!stm()) tkerr(crtTk, "missing statement after for");
        return 1;
    }

    if (consume(BREAK)) {
        if (!consume(SEMICOLON)) tkerr(crtTk, "missing ; after break");
        return 1;
    }

    if (consume(RETURN)) {
        expr(); // optional expression
        if (!consume(SEMICOLON)) tkerr(crtTk, "missing ; after return");
        return 1;
    }

    // expr? SEMICOLON
    expr(); // optional
    if (!consume(SEMICOLON)) {
        crtTk = startTk;
        return 0;
    }

    return 1;
}


int stmCompound() {
    if (!consume(LACC)) return 0;

    Symbol *start = symbols.end[-1]; // mark where to clean up
    crtDepth++;

    while (1) {
        if (declVar()) { }
        else if (stm()) { }
        else break;
    }

    crtDepth--;
    deleteSymbolsAfter(&symbols, start);

    if (!consume(RACC)) tkerr(crtTk, "missing } or syntax error");
    return 1;
}

