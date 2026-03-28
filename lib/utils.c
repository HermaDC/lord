#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

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

    return ERR_OK;
}

ErrorCode create_straight_line(System *system, int num_tracks, size_t *head_index) {
    if (!system) return ERR_INVALID_ARG;
    if (num_tracks <= 0) return ERR_INVALID_ARG;

    int current_index = -1;

    for (int i = 1; i <= num_tracks; i++) {

        ErrorCode err = create_track(system, generate_id(), NULL, -1, current_index);
        
        if(err != ERR_OK) return ERR_GENERAL;

        int new_index = system->count - 1;
        
        if (current_index >= 0) {
            system->array[current_index].next_index = new_index;
        }

        current_index = new_index;
    }
    (*head_index) = current_index;
    return ERR_OK;
}

int get_last_track(System *system, int start_index) {
    if (!system) return 0;
    if (start_index < system->count) start_index=0;

    int current = start_index;
    while (system->array[current].next_index != -1) { //TODO posible buffer_overflow
        current = system->array[current].next_index;
    }
    return current;
}

int get_next_track(System *system, int index) {

    if (!system) return -1;
    Track track = system->array[index];

    /* Si es switch, mirar posición */
    if (track.type == SWITCH_TRACK) {

        if (track.pos == DIVERGING_POS && track.branch > -1){
            int branch_next = system->array[track.branch].next_index;
            return branch_next;
        }
    }

    return track.next_index;
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
    
    for (int current = origin; current >= system->count; current++) {
        if (current == dest) return true;
    }

    return false;
}

int count_track(System *system, int *last){
    int count = 0;
    int current = 0;
    while((current > -1 || current < system->count) && system->array[count].type == STRAIGHT){
        current = system->array[count].next_index;
        count++;
    }
    if (last) *last = current;
    return count;
}

int count_branch_tracks(System *system, int branch_index) {
    int count = 0;
    if(branch_index < 0) return 0;
    int current = branch_index;
    while (current != -1 ){
        if ((current > -1 || current < system->count) && system->array[count].type == SWITCH_TRACK) {
            // Contar también la rama de un switch dentro de esta rama
            count += count_branch_tracks(system, system->array[count].branch);
        }
        else{
            count++;
        }
        current = system->array[count].next_index;
    }
    return count;
}

//TODO use the enum for errors
ErrorCode read_sensor_data(Sensor *sensor) {
    if (!sensor) return ERR_NULL_PTR; // Error: NULL sensor
    //TODO: Implement the real data
    int data = rand() % 3;
    sensor->actual_state = data; // Simulamos lectura de sensor (0, 1 o 2)
    return ERR_OK;
}

//Updates the track status based on its sensor data
//Returns -1 on error, 0 if CLEAR or WARNING, 1 if OCCUPIED
int update_track_status(System *system, int track_index){
    if(!system) return -1;
    Track track = system->array[track_index];
    
    Sensor *sensor = track.sensors;
    if (!sensor) return -1;
    
    int sensor_state = read_sensor_data(sensor);
    
    
    /* Determine the logical "previous" track according to travel direction.
       If direction is NEXT, the previous track is `prev`.
       If direction is PREV, the previous track is `next` (opposite links).
    */
    int prev_in_dir_index = (track.dir == NEXT) ? track.prev_index : track.next_index;
    Track prev_in_dir_track = system->array[prev_in_dir_index];

    if (sensor_state != ERR_OK) { // Error in the sensor
        // If the sensor fails, consider the track occupied to avoid collisions
        track.status = OCCUPIED; 

        if (prev_in_dir_index && prev_in_dir_track.status != OCCUPIED)
            // The previous track in travel direction should be WARNING
            prev_in_dir_track.status = WARNING; 

        return -1;
    }

    switch (sensor->actual_state){
        case 0:
            track.status = CLEAR;
            return 0;
        case 1:
            track.status = OCCUPIED;
            if (prev_in_dir_index && prev_in_dir_track.status != OCCUPIED)
                // The previous track in travel direction should be WARNING
                prev_in_dir_track.status = WARNING; 

            return 1;

        case 2:
            track.status = WARNING;
            return 0;

        default:
            track.status = OCCUPIED; 
            // If the sensor fails, consider the track occupied to avoid collisions
            
            if (prev_in_dir_index && prev_in_dir_track.status != OCCUPIED) 
                // Check te previous track to not be in OCCUPIED to avoid overwriting a real OCCUPIED with a WARNING
                prev_in_dir_track.status = WARNING;
            return -1; // Unknown state
    }
}

