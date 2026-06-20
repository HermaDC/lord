#include "builtins.h"

#include <stddef.h>
#include <stdio.h>

#include "../../utils.h"
#include "eval.h"
#include "utils.h"

const struct call_options funcs[] = {{"print", 1, vm_print},
                                     {"print_layout", 1, vm_print_layout},
                                     {"foo", 0, vm_foo},
                                     {"exit", 0, vm_exit},
                                     {"update_system", 1, vm_update_system_status},
                                     {"systems_loaded", 0, vm_system_loaded}};

const size_t count_call_options = sizeof(funcs) / sizeof(funcs[0]);

Value vm_help(VM *vm, Value arg) {
    (void) vm;
    (void) arg;
    printf("Welcome to lord help :)\n");
    return make_none();
}

Value vm_print(VM *vm, Value arg) {
    (void) vm;
    switch(arg.type) {
    case VALUE_NUMBER:
        printf("%g", arg.data.number_value);
        break;
    case VALUE_BOOL:
        printf(arg.data.bool_value ? "True" : "False");
        break;
    default:
        printf("None");
        break;
    }
    printf("\n");
    return make_none();
}

Value vm_foo(VM *vm, Value arg) {
    (void) vm;
    (void) arg;
    printf("FOO\n");
    return make_none();
}

Value vm_exit(VM *vm, Value arg) {
    vm->should_exit = 1;
    vm->error_code = 0;
    if(arg.type != VALUE_NONE) vm->error_code = (int) value_to_number(arg);
    return make_none();
}

Value vm_print_layout(VM *vm, Value arg) {
    (void) vm;
    if(arg.type == VALUE_NONE) {
        printf("Expected layout id to print\n");
        return make_none();
    }

    double id_number = value_to_number(arg);
    int id = (int) id_number;
    if(id_number != (double) id) {
        printf("Layout id must be an integer\n");
        return make_none();
    }

    if(id < 0 || (size_t) id >= app_context.count) {
        printf("Layout id %d out of range\n", id);
        return make_none();
    }

    print_tracks_with_switches(&app_context.systems[id], 0);
    printf("\n");
    return make_none();
}

Value vm_update_system_status(VM *vm, Value arg) {
    (void) vm;
    if(arg.type == VALUE_NONE) {
        printf("Expected a system to update");
        return make_none();
    }
    double id_number = value_to_number(arg);
    int id = (int) id_number;
    if(id_number != (double) id) {
        printf("Layout id must be an integer\n");
        return make_none();
    }
    if(id < 0 || (size_t) id > app_context.count) {
        printf("layout id out of range");
        return make_none();
    }
    update_system_status(&app_context.systems[id], 0);
    return make_none();
}

Value vm_system_loaded(VM *vm, Value arg) {
    (void) arg;
    (void) vm;
    return make_number(app_context.count);
}
