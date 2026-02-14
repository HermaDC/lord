#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "utils.h"

//TODO: Finish the implementation of the file loading
//TODO: Implement the saving of the system layout to a file
//TODO: get_last_track() should use switch branch
//TODO: update_system_status() should use switch branch
 
#define TOKEN_FOR_FILE " ,;"

Track *create_track(int id, Sensor *sensor, Track *next, Track *prev) {

    Track *new_track = malloc(sizeof(Track));

    if (!new_track) {
        perror("malloc Track");
        return NULL;
    }

    new_track->type = STRAIGHT;
    new_track->id = id;
    new_track->status = CLEAR;

    new_track->next = next;
    new_track->prev = prev;

    new_track->dir = NEXT;

    new_track->sensors = sensor;

    return new_track;
}

Switch *create_switch(int id, Sensor *sensor, Track *next, Track *prev, Track *branch){

    Switch *sw = malloc(sizeof(Switch));

    if (!sw) {
        perror("malloc Switch");
        return NULL;
    }

    /* Base */
    sw->base.type = SWITCH_TRACK;
    sw->base.id = id;

    sw->base.status = CLEAR;

    sw->base.next = next;
    sw->base.prev = prev;

    sw->base.dir = NEXT;

    sw->base.sensors = sensor;

    /* Switch */
    sw->branch = branch;
    sw->pos = STRAIGHT_POS;

    return sw;
}

Switch *insert_switch(Track *track_prev, Track *track_next, Sensor *sensor, Track *branch) {
    if (!track_prev || !track_next) return NULL;

    // Crear el switch
    Switch *sw = create_switch(0, sensor, track_next, track_prev, branch);
    if (!sw) return NULL;

    // Conectar prev -> switch
    track_prev->next = (Track *)sw;

    // Conectar next <- switch
    track_next->prev = (Track *)sw;

    return sw;
}

//Returns the head of the n tracks created
Track *create_straight_line(int num_tracks) {
    if (num_tracks <= 0) return NULL;

    Track *head = NULL;
    Track *current = NULL;

    for (int i = 1; i <= num_tracks; i++) {

        Sensor *sensor = malloc(sizeof(Sensor));

        if (!sensor) {
            perror("malloc Sensor");
            return NULL;
        }

        sensor->hex_direction = 0x00;
        sensor->actual_state = 0;

        Track *new_track =
            create_track(i, sensor, NULL, current);

        if (!new_track) {
            free(sensor);
            return NULL;
        }

        if (current)
            current->next = new_track;
        else
            head = new_track;

        current = new_track;
    }

    return head;
}

Track *get_last_track(Track *head) {
    if (!head) return NULL;

    Track *current = head;
    while (current->next != NULL) {
        current = current->next;
    }
    return current;
}

bool is_in_chain(Track *head, Track *target) {

    for (Track *t = head; t != NULL; t = t->next) {
        if (t == target) return true;
    }

    return false;
}

void free_tracks(Track *head, Track *original) {
    if (!original) original = head;

    Track *current = head;

    while (current != NULL) {

        /* Si es switch, liberar rama */
        if (current->type == SWITCH_TRACK) {

            Switch *sw = (Switch *)current;

            if (sw->branch &&
                !is_in_chain(original, sw->branch)) {

                free_tracks(sw->branch, original);
            }
        }

        Track *next = current->next;

        free(current->sensors);

        if (current->type == SWITCH_TRACK)
            free((Switch *)current);
        else
            free(current);

        current = next;
    }
}

int count_branch_tracks(Track *branch) {
    int count = 0;
    Track *current = branch;
    while (current != NULL) {
        count++;
        if (current->type == SWITCH_TRACK) {
            // Contar también la rama de un switch dentro de esta rama
            Switch *sw = (Switch *)current;
            count += count_branch_tracks(sw->branch);
        }
        current = current->next;
    }
    return count;
}

int read_sensor_data(Sensor *sensor) {
    if (!sensor) return -1; // Error: sensor nulo
    //TODO: Implement the real data
    sensor->actual_state = rand() % 3; // Simulamos lectura de sensor (0, 1 o 2)
    return sensor->actual_state;
}

//Updates the track status based on its sensor data
//Returns -1 on error, 0 if CLEAR or WARNING, 1 if OCCUPIED
int update_track_status(Track *track){
    if (!track || !track->sensors) return -1;
    int sensor_state = read_sensor_data(track->sensors);
    
    
    /* Determine the logical "previous" track according to travel direction.
       If direction is NEXT, the previous track is `prev`.
       If direction is PREV, the previous track is `next` (opposite links).
    */
    Track *prev_in_dir = (track->dir == NEXT) ? track->prev : track->next;

    if (sensor_state < 0) { // Error in the sensor
        // If the sensor fails, consider the track occupied to avoid collisions
        track->status = OCCUPIED; 

        if (prev_in_dir && prev_in_dir->status != OCCUPIED)
            // The previous track in travel direction should be WARNING
            prev_in_dir->status = WARNING; 

        return -1;
    }

    switch (sensor_state){
        case 0:
            track->status = CLEAR;
            return 0;
        case 1:
            track->status = OCCUPIED;
            if (prev_in_dir && prev_in_dir->status != OCCUPIED)
                // The previous track in travel direction should be WARNING
                prev_in_dir->status = WARNING; 

            return 1;

        case 2:
            track->status = WARNING;
            return 0;

        default:
            track->status = OCCUPIED; 
            // If the sensor fails, consider the track occupied to avoid collisions
            
            if (prev_in_dir && prev_in_dir->status != OCCUPIED) 
                // Check te previous track to not be in OCCUPIED to avoid overwriting a real OCCUPIED with a WARNING
                prev_in_dir->status = WARNING;
            return -1; // Unknown state
    }
}

//forces the status without reading the sensor
//NOT CHECKED, use with care
int force_update_track_status(Track *track, Status new_status){
    if (!track) return -1;
    track->status = new_status;
    return 0;
}

void update_system_status(Track *head){
    Track *current = head;
    while (current != NULL) {
        update_track_status(current);
        if (current->type == SWITCH_TRACK) {
            Switch *sw = (Switch *)current;
            if (sw->branch) {
                update_system_status(sw->branch);
            }
        }
        current = current->next;
        if (current == head) break; // Evitar bucle infinito en caso de lista circular
        if (!current) break; // Evitar acceso a puntero nulo
    }
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

    char *line = NULL;
    size_t len = 0;

    while (getline(&line, &len, file) != -1)
    {
        // Remove newline
        size_t line_len = strlen(line);
        if (line_len > 0 && line[line_len - 1] == '\n')
            line[line_len - 1] = '\0';

        char *token = strtok(line, TOKEN_FOR_FILE);
        Track *line_head = NULL;
        Track *last_track = NULL;

        while (token)
        {
            char *end;
            long num = strtol(token, &end, 10);

            if (*end != '\0' || num <= 0) {
                token = strtok(NULL, TOKEN_FOR_FILE);
                continue;
            }

            Track *new_track = create_straight_line((int)num);
            if (!new_track) goto error;

            if (last_track) {
                last_track->next = new_track;
                new_track->prev = last_track;
            } else {
                line_head = new_track;
            }

            last_track = get_last_track(new_track);

            // TODO: switches / branches
            

            token = strtok(NULL, TOKEN_FOR_FILE);
        }

        // Añadir solo si hay algo en la línea
        if (line_head)
        {
            Track **tmp = realloc(head, sizeof(Track *) * (*count + 1));
            if (!tmp) {
                free_tracks(line_head, NULL);
                goto error;
            }

            head = tmp;
            head[*count] = line_head;
            (*count)++;
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