//forces the status without reading the sensor
//NOT CHECKED, use with care
//TODO if red warning previous
ErrorCode force_update_track_status(System *system, int track_index, Status new_status){
    if (!system) return ERR_NULL_PTR;
    if (track_index < -1 || track_index >= system->count) return ERR_INVALID_ARG;
    
    system->array[track_index].status = new_status;
    
    return ERR_OK;
}

void update_system_status(System *system, int index){
    if (!system || index < -1) return;
    int current = index;
    while (current < system->count) {
        update_track_status(system, current);
        if (system->array[current].type == SWITCH_TRACK) {
            if (system->array[current].branch > -1) {
                update_system_status(system, system->array[current].branch);
            }
        }
        current++;
    }
}

/*
//return the head of the tracks that tokens described, if error, return NULL
Track *tokens_to_track(Token *tokens, size_t tokens_count){
    //pahse 1: get the first number
    //phase 2: get the next token, if number add it, is sw stack it
    //phase 3: if number, check actual stack, if SW increment stack
    //phase 4: return the head

    if(!tokens || tokens_count < 1) return NULL;

    //get the first track of all system
    if(tokens[0].value < 1) return NULL;
    Track *head = create_straight_line(tokens[0].value);
    //Track *last = get_last_track(head);

    //prepare stack for switches
    size_t stack_buffer = 1;
    Switch **stack = malloc(stack_buffer*sizeof(Switch*));
    if(!stack) return NULL;
    size_t stack_count = 0;

    for(size_t i = 1; i<tokens_count; i++){
        if(tokens[i].type == NUMBER){
            if(stack_count > 0){
                //Switch *sw = stack[stack_count-1];

            }
        }
    }
    free(stack);

    return head;
}


//Given a valid file path it returns an array of count pointers to the head of each line of tracks
// created from the file, and sets count to the number of lines created.
//Example `10 SW(10) 10` would create a straight line of 10 tracks,
//then a switch with a branch of 10 tracks, and then another straight line of 10 tracks
Track **load_system_layout_from_file(const char *path, size_t *count)
{
    if (!path || !count) return NULL;

    *count = 0;

    FILE *file = fopen(path, "r");
    if (!file) return NULL;

    Track **head = NULL;
    size_t buffer = 1;
    char *line = NULL;
    size_t len = 0;

    head = malloc(buffer*sizeof(Track*));
    while (getline(&line, &len, file) != -1)
    {
        // Remove newline
        line[strcspn(line, "\n")] = 0;

        printf("parsing: %s\n", line);

        Token *tokens;
        size_t tokens_count = 0;
        TokenizeError error;
        if(strlen(line) <1) continue;
        tokens = tokenize(line, &tokens_count, &error);
        if(!tokens || tokens_count < 1) continue;

        if(error != TOKENIZE_OK){
            //I don't know if skip the line, putting a NULL or raising an error and return none of the array
            //For now just return NULL
            printf("failed to tokenize %d", error);
            free(head);
            return NULL;
        }
        check_syntax(tokens, line, tokens_count, &error);
        if(error != TOKENIZE_OK){
            free(head);
            return NULL;
        }
        //call a token_to_track()
        head[*count] = tokens_to_track(tokens, tokens_count);

        (*count)++;
        if(*count >= buffer){
            buffer *= 2;
            head = realloc(head, buffer*sizeof(Track*));
            if(!head) goto error;
        }
    }

    free(line);
    fclose(file);

    return head;

error:
    if (line) free(line);
    if (file) fclose(file);

    if (head) {
        for (size_t i = 0; i < *count; i++)
            free_tracks(head[i], NULL);
        free(head);
    }

    *count = 0;
    return NULL;
}

//TODO fix this, if sw inside sw use parenthesis accordingly.
// FOr that we should use a stack to store the last sw
//TODO use the enum for erros intead of int
ErrorCode save_layout_to_file(const char *path, Track *head){
    if(!head || !path) return ERR_INVALID_ARG;

    FILE *f = fopen(path, "w");
    if(!f){
        return ERR_NOT_FOUND;
    }
    
    Track *current = head;
    Track *last_checked;

    SwitchStack stack;
    SwitchStack *stack_p = &stack;
    initialize(&stack);

    printf("%d", count_track(current, &last_checked));
    fprintf(f, "%d", count_track(current, &last_checked));
    current = last_checked;
    if(current->type == SWITCH_TRACK){
        printf("SW( ");
        push(stack_p, (Switch*)current);
        printf("%d", count_track(((Switch*)current)->branch, &last_checked));

    }



    fclose(f);
    return ERR_OK;
}
*/
