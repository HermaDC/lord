#include "utils.h"

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <time.h>

#include "config.h"
#include "layout-parser.h"
#include "types.h"

#define TOKEN_FOR_FILE " ,;"

#define TRACK_JSON_FMT                                                                   \
    "\"id\": %d, \"direction\": \"%s\", \"status\": \"%s\", "                            \
    "\"next\": %d, \"prev\": %d, \"sensor\": %#X"

#define TRACK_JSON_FMT_SWITCH TRACK_JSON_FMT ", \"position\": \"%s\", \"branch\": %d"

#define TRACK_JSON_ARGS(t)                                                               \
    (t)->id, (t)->dir == NEXT ? "NEXT" : "PREVIOUS", status_to_str((t)->status),         \
        (t)->next_index, (t)->prev_index, (t)->sensors->hex_direction

#define TRACK_JSON_FMT_SWITCH_ARGS(t)                                                    \
    TRACK_JSON_ARGS(t), (t)->pos == STRAIGHT_POS ? "STRAIGHT_POS" : "DIVERGING_POS",     \
        (t)->branch

int generate_id() {
    static int global_id_counter = 0;
    return global_id_counter++;
}

void log_message(LogLevel level, const char *format, ...) {
    if(level == LOG_DEBUG && !global_config.VERBOSE) return;
    const char *level_str;
    switch(level) {
    case LOG_ERROR:
        level_str = "ERROR";
        break;
    case LOG_WARNING:
        level_str = "WARNING";
        break;
    case LOG_INFO:
        level_str = "INFO";
        break;
    case LOG_DEBUG:
        level_str = "DEBUG";
        break;
    default:
        level_str = "UNKNOWN";
        break;
    }
    time_t t;
    struct tm *tm_info;
    char time_stamp_buffer[32]; // 20 for format, 12 more for extra safety

    time(&t);                // Get actual time
    tm_info = localtime(&t); // Use local time

    strftime(time_stamp_buffer, sizeof(time_stamp_buffer), "%Y-%m-%d %H:%M:%S", tm_info);

    va_list args;
    va_start(args, format);

    FILE *f = fopen(LOG_PATH, "a");
    if(!f) {
        fprintf(stderr, "[CRITICAL] Failed to open log file\n");
        return;
    }

    fprintf(f, "%s [%s] ", time_stamp_buffer, level_str);
    vfprintf(f, format, args);
    fprintf(f, "\n");
    fclose(f);

    va_end(args);
}

ErrorCode create_track(System *system, int id, Sensor *sensor, int next, int prev) {
    if(!system || !sensor) return ERR_INVALID_ARG;
    if(next < NO_FOLLOWING_TRACK || next >= system->count) return ERR_INVALID_ARG;
    if(prev < NO_FOLLOWING_TRACK || prev >= system->count) return ERR_INVALID_ARG;

    if((size_t) (system->count + 1) >= system->buffer) {
        system->buffer *= 2;
        Track *temp = realloc(system->array, system->buffer * sizeof(Track));
        if(!temp) return ERR_NO_MEMORY;
        system->array = temp;
    }
    if(id == 0) id = generate_id();
    Track temp_track = {.id = id,
                        .type = STRAIGHT,
                        .status = OCCUPIED,
                        .next_index = next,
                        .prev_index = prev,
                        .dir = NEXT,
                        .pos = NO_SWITCH,
                        .branch = NO_FOLLOWING_TRACK,
                        .sensors = sensor};

    system->array[system->count] = temp_track;
    system->count++;

    return ERR_OK;
}

ErrorCode create_switch(System *system, int id, Sensor *sensor, int next, int prev,
                        int branch) {
    if(!system || !sensor) return ERR_INVALID_ARG;
    if(next < NO_FOLLOWING_TRACK || next >= system->count) return ERR_INVALID_ARG;
    if(prev < NO_FOLLOWING_TRACK || prev >= system->count) return ERR_INVALID_ARG;
    if(branch < NO_FOLLOWING_TRACK || branch >= system->count) return ERR_INVALID_ARG;

    if((size_t) (system->count + 1) >= system->buffer) {
        system->buffer *= 2;
        Track *temp = realloc(system->array, system->buffer * sizeof(Track));
        if(!temp) return ERR_NO_MEMORY;
        system->array = temp;
    }
    if(id == 0) id = generate_id();
    Track temp_track = {.id = id,
                        .type = SWITCH_TRACK,
                        .status = OCCUPIED,
                        .next_index = next,
                        .prev_index = prev,
                        .dir = NEXT,
                        .branch = branch,
                        .pos = STRAIGHT_POS,
                        .sensors = sensor};

    system->array[system->count] = temp_track;
    system->count++;

    return ERR_OK;
}

