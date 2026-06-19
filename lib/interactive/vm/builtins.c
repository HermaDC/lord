#include "builtins.h"

#include <stddef.h>
#include <stdio.h>

#include "../../utils.h"
#include "eval.h"

static Value make_none(void) {
    return (Value){.data.number_value = 0, .type = VALUE_NONE};
}

Value vm_help(Value arg) {
    (void) arg;
    printf("Welcome to lord help :)\n");
    return make_none();
}

static double value_to_number(Value value) {
    switch(value.type) {
    case VALUE_NUMBER:
        return value.data.number_value;
    case VALUE_BOOL:
        return value.data.bool_value ? 1.0 : 0.0;
    default:
        return 0.0;
    }
}

Value vm_print_layout(Value arg) {
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

Value vm_update_system_status(Value arg) {
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
