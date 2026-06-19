#pragma once
#include <stddef.h>

#include "eval.h"

Value vm_print_layout(Value arg);
Value vm_update_system_status(Value arg);
Value vm_print(Value arg);
Value vm_foo(Value arg);
Value vm_exit(Value arg);

extern const struct call_options funcs[];
extern const size_t count_call_options;