ErrorCode insert_switch(System *system, int id, Sensor *sensor, int track_next,
                        int track_prev, int branch) {
    if(!system || !sensor) return ERR_INVALID_ARG;
    if(track_next < NO_FOLLOWING_TRACK || track_next >= system->count)
        return ERR_INVALID_ARG;
    if(track_prev < NO_FOLLOWING_TRACK || track_prev >= system->count)
        return ERR_INVALID_ARG;
    if(branch < NO_FOLLOWING_TRACK || branch >= system->count) return ERR_INVALID_ARG;

    if(track_next == track_prev) return ERR_INVALID_ARG;

    // Create the switch
    ErrorCode err = create_switch(system, id, sensor, track_next, track_prev, branch);
    if(err != ERR_OK) return ERR_NO_MEMORY;

    int sw_index = system->count - 1;

    // Connect the neighbor tracks
    if(track_prev >= 0) system->array[track_prev].next_index = sw_index;
    if(track_next >= 0) system->array[track_next].prev_index = sw_index;

    log_message(LOG_INFO, "Inserted switch with ID %d between tracks %d and %d", id,
                track_prev, track_next);

    return ERR_OK;
}

ErrorCode create_straight_line(System *system, int num_tracks, size_t *head_index) {
    if(!system) return ERR_INVALID_ARG;
    if(num_tracks < 0) return ERR_INVALID_ARG;
    if(num_tracks == 0) {
        if(head_index) *head_index = NO_FOLLOWING_TRACK;
        return ERR_OK;
    }

    int current_index = NO_FOLLOWING_TRACK;
    int first_created_index = NO_FOLLOWING_TRACK;

    for(int i = 1; i <= num_tracks; i++) {
        Sensor *sen = malloc(sizeof(Sensor));
        if(!sen) return ERR_NO_MEMORY;
        sen->hex_direction = 0;
        sen->actual_state = SENSOR_CLEAR;
        ErrorCode err =
            create_track(system, generate_id(), sen, NO_FOLLOWING_TRACK, current_index);

        if(err != ERR_OK) return ERR_GENERAL;

        int new_index = system->count - 1;

        if(first_created_index == NO_FOLLOWING_TRACK) { first_created_index = new_index; }

        if(current_index > NO_FOLLOWING_TRACK) {
            system->array[current_index].next_index = new_index;
        }

        current_index = new_index;
    }
    if(first_created_index >= NO_FOLLOWING_TRACK) {
        if(head_index) *head_index = first_created_index;
    }

    log_message(LOG_INFO, "Created straight line of %d tracks starting at index %zu",
                num_tracks, first_created_index);

    return ERR_OK;
}

int get_last_track(System *system, int start_index) {
    if(!system) return 0;
    if(start_index < 0 || start_index >= system->count) start_index = 0;

    int current = start_index;
    while(current > NO_FOLLOWING_TRACK && current < system->count &&
          system->array[current].next_index != NO_FOLLOWING_TRACK) {
        current = system->array[current].next_index;
    }
    return current;
}

int get_next_track(System *system, int start_index) {

    if(!system) return NO_FOLLOWING_TRACK;
    Track track = system->array[start_index];

    /* Si es switch, mirar posición */
    if(track.type == SWITCH_TRACK) {

        if(track.pos == DIVERGING_POS && track.branch > NO_FOLLOWING_TRACK) {
            int branch_next = system->array[track.branch].next_index;
            return branch_next;
        }
    }

    return track.next_index;
}

