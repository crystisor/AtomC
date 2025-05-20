#include "types.h"


int consume(int code);

int unit();

int declStruct();
int declVar();
int declFunc();
int funcArg();

int typeName();
int typeBase();
int arrayDecl();

int factor();
int ruleWhile();

int expr(RetVal *rv);
int exprAssign(RetVal *rv);
int exprOr(RetVal *rv);
int exprAnd(RetVal *rv);
int exprEq(RetVal *rv);
int exprRel(RetVal *rv);
int exprAdd(RetVal *rv);
int exprMul(RetVal *rv);
int exprCast(RetVal *rv);
int exprUnary(RetVal *rv);
int exprPostfix(RetVal *rv);
int exprPrimary(RetVal *rv);
int isTypeName();

int stm();
int stmCompound();