#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include "lexer.h"
#include "token.h"
#include "utils.h"

int getNextToken() {
    int state = 0;
    char ch;
    const char *pStartCh;
    Token *tk;

    while (1) {
        ch = *pCrtCh;
        //printf("State %d, char '%c' (ASCII %d)\n", state, ch, ch);

        switch (state) {
        case 0:
            if (isalpha(ch) || ch == '_') {
                pStartCh = pCrtCh++;
                state = 1;
            } else if (isdigit(ch)) {
                pStartCh = pCrtCh++;
                state = 10;
            } else if (ch == '\'') {
                pStartCh = pCrtCh++;
                state = 30;
            } else if (ch == '"') {
                pStartCh = pCrtCh++;
                state = 40;
            } else if (ch == '=') {
                pCrtCh++;
                state = 20;
            } else if (ch == '!') {
                pCrtCh++;
                state = 21;
            } else if (ch == '<') {
                pCrtCh++;
                state = 22;
            } else if (ch == '>') {
                pCrtCh++;
                state = 23;
            } else if (ch == '&') {
                if (*(pCrtCh + 1) == '&') {
                    pCrtCh += 2;
                    addTk(AND);
                    return AND;
                } else {
                    pCrtCh++;
                    addTk(AMP);
                    return AMP;
                    tkerr(addTk(END), "invalid character: expected &&");
                }
            } else if (ch == '|') {
                if (*(pCrtCh + 1) == '|') {
                    pCrtCh += 2;
                    addTk(OR);
                    return OR;
                } else {
                    tkerr(addTk(END), "invalid character: expected ||");
                }
            } else if (ch == '+') {
                pCrtCh++;
                addTk(ADD);
                return ADD;
            } else if (ch == '-') {
                pCrtCh++;
                addTk(SUB);
                return SUB;
            } else if (ch == '*') {
                pCrtCh++;
                addTk(MUL);
                return MUL;
            } else if (ch == '/') {
                if (*(pCrtCh + 1) == '/') {
                    pCrtCh += 2;
                    while (*pCrtCh && *pCrtCh != '\n') pCrtCh++;
                } else if (*(pCrtCh + 1) == '*') {
                    pCrtCh += 2;
                    while (*pCrtCh && !(*pCrtCh == '*' && *(pCrtCh + 1) == '/')) {
                        if (*pCrtCh == '\n') line++;
                        pCrtCh++;
                    }
                    if (*pCrtCh) pCrtCh += 2;
                } else {
                    pCrtCh++;
                    addTk(DIV);
                    return DIV;
                }
            } else if (ch == '.') {
                pCrtCh++;
                addTk(DOT);
                return DOT;
            } else if (ch == '(') {
                pCrtCh++;
                addTk(LPAR);
                return LPAR;
            } else if (ch == ')') {
                pCrtCh++;
                addTk(RPAR);
                return RPAR;
            } else if (ch == '{') {
                pCrtCh++;
                addTk(LACC);
                return LACC;
            } else if (ch == '}') {
                pCrtCh++;
                addTk(RACC);
                return RACC;
            } else if (ch == '[') {
                pCrtCh++;
                addTk(LBRACKET);
                return LBRACKET;
            } else if (ch == ']') {
                pCrtCh++;
                addTk(RBRACKET);
                return RBRACKET;
            } else if (ch == ';') {
                pCrtCh++;
                addTk(SEMICOLON);
                return SEMICOLON;
            } else if (ch == ',') {
                pCrtCh++;
                addTk(COMMA);
                return COMMA;
            } else if (ch == ' ' || ch == '\t' || ch == '\r') {
                pCrtCh++;
            } else if (ch == '\n') {
                line++;
                pCrtCh++;
            } else if (ch == 0) {
                addTk(END);
                return END;
            } else {
                tkerr(addTk(END), "invalid character");
            }
            break;

        case 1:
            if (isalnum(ch) || ch == '_') {
                pCrtCh++;
            } else {
                state = 2;
            }
            break;

        case 2:
        {
            int nCh = pCrtCh - pStartCh;
            if (nCh == 5 && !memcmp(pStartCh, "break", 5)) tk = addTk(BREAK);
            else if (nCh == 4 && !memcmp(pStartCh, "char", 4)) tk = addTk(CHAR);
            else if (nCh == 6 && !memcmp(pStartCh, "double", 6)) tk = addTk(DOUBLE);
            else if (nCh == 4 && !memcmp(pStartCh, "else", 4)) tk = addTk(ELSE);
            else if (nCh == 5 && !memcmp(pStartCh, "float", 5)) tk = addTk(FLOAT);
            else if (nCh == 3 && !memcmp(pStartCh, "for", 3)) tk = addTk(FOR);
            else if (nCh == 2 && !memcmp(pStartCh, "if", 2)) tk = addTk(IF);
            else if (nCh == 3 && !memcmp(pStartCh, "int", 3)) tk = addTk(INT);
            else if (nCh == 6 && !memcmp(pStartCh, "return", 6)) tk = addTk(RETURN);
            else if (nCh == 6 && !memcmp(pStartCh, "struct", 6)) tk = addTk(STRUCT);
            else if (nCh == 4 && !memcmp(pStartCh, "void", 4)) tk = addTk(VOID);
            else if (nCh == 5 && !memcmp(pStartCh, "while", 5)) tk = addTk(WHILE);
            else {
                tk = addTk(ID);
                tk->text = createString(pStartCh, pCrtCh);
            }
            return tk->code;
        }

        case 10:
            if (isdigit(ch)) {
                pCrtCh++;
            } else if (ch == '.') {
                pCrtCh++;
                state = 11;
            } else if (ch == 'e' || ch == 'E') {
                pCrtCh++;
                state = 13;
            } else {
                tk = addTk(CT_INT);
                char *s = createString(pStartCh, pCrtCh);
                tk->i = strtol(s, NULL, 10);
                free(s);
                return CT_INT;
            }
            break;

        case 11:
            if (isdigit(ch)) {
                pCrtCh++;
                state = 12;
            } else {
                tkerr(addTk(END), "invalid real constant");
            }
            break;

        case 12:
            if (isdigit(ch)) {
                pCrtCh++;
            } else if (ch == 'e' || ch == 'E') {
                pCrtCh++;
                state = 13;
            } else {
                tk = addTk(CT_REAL);
                char *s = createString(pStartCh, pCrtCh);
                tk->r = strtod(s, NULL);
                free(s);
                return CT_REAL;
            }
            break;

        case 13:
            if (ch == '+' || ch == '-') {
                pCrtCh++;
                state = 14;
            } else if (isdigit(ch)) {
                state = 14;
            } else {
                tkerr(addTk(END), "invalid exponent in real constant");
            }
            break;

        case 14:
            if (isdigit(ch)) {
                pCrtCh++;
                state = 15;
            } else {
                tkerr(addTk(END), "expected digit in exponent");
            }
            break;

        case 15:
            if (isdigit(ch)) {
                pCrtCh++;
            } else {
                tk = addTk(CT_REAL);
                char *s = createString(pStartCh, pCrtCh);
                tk->r = strtod(s, NULL);
                free(s);
                return CT_REAL;
            }
            break;

        case 20:
            if (ch == '=') {
                pCrtCh++;
                addTk(EQUAL);
                return EQUAL;
            } else {
                addTk(ASSIGN);
                return ASSIGN;
            }

        case 21:
            if (ch == '=') {
                pCrtCh++;
                addTk(NOTEQ);
                return NOTEQ;
            } else {
                addTk(NOT);
                return NOT;
            }

        case 22:
            if (ch == '=') {
                pCrtCh++;
                addTk(LESSEQ);
                return LESSEQ;
            } else {
                addTk(LESS);
                return LESS;
            }

        case 23:
            if (ch == '=') {
                pCrtCh++;
                addTk(GREATEREQ);
                return GREATEREQ;
            } else {
                addTk(GREATER);
                return GREATER;
            }

        case 30: // Char constant
            if (ch == '\\') {
                pCrtCh++;
                if (strchr("abfnrtv'\"?\\0", *pCrtCh)) {
                    pCrtCh++;
                } else {
                    tkerr(addTk(END), "invalid escape sequence in character");
                }
            } else if (ch && ch != '\'' && ch != '\\') {
                pCrtCh++;
            } else {
                tkerr(addTk(END), "invalid character constant");
            }

            if (*pCrtCh == '\'') {
                pCrtCh++;
                tk = addTk(CT_CHAR);
                return CT_CHAR;
            } else {
                tkerr(addTk(END), "unterminated character constant");
            }

        case 40: // String constant
            while (*pCrtCh && *pCrtCh != '"') {
                if (*pCrtCh == '\\') pCrtCh++;
                if (*pCrtCh) pCrtCh++;
            }
            if (*pCrtCh == '"') {
                pCrtCh++;
                tk = addTk(CT_STRING);
                tk->text = createString(pStartCh + 1, pCrtCh - 1); // exclude quotes
                return CT_STRING;
            } else {
                tkerr(addTk(END), "unterminated string constant");
            }

        default:
            tkerr(addTk(END), "invalid state");
        }
    }
}
