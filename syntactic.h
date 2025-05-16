#include "token.h"

extern Token *consumedTk;
extern Token *crtTk;

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

int expr();
int exprAssign();
int exprOr();
int exprAnd();
int exprEq();
int exprRel();
int exprAdd();
int exprMul();
int exprCast();
int exprUnary();
int exprPostfix();
int exprPrimary();

int stm();
int stmCompound();