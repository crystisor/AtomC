#include "utils.h"

#define SAFEALLOC(var, Type)                          \
    if ((var = (Type *)malloc(sizeof(Type))) == NULL) \
        err("not enough memory");

typedef struct _Token
{
    int code; // code (name)
    union
    {
        char *text; // used for ID, CT_STRING (dynamically allocated)
        long int i; // used for CT_INT, CT_CHAR
        double r;   // used for CT_REAL
    };
    int line;            // the input file line
    struct _Token *next; // link to the next token
} Token;


enum
{
    // Identifiers
    ID, // identifier

    // Constants
    CT_INT,    // integer constant
    CT_REAL,   // real constant
    CT_CHAR,   // character constant
    CT_STRING, // string constant

    // Keywords
    BREAK,
    CHAR,
    DOUBLE,
    FLOAT,
    ELSE,
    FOR,
    IF,
    INT,
    RETURN,
    STRUCT,
    VOID,
    WHILE,
    AMP,

    // Delimiters
    COMMA,     // ,
    SEMICOLON, // ;
    LPAR,      // (
    RPAR,      // )
    LBRACKET,  // [
    RBRACKET,  // ]
    LACC,      // {
    RACC,      // }

    // Operators
    ADD,       // +
    SUB,       // -
    MUL,       // "*"
    DIV,       // /
    DOT,       // .
    AND,       // &&
    OR,        // ||
    NOT,       // "!"
    ASSIGN,    // =
    EQUAL,     // ==
    NOTEQ,     // "!="
    LESS,      // <
    LESSEQ,    // <=
    GREATER,   // >
    GREATEREQ, // >=

    // Special
    END // end of input
};

Token *addTk(int code);
void showTokens(void);
void done(void);
extern Token *lastToken;
extern Token *tokens;
extern int line;
void tkerr(const Token *tk, const char *fmt, ...);