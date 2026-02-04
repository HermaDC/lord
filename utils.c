#include "utils.h"
#include <stdlib.h>
#include <time.h>

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
    //TODO: Implement the real data
    srand(time(NULL));
    sensor->actual_state = rand() % 3; // Simulamos lectura de sensor (0, 1 o 2)
    return sensor->actual_state;
}

//Updates the track status based on its sensor data
//Returns -1 on error, 0 if CLEAR or WARNING, 1 if OCCUPIED
int update_track_status(Track* track){
    if (!track || !track->sensors) return -1;
    int sensor_state = read_sensor_data(track->sensors);
    
    if (sensor_state < 0) { // Error al leer el sensor
        track->status = OCCUPIED; //If the sensor fails, consider the track occupied to avoid colisions
        if (track->prev)
            track->prev->status = WARNING; //The previous track should be in WARNING state
        return -1;
    } 

    switch (sensor_state){
        case 0:
            track->status = CLEAR;
            return 0;
            break;
        case 1:
            track->status = OCCUPIED;
            if (track->prev && track->prev->status != OCCUPIED)
                track->prev->status = WARNING; //The previous track should be in WARNING state
            return 1;
            break;
        case 2:
            track->status = WARNING;
            return 0;
            break;
        default:
            track->status = OCCUPIED; //If the sensor fails, consider the track occupied to avoid colisions
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

