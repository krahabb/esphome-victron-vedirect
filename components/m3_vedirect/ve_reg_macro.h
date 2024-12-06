/*
    Helper macros for some funny conditional macro processing
    code from: https://stackoverflow.com/questions/5586429/ifdef-inside-define

*/
#define PASTE_(x, y) x##y
#define PASTE(x, y) PASTE_(x, y)
#define PASTE3_(x, y, z) x##y##z
#define PASTE3(x, y, z) PASTE3_(x, y, z)
#define Y(...) __VA_ARGS__
#define N(...)
#define IF(x) x  // alternate method similar to IFNOT()

#define NOT_N Y
#define NOT_Y N
#define IF_NOT(x) PASTE(NOT_, x)
#define NOT(x) PASTE(NOT_, x)

#define N_OR_N N
#define N_OR_Y Y
#define Y_OR_N Y
#define Y_OR_Y Y
#define OR(x, y) PASTE3(x, _OR_, y)

#define N_AND_N N
#define N_AND_Y N
#define Y_AND_N N
#define Y_AND_Y Y
#define AND(x, y) PASTE3(x, _AND_, y)

#define N_XOR_N N
#define N_XOR_Y Y
#define Y_XOR_N Y
#define Y_XOR_Y N
#define XOR(x, y) PASTE3(x, _XOR_, y)

#define N_NOR_N Y
#define N_NOR_Y N
#define Y_NOR_N N
#define Y_NOR_Y N
#define NOR(x, y) PASTE3(x, _NOR_, y)

#define N_NAND_N Y
#define N_NAND_Y Y
#define Y_NAND_N Y
#define Y_NAND_Y N
#define NAND(x, y) PASTE3(x, _NAND_, y)

#define N_XNOR_N Y
#define N_XNOR_Y N
#define Y_XNOR_N N
#define Y_XNOR_Y Y
#define XNOR(x, y) PASTE3(x, _XNOR_, y)

#define IF2(x, y, z) PASTE3(x, y, z)
