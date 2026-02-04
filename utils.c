#include "utils.h"
#include <stdlib.h>

int count_branch_tracks(Track* branch) {
    int count = 0;
    Track* current = branch;
    while (current != NULL) {
        count++;
        if (current->type == SWITCH_TRACK) {
            // Contar también la rama de un switch dentro de esta rama
            Switch* sw = (Switch*)current;
            count += count_branch_tracks(sw->branch);
        }
        current = current->next;
    }
    return count;
}

int read_sensor_data(Sensor* sensor) {
    if (!sensor) return -1; // Error: sensor nulo
    return sensor->actual_state;
}

int update_track_status(Track* track){
    if (!track || !track->sensors) return -1;
    int sensor_state = read_sensor_data(track->sensors);
    
    if (sensor_state < 0) return -1; // Error al leer el sensor

    switch (sensor_state){
        case 0:
            track->status = CLEAR;
            break;
        case 1:
            track->status = OCCUPIED;
            break;
        case 2:
            track->status = WARNING;
            break;
        default:
            return -1; // Estado desconocido
            break;
    }
}

//forces the status without reading the sensor
//NOT CHECKED, use with care
int force_update_track_status(Track* track, Status new_status){
    if (!track) return -1;
    track->status = new_status;
    return 0;
}

