#ifndef CTHULHU_H
#define CTHULHU_H

#include <stdint.h>
#include <stddef.h>



typedef struct CtAST {
    enum { 
        UNARY,
        BINARY,
        TERNARY,
        LITERAL
    } type;

    union {
        struct CtAST *unary;

        struct {
            struct CtAST *lhs;
            struct CtAST *rhs;
        } binary;

        struct {
            struct CtAST *cond;
            struct CtAST *yes;
            struct CtAST *no;
        } ternary;


    };
} CtAST;

#endif /* CTHULHU_H */
