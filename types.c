#include "types.h"

extern Token *crtTk;
Type createType(int typeBase, int nElements)
{
    Type t;
    t.typeBase = typeBase;
    t.nElements = nElements;
    return t;
}

void cast(Type *dst, Type *src)
{
    if (src->nElements > -1)
    {
        if (dst->nElements > -1)
        {
            if (src->typeBase != dst->typeBase)
                tkerr(crtTk, "an array cannot be converted to an array of another type");
        }
        else
        {
            tkerr(crtTk, "an array cannot be converted to a non-array");
        }
    }
    else
    {
        if (dst->nElements > -1)
        {
            tkerr(crtTk, "a non-array cannot be converted to an array");
        }
    }
    switch (src->typeBase)
    {
    case TB_CHAR:
    case TB_INT:
    case TB_DOUBLE:
        switch (dst->typeBase)
        {
        case TB_CHAR:
        case TB_INT:
        case TB_DOUBLE:
            return;
        }
    case TB_STRUCT:
        if (dst->typeBase == TB_STRUCT)
        {
            if (src->s != dst->s)
                tkerr(crtTk, "a structure cannot be converted to another one");
            return;
        }
    }
    tkerr(crtTk, "incompatible types");
}

int isArithmeticType(Type *t) {
    return t->nElements == -1 &&
           (t->typeBase == TB_INT || t->typeBase == TB_DOUBLE || t->typeBase == TB_CHAR);
}

int isStructType(Type *t) {
    return t->typeBase == TB_STRUCT;
}

int isArrayType(Type *t) {
    return t->nElements >= 0;
}

Type getArithType(Type *s1, Type *s2) {
    if (!isArithmeticType(s1) || !isArithmeticType(s2))
        tkerr(crtTk, "non-arithmetic types");

    if (s1->typeBase == TB_DOUBLE || s2->typeBase == TB_DOUBLE)
        return createType(TB_DOUBLE, -1);

    if (s1->typeBase == TB_INT || s2->typeBase == TB_INT)
        return createType(TB_INT, -1);

    return createType(TB_CHAR, -1);
}

