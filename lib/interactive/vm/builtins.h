#pragma once
#include <stddef.h>
#include "eval.h"

Value vm_print_layout(Value arg);
Value vm_update_system_status(Value arg);
Value vm_print(Value arg);
Value vm_foo(Value arg);
Value vm_exit(Value arg);


const struct call_options funcs[] = {{"print", 1, vm_print},
                                     {"print_layout", 1, vm_print_layout},
                                     {"foo", 0, vm_foo},
                                     {"exit", 0, vm_exit},
                                     {"update_system", 1, vm_update_system_status}};

const size_t count_call_options = sizeof(funcs) / sizeof(funcs[0]);
