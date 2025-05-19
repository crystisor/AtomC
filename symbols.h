#ifndef SYMBOLS_H
#define SYMBOLS_H

#include "token.h"

// --- Type Base ---
enum { TB_INT, TB_DOUBLE, TB_CHAR, TB_STRUCT, TB_VOID };

// --- Symbol Class ---
enum { CLS_VAR, CLS_FUNC, CLS_EXTFUNC, CLS_STRUCT };

// --- Memory Class ---
enum { MEM_GLOBAL, MEM_ARG, MEM_LOCAL };

// Forward declaration
struct _Symbol;
typedef struct _Symbol Symbol;

// Type
typedef struct {
    int typeBase;     // TB_*
    Symbol *s;        // struct definition if TB_STRUCT
    int nElements;    // >0: array, 0: array without size, <0: non-array
} Type;

// Symbols list
typedef struct {
    Symbol **begin;
    Symbol **end;
    Symbol **after;
} Symbols;

// Symbol
struct _Symbol {
    const char *name;
    int cls;          // CLS_*
    int mem;          // MEM_*
    Type type;
    int depth;        // 0-global, 1-func, 2+ nested blocks

    union {
        Symbols args;     // for functions
        Symbols members;  // for structs
    };
};

// Global symbol table
extern Symbols symbols;

// Functions
void initSymbols(Symbols *symbols);
Symbol *addSymbol(Symbols *symbols, const char *name, int cls);
Symbol *findSymbol(Symbols *symbols, const char *name);
void dropSymbols(Symbols *symbols, int depth);
void showSymbolTable(Symbols *symbols);

#endif
