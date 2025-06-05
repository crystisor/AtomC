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
    printf("DEBUG: s1->nElements = %d, s2->nElements = %d\n", s1->nElements, s2->nElements);

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

int typeEquals(Type *a, Type *b) {
    return a->typeBase == b->typeBase &&
           a->nElements == b->nElements;
           //a->nPtr == b->nPtr;
}

void cast(Type *dst, Type *src)
{
    // Ensure types match structurally; no semantic checks here

    // Check arrays must be same base type and both arrays
    if (src->nElements > -1 || dst->nElements > -1)
    {
        if (!(src->nElements > -1 && dst->nElements > -1 && src->typeBase == dst->typeBase))
        {
            tkerr(crtTk, "invalid array cast");
        }
        return;
    }

    // For structs, only allow same struct type
    if (src->typeBase == TB_STRUCT || dst->typeBase == TB_STRUCT)
    {
        if (!(src->typeBase == TB_STRUCT && dst->typeBase == TB_STRUCT && src->s == dst->s))
        {
            tkerr(crtTk, "invalid struct cast");
        }
        return;
    }
}

int canCast(Type *src, Type *dst) {

    if (typeEquals(src, dst)) return 1;

    // 1. Array vs Scalar Mismatch: reject
    if ((src->nElements >= 0 && dst->nElements == -1) ||
        (src->nElements == -1 && dst->nElements >= 0)) {
        return 0; // Can't cast between array and scalar
    }

    // 2. Array to Array: must match base type and size
    if (src->nElements >= 0 && dst->nElements >= 0) {
        return src->typeBase == dst->typeBase &&
               src->nElements == dst->nElements;
    }

    // 3. Pointer types
    if (src->nPtr || dst->nPtr) {
        if (src->nPtr && dst->nPtr) {
            // Same base type, or one is void
            return src->typeBase == dst->typeBase ||
                   src->typeBase == TB_VOID ||
                   dst->typeBase == TB_VOID;
        }
        return 0; // Mismatch: one is pointer, the other is not
    }

    // 4. Structs: must match exactly
    if (src->typeBase == TB_STRUCT || dst->typeBase == TB_STRUCT) {
        return src->typeBase == TB_STRUCT &&
               dst->typeBase == TB_STRUCT &&
               src->s == dst->s;
    }

    // 5. Void is not castable
    if (src->typeBase == TB_VOID || dst->typeBase == TB_VOID) {
        return 0;
    }

    // 6. Scalar types: allow standard numeric conversions
    switch (src->typeBase) {
        case TB_CHAR:
        case TB_INT:
        case TB_FLOAT:
        case TB_DOUBLE:
            switch (dst->typeBase) {
                case TB_CHAR:
                case TB_INT:
                case TB_FLOAT:
                case TB_DOUBLE:
                    return 1;
            }
            break;
    }

    return 0;
}

