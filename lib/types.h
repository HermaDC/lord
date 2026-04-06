#ifndef TYPES_H
#define TYPES_H

#include <stdbool.h>
#include <stdlib.h>
#include "config.h"

#define NO_FOLLOWING_TRACK -1 

//\x1b[38;2;R;G;Bm
//    rgb(39, 180, 26)
#define BLUE "\x1b[38;2;0;160;200m"
#define RED    "\x1b[38;2;255;20;20m"
#define GREEN  "\x1b[38;2;39;180;26m"
#define YELLOW "\x1b[38;2;181;137;0m"
#define RESET  "\x1b[0m"

//Enum for the different errors that can occur during execution
typedef enum {
    ERR_OK = 0,
    ERR_GENERAL,
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

//parsing the error value to a string to help debugging
const char *error_to_string(ErrorCode err);

typedef enum LogLevel {
    LOG_ERROR,
    LOG_WARNING,
    LOG_INFO,
    LOG_DEBUG,
    LOG_TOTAL_LEVELS
} LogLevel;


//Determines the type of the token
typedef enum {
    NUMBER,
    OPEN,
    CLOSE,
    SW
} TokenType;

typedef struct {
    int value;
    TokenType type;
    size_t column; //Column were the token appears in the str
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
    NO_SWITCH = -1,
    STRAIGHT_POS = 0,
    DIVERGING_POS
} SwitchPosition;



typedef struct Track {
    TrackType type;
    int id;

    Status status;

    int next_index; // -1 if not following
    int prev_index; // -1 if not following

    Direction dir;
    
    SwitchPosition pos;
    int branch; // -1 if not following

    Sensor *sensors;

} Track;

//Holds all the info for a line of the system
typedef struct System {
    Track *array; // Is the array of track
    int count; // count of the tracks saved, 0 none, 1 one track...
    size_t buffer; //size of the reserved buffer
} System;

//Init correctly the system struct
ErrorCode init_system(System *sys, size_t initial_capacity);

//Correctly free the system data
void free_system(System *sys);

//
// 📦 STACK
//
typedef struct {
    int top;
    int data[MAX_STACK_SIZE];
} SwitchStack;

void initialize_stack(SwitchStack *stack);

void push(SwitchStack *stack, int value);

int pop(SwitchStack *stack);

bool is_empty(SwitchStack *stack);

bool is_full(SwitchStack *stack);

#endif // TYPES_H
