#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "token.h"

Token *addTk(int code)
{
    Token *tk;
    SAFEALLOC(tk, Token)
    tk->code = code;
    tk->line = line;
    tk->next = NULL;
    if (lastToken)
    {
        lastToken->next = tk;
    }
    else
    {
        tokens = tk;
    }
    lastToken = tk;
    return tk;
}


void tkerr(const Token *tk, const char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    fprintf(stderr, "error in line %d: ", tk->line);
    vfprintf(stderr, fmt, va);
    fputc('\n', stderr);
    va_end(va);
    exit(-1);
}

void showTokens() {
    for (Token *tk = tokens; tk != NULL; tk = tk->next) {
        printf("%d", tk->code);
        switch (tk->code) {
            case ID:
            case CT_STRING:
                printf(":%s\n", tk->text);
                break;
            case CT_INT:
                printf(":%ld\n", tk->i);
                break;
            case CT_REAL:
                printf(":%lf\n", tk->r);
                break;
            case CT_CHAR:
                printf(":'%c'\n", (char)tk->i);
                break;
        }
        printf(" ");
    }
    printf("\n");
}


void done() {
    Token *tk;
    while (tokens) {
        tk = tokens;
        tokens = tokens->next;
        if (tk->code == ID || tk->code == CT_STRING) {
            free(tk->text);
        }
        free(tk);
    }
}