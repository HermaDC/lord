#include <stdio.h>

typedef enum {
    CMD_ERR_OK = 0,
    CMD_ERR_UNKNOWN_CMD,
    CMD_ERR_TOO_FEW_ARGS,
    CMD_ERR_INVALID_ARG,
    CMD_ERR_SYNTAX,
    CMD_ERR_VAR_NOT_DEFINED,
    CMD_ERR_OUT_OF_BOUNDS,
    CMD_ERR_OOM

} CmdErrorCode;

static inline char* cmderror_to_str(CmdErrorCode err){
    switch (err)
    {
    case CMD_ERR_UNKNOWN_CMD:
        return "unknown command";
    case CMD_ERR_TOO_FEW_ARGS:
        return "too few args were passed";
    case CMD_ERR_INVALID_ARG:
        return "args value is not valid";
    case CMD_ERR_VAR_NOT_DEFINED:
        return "variable not defined";
    case CMD_ERR_OUT_OF_BOUNDS:
        return "out of bounds access";    
    case CMD_ERR_OOM:
        return "not enough memory";
    default:
        return "unknown error";
    }
}

int interactive_main_loop(void);

