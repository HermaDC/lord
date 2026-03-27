#ifndef TYPES_H
#define TYPES_H

#include <stdlib.h>
#include "config.h"

//
// 🔴 ERROR CODES
//
typedef enum {
    ERR_OK = 0,
    ERR_INVALID_ARG,
    ERR_NULL_PTR,
    ERR_NO_MEMORY,
    ERR_NOT_FOUND,
    ERR_ALREADY_EXISTS,
    ERR_EMPTY,
    ERR_INVALID_STATE,
    ERR_OUT_OF_BOUNDS,
    ERR_NOT_CONNECTED,
    ERR_BROKEN_LINK
} ErrorCode;

const char *error_to_string(ErrorCode err);


//
// 🔢 TOKENIZER
//
typedef enum {
    NUMBER,
    OPEN,
    CLOSE,
    SW
} TokenType;

typedef struct {
    int value;
    TokenType type;
    size_t column;
} Token;

typedef enum {
    TOKENIZE_OK,
    TOKENIZE_MISSING_NUM,
    TOKENIZE_UNKNOWN_CHAR,
    TOKENIZE_UNMATCHED_PARENTHESES,
    TOKENIZE_OOM,
    TOKENIZE_EMPTY_STR,
    TOKENIZE_SWITCH_STACK_OVERFLOW
} TokenizeError;



//
// 🚦 TRACK SYSTEM
//
typedef enum {
    CLEAR,
    OCCUPIED,
    WARNING
} Status;

typedef enum {
    NEXT = 1,
    PREV = -1
} Direction;

typedef struct Sensor {
    int hex_direction;
    int actual_state; //TODO add enum for the state instead of a int
} Sensor;

typedef enum {
    STRAIGHT,
    SWITCH_TRACK
} TrackType;

typedef enum {
    STRAIGHT_POS,
    DIVERGING_POS
} SwitchPosition;


//
// 🔗 TRACK STRUCTURES
//
typedef struct Track {
    TrackType type;
    int id;

    Status status;

    struct Track *next;
    struct Track *prev;

    Direction dir;

    Sensor *sensors;

} Track;

typedef struct {
    Track base;
    Track *branch;
    SwitchPosition pos;
} Switch;


//
// 📦 STACK
//
typedef struct {
    int top;
    Switch *data[MAX_STACK_SIZE];
} SwitchStack;

void initialize(SwitchStack *stack);

void push(SwitchStack *stack, Switch *value);

Switch *pop(SwitchStack *stack);

#endif // TYPES_H