// Recursive printer for a chain starting at `start` following next pointers
// and printing branch contents in braces.
static void print_chain(System *system, int start, bool *visited) {
    int current = start;
    while(current > -1 && current < system->count && !visited[current]) {
        visited[current] = true;
        Track *t = &system->array[current];

        if(t->type == SWITCH_TRACK) {
            if(t->pos == STRAIGHT_POS)
                printf("(%d)SW[→]", t->id);
            else
                printf("(%d)SW[~]", t->id);

            // print branch recursively if exists
            if(t->branch > NO_FOLLOWING_TRACK) {
                printf("{");
                print_chain(system, t->branch, visited);
                printf("}");
            }

            // indicate connection or stop for switch
            int next_in_dir = (t->dir == NEXT) ? t->next_index : t->prev_index;
            if(next_in_dir > NO_FOLLOWING_TRACK) {
                if(t->dir == NEXT)
                    printf(" " GREEN "------ " RESET "→ ");
                else
                    printf(" ← " GREEN "------ " RESET " ");
            } else {
                printf(" | ");
            }
        } else {
            // Straight track
            printf("(%d)", t->id);
            switch(t->status) {
            case CLEAR:
                printf(GREEN "------ " RESET);
                break;
            case OCCUPIED:
                printf(RED "------ " RESET);
                break;
            case WARNING:
                printf(YELLOW "------ " RESET);
                break;
            }

            int next_in_dir = (t->dir == NEXT) ? t->next_index : t->prev_index;
            if(next_in_dir > NO_FOLLOWING_TRACK) {
                if(t->dir == NEXT)
                    printf("→ ");
                else
                    printf("← ");
            } else {
                printf("| ");
            }
        }

        // move to the next track following travel direction (handles switches)
        current = get_next_track(system, current);
    }
}

void print_tracks_with_switches(System *system, int index) {
    if(!system) {
        printf("NULL pointer passed\n");
        return;
    }
    // Track visitation map to avoid duplicate prints
    bool *visited = calloc(system->count, sizeof(bool));
    if(!visited) {
        printf("Memory allocation failed\n");
        return;
    }

    // Start printing from requested index if valid, otherwise from 0
    if(index >= 0 && index < system->count) {
        print_chain(system, index, visited);
        printf("\n");
    }

    // Ensure all tracks are printed: any unvisited tracks are printed as separate
    // chains so the whole system is displayed.
    for(int i = 0; i < system->count; i++) {
        if(!visited[i]) {
            print_chain(system, i, visited);
            printf("\n");
        }
    }

    free(visited);
}

bool is_in_chain(System *system, int origin, int dest, ErrorCode *exit_err) {
    if(!system) return ERR_INVALID_ARG;

    if(dest < 0 || dest >= system->count) {
        *(exit_err) = ERR_INVALID_ARG;
        return false;
    }
    if(origin < 0 || origin >= system->count) {
        *(exit_err) = ERR_INVALID_ARG;
        return false;
    }

    int current = origin;
    while(current > -1 && current < system->count) {
        if(current == dest) return true;
        current = system->array[current].next_index;
    }

    return false;
}

int count_track(System *system, int start, int *last) {
    if(!system || system->count <= 0) {
        if(last) *last = -1;
        return 0;
    }

    int count = 0;
    int current = start;

    while(current > -1 && current < system->count &&
          system->array[current].type == STRAIGHT) {
        current = system->array[current].next_index;
        count++;
    }

    if(last) *last = current;
    return count;
}

int count_branch_tracks(System *system, int branch_index) {
    int count = 0;
    int current = branch_index;
    while(current > -1 && current < system->count) {
        if(system->array[current].type == SWITCH_TRACK) {
            // Contar también la rama de un switch dentro de esta rama
            count += count_branch_tracks(system, system->array[current].branch);
        } else {
            count++;
        }
        current = system->array[current].next_index;
    }
    return count;
}

ErrorCode read_sensor_data(Sensor *sensor) {
    if(!sensor) return ERR_NULL_PTR;

    sensor->actual_state = (rand() % 2 == 0) ? SENSOR_CLEAR : SENSOR_OCCUPIED;
    log_message(LOG_DEBUG, "Read sensor data: %d", sensor->actual_state);
    return ERR_OK;
}

// Updates the track status based on its sensor data.
// A track becomes WARNING only when its own sensor is clear and the next track in
// the current travel direction is occupied.
int update_track_status(System *system, int track_index) {
    if(!system || track_index < 0 || track_index >= system->count) return -1;

    Track *track = &system->array[track_index];
    Sensor *sensor = track->sensors;
    if(!sensor) return -1;

    if(read_sensor_data(sensor) != ERR_OK) {
        track->status = OCCUPIED;
        return -1;
    }

    int next_in_dir_index = (track->dir == NEXT) ? track->next_index : track->prev_index;
    Track *next_in_dir_track = NULL;
    if(next_in_dir_index >= 0 && next_in_dir_index < system->count) {
        next_in_dir_track = &system->array[next_in_dir_index];
    }

    switch(sensor->actual_state) {
    case SENSOR_CLEAR:
        if(next_in_dir_track && next_in_dir_track->status == OCCUPIED) {
            track->status = WARNING;
        } else {
            track->status = CLEAR;
        }
        return 0;

    case SENSOR_OCCUPIED:
        track->status = OCCUPIED;
        return 1;

    default:
        track->status = OCCUPIED;
        log_message(
            LOG_WARNING,
            "Unknown sensor state %d for track index %d, setting status to OCCUPIED",
            sensor->actual_state, track_index);
        return -1;
    }
}

