#include <stdio.h>
#include <stdlib.h>
#include "syntactic.h"

Token *crtTk = NULL;
Token *consumedTk = NULL;

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

int declStruct()
{
    Token *startTk = crtTk;
    if (!consume(STRUCT))
    {
        return 0;
    }
    if (!consume(ID))
        tkerr(crtTk, "missing struct name");
    if (!consume(LACC))
        tkerr(crtTk, "missing { after struct name");
    while (declVar())
    { /* consume declVar zero or more times */
    }
    if (!consume(RACC))
        tkerr(crtTk, "missing } after struct body");
    if (!consume(SEMICOLON))
        tkerr(crtTk, "missing ; after struct declaration");
    return 1;
}

int declVar()
{
    Token *startTk = crtTk;
    if (!typeBase())
    {
        return 0;
    }
    if (!consume(ID))
        tkerr(crtTk, "missing variable name");
    arrayDecl(); // optional

    while (consume(COMMA))
    {
        if (!consume(ID))
            tkerr(crtTk, "missing variable name after ,");
        arrayDecl(); // optional
    }

    if (!consume(SEMICOLON))
        tkerr(crtTk, "missing ; after variable declaration");
    return 1;
}

int funcArg()
{
    if (!typeBase())
        return 0;
    if (!consume(ID))
        tkerr(crtTk, "missing argument name");
    arrayDecl(); // optional
    return 1;
}

int declFunc()
{
    Token *startTk = crtTk;

    if (typeBase())
    {
        consume(MUL); // optional *
    }
    else if (consume(VOID))
    {
        // OK
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

    if (!consume(LPAR))
        tkerr(crtTk, "missing ( in function declaration");

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

    if (!stmCompound())
        tkerr(crtTk, "missing function body");

    return 1;
}

int typeName()
{
    if (!typeBase())
        return 0;
    arrayDecl(); // optional
    return 1;
}

int typeBase()
{
    if (consume(INT))
        return 1;
    if (consume(DOUBLE))
        return 1;
    if (consume(CHAR))
        return 1;
    if (consume(STRUCT))
    {
        if (consume(ID))
        {
            return 1;
        }
        else
        {
            tkerr(crtTk, "missing identifier after struct");
        }
    }
    return 0;
}

int arrayDecl()
{
    if (consume(LBRACKET))
    {
        if (!expr())
            tkerr(crtTk, "invalid expression in array declaration");
        if (!consume(RBRACKET))
            tkerr(crtTk, "missing ] in array declaration");
        return 1;
    }
    return 0; // optional, so no error here
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
        if (typeName()) {
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
    if (!consume(LACC))
        return 0;

    while (1) {
        if (declVar()) {
            // variable declaration
        }
        else if (stm()) {
            // statement
        }
        else {
            break;
        }
    }

    if (!consume(RACC))
        tkerr(crtTk, "missing } or syntax error");

    return 1;
}
