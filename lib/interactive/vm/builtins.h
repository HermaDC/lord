#pragma once
#include <stddef.h>

#include "eval.h"

Value vm_print_layout(VM *vm, Value arg);
Value vm_update_system_status(VM *vm, Value arg);
Value vm_print(VM *vm, Value arg);
Value vm_foo(VM *vm, Value arg);
Value vm_exit(VM *vm, Value arg);
Value vm_system_loaded(VM *vm, Value arg);

extern const struct call_options funcs[];
extern const size_t count_call_options;
