int a;                     // simple int
int b = 10;                // int with initializer
char c = 'x';              // char with initializer
float f = 3.14;            // float with initializer

int x[10];                 // array of ints
char s[5] = "test";        // char array with initializer
float arr[3];              // array of floats
int emptyArr[];            // should fail (no size or initializer)

int *p;                    // pointer to int
float *q = &f;             // pointer to float with initializer
int *r = &b;               // pointer to int with initializer

void simpleFunc();         // function declaration with void return
int sum(int x, int y);     // function declaration with params
char getChar();            // function returning char

int testCast() {
    int i = 5;
    float f = i;           // implicit cast
    i = f;                 // implicit cast
    return i;
}

int main() {
    a = 5;
    b = a + 3;
    c = 'z';
    f = 2.71;
    x[0] = 1;
    s[1] = 'o';
    //p = &a;
   // r = &b;

    simpleFunc();
    sum(1, 2);

    return 0;
}

void simpleFunc() {
    // nothing
}

int sum(int x, int y) {
    return x + y;
}

char getChar() {
    return 'G';
}