// Forces the status without reading the sensor.
ErrorCode force_update_track_status(System *system, int track_index, Status new_status) {
    if(!system || track_index < 0 || track_index >= system->count)
        return ERR_OUT_OF_BOUNDS;

    Track *track = &system->array[track_index];

    int next_in_dir_index = (track->dir == NEXT) ? track->next_index : track->prev_index;
    Track *next_in_dir_track = NULL;
    if(next_in_dir_index >= 0 && next_in_dir_index < system->count) {
        next_in_dir_track = &system->array[next_in_dir_index];
    }

    switch(new_status) {
    case CLEAR:
        if(next_in_dir_track && next_in_dir_track->status == OCCUPIED) {
            track->status = WARNING;
        } else {
            track->status = CLEAR;
        }
        return ERR_OK;

    case OCCUPIED:
        track->status = OCCUPIED;
        return ERR_OK;

    case WARNING:
        track->status = WARNING;
        return ERR_OK;

    default:
        track->status = OCCUPIED;
        return ERR_INVALID_ARG;
    }
}

void update_system_status(System *system, int index) {
    if(!system || index < 0) return;
    bool *visited = calloc(system->count, sizeof(bool));
    if(!visited) return;

    int current = (index >= 0 && index < system->count) ? index : 0;
    while(current >= 0 && current < system->count) {
        if(!visited[current]) {
            update_track_status(system, current);
            visited[current] = true;
        }

        int next = get_next_track(system, current);
        if(next == NO_FOLLOWING_TRACK || next == current || visited[next]) { break; }

        current = next;
    }

    for(int i = 0; i < system->count; i++) {
        if(!visited[i]) {
            update_track_status(system, i);
            visited[i] = true;
        }
    }
}

