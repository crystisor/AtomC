#include <stdio.h>
#include <stdlib.h>
#include "utils.h"
#include "lexer.h"
// #include "token.h"
#include "syntactic.h"
#include "symbols.h"

int line = 1;
Token *tokens = NULL, *lastToken = NULL;
const char *pCrtCh;
char *input;
Token *crtTk = NULL;

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <source-file>\n", argv[0]);
        return 1;
    }

    // Open the file
    FILE *f = fopen(argv[1], "rb");
    if (!f)
    {
        perror("Could not open file");
        return 1;
    }

    // Get file size
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);
    printf("Size of the file is: %ld\n", fsize);

    // Read content into memory (+1 for '\0')
    input = (char *)malloc(fsize + 1);
    if (!input)
    {
        fclose(f);
        err("not enough memory to read file");
    }

    fread(input, 1, fsize, f);
    fclose(f);
    input[fsize] = '\0'; // null terminator

    pCrtCh = input; // start processing

    // Tokenization loop
    if ((unsigned char)input[0] == 0xEF &&
        (unsigned char)input[1] == 0xBB &&
        (unsigned char)input[2] == 0xBF)
    {
        pCrtCh += 3;
    }
    while (getNextToken() != END)
        ;

    crtTk = tokens;
    initSymbols(&symbols);
    showTokens();
    printf("a");
    crtTk = tokens;
    printf("aa");

    if (unit())
    {
        printf("Syntax is valid.\n");
    }
    else
    {
        fprintf(stderr, "Syntax error.\n");
    }

    if (crtTk->code != END)
    {
        tkerr(crtTk, "Syntax error: extra tokens at the end");
    }

    // Free everything
    done();
    free(input);

    return 0;
}