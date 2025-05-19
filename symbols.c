#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symbols.h"
#include "utils.h"

// Global variables
Symbols symbols;
int crtDepth = 0;

void initSymbols(Symbols *symbols) {
    symbols->begin = NULL;
    symbols->end = NULL;
    symbols->after = NULL;
    printf("Initialized symbols\n");
}

Symbol *addSymbol(Symbols *symbols, const char *name, int cls) {
    // Redefinition check at the same depth
    for (Symbol **p = symbols->end - 1; p >= symbols->begin; p--) {
        if ((*p)->depth < crtDepth) break;
        if (strcmp((*p)->name, name) == 0)
            err("symbol redefinition: %s", name);
    }

    // Resize if needed
    if (symbols->end == symbols->after) {
        int count = symbols->after - symbols->begin;
        int n = count * 2;
        if (n == 0) n = 1;

        symbols->begin = (Symbol **)realloc(symbols->begin, n * sizeof(Symbol *));
        if (!symbols->begin) err("not enough memory");

        symbols->end = symbols->begin + count;
        symbols->after = symbols->begin + n;
    }

    Symbol *s;
    SAFEALLOC(s, Symbol);
    *symbols->end++ = s;

    s->name = name;
    s->cls = cls;
    s->depth = crtDepth;

    // For functions/structs, init sub-symbols
    if (cls == CLS_FUNC) initSymbols(&s->args);
    else if (cls == CLS_STRUCT) initSymbols(&s->members);

    return s;
}

Symbol *findSymbol(Symbols *symbols, const char *name) {
    for (Symbol **p = symbols->end - 1; p >= symbols->begin; p--) {
        if (strcmp((*p)->name, name) == 0)
            return *p;
    }
    return NULL;
}

void dropSymbols(Symbols *symbols, int depth) {
    while (symbols->end > symbols->begin && (*(symbols->end - 1))->depth == depth) {
        symbols->end--;
    }
}

void showSymbolTable(Symbols *symbols) {
    printf("----- Symbol Table -----\n");
    for (Symbol **p = symbols->begin; p < symbols->end; p++) {
        Symbol *s = *p;
        printf("Symbol: %s, cls: %d, mem: %d, typeBase: %d, depth: %d\n",
               s->name, s->cls, s->mem, s->type.typeBase, s->depth);
    }
    printf("------------------------\n");
}


void deleteSymbolsAfter(Symbols *symbols, Symbol *start) {
    // Remove all symbols declared after 'start' (including start if needed)
    while (symbols->end > symbols->begin) {
        Symbol *last = *(symbols->end - 1);
        if (last == start)
            break;
        symbols->end--;
        // Optionally, free symbol if you allocated dynamically:
        // free(last);
    }
}

