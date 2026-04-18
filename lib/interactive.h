typedef enum {
    CMD_ERR_OK = 0,
    CMD_ERR_INVALID_COMMAND,
    CMD_ERR_TOO_FEW_ARGS,
    CMD_ERR_INVALID_ARG,
    CMD_ERR_VAR_NOT_DEFINED,
    CMD_ERR_OOM

} CmdErrorCode;

int interactive_main_loop(void);
