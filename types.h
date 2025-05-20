#include "symbols.h"

typedef union
{
    long int i;      // int, char
    double d;        // double
    const char *str; // char[]
} CtVal;

typedef struct
{
    Type type;   // type of the result
    int isLVal;  // if it is a LVal
    int isCtVal; // if it is a constant value (int, real, char, char[])
    CtVal ctVal; // the constat value
    int wasCast; // if it was casted
} RetVal;

extern Token *crtTk;

Type getArithType(Type *s1, Type *s2);
void cast(Type *dst, Type *src);
Type createType(int typeBase, int nElements);
int equalTypes(Type *t1, Type *t2);
const char* typeToString(Type *t);
int canCast(Type *src, Type *dst);