int a;                // Simple int declaration
int b = 10;           // int with initializer
char c = 'x';         // char with initializer
float f = 3.14F;      // float with initializer (using F suffix for clarity)
double d = 2.718;     // double with initializer
void *ptr_void = 0;   // void pointer initialized with null

// Test implicit int to float/double conversion (should succeed)
float fx = 30;
double dx = 50;
char cx = 70; // int to char (truncation allowed)

// --- 2. Array Declarations and Initializers ---

int x[10];                            // Array of ints (uninitialized)
char s[5] = "test";                   // char array with string initializer (exactly fitting)
char s2[] = "hello world";            // char array with implicit size from string initializer
float arr[3] = {1.0F, 2.0F, 3.0F};    // Array of floats with initializer list

// --- 3. Pointer Declarations and Initializers ---

int *p_int;                           // Pointer to int (uninitialized)
float *p_float = &f;                  // Pointer to float initialized with address of float
double *p_double = &d;                // Pointer to double initialized with address of double
char *p_char_str = "hello";           // Pointer to char initialized with string literal

// --- 4. Function Declarations ---

void simpleFunc();                  // Function declaration with void return and no params
int sum(int x, int y);              // Function declaration with int params and int return
char getChar();                     // Function declaration returning char with no params
float calculate(float val);         // Function declaration for float type

// --- 5. Type Casting Tests (within a function) ---

int testCast() {
    int i = 100;
    float fl = 200.5F;
    double db = 300.7;
    char ch = 'A';

    // Implicit numeric casts (should succeed)
    fl = i;        // int to float
    db = i;        // int to double
    i = fl;        // float to int (truncation)
    i = db;        // double to int (truncation)
    fl = db;       // double to float (truncation/precision loss)
    db = fl;       // float to double (promotion)
    ch = i;        // int to char (truncation)
    i = ch;        // char to int (promotion)

    // Pointer casting (should require explicit cast or lead to warnings/errors)
    // int *ptr_i = &db; // ERROR: Incompatible pointer types
    // void *ptr_v = &i; // SUCCESS: T* to void*
    // int *ptr_i2 = ptr_v; // SUCCESS: void* to T* (assuming ptr_v holds int*)
    // int **ptr_ptr_i = &p_int; // SUCCESS: Pointer to pointer

    return i;
}

// --- 6. Main Function - Test Assignments and Function Calls ---

int main() {
    a = 5;                  // Assignment
    b = a + 3;              // Expression assignment
    c = 'z';                // char assignment
    f = 2.71F;              // float assignment
    d = 1.618;              // double assignment

    x[0] = 1;               // Array element assignment
    x[9] = 99;
    s[1] = 'o';             // char array element assignment
    // s[5] = 'a';          // This line should produce an array out-of-bounds error (if implemented)

    p_int = &a;             // Pointer assignment
    *p_int = 15;            // Dereference and assign
    // p_double = &f;       // ERROR: Incompatible pointer types (double* = float*)
    // p_int = p_double;    // ERROR: Incompatible pointer types

    simpleFunc();           // Function call
    sum(1, 2);              // Function call with arguments
    char gc = getChar();    // Function call with return value
    float calc_res = calculate(10.5F); // Function call with float argument

    // --- 7. Expected Compile-Time Errors/Warnings (Commented out for initial compilation) ---

    // int emptyArr[];          // ERROR: Array size missing and no initializer
    // int *invalid_ptr = 10;   // ERROR: Cannot initialize pointer with int literal (unless it's 0)
    // int *p_err = &d;         // ERROR: Incompatible pointer types (int* = double*)
    // double *q_err = &f;      // ERROR: Incompatible pointer types (double* = float*) - this was our original problem!
    // double **pp_err = &p_double; // SUCCESS: pointer to pointer initialization
    // int *invalid_ptr_assign = &p_int; // ERROR: int* = int** (different levels of indirection)
    // double *q_ptr_err = &f; // ERROR: double* = float* (from previous tests)
    // double *qq = &q_ptr_err; // ERROR: double* = double** (different levels of indirection)

    return 0;
}

// --- 8. Function Definitions ---

void simpleFunc() {
    // nothing
}

int sum(int x, int y) {
    return x + y;
}

char getChar() {
    return 'G';
}

float calculate(float val) {
    return val * 2.0F;
}