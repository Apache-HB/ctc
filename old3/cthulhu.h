#ifndef CTHULHU_H
#define CTHULHU_H

typedef struct {

} CtError;

typedef struct {

} CtState;

typedef struct {

} CtStream;

typedef struct {

} CtParser;

typedef struct {

} CtNode;

typedef struct {

} CtInterp;

typedef struct {
    enum {
        ID_FUNC,
        ID_TYPE,
        ID_VAR,
        ID_BUILTIN
    } type;
    const char *name;
    void *data;
} CtBuiltin;

typedef struct {

} CtCall;

#endif /* CTHULHU_H */
