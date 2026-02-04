#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "utils.h"

#define RED    "\x1b[31m"
#define GREEN  "\x1b[32m"
#define YELLOW "\x1b[33m"
#define RESET  "\x1b[0m"


/* ---------- Creación ---------- */

Track* create_track(int id, Sensor* sensor, Track* next, Track* prev) {

    Track* new_track = malloc(sizeof(Track));

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

Switch* create_switch(int id, Sensor* sensor, Track* next, Track* prev, Track* branch){

    Switch* sw = malloc(sizeof(Switch));

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

Switch* insert_switch(Track* track_prev, Track* track_next, Sensor* sensor, Track* branch) {
    if (!track_prev || !track_next) return NULL;

    // Crear el switch
    Switch* sw = create_switch(0, sensor, track_next, track_prev, branch);
    if (!sw) return NULL;

    // Conectar prev -> switch
    track_prev->next = (Track*)sw;

    // Conectar next <- switch
    track_next->prev = (Track*)sw;

    return sw;
}

//Returns the head of the n tracks created
Track* create_straight_line(int num_tracks) {
    if (num_tracks <= 0) return NULL;

    Track* head = NULL;
    Track* current = NULL;

    for (int i = 1; i <= num_tracks; i++) {

        Sensor* sensor = malloc(sizeof(Sensor));

        if (!sensor) {
            perror("malloc Sensor");
            return NULL;
        }

        sensor->hex_direction = 0x00;
        sensor->actual_state = 0;

        Track* new_track =
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

/* ---------- Lógica ---------- */

Track* get_next_track(Track* track) {

    if (!track) return NULL;

    /* Si es switch, mirar posición */
    if (track->type == SWITCH_TRACK) {

        Switch* sw = (Switch*)track;

        if (sw->pos == DIVERGING_POS && sw->branch)
            return sw->branch;
    }

    return track->next;
}


/* ---------- Print ---------- */

void print_tracks_with_switches(Track* head) {
    Track* current = head;

    while (current != NULL) {

        if (current->type == SWITCH_TRACK) {
            Switch* sw = (Switch*)current;

            // Contar tracks en la rama
            int branch_count = count_branch_tracks(sw->branch);

            if (sw->pos == STRAIGHT_POS)
                printf("SW[→](%d) ", branch_count);
            else
                printf("SW[~](%d) ", branch_count);

            // También podemos imprimir la rama recursivamente
            if (sw->branch){
                printf("{");
                print_tracks_with_switches(sw->branch);
                printf("} ");
            }
        } else {
            // Track normal
            switch (current->status) {
                case CLEAR:   printf(GREEN "------ " RESET); break;
                case OCCUPIED:printf(RED "------ " RESET); break;
                case WARNING: printf(YELLOW "------ " RESET); break;
            }
        }

        current = get_next_track(current);
    }

    //printf("\n");
}


/* ---------- Liberación ---------- */

bool is_in_chain(Track* head, Track* target) {

    for (Track* t = head; t != NULL; t = t->next) {
        if (t == target) return true;
    }

    return false;
}


void free_tracks(Track* head, Track* original) {
    if (!original) original = head;

    Track* current = head;

    while (current != NULL) {

        /* Si es switch, liberar rama */
        if (current->type == SWITCH_TRACK) {

            Switch* sw = (Switch*)current;

            if (sw->branch &&
                !is_in_chain(original, sw->branch)) {

                free_tracks(sw->branch, original);
            }
        }

        Track* next = current->next;

        free(current->sensors);

        if (current->type == SWITCH_TRACK)
            free((Switch*)current);
        else
            free(current);

        current = next;
    }
}


/* ---------- MAIN ---------- */

int main() {
// Creamos una línea recta de 5 tracks
    Track* line = create_straight_line(5);

    // Creamos un branch para el switch
    //Track* branch_track = create_straight_line(3);

    // Insertamos switch entre track 2 y 3
    //Switch* sw = insert_switch(line->next, line->next->next, NULL, branch_track);
    

    // Probamos cambiar la posición
    //sw->pos = DIVERGING_POS;

    
    update_track_status(line->next->next->next);


    // Imprimimos
    print_tracks_with_switches(line);
    printf("\n");
    // Liberamos todo
    free_tracks(line, NULL);
    line = NULL;

    return 0;
}