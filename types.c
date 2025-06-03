#include "types.h"
#include <stdio.h>

Type createType(int typeBase, int nElements)
{
    Type t;
    t.typeBase = typeBase;
    t.nElements = nElements;
    return t;
}

Type getArithType(Type *s1, Type *s2)
{
    if (s1->nElements >= 0 || s2->nElements >= 0)
        tkerr(crtTk, "cannot do arithmetic with arrays");
    if (s1->typeBase == TB_STRUCT || s2->typeBase == TB_STRUCT)
        tkerr(crtTk, "cannot do arithmetic with structs");

    if (s1->typeBase == TB_DOUBLE || s2->typeBase == TB_DOUBLE)
        return createType(TB_DOUBLE, -1);
    if (s1->typeBase == TB_INT || s2->typeBase == TB_INT)
        return createType(TB_INT, -1);
    return createType(TB_CHAR, -1);
}

int equalTypes(Type *t1, Type *t2)
{
    if (t1->typeBase != t2->typeBase)
        return 0;
    if (t1->nElements != t2->nElements)
        return 0;
    if (t1->typeBase == TB_STRUCT && t1->s != t2->s)
        return 0;
    return 1;
}

const char *typeToString(Type *t)
{
    static char buf[128];
    const char *base;
    switch (t->typeBase)
    {
    case TB_INT:
        base = "int";
        break;
    case TB_DOUBLE:
        base = "double";
        break;
    case TB_CHAR:
        base = "char";
        break;
    case TB_STRUCT:
        base = t->s->name;
        break;
    default:
        base = "unknown";
        break;
    }
    if (t->nElements >= 0)
        snprintf(buf, sizeof(buf), "%s[%d]", base, t->nElements);
    else
        snprintf(buf, sizeof(buf), "%s", base);
    return buf;
}

void cast(Type *dst, Type *src) {
    // Ensure types match structurally; no semantic checks here

    // Check arrays must be same base type and both arrays
    if (src->nElements > -1 || dst->nElements > -1) {
        if (!(src->nElements > -1 && dst->nElements > -1 && src->typeBase == dst->typeBase)) {
            tkerr(crtTk, "invalid array cast");
        }
        return;
    }

    // For structs, only allow same struct type
    if (src->typeBase == TB_STRUCT || dst->typeBase == TB_STRUCT) {
        if (!(src->typeBase == TB_STRUCT && dst->typeBase == TB_STRUCT && src->s == dst->s)) {
            tkerr(crtTk, "invalid struct cast");
        }
        return;
    }
}


int canCast(Type *src, Type *dst) {
    // Array rules
    if (src->nElements > -1 || dst->nElements > -1) {
        return (src->nElements > -1 && dst->nElements > -1 && src->typeBase == dst->typeBase);
    }

    // Primitive type casting rules
    switch (src->typeBase) {
    case TB_CHAR:
    case TB_INT:
    case TB_DOUBLE:
        switch (dst->typeBase) {
        case TB_CHAR:
        case TB_INT:
        case TB_DOUBLE:
            return 1;
        default:
            return 0;
        }

    case TB_STRUCT:
        return (dst->typeBase == TB_STRUCT && src->s == dst->s);

    default:
        return 0;
    }
}

