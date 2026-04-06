#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>
#include <stdint.h>

#include "utils.h"
#include "parser.h"
#include "config.h"

//TODO: Finish the implementation of the file loading
//TODO: Implement the saving of the system layout to a file
//TODO: use a define instead of hardcoding the -1 for "no track"
 
#define TOKEN_FOR_FILE " ,;"

int generate_id(){
    static int global_id_counter = 0;
    return global_id_counter++;
}

void log_message(LogLevel level, const char *format, ...){

    const char *level_str;
    switch (level) {
        case LOG_ERROR:   level_str = "ERROR"; break;
        case LOG_WARNING: level_str = "WARNING"; break;
        case LOG_INFO:    level_str = "INFO"; break;
        case LOG_DEBUG:   level_str = "DEBUG"; break;
        default:          level_str = "UNKNOWN"; break;
    }
    time_t t;
    struct tm *tm_info;
    char time_stamp_buffer[32];  // 20 for format, 12 more for extra safety

    time(&t);                     // Get actual time
    tm_info = localtime(&t);      // Use local time

    strftime(time_stamp_buffer, sizeof(time_stamp_buffer), "%Y-%m-%d %H:%M:%S", tm_info);

    va_list args;
    va_start(args, format);

    FILE *f = fopen(LOG_PATH, "a");
    if (!f) {
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
    if (!system || !sensor) return ERR_INVALID_ARG;
    if (next < -1 || next >= system->count) return ERR_INVALID_ARG;
    if (prev < -1 || prev >= system->count) return ERR_INVALID_ARG;

    if((size_t)(system->count + 1) >= system->buffer){
        system->buffer *= 2;
        Track *temp = realloc(system->array, system->buffer * sizeof(Track));
        if(!temp) return ERR_NO_MEMORY;
        system->array = temp;
    }
    if(id==0) id = generate_id();
    Track temp_track = {
        .id = id,
        .type = STRAIGHT,
        .status = OCCUPIED,
        .next_index = next,
        .prev_index = prev,
        .dir = NEXT,
        .pos = NO_SWITCH,
        .branch = -1,
        .sensors = sensor
    };
    
    system->array[system->count] = temp_track;
    system->count++;

    return ERR_OK;

}

ErrorCode create_switch(System *system, int id, Sensor *sensor, int next, int prev, int branch){
    if (!system || !sensor) return ERR_INVALID_ARG;
    if (next < -1 || next >= system->count) return ERR_INVALID_ARG;
    if (prev < -1 || prev >= system->count) return ERR_INVALID_ARG;
    if (branch < -1 || branch >= system->count) return ERR_INVALID_ARG;

    if((size_t)(system->count + 1) >= system->buffer){
        system->buffer *= 2;
        Track *temp = realloc(system->array, system->buffer * sizeof(Track));
        if(!temp) return ERR_NO_MEMORY;
        system->array = temp;
    }
    if (id ==0) id = generate_id();
    Track temp_track = {
        .id = id,
        .type = SWITCH_TRACK,
        .status = OCCUPIED,
        .next_index = next,
        .prev_index = prev,
        .dir = NEXT,
        .branch = branch,
        .pos = STRAIGHT_POS,
        .sensors = sensor
    };
    
    system->array[system->count] = temp_track;
    system->count++;

    return ERR_OK;

}

ErrorCode insert_switch(System *system, int id, Sensor *sensor, int track_next, int track_prev, int branch) {
    if (!system || !sensor) return ERR_INVALID_ARG;
    if (track_next < -1 || track_next >= system->count) return ERR_INVALID_ARG;
    if (track_prev < -1 || track_prev >= system->count) return ERR_INVALID_ARG;
    if (branch < -1 || branch >= system->count) return ERR_INVALID_ARG;

    if(track_next == track_prev) return ERR_INVALID_ARG;

    // Create the switch
    ErrorCode err = create_switch(system, id, sensor, track_next, track_prev, branch);
    if (err != ERR_OK) return ERR_NO_MEMORY;

    
    int sw_index = system->count - 1;

    // Connect the neighbor tracks
    if (track_prev >= 0) system->array[track_prev].next_index = sw_index;
    if (track_next >= 0) system->array[track_next].prev_index = sw_index;

    log_message(LOG_INFO, "Inserted switch with ID %d between tracks %d and %d", id, track_prev, track_next);

    return ERR_OK;
}

ErrorCode create_straight_line(System *system, int num_tracks, size_t *head_index) {
    if (!system) return ERR_INVALID_ARG;
    if (num_tracks < 0) return ERR_INVALID_ARG;
    if (num_tracks == 0) {
        if(head_index) *head_index = NO_FOLLOWING_TRACK;
        return ERR_OK;
    }

    int current_index = -1;
    int first_created_index = -1;

    for (int i = 1; i <= num_tracks; i++) {
        Sensor *sen = malloc(sizeof(Sensor));
        if (!sen) return ERR_NO_MEMORY;
        sen->hex_direction = 0; //TODO: set a real direction
        sen->actual_state = 0; //TODO: set a real state
        ErrorCode err = create_track(system, generate_id(), sen, -1, current_index);
        
        if(err != ERR_OK) return ERR_GENERAL;

        int new_index = system->count - 1;

        if (first_created_index == -1) {
            first_created_index = new_index;
        }
        
        if (current_index >= 0) {
            system->array[current_index].next_index = new_index;
        }

        current_index = new_index;
    }
    if (first_created_index >= 0) {
        if(head_index) *head_index = first_created_index;
    }
    
    log_message(LOG_INFO, "Created straight line of %d tracks starting at index %zu", 
        num_tracks, first_created_index);

    return ERR_OK;
}

int get_last_track(System *system, int start_index) {
    if (!system) return 0;
    if (start_index < 0 || start_index >= system->count) start_index = 0;

    int current = start_index;
    while (current > -1 && current < system->count && system->array[current].next_index != -1) {
        current = system->array[current].next_index;
    }
    return current;
}

int get_next_track(System *system, int start_index) {

    if (!system) return -1;
    Track track = system->array[start_index];

    /* Si es switch, mirar posición */
    if (track.type == SWITCH_TRACK) {

        if (track.pos == DIVERGING_POS && track.branch > -1){
            int branch_next = system->array[track.branch].next_index;
            return branch_next;
        }
    }

    return track.next_index;
}

void print_tracks_with_switches(System *system, int index) {
    if(!system){
        printf("NULL pointer passed\n");
        return;
    }
    int current = index;
    while (current > -1 && current < system->count) {
        Track current_track = system->array[current];
        
        if (current_track.type == SWITCH_TRACK) {

            if (current_track.pos == STRAIGHT_POS)
                printf("(%d)SW[→]", current_track.id);
            else
                printf("(%d)SW[~]", current_track.id);

            // También podemos imprimir la rama recursivamente
            if (current_track.branch){
                printf("{");
                print_tracks_with_switches(system, current_track.branch);
                printf("} ");
            }
        } else {
            // Track normal
            printf("(%d)", current_track.id);
            switch (current_track.status) {
                case CLEAR:   printf(GREEN "------ " RESET); break;
                case OCCUPIED:printf(RED "------ " RESET); break;
                case WARNING: printf(YELLOW "------ " RESET); break;
            }
        }

        current = get_next_track(system, current);
    }

    //printf("\n");
}

bool is_in_chain(System *system, int origin, int dest, ErrorCode *exit_err) {
    if(!system) return ERR_INVALID_ARG;

    if (dest < 0 || dest >= system->count){
        *(exit_err) = ERR_INVALID_ARG;
        return false;
    }
    if (origin < 0 || origin >= system->count){
        *(exit_err) = ERR_INVALID_ARG;
        return false;
    }

    int current = origin;
    while (current > -1 && current < system->count) {
        if (current == dest) return true;
        current = system->array[current].next_index;
    }

    return false;
}

int count_track(System *system, int start, int *last){
    if (!system || system->count <= 0) {
        if (last) *last = -1;
        return 0;
    }

    int count = 0;
    int current = start;

    while (current > -1 && current < system->count && system->array[current].type == STRAIGHT) {
        current = system->array[current].next_index;
        count++;
    }

    if (last) *last = current;
    return count;
}

int count_branch_tracks(System *system, int branch_index) {
    int count = 0;
    int current = branch_index;
    while (current > -1 && current < system->count) {
        if (system->array[current].type == SWITCH_TRACK) {
            // Contar también la rama de un switch dentro de esta rama
            count += count_branch_tracks(system, system->array[current].branch);
        } else {
            count++;
        }
        current = system->array[current].next_index;
    }
    return count;
}

//TODO use the enum for errors
ErrorCode read_sensor_data(Sensor *sensor) {
    if (!sensor) return ERR_NULL_PTR; // Error: NULL sensor
    //TODO: Implement the real data
    int data = rand() % 3;
    sensor->actual_state = data; // Simulamos lectura de sensor (0, 1 o 2)
    log_message(LOG_DEBUG, "Read sensor data: %d", data);
    return ERR_OK;
}

//Updates the track status based on its sensor data
//Returns -1 on error, 0 if CLEAR or WARNING, 1 if OCCUPIED
int update_track_status(System *system, int track_index){
    if(!system || track_index < 0 || track_index >= system->count) return -1;
    Track *track = &system->array[track_index];
    
    Sensor *sensor = track->sensors;
    if (!sensor) return -1;
    
    ErrorCode sensor_state = read_sensor_data(sensor);
    
    /* Determine the logical "previous" track according to travel direction.
       If direction is NEXT, the previous track is `prev`.
       If direction is PREV, the previous track is `next` (opposite links).
    */
    int prev_in_dir_index = (track->dir == NEXT) ? track->prev_index : track->next_index;
    Track *prev_in_dir_track = NULL;
    if (prev_in_dir_index >= 0 && prev_in_dir_index < system->count)
        prev_in_dir_track = &system->array[prev_in_dir_index];

    if (sensor_state != ERR_OK) { // Error in the sensor
        // If the sensor fails, consider the track occupied to avoid collisions
        track->status = OCCUPIED;

        if (prev_in_dir_track && prev_in_dir_track->status != OCCUPIED)
            // The previous track in travel direction should be WARNING
            prev_in_dir_track->status = WARNING;

        return -1;
    }

    switch (sensor->actual_state){
        case 0:
            track->status = CLEAR;
            return 0;
        case 1:
            track->status = OCCUPIED;
            if (prev_in_dir_track && prev_in_dir_track->status != OCCUPIED)
                // The previous track in travel direction should be WARNING
                prev_in_dir_track->status = WARNING;

            return 1;

        case 2:
            track->status = WARNING;
            return 0;

        default:
            track->status = OCCUPIED;
            
            if (prev_in_dir_track && prev_in_dir_track->status != OCCUPIED)
                prev_in_dir_track->status = WARNING;
            
            log_message(LOG_WARNING, "Unknown sensor state %d for track index %d, setting status to OCCUPIED", sensor->actual_state, track_index);
            return -1; // Unknown state
    }
    //log_message(LOG_DEBUG, "Updated track index %d to status %d based on sensor state %d", track_index, track->status, sensor->actual_state);
}

//forces the status without reading the sensor
//NOT CHECKED, use with care
//TODO if red warning previous
ErrorCode force_update_track_status(System *system, int track_index, Status new_status){
    if (!system) return ERR_NULL_PTR;
    if (track_index < -1 || track_index >= system->count) return ERR_INVALID_ARG;
    
    system->array[track_index].status = new_status;
    
    log_message(LOG_INFO, "Force updated track index %d to status %d", track_index, new_status);

    return ERR_OK;
}

void update_system_status(System *system, int index){
    if (!system || index < 0) return;
    for(int current = 0; current < system->count; current++){
        update_track_status(system, current);
    }
}

//Returns the index of the first track
int tokens_to_track(System *system, Token *tokens, size_t token_count){
    if (!system || !tokens || token_count <= 0) return NO_FOLLOWING_TRACK;

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

    for (size_t i = 0; i < token_count; i++) {
        Token actual = tokens[i];

        switch (actual.type) {
            case SW:
                if (pending_switch) {
                    log_message(LOG_ERROR, "Unexpected SW nested before OPEN at token %zu", i);
                    return NO_FOLLOWING_TRACK;
                }
                pending_switch = true;
                continue;

            case OPEN:
                if (!pending_switch) {
                    log_message(LOG_ERROR, "Unexpected OPEN without SW at token %zu", i);
                    return NO_FOLLOWING_TRACK;
                }

                {
                    Sensor *sen = malloc(sizeof(Sensor));
                    if (!sen) return NO_FOLLOWING_TRACK;
                    sen->hex_direction = 0;
                    sen->actual_state = 0;

                    ErrorCode err = create_switch(system, generate_id(), sen, NO_FOLLOWING_TRACK, prev_index, NO_FOLLOWING_TRACK);
                    if (err != ERR_OK) {
                        log_message(LOG_ERROR, "Failed to create switch for token %zu: %s", i, error_to_string(err));
                        return NO_FOLLOWING_TRACK;
                    }

                    int switch_index = system->count - 1;
                    if (prev_index >= 0) {
                        system->array[prev_index].next_index = switch_index;
                    }
                    if (head_index == NO_FOLLOWING_TRACK) {
                        head_index = switch_index;
                    }

                    if (switch_stack_top + 1 >= MAX_STACK_SIZE) {
                        log_message(LOG_ERROR, "Switch stack overflow at token %zu", i);
                        return NO_FOLLOWING_TRACK;
                    }

                    switch_stack[++switch_stack_top] = (SwitchContext){
                        .switch_index = switch_index,
                        .outer_prev = prev_index,
                        .branch_head = NO_FOLLOWING_TRACK,
                        .branch_last = NO_FOLLOWING_TRACK
                    };

                    prev_index = NO_FOLLOWING_TRACK;
                    pending_switch = false;
                }
                continue;

            case CLOSE:
                if (switch_stack_top < 0) {
                    log_message(LOG_ERROR, "Unexpected CLOSE without matching switch at token %zu", i);
                    return NO_FOLLOWING_TRACK;
                }

                {
                    SwitchContext ctx = switch_stack[switch_stack_top--];
                    int swidx = ctx.switch_index;

                    system->array[swidx].branch = ctx.branch_head;
                    if (ctx.branch_head >= 0) {
                        system->array[ctx.branch_head].prev_index = swidx;
                    }

                    prev_index = swidx;
                }
                continue;

            case NUMBER:
                if (actual.value <= 0) {
                    log_message(LOG_ERROR, "Invalid number of tracks %d at token %zu", actual.value, i);
                    return NO_FOLLOWING_TRACK;
                }

                {
                    size_t chain_head;
                    ErrorCode err = create_straight_line(system, actual.value, &chain_head);
                    if (err != ERR_OK) {
                        log_message(LOG_ERROR, "Failed to create track chain for token %zu: %s", i, error_to_string(err));
                        return NO_FOLLOWING_TRACK;
                    }

                    if (head_index == NO_FOLLOWING_TRACK) {
                        head_index = chain_head;
                    }

                    if (prev_index >= 0) {
                        system->array[prev_index].next_index = chain_head;
                        system->array[chain_head].prev_index = prev_index;
                    }

                    int chain_last = system->count - 1;
                    prev_index = chain_last;

                    if (switch_stack_top >= 0) {
                        SwitchContext *ctx = &switch_stack[switch_stack_top];
                        if (ctx->branch_head == NO_FOLLOWING_TRACK) {
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

        printf("Stack %d\n", switch_stack_top + 1);
    }

    if (switch_stack_top >= 0) {
        log_message(LOG_ERROR, "Unclosed switch expression at end of token stream");
        return NO_FOLLOWING_TRACK;
    }

    log_message(LOG_INFO, "Converted %zu tokens to track chain starting at index %d", token_count, head_index);
    return head_index;
}

//returns an array of systems
System* load_system_layout_from_file(const char* path, size_t *out_count){
    if(!path || !out_count) return NULL;
    *out_count = 0;

    size_t buffer = 1;
    FILE *f = fopen(path, "r");
    if(!f) return NULL;

    char *line = NULL;
    size_t read = 0;
    System *system_arr = malloc(buffer * sizeof(System));
    if (!system_arr) {
        fclose(f);
        return NULL;
    }

    for (size_t i = 0; i < buffer; i++) {
        system_arr[i].array = NULL;
        system_arr[i].count = 0;
        system_arr[i].buffer = 0;
    }

    if (init_system(&system_arr[0], 16) != ERR_OK) {
        free(system_arr);
        fclose(f);
        return NULL;
    }

    while(getline(&line, &read, f)>0){
        line[strcspn(line, "\n")] = 0; //remove newline
        if(strlen(line)<1) continue; //skip empty lines
        log_message(LOG_DEBUG, "Parsing line: \"%s\"", line);

        if (system_arr[*out_count].array == NULL) {
            if (init_system(&system_arr[*out_count], 16) != ERR_OK) {
                log_message(LOG_ERROR, "Failed to initialize system array for line %zu", *out_count);
                free(line);
                fclose(f);
                return NULL;
            }
        }

        size_t num_tokens;
        TokenizeError err = 0;
        Token *tokens = tokenize(line, &num_tokens, &err);
        if (err != TOKENIZE_OK) {
            log_message(LOG_ERROR, "Tokenization failed for line %zu: %d", *out_count, err);
            free(line);
            fclose(f);
            return NULL;
        }
        if(!tokens) continue; //empty line or comments

        check_syntax(tokens, line, num_tokens, &err);
        if(err != TOKENIZE_OK){
            log_message(LOG_ERROR, "Syntax check failed for line %zu: %d", *out_count, err);
        }

        int head_index = tokens_to_track(&system_arr[*out_count], tokens, num_tokens);
        if (head_index == NO_FOLLOWING_TRACK) {
            log_message(LOG_ERROR, "Failed to convert tokens to track chain for line %zu", *out_count);
            free(tokens);
            free(line);
            fclose(f);
            return NULL;
        }

        free(tokens);

        (*out_count)++;
        if (*out_count >= buffer) {
            size_t old_buffer = buffer;
            buffer *= 2;
            System *temp = realloc(system_arr, buffer * sizeof(System));
            if (!temp) {
                log_message(LOG_ERROR, "Failed to expand system array buffer to %zu", buffer);
                free(line);
                fclose(f);
                return NULL;
            }
            system_arr = temp;

            for (size_t j = old_buffer; j < buffer; j++) {
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

static ErrorCode save_system_chain(FILE *f, System *system, int start_index, bool *need_separator) {
    if (!f || !system || !need_separator) return ERR_INVALID_ARG;

    int current = start_index;
    while (current > -1 && current < system->count) {
        int next_index = NO_FOLLOWING_TRACK;
        int straight_count = count_track(system, current, &next_index);
        if (straight_count > 0) {
            fprintf(f, "%d ", straight_count);
            *need_separator = true;
            current = next_index;
        }

        if (current == -1 || current >= system->count) {
            break;
        }

        Track *track = &system->array[current];
        if (track->type != SWITCH_TRACK) {
            break;
        }

        fprintf(f, "SW( ");

        bool inner_separator = false;
        if (track->branch > -1) {
            ErrorCode err = save_system_chain(f, system, track->branch, &inner_separator);
            if (err != ERR_OK) return err;
        }

        fprintf(f, ") ");
        *need_separator = true;
        current = track->next_index;
    }

    return ERR_OK;
}

//Saves the layout of system to path
ErrorCode save_system_to_file(System *system, const char* path){
    if (!system || !path) return ERR_INVALID_ARG;

    FILE *f = fopen(path, "w");
    if (!f) return ERR_GENERAL;

    bool need_separator = false;
    ErrorCode err = save_system_chain(f, system, 0, &need_separator);
    if (err != ERR_OK) {
        fclose(f);
        return err;
    }

    fprintf(f, "\n");
    fclose(f);
    log_message(LOG_INFO, "Saved system layout to file: %s", path);
    return ERR_OK;
}