// Returns the index of the first track
int tokens_to_track(System *system, LayoutToken *tokens, size_t token_count) {
    if(!system || !tokens || token_count <= 0) return NO_FOLLOWING_TRACK;

    int head_index = NO_FOLLOWING_TRACK;
    int prev_index = NO_FOLLOWING_TRACK;
    bool pending_switch = false;

    typedef struct {
        int switch_index;
        int outer_prev;
        int branch_head;
        int branch_last;
    } SwitchContext;

    SwitchContext switch_stack[MAX_STACK_SIZE];
    int switch_stack_top = -1;

    for(size_t i = 0; i < token_count; i++) {
        LayoutToken actual = tokens[i];

        switch(actual.type) {
        case SW:
            if(pending_switch) {
                log_message(LOG_ERROR, "Unexpected SW nested before OPEN at token %zu",
                            i);
                return NO_FOLLOWING_TRACK;
            }
            pending_switch = true;
            continue;

        case OPEN:
            if(!pending_switch) {
                log_message(LOG_ERROR, "Unexpected OPEN without SW at token %zu", i);
                return NO_FOLLOWING_TRACK;
            }

            {
                Sensor *sen = malloc(sizeof(Sensor));
                if(!sen) return NO_FOLLOWING_TRACK;
                sen->hex_direction = 0;
                sen->actual_state = SENSOR_CLEAR;

                ErrorCode err =
                    create_switch(system, generate_id(), sen, NO_FOLLOWING_TRACK,
                                  prev_index, NO_FOLLOWING_TRACK);
                if(err != ERR_OK) {
                    log_message(LOG_ERROR, "Failed to create switch for token %zu: %s", i,
                                error_to_string(err));
                    return NO_FOLLOWING_TRACK;
                }

                int switch_index = system->count - 1;
                if(prev_index >= 0) {
                    system->array[prev_index].next_index = switch_index;
                }
                if(head_index == NO_FOLLOWING_TRACK) { head_index = switch_index; }

                if(switch_stack_top + 1 >= MAX_STACK_SIZE) {
                    log_message(LOG_ERROR, "Switch stack overflow at token %zu", i);
                    return NO_FOLLOWING_TRACK;
                }

                switch_stack[++switch_stack_top] =
                    (SwitchContext){.switch_index = switch_index,
                                    .outer_prev = prev_index,
                                    .branch_head = NO_FOLLOWING_TRACK,
                                    .branch_last = NO_FOLLOWING_TRACK};

                prev_index = NO_FOLLOWING_TRACK;
                pending_switch = false;
            }
            continue;

        case CLOSE:
            if(switch_stack_top < 0) {
                log_message(LOG_ERROR,
                            "Unexpected CLOSE without matching switch at token %zu", i);
                return NO_FOLLOWING_TRACK;
            }

            {
                SwitchContext ctx = switch_stack[switch_stack_top--];
                int swidx = ctx.switch_index;

                system->array[swidx].branch = ctx.branch_head;
                if(ctx.branch_head >= 0) {
                    system->array[ctx.branch_head].prev_index = swidx;
                }

                prev_index = swidx;
            }
            continue;

        case NUMBER:
            if(actual.value <= 0) {
                log_message(LOG_ERROR, "Invalid number of tracks %d at token %zu",
                            actual.value, i);
                return NO_FOLLOWING_TRACK;
            }

            {
                size_t chain_head;
                ErrorCode err = create_straight_line(system, actual.value, &chain_head);
                if(err != ERR_OK) {
                    log_message(LOG_ERROR,
                                "Failed to create track chain for token %zu: %s", i,
                                error_to_string(err));
                    return NO_FOLLOWING_TRACK;
                }

                if(head_index == NO_FOLLOWING_TRACK) { head_index = chain_head; }

                if(prev_index >= 0) {
                    system->array[prev_index].next_index = chain_head;
                    system->array[chain_head].prev_index = prev_index;
                }

                int chain_last = system->count - 1;
                prev_index = chain_last;

                if(switch_stack_top >= 0) {
                    SwitchContext *ctx = &switch_stack[switch_stack_top];
                    if(ctx->branch_head == NO_FOLLOWING_TRACK) {
                        ctx->branch_head = chain_head;
                    }
                    ctx->branch_last = chain_last;
                }
            }
            break;

        default:
            log_message(LOG_ERROR, "Unknown token type %d at token %zu", actual.type, i);
            return NO_FOLLOWING_TRACK;
        }
    }

    if(switch_stack_top >= 0) {
        log_message(LOG_ERROR, "Unclosed switch expression at end of token stream");
        return NO_FOLLOWING_TRACK;
    }

    log_message(LOG_INFO, "Converted %zu tokens to track chain starting at index %d",
                token_count, head_index);
    return head_index;
}

// returns an array of systems
System *load_system_layout_from_file(const char *path, size_t *out_count) {
    if(!path || !out_count) return NULL;
    *out_count = 0;

    size_t buffer = 1;
    FILE *f = fopen(path, "r");
    if(!f) return NULL;

    char *line = NULL;
    size_t read = 0;
    System *system_arr = malloc(buffer * sizeof(System));
    if(!system_arr) {
        fclose(f);
        return NULL;
    }

    for(size_t i = 0; i < buffer; i++) {
        system_arr[i].array = NULL;
        system_arr[i].count = 0;
        system_arr[i].buffer = 0;
    }

    if(init_system(&system_arr[0], 16) != ERR_OK) {
        free(system_arr);
        fclose(f);
        return NULL;
    }

    while(getline(&line, &read, f) > 0) {
        line[strcspn(line, "\n")] = 0; // remove newline
        if(strlen(line) < 1) continue; // skip empty lines
        log_message(LOG_DEBUG, "Parsing line: \"%s\"", line);

        if(system_arr[*out_count].array == NULL) {
            if(init_system(&system_arr[*out_count], 16) != ERR_OK) {
                log_message(LOG_ERROR, "Failed to initialize system array for line %zu",
                            *out_count);
                free(line);
                fclose(f);
                return NULL;
            }
        }

        size_t num_tokens;
        TokenizeError err = 0;
        LayoutToken *tokens = tokenize_layout(line, &num_tokens, &err);
        if(err != TOKENIZE_OK) {
            log_message(LOG_ERROR, "Tokenization failed for line %zu: %d", *out_count,
                        err);
            free(line);
            fclose(f);
            return NULL;
        }
        if(!tokens) continue; // empty line or comments

        check_syntax_layout_config(tokens, line, num_tokens, &err);
        if(err != TOKENIZE_OK) {
            log_message(LOG_ERROR, "Syntax check failed for line %zu: %d", *out_count,
                        err);
        }

        int head_index = tokens_to_track(&system_arr[*out_count], tokens, num_tokens);
        if(head_index == NO_FOLLOWING_TRACK) {
            log_message(LOG_ERROR, "Failed to convert tokens to track chain for line %zu",
                        *out_count);
            free(tokens);
            free(line);
            fclose(f);
            return NULL;
        }

        free(tokens);

        (*out_count)++;
        if(*out_count >= buffer) {
            size_t old_buffer = buffer;
            buffer *= 2;
            System *temp = realloc(system_arr, buffer * sizeof(System));
            if(!temp) {
                log_message(LOG_ERROR, "Failed to expand system array buffer to %zu",
                            buffer);
                free(line);
                fclose(f);
                return NULL;
            }
            system_arr = temp;

            for(size_t j = old_buffer; j < buffer; j++) {
                system_arr[j].array = NULL;
                system_arr[j].count = 0;
                system_arr[j].buffer = 0;
            }
        }
    }
    free(line);
    fclose(f);

    return system_arr;
}

static ErrorCode save_system_chain(FILE *f, System *system, int start_index,
                                   bool *need_separator) {
    if(!f || !system || !need_separator) return ERR_INVALID_ARG;

    int current = start_index;
    while(current > -1 && current < system->count) {
        int next_index = NO_FOLLOWING_TRACK;
        int straight_count = count_track(system, current, &next_index);
        if(straight_count > 0) {
            fprintf(f, "%d ", straight_count);
            *need_separator = true;
            current = next_index;
        }

        if(current == -1 || current >= system->count) { break; }

        Track *track = &system->array[current];
        if(track->type != SWITCH_TRACK) { break; }

        fprintf(f, "SW( ");

        bool inner_separator = false;
        if(track->branch > -1) {
            ErrorCode err = save_system_chain(f, system, track->branch, &inner_separator);
            if(err != ERR_OK) return err;
        }

        fprintf(f, ") ");
        *need_separator = true;
        current = track->next_index;
    }

    return ERR_OK;
}

// Saves the layout of system to path
ErrorCode save_system_to_file(System *system, const char *path) {
    if(!system || !path) return ERR_INVALID_ARG;

    FILE *f = fopen(path, "w");
    if(!f) return ERR_GENERAL;

    bool need_separator = false;
    ErrorCode err = save_system_chain(f, system, 0, &need_separator);
    if(err != ERR_OK) {
        fclose(f);
        return err;
    }

    fprintf(f, "\n");
    fclose(f);
    log_message(LOG_INFO, "Saved system layout to file: %s", path);
    return ERR_OK;
}

// Returns a malloc string
char *track_to_json_object(Track *track) {
    if(track->type == SWITCH_TRACK) {
        size_t len = snprintf(NULL, 0, "{ " TRACK_JSON_FMT_SWITCH " }",
                              TRACK_JSON_FMT_SWITCH_ARGS(track));

        char *str = malloc(len + 1);
        snprintf(str, len + 1, "{ " TRACK_JSON_FMT_SWITCH " }",
                 TRACK_JSON_FMT_SWITCH_ARGS(track));
        return str;

    } else {
        size_t len = snprintf(NULL, 0, "{ " TRACK_JSON_FMT " }", TRACK_JSON_ARGS(track));

        char *str = malloc(len + 1);
        snprintf(str, len + 1, "{ " TRACK_JSON_FMT " }", TRACK_JSON_ARGS(track));
        return str;
    }
    return NULL;
}

ErrorCode save_system_to_json(System *system, const char *path) {
    if(!system || !path) return ERR_INVALID_ARG;

    FILE *f = fopen(path, "w");
    if(!f) return ERR_GENERAL;

    fprintf(f, "[\n");
    for(size_t i = 0; i < (size_t) system->count; i++) {
        Track *t = &system->array[i];
        char *str = track_to_json_object(t);
        fprintf(f, "  %s", str);
        free(str);
        if((i + 1) < (size_t) system->count) { fprintf(f, ","); }
        fprintf(f, "\n");
    }
    fprintf(f, "]");

    return ERR_OK;
}